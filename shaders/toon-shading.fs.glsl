#version 400 core
//------------------------------------------------------------------------------------------
// fragment shader, toon shading
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
// uniforms
layout(std140) uniform Light
{
    vec4 direction;
    vec4 color;
    float intensity;
} light;

layout(std140) uniform Material
{
    vec4 diffuseColor;
    vec4 specularColor;
    float reflection;
    float shininess;
} material;

uniform float ambientLight;

uniform sampler2DShadow depthTex;
uniform bool hasDepthTex;

//------------------------------------------------------------------------------------------
// in variables
in VS_OUT
{
    vec4 f_shadowCoord;
    vec3 f_normal;
    vec3 f_viewDir;
};

//------------------------------------------------------------------------------------------
// out variables
out vec4 fragColor;

//------------------------------------------------------------------------------------------
void main()
{
    vec3 normal = normalize(f_normal);
    vec3 lightDir = -normalize(vec3(light.direction));
    vec3 viewDir = normalize(f_viewDir);

    vec3 surfaceColor = vec3(material.diffuseColor);
    vec3 ambient = ambientLight * surfaceColor;

    float diffuseLight = max(dot(normal, lightDir), 0.0f);


    // Discretize the intensity, based on a few cutoff points
    if (diffuseLight > 0.95)
        diffuseLight = 1.0;
    else if (diffuseLight > 0.5)
        diffuseLight = 0.7;
    else if (diffuseLight > 0.05)
        diffuseLight = 0.35;
    else
        diffuseLight = 0.0;

    vec3 diffuse = diffuseLight * surfaceColor;

    vec3 specular = vec3(0.0f);
    float isNoShadow = 1.0f;

    vec3 halfDir = normalize(lightDir + viewDir);
    float specularLight = pow(max(dot(halfDir, normal), 0.0f), material.shininess);

    if (specularLight > 0.98)
        specularLight = 1.0;
    else if (specularLight > 0.8)
        specularLight = 0.7;
    else if (specularLight > 0.4)
        specularLight = 0.35;
    else
        specularLight = 0.0;

    specular = specularLight * vec3(material.specularColor);

    if(hasDepthTex)
    {
        if(any(lessThan(vec3(f_shadowCoord), vec3(0.0))) )
            isNoShadow = 1.0f;
        else
            isNoShadow = textureProj(depthTex, f_shadowCoord);
    }


    /////////////////////////////////////////////////////////////////
    // output
    fragColor = vec4(ambient + isNoShadow * light.intensity * (diffuse + specular), 1.0);
}
