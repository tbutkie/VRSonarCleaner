layout(binding = DIFFUSE_TEXTURE_BINDING)
	uniform sampler2D diffuseTex;
layout(binding = SPECULAR_TEXTURE_BINDING)
	uniform sampler2D specularTex;
layout(binding = EMISSIVE_TEXTURE_BINDING)
	uniform sampler2D emissiveTex;
layout(location = MATERIAL_SHININESS_UNIFORM_LOCATION)
	uniform float shininess;
layout(location = DIR_LIGHTS_COUNT_UNIFORM_LOCATION)
	uniform int numDirectionalLights;
layout(location = POINT_LIGHTS_COUNT_UNIFORM_LOCATION)
	uniform int numPointLights;
layout(location = SPOT_LIGHTS_COUNT_UNIFORM_LOCATION)
	uniform int numSpotLights;

float lineWidth = 1.f;
vec4 lineColor = vec4(0.f, 0.f, 0.f, 1.f);
vec4 fillColor = vec4(0.f, 0.f, 0.f, 0.f);

in vec3 GNorm;
in vec3 GPos;
in vec2 GTex;
noperspective in vec3 GEdgeDist;

out vec4 color;

struct DirLight {
    vec4 direction;
    vec4 color;
	float ambientCoeff;
};

struct PointLight {
    vec4 position;
    vec4 color;
	float ambientCoeff;
    float constant;
    float linear;
    float quadratic;
};

struct SpotLight {
    vec4 position;
    vec4 direction;
    vec4 color;
	float ambientCoeff;
    float constant;
    float linear;
    float quadratic;
    float cutOff;
    float outerCutOff;
};

uniform DirLight dirLights[MAX_DIRECTIONAL_LIGHTS];
uniform PointLight pointLights[MAX_POINT_LIGHTS];
uniform SpotLight spotLights[MAX_SPOT_LIGHTS];

vec3 CalcDirLight(DirLight light, vec3 surfDiffCol, vec3 surfSpecCol, vec3 normal, vec3 surfToViewDir);
vec3 CalcPointLight(PointLight light, vec3 surfDiffCol, vec3 surfSpecCol, vec3 normal, vec3 fragPos, vec3 surfToViewDir);
vec3 CalcSpotLight(SpotLight light, vec3 surfDiffCol, vec3 surfSpecCol, vec3 normal, vec3 fragPos, vec3 surfToViewDir);

void main()
{
    vec3 norm = normalize(GNorm);
	norm = float(gl_FrontFacing) * norm + (1.f - float(gl_FrontFacing)) * -norm;
    vec3 fragToViewDir = normalize(-GPos);
	vec4 surfaceDiffColor = texture(diffuseTex, GTex);
	vec4 surfaceSpecColor = texture(specularTex, GTex);
	vec4 surfaceEmisColor = texture(emissiveTex, GTex);
	
    vec3 result = vec3(0.f);

	for(int i = 0; i < numDirectionalLights; i++)
		result += CalcDirLight(dirLights[i], surfaceDiffColor.rgb, surfaceSpecColor.rgb, norm, fragToViewDir);

	for(int i = 0; i < numPointLights; i++)
		result += CalcPointLight(pointLights[i], surfaceDiffColor.rgb, surfaceSpecColor.rgb, norm, GPos, fragToViewDir);

	for(int i = 0; i < numSpotLights; i++)
		result += CalcSpotLight(spotLights[i], surfaceDiffColor.rgb, surfaceSpecColor.rgb, norm, GPos, fragToViewDir);

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

vec3 CalcDirLight(DirLight light, vec3 surfDiffCol, vec3 surfSpecCol, vec3 normal, vec3 surfToViewDir)
{
	vec3 ambient = light.color.rgb * light.ambientCoeff * surfDiffCol;

    float diffCoeff = max(dot(normal, -light.direction.xyz), 0.0);
    vec3 diffuse = light.color.rgb * diffCoeff * surfDiffCol;
	
    float specCoeff = pow(max(dot(surfToViewDir, reflect(light.direction.xyz, normal)), 0.f), shininess);
    vec3 specular =  specCoeff * surfSpecCol;
	
    return (ambient + diffuse + specular);
}

vec3 CalcPointLight(PointLight light, vec3 surfDiffCol, vec3 surfSpecCol, vec3 normal, vec3 fragPos, vec3 surfToViewDir)
{
    float distance = length(light.position.xyz - fragPos);
    float attenuation = 1.0f / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
	
	vec3 ambient = light.color.rgb * light.ambientCoeff * surfDiffCol;
	
    vec3 fragToLightDir = normalize(light.position.xyz - fragPos);
	
    float diffCoeff = max(dot(normal, fragToLightDir), 0.0);
    vec3 diffuse = light.color.rgb * diffCoeff * surfDiffCol;
	
    float specCoeff = pow(max(dot(surfToViewDir, reflect(-fragToLightDir, normal)), 0.0), shininess);
    vec3 specular = light.color.rgb * specCoeff * surfSpecCol;
	
	ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
	
    return (ambient + diffuse + specular);
}

vec3 CalcSpotLight(SpotLight light, vec3 surfDiffCol, vec3 surfSpecCol, vec3 normal, vec3 fragPos, vec3 surfToViewDir)
{
    float distance = length(light.position.xyz - fragPos);
    float attenuation = 1.0f / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
	
	vec3 ambient = light.color.rgb * light.ambientCoeff * surfDiffCol;
	
    vec3 fragToLightDir = normalize(light.position.xyz - fragPos);
	
    float diffCoeff = max(dot(normal, fragToLightDir), 0.0);
    vec3 diffuse = light.color.rgb * diffCoeff * surfDiffCol;
	
    float specCoeff = pow(max(dot(surfToViewDir, reflect(-fragToLightDir, normal)), 0.0), shininess);
    vec3 specular = light.color.rgb * specCoeff * surfSpecCol;
	
    float theta = dot(fragToLightDir, -light.direction.xyz);
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
	
    ambient *= attenuation * intensity;
    diffuse *= attenuation * intensity;
    specular *= attenuation * intensity;
    return (ambient + diffuse + specular);
}
