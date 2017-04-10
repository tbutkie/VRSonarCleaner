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

float lineWidth = 1.f;
vec4 lineColor = vec4(0.f, 0.f, 0.f, 1.f);
vec4 fillColor = vec4(0.f, 0.f, 0.f, 0.f);

in vec3 GNorm;
in vec3 GPos;
in vec2 GTex;
noperspective in vec3 GEdgeDist;

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
    vec3 norm = normalize(GNorm);
	norm = float(gl_FrontFacing) * norm + (1.f - float(gl_FrontFacing)) * -norm;
    vec3 fragToViewDir = normalize(-GPos);
	vec4 surfaceDiffColor = texture(diffuseTex, GTex);
	vec4 surfaceSpecColor = texture(specularTex, GTex);
	vec4 surfaceEmisColor = texture(emissiveTex, GTex);
	
    vec3 result = vec3(0.f);

	for(int i = 0; i < numLights; i++)
		result += ifelse3v(phong(lights[i], surfaceDiffColor.rgb, surfaceSpecColor.rgb, norm, GPos, fragToViewDir), vec3(0.f), lights[i].isOn);

	result += surfaceEmisColor.rgb;
	//vec3 gammaCorrection = vec3(1.f/2.2f);
	//color = vec4(pow(result, gammaCorrection), 1.0);	
	color = vec4(result, surfaceDiffColor.a);
	
	// Smallest distance to edge
	float d = min(min(GEdgeDist.x, GEdgeDist.y), GEdgeDist.z);
	
	// Nice interpolation
	float mixVal = smoothstep(lineWidth - 1.f, lineWidth + 1.f, d);

	// Set wireframe line and transparent fill
	lineColor = color;
	fillColor = vec4(lineColor.rbg, 0.f);
	color = mix(lineColor, fillColor, mixVal);
	
	// Discard transparent fragments so they don't write to depth buffer
	if (color.a <= 0.f)
		discard;
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