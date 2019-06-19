layout(binding = DIFFUSE_TEXTURE_BINDING)
	uniform sampler2D diffuseTex;
layout(binding = SPECULAR_TEXTURE_BINDING)
	uniform sampler2D specularTex;
layout(binding = EMISSIVE_TEXTURE_BINDING)
	uniform sampler2D emissiveTex;
layout(location = MATERIAL_SHININESS_UNIFORM_LOCATION)
	uniform float shininess;
layout(location = LIGHT_COUNT_UNIFORM_LOCATION)
	uniform int numLights;
layout(location = DIFFUSE_COLOR_UNIFORM_LOCATION)
	uniform vec4 diffColor;
layout(location = SPECULAR_COLOR_UNIFORM_LOCATION)
	uniform vec4 specColor;
	
	
layout(std140, binding = SCENE_UNIFORM_BUFFER_LOCATION) 
	uniform FrameUniforms
	{
		vec4 v4Viewport;
		mat4 m4View;
		mat4 m4Projection;
		mat4 m4ViewProjection;
		float fGlobalTime;
	};


in vec3 v3Normal;
in vec3 v3FragPos;
in vec4 v4Color;
in vec2 v2TexCoords;

out vec4 color;

struct Light {
    vec4 position;
    vec4 direction;
    vec4 color;
	float ambientCoeff;
    float constant;
    float linear;
    float quadratic;
    float cutOff;
    float outerCutOff;
	float isOn;
	float isSpotLight;
};

uniform Light lights[MAX_LIGHTS];


// Helper functions to apply control flow without shader branchings from normal if/else statements
float ifelsef(float valueIf, float valueElse, float valueIn)
{
    return valueIn * valueIf + (1.f - valueIn) * valueElse;
}

vec3 ifelse3v(vec3 valueIf, vec3 valueElse, float valueIn)
{
    return valueIn * valueIf + (1.f - valueIn) * valueElse;
}


// Declare light calc function
vec3 phong(Light light, vec3 surfDiffCol, vec3 surfSpecCol, vec3 normal, vec3 fragPos, vec3 surfToViewDir);


void main()
{
	float rate = 0.5f; // seconds per segment
	float bandSize = 0.5f;
	vec3 bandColor = vec3(0.f);
	float loopCount;
	float timeRatio = modf(fGlobalTime / rate, loopCount);

    vec3 norm = normalize(v3Normal);
    vec3 fragToViewDir = normalize(-v3FragPos);
	vec4 surfaceDiffColor = v4Color * diffColor;
	//surfaceDiffColor *= texture(diffuseTex, v2TexCoords);

	float intPart;
	float ratioAlongSegment = modf(timeRatio, intPart);
	float beginSegmentStripe =  ratioAlongSegment - bandSize;
	float endSegmentStripe = ratioAlongSegment;

	if (beginSegmentStripe < 0.f)
	{
		beginSegmentStripe += 1.f;		

		if (modf(v2TexCoords.y, intPart) > beginSegmentStripe || modf(v2TexCoords.y, intPart) < endSegmentStripe)
			surfaceDiffColor.rgb *= bandColor;
		else
			surfaceDiffColor.rgb *= vec3(1.f);
	}
	else
	{
		if (modf(v2TexCoords.y, intPart) > beginSegmentStripe && modf(v2TexCoords.y, intPart) < endSegmentStripe)
			surfaceDiffColor.rgb *= bandColor;
		else
			surfaceDiffColor.rgb *= vec3(1.f);
	}

	//surfaceDiffColor.a *= ratioAlongSegment;
	//surfaceDiffColor.rgb *= mix(vec3(0.5f), vec3(1.f), ratioAlongSegment);
	
	if (v2TexCoords.x > 0.49999f && v2TexCoords.x < 0.50001)
		surfaceDiffColor = v4Color * diffColor;

	if (surfaceDiffColor.a == 0.f)
	    discard;

	vec4 surfaceSpecColor = texture(specularTex, v2TexCoords) * specColor;
	vec4 surfaceEmisColor = texture(emissiveTex, v2TexCoords);
	
    vec3 result = vec3(0.f);

	for(int i = 0; i < numLights; i++)
		result += ifelse3v(phong(lights[i], surfaceDiffColor.rgb, surfaceSpecColor.rgb, norm, v3FragPos, fragToViewDir), vec3(0.f), lights[i].isOn);

	result += surfaceEmisColor.rgb;
	//vec3 gammaCorrection = vec3(1.f/2.2f);
	//color = vec4(pow(result, gammaCorrection), 1.0);
    color = vec4(result, surfaceDiffColor.a);
}


vec3 phong(Light light, vec3 surfDiffCol, vec3 surfSpecCol, vec3 normal, vec3 fragPos, vec3 surfToViewDir)
{
	// Point light vars
    float distance = length(light.position.xyz - fragPos);
    float attenuation = 1.0f / ifelsef((light.constant + light.linear * distance + light.quadratic * (distance * distance)), 1.f, light.position.w);
	// Directional (infinite) lights have a 0 w component for their position; point and spot lights have a 1
    vec3 fragToLightDir = ifelse3v(normalize(light.position.xyz - fragPos), -light.direction.xyz, light.position.w);
	
	// Spotlight vars
    float theta = dot(fragToLightDir, -light.direction.xyz);
	// Avoid div by 0
    float invEpsilon = 1.f / ifelsef(light.cutOff - light.outerCutOff, 1.f, light.position.w);
    float intensity = clamp((theta - light.outerCutOff) * invEpsilon, 0.0, 1.0);
	intensity = ifelsef(intensity, 1.f, light.isSpotLight);
	
	// Calculate lighting
	vec3 ambient = light.color.rgb * light.ambientCoeff * surfDiffCol;
	
    float diffCoeff = max(dot(normal, fragToLightDir), 0.0);
    vec3 diffuse = light.color.rgb * diffCoeff * surfDiffCol;
	
    float specCoeff = pow(max(dot(surfToViewDir, reflect(-fragToLightDir, normal)), 0.0), shininess);
    vec3 specular = light.color.rgb * specCoeff * surfSpecCol;
	
    ambient *= attenuation * intensity;
    diffuse *= attenuation * intensity;
    specular *= attenuation * intensity;
    return (ambient + diffuse + specular);
}