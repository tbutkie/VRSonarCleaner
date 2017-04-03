#ifndef PREAMBLE_GLSL
#define PREAMBLE_GLSL


// VERTEX ATTRIBUTES: layout(location = _____)

#define POSITION_ATTRIB_LOCATION				0
#define NORMAL_ATTRIB_LOCATION					1
#define TEXCOORD_ATTRIB_LOCATION				2
#define COLOR_ATTRIB_LOCATION					3
#define INSTANCE_POSITION_ATTRIB_LOCATION		4
#define INSTANCE_FORWARD_ATTRIB_LOCATION		5


// SHADER UNIFORMS: layout(location = _____)

#define MVP_UNIFORM_LOCATION					0
#define MV_UNIFORM_LOCATION						1
#define MV_INV_TRANS_UNIFORM_LOCATION			2
#define LIGHTDIR_UNIFORM_LOCATION				3
#define VIEW_POSITION_UNIFORM_LOCATION			4
#define MATERIAL_SHININESS_UNIFORM_LOCATION		5
#define DIR_LIGHTS_COUNT_UNIFORM_LOCATION		6
#define POINT_LIGHTS_COUNT_UNIFORM_LOCATION		7
#define SPOT_LIGHTS_COUNT_UNIFORM_LOCATION		8


// UNIFORM BLOCKS: layout(std40, binding = _____)

#define SCENE_UNIFORM_BUFFER_LOCATION			0
#define LIGHTS_UNIFORM_BUFFER_LOCATION			1


// TEXTURE UNITS: layout(binding = _____)

#define DIFFUSE_TEXTURE_BINDING					0
#define SPECULAR_TEXTURE_BINDING				1
#define EMISSIVE_TEXTURE_BINDING				2


// LIGHTING DEFINITIONS
#define MAX_DIRECTIONAL_LIGHTS	5
#define MAX_POINT_LIGHTS		10
#define MAX_SPOT_LIGHTS			10
#define MAX_N_LIGHTS MAX_DIRECTIONAL_LIGHTS + MAX_POINT_LIGHTS + MAX_SPOT_LIGHTS

#endif // PREAMBLE_GLSL