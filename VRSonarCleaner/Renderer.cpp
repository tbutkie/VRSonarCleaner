#include "Renderer.h"

#include <vector>
#include <shared/glm/glm.hpp>

#include "DebugDrawer.h"
#include "InfoBoxManager.h"
#include "ShaderUtils.h"

Renderer::Renderer()
	: m_pHMD(NULL)
	, m_pTDM(NULL)
	, m_pLighting(NULL)
	, m_unLensProgramID(0)
	, m_unLensVAO(0)
	, m_bVblank(false)
	, m_bGlFinishHack(true)
	, m_fNearClip(0.1f)
	, m_fFarClip(30.0f)
{
}


Renderer::~Renderer()
{
	Shutdown();
}

bool Renderer::init(vr::IVRSystem *pHMD, TrackedDeviceManager *pTDM, LightingSystem *pLS)
{
	m_pHMD = pHMD;
	m_pTDM = pTDM;
	m_pLighting = pLS;
	
	if (SDL_GL_SetSwapInterval(m_bVblank ? 1 : 0) < 0)
	{
		printf("%s - Warning: Unable to set VSync! SDL Error: %s\n", __FUNCTION__, SDL_GetError());
		return false;
	}

	if (!CreateLensShader())
		return false;

	SetupCameras();
	SetupStereoRenderTargets();
	SetupDistortion();

	return true;
}


//-----------------------------------------------------------------------------
// Purpose: Creates all the shaders used by HelloVR SDL
//-----------------------------------------------------------------------------
bool Renderer::CreateLensShader()
{
	m_unLensProgramID = CompileGLShader(
		"Distortion",

		// vertex shader
		"#version 410 core\n"
		"layout(location = 0) in vec4 position;\n"
		"layout(location = 1) in vec2 v2UVredIn;\n"
		"layout(location = 2) in vec2 v2UVGreenIn;\n"
		"layout(location = 3) in vec2 v2UVblueIn;\n"
		"noperspective  out vec2 v2UVred;\n"
		"noperspective  out vec2 v2UVgreen;\n"
		"noperspective  out vec2 v2UVblue;\n"
		"void main()\n"
		"{\n"
		"	v2UVred = v2UVredIn;\n"
		"	v2UVgreen = v2UVGreenIn;\n"
		"	v2UVblue = v2UVblueIn;\n"
		"	gl_Position = position;\n"
		"}\n",

		// fragment shader
		"#version 410 core\n"
		"uniform sampler2D mytexture;\n"

		"noperspective  in vec2 v2UVred;\n"
		"noperspective  in vec2 v2UVgreen;\n"
		"noperspective  in vec2 v2UVblue;\n"

		"out vec4 outputColor;\n"

		"void main()\n"
		"{\n"
		"	float fBoundsCheck = ( (dot( vec2( lessThan( v2UVgreen.xy, vec2(0.05, 0.05)) ), vec2(1.0, 1.0))+dot( vec2( greaterThan( v2UVgreen.xy, vec2( 0.95, 0.95)) ), vec2(1.0, 1.0))) );\n"
		"	if( fBoundsCheck > 1.0 )\n"
		"	{ outputColor = vec4( 0, 0, 0, 1.0 ); }\n"
		"	else\n"
		"	{\n"
		"		float red = texture(mytexture, v2UVred).x;\n"
		"		float green = texture(mytexture, v2UVgreen).y;\n"
		"		float blue = texture(mytexture, v2UVblue).z;\n"
		"		outputColor = vec4( red, green, blue, 1.0  ); }\n"
		"}\n"
	);


	return m_unLensProgramID != 0;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool Renderer::CreateFrameBuffer(int nWidth, int nHeight, FramebufferDesc &framebufferDesc)
{
	glGenFramebuffers(1, &framebufferDesc.m_nRenderFramebufferId);
	glBindFramebuffer(GL_FRAMEBUFFER, framebufferDesc.m_nRenderFramebufferId);

	glGenRenderbuffers(1, &framebufferDesc.m_nDepthBufferId);
	glBindRenderbuffer(GL_RENDERBUFFER, framebufferDesc.m_nDepthBufferId);
	glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH_COMPONENT, nWidth, nHeight);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, framebufferDesc.m_nDepthBufferId);

	glGenTextures(1, &framebufferDesc.m_nRenderTextureId);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, framebufferDesc.m_nRenderTextureId);
	glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGBA8, nWidth, nHeight, true);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, framebufferDesc.m_nRenderTextureId, 0);

	glGenFramebuffers(1, &framebufferDesc.m_nResolveFramebufferId);
	glBindFramebuffer(GL_FRAMEBUFFER, framebufferDesc.m_nResolveFramebufferId);

	glGenTextures(1, &framebufferDesc.m_nResolveTextureId);
	glBindTexture(GL_TEXTURE_2D, framebufferDesc.m_nResolveTextureId);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, nWidth, nHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, framebufferDesc.m_nResolveTextureId, 0);

	// check FBO status
	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE)
	{
		return false;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	return true;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool Renderer::SetupStereoRenderTargets()
{
	if (!m_pHMD)
		return false;

	m_pHMD->GetRecommendedRenderTargetSize(&m_nRenderWidth, &m_nRenderHeight);

	CreateFrameBuffer(m_nRenderWidth, m_nRenderHeight, leftEyeDesc);
	CreateFrameBuffer(m_nRenderWidth, m_nRenderHeight, rightEyeDesc);

	return true;
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void Renderer::SetupDistortion()
{
	if (!m_pHMD)
		return;

	GLushort m_iLensGridSegmentCountH = 43;
	GLushort m_iLensGridSegmentCountV = 43;

	float w = (float)(1.0 / float(m_iLensGridSegmentCountH - 1));
	float h = (float)(1.0 / float(m_iLensGridSegmentCountV - 1));

	float u, v = 0;

	std::vector<VertexDataLens> vVerts(0);
	VertexDataLens vert;

	//left eye distortion verts
	float Xoffset = -1;
	for (int y = 0; y<m_iLensGridSegmentCountV; y++)
	{
		for (int x = 0; x<m_iLensGridSegmentCountH; x++)
		{
			u = x*w; v = 1 - y*h;
			vert.position = glm::vec2(Xoffset + u, -1 + 2 * y*h);

			vr::DistortionCoordinates_t dc0 = m_pHMD->ComputeDistortion(vr::Eye_Left, u, v);

			vert.texCoordRed = glm::vec2(dc0.rfRed[0], 1 - dc0.rfRed[1]);
			vert.texCoordGreen = glm::vec2(dc0.rfGreen[0], 1 - dc0.rfGreen[1]);
			vert.texCoordBlue = glm::vec2(dc0.rfBlue[0], 1 - dc0.rfBlue[1]);

			vVerts.push_back(vert);
		}
	}

	//right eye distortion verts
	Xoffset = 0;
	for (int y = 0; y<m_iLensGridSegmentCountV; y++)
	{
		for (int x = 0; x<m_iLensGridSegmentCountH; x++)
		{
			u = x*w; v = 1 - y*h;
			vert.position = glm::vec2(Xoffset + u, -1 + 2 * y*h);

			vr::DistortionCoordinates_t dc0 = m_pHMD->ComputeDistortion(vr::Eye_Right, u, v);

			vert.texCoordRed = glm::vec2(dc0.rfRed[0], 1 - dc0.rfRed[1]);
			vert.texCoordGreen = glm::vec2(dc0.rfGreen[0], 1 - dc0.rfGreen[1]);
			vert.texCoordBlue = glm::vec2(dc0.rfBlue[0], 1 - dc0.rfBlue[1]);

			vVerts.push_back(vert);
		}
	}

	std::vector<GLushort> vIndices;
	GLushort a, b, c, d;

	GLushort offset = 0;
	for (GLushort y = 0; y<m_iLensGridSegmentCountV - 1; y++)
	{
		for (GLushort x = 0; x<m_iLensGridSegmentCountH - 1; x++)
		{
			a = m_iLensGridSegmentCountH*y + x + offset;
			b = m_iLensGridSegmentCountH*y + x + 1 + offset;
			c = (y + 1)*m_iLensGridSegmentCountH + x + 1 + offset;
			d = (y + 1)*m_iLensGridSegmentCountH + x + offset;
			vIndices.push_back(a);
			vIndices.push_back(b);
			vIndices.push_back(c);

			vIndices.push_back(a);
			vIndices.push_back(c);
			vIndices.push_back(d);
		}
	}

	offset = (m_iLensGridSegmentCountH)*(m_iLensGridSegmentCountV);
	for (GLushort y = 0; y<m_iLensGridSegmentCountV - 1; y++)
	{
		for (GLushort x = 0; x<m_iLensGridSegmentCountH - 1; x++)
		{
			a = m_iLensGridSegmentCountH*y + x + offset;
			b = m_iLensGridSegmentCountH*y + x + 1 + offset;
			c = (y + 1)*m_iLensGridSegmentCountH + x + 1 + offset;
			d = (y + 1)*m_iLensGridSegmentCountH + x + offset;
			vIndices.push_back(a);
			vIndices.push_back(b);
			vIndices.push_back(c);

			vIndices.push_back(a);
			vIndices.push_back(c);
			vIndices.push_back(d);
		}
	}
	m_uiIndexSize = vIndices.size();

	glGenVertexArrays(1, &m_unLensVAO);
	glBindVertexArray(m_unLensVAO);

	glGenBuffers(1, &m_glIDVertBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, m_glIDVertBuffer);
	glBufferData(GL_ARRAY_BUFFER, vVerts.size() * sizeof(VertexDataLens), &vVerts[0], GL_STATIC_DRAW);

	glGenBuffers(1, &m_glIDIndexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_glIDIndexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, vIndices.size() * sizeof(GLushort), &vIndices[0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(VertexDataLens), (void *)offsetof(VertexDataLens, position));

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(VertexDataLens), (void *)offsetof(VertexDataLens, texCoordRed));

	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(VertexDataLens), (void *)offsetof(VertexDataLens, texCoordGreen));

	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(VertexDataLens), (void *)offsetof(VertexDataLens, texCoordBlue));

	glBindVertexArray(0);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);
	glDisableVertexAttribArray(3);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void Renderer::SetupCameras()
{
	m_mat4ProjectionLeft = GetHMDMatrixProjectionEye(vr::Eye_Left);
	m_mat4ProjectionRight = GetHMDMatrixProjectionEye(vr::Eye_Right);
	m_mat4eyePoseLeft = GetHMDMatrixPoseEye(vr::Eye_Left);
	m_mat4eyePoseRight = GetHMDMatrixPoseEye(vr::Eye_Right);
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void Renderer::RenderFrame(SDL_Window *win, glm::mat4 &HMDPose)
{
	m_mat4CurrentHMDPose = HMDPose;
	SDL_GetWindowSize(win, &m_nWindowWidth, &m_nWindowHeight);

	// for now as fast as possible
	if (m_pHMD)
	{
		RenderStereoTargets();
		RenderDistortion();

		vr::Texture_t leftEyeTexture = { (void*)leftEyeDesc.m_nResolveTextureId, vr::API_OpenGL, vr::ColorSpace_Gamma };
		vr::VRCompositor()->Submit(vr::Eye_Left, &leftEyeTexture);
		vr::Texture_t rightEyeTexture = { (void*)rightEyeDesc.m_nResolveTextureId, vr::API_OpenGL, vr::ColorSpace_Gamma };
		vr::VRCompositor()->Submit(vr::Eye_Right, &rightEyeTexture);
	}

	if (m_bVblank && m_bGlFinishHack)
	{
		//$ HACKHACK. From gpuview profiling, it looks like there is a bug where two renders and a present
		// happen right before and after the vsync causing all kinds of jittering issues. This glFinish()
		// appears to clear that up. Temporary fix while I try to get nvidia to investigate this problem.
		// 1/29/2014 mikesart
		glFinish();
	}

	// SwapWindow
	{
		SDL_GL_SwapWindow(win);
	}

	// Clear
	{
		// We want to make sure the glFinish waits for the entire present to complete, not just the submission
		// of the command. So, we do a clear here right here so the glFinish will wait fully for the swap.
		glClearColor(0, 0, 0, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	// Flush and wait for swap.
	if (m_bVblank)
	{
		glFlush();
		glFinish();
	}

	DebugDrawer::getInstance().flushLines();
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void Renderer::RenderStereoTargets()
{
	//glClearColor(0.15f, 0.15f, 0.18f, 1.0f); // nice background color, but not black
	glClearColor(0.33, 0.39, 0.49, 1.0); //VTT4D background
	glEnable(GL_MULTISAMPLE);

	// Left Eye
	glBindFramebuffer(GL_FRAMEBUFFER, leftEyeDesc.m_nRenderFramebufferId);
	glViewport(0, 0, m_nRenderWidth, m_nRenderHeight);
	RenderScene(vr::Eye_Left);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glDisable(GL_MULTISAMPLE);

	glBindFramebuffer(GL_READ_FRAMEBUFFER, leftEyeDesc.m_nRenderFramebufferId);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, leftEyeDesc.m_nResolveFramebufferId);

	glBlitFramebuffer(0, 0, m_nRenderWidth, m_nRenderHeight, 0, 0, m_nRenderWidth, m_nRenderHeight,
		GL_COLOR_BUFFER_BIT,
		GL_LINEAR);

	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

	glEnable(GL_MULTISAMPLE);

	// Right Eye
	glBindFramebuffer(GL_FRAMEBUFFER, rightEyeDesc.m_nRenderFramebufferId);
	glViewport(0, 0, m_nRenderWidth, m_nRenderHeight);
	RenderScene(vr::Eye_Right);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glDisable(GL_MULTISAMPLE);

	glBindFramebuffer(GL_READ_FRAMEBUFFER, rightEyeDesc.m_nRenderFramebufferId);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, rightEyeDesc.m_nResolveFramebufferId);

	glBlitFramebuffer(0, 0, m_nRenderWidth, m_nRenderHeight, 0, 0, m_nRenderWidth, m_nRenderHeight,
		GL_COLOR_BUFFER_BIT,
		GL_LINEAR);

	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void Renderer::RenderScene(vr::Hmd_Eye nEye)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	
	glm::mat4 thisEyesViewMatrix = (nEye == vr::Eye_Left ? m_mat4eyePoseLeft : m_mat4eyePoseRight) * m_mat4CurrentHMDPose;
	glm::mat4 thisEyesViewProjectionMatrix = (nEye == vr::Eye_Left ? m_mat4ProjectionLeft : m_mat4ProjectionRight) * thisEyesViewMatrix;
	
	m_pLighting->updateView(thisEyesViewMatrix);

	if (!m_pHMD->IsInputFocusCapturedByAnotherProcess())
	{
		m_pTDM->renderTrackedDevices(thisEyesViewProjectionMatrix);
	}

	InfoBoxManager::getInstance().render(glm::value_ptr(thisEyesViewProjectionMatrix));

	// DEBUG DRAWER RENDER CALL
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	DebugDrawer::getInstance().render(thisEyesViewProjectionMatrix);
	glDisable(GL_BLEND);
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void Renderer::RenderDistortion()
{
	glDisable(GL_DEPTH_TEST);
	glViewport(0, 0, m_nWindowWidth, m_nWindowHeight);

	glBindVertexArray(m_unLensVAO);
	glUseProgram(m_unLensProgramID);

	//render left lens (first half of index array )
	glBindTexture(GL_TEXTURE_2D, leftEyeDesc.m_nResolveTextureId);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glDrawElements(GL_TRIANGLES, m_uiIndexSize / 2, GL_UNSIGNED_SHORT, 0);

	//render right lens (second half of index array )
	glBindTexture(GL_TEXTURE_2D, rightEyeDesc.m_nResolveTextureId);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glDrawElements(GL_TRIANGLES, m_uiIndexSize / 2, GL_UNSIGNED_SHORT, (const void *)(m_uiIndexSize));

	glBindVertexArray(0);
	glUseProgram(0);
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
glm::mat4 Renderer::GetHMDMatrixProjectionEye(vr::Hmd_Eye nEye)
{
	if (!m_pHMD)
		return glm::mat4();

	vr::HmdMatrix44_t mat = m_pHMD->GetProjectionMatrix(nEye, m_fNearClip, m_fFarClip, vr::API_OpenGL);

	return glm::mat4(
		mat.m[0][0], mat.m[1][0], mat.m[2][0], mat.m[3][0],
		mat.m[0][1], mat.m[1][1], mat.m[2][1], mat.m[3][1],
		mat.m[0][2], mat.m[1][2], mat.m[2][2], mat.m[3][2],
		mat.m[0][3], mat.m[1][3], mat.m[2][3], mat.m[3][3]
	);
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
glm::mat4 Renderer::GetHMDMatrixPoseEye(vr::Hmd_Eye nEye)
{
	if (!m_pHMD)
		return glm::mat4();

	vr::HmdMatrix34_t matEyeToHead = m_pHMD->GetEyeToHeadTransform(nEye);
	glm::mat4 matrixObj(
		matEyeToHead.m[0][0], matEyeToHead.m[1][0], matEyeToHead.m[2][0], 0.f,
		matEyeToHead.m[0][1], matEyeToHead.m[1][1], matEyeToHead.m[2][1], 0.f,
		matEyeToHead.m[0][2], matEyeToHead.m[1][2], matEyeToHead.m[2][2], 0.f,
		matEyeToHead.m[0][3], matEyeToHead.m[1][3], matEyeToHead.m[2][3], 1.f
	);

	return glm::inverse(matrixObj);
}

void Renderer::Shutdown()
{
	glDeleteBuffers(1, &m_glIDVertBuffer);
	glDeleteBuffers(1, &m_glIDIndexBuffer);

	if (m_unLensProgramID)
	{
		glDeleteProgram(m_unLensProgramID);
	}

	glDeleteRenderbuffers(1, &leftEyeDesc.m_nDepthBufferId);
	glDeleteTextures(1, &leftEyeDesc.m_nRenderTextureId);
	glDeleteFramebuffers(1, &leftEyeDesc.m_nRenderFramebufferId);
	glDeleteTextures(1, &leftEyeDesc.m_nResolveTextureId);
	glDeleteFramebuffers(1, &leftEyeDesc.m_nResolveFramebufferId);

	glDeleteRenderbuffers(1, &rightEyeDesc.m_nDepthBufferId);
	glDeleteTextures(1, &rightEyeDesc.m_nRenderTextureId);
	glDeleteFramebuffers(1, &rightEyeDesc.m_nRenderFramebufferId);
	glDeleteTextures(1, &rightEyeDesc.m_nResolveTextureId);
	glDeleteFramebuffers(1, &rightEyeDesc.m_nResolveFramebufferId);

	if (m_unLensVAO != 0)
	{
		glDeleteVertexArrays(1, &m_unLensVAO);
	}
}
