#version 330 core
struct Material {
    sampler2D diffuse;
    sampler2D specular;
    float shininess;
}; 

struct DirLight {
    vec3 direction;
	
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct PointLight {
    vec3 position;
    
    float constant;
    float linear;
    float quadratic;
	
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct SpotLight {
    vec3 position;
    vec3 direction;
    float innerCutOff;
    float outerCutOff;
  
    float constant;
    float linear;
    float quadratic;
  
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;       
};

#define NR_POINT_LIGHTS 4

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

out vec4 color;

uniform vec3 viewPos;
uniform DirLight dirLight;
uniform PointLight pointLights[NR_POINT_LIGHTS];
uniform SpotLight spotLight;
uniform Material material;

// Function prototypes
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir);
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir);

void main()
{    
    // Properties
    vec3 uNormal = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos); // from fragPos to Camera
    
    // Phase 1: Directional lighting
    vec3 result = CalcDirLight(dirLight, uNormal, viewDir);
    // Phase 2: Point lights
    for(int i = 0; i < NR_POINT_LIGHTS; i++)
        result += CalcPointLight(pointLights[i], uNormal, FragPos, viewDir);    
    // Phase 3: Spot light
    //result += CalcSpotLight(spotLight, uNormal, FragPos, viewDir);    
    
    color = vec4(result, 1.0);
}

// Functions bodies
// Calculates the color when using a directional light.
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir)
{
    vec3 dirToLight = normalize(-light.direction);
    // Diffuse shading
    float diffIntensity = max(dot(normal, dirToLight), 0.0);
    // Specular shading
    vec3 reflectDir = reflect(-dirToLight, normal);
    float specIntensity = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    // Combine results
    vec3 ambientColor = light.ambient * vec3(texture(material.diffuse, TexCoords));
    vec3 diffuseColor = light.diffuse * diffIntensity * vec3(texture(material.diffuse, TexCoords));
    vec3 specularColor = light.specular * specIntensity * vec3(texture(material.specular, TexCoords));
    return (ambientColor + diffuseColor + specularColor);
}

// Calculates the color when using a point light.
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 dirFragToLight = normalize(light.position - fragPos);
    // Diffuse shading
    float diffIntensity = max(dot(normal, dirFragToLight), 0.0);
    // Specular shading
    vec3 reflectDir = reflect(-dirFragToLight, normal);
    float specIntensity = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    // Attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0f / (light.constant + light.linear * distance + light.quadratic * (distance * distance));    
    // Combine results
    vec3 ambientColor = light.ambient * vec3(texture(material.diffuse, TexCoords));
    vec3 diffuseColor = light.diffuse * diffIntensity * vec3(texture(material.diffuse, TexCoords));
    vec3 specularColor = light.specular * specIntensity * vec3(texture(material.specular, TexCoords));
    ambientColor *= attenuation;
    diffuseColor *= attenuation;
    specularColor *= attenuation;
    return (ambientColor + diffuseColor + specularColor);
}

// Calculates the color when using a spot light.
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 dirFragToLight = normalize(light.position - fragPos);
    // Diffuse shading
    float diffIntensity = max(dot(normal, dirFragToLight), 0.0);
    // Specular shading
    vec3 reflectDir = reflect(-dirFragToLight, normal);
    float specIntensity = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    // Attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0f / (light.constant + light.linear * distance + light.quadratic * (distance * distance));    
    // Spotlight intensity
    float theta = dot(dirFragToLight, normalize(-light.direction)); 
    float epsilon = light.innerCutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
    // Combine results
    vec3 ambientColor = light.ambient * vec3(texture(material.diffuse, TexCoords));
    vec3 diffuseColor = light.diffuse * diffIntensity * vec3(texture(material.diffuse, TexCoords));
    vec3 specularColor = light.specular * specIntensity * vec3(texture(material.specular, TexCoords));
    ambientColor *= attenuation * intensity;
    diffuseColor *= attenuation * intensity;
    specularColor *= attenuation * intensity;
    return (ambientColor + diffuseColor + specularColor);
}