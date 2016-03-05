#version 400 core
//------------------------------------------------------------------------------------------
// fragment shader, phong shading
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

// texture unit: objTex(colorMap) = 0, normalTex = 1, depthTex = 2
uniform sampler2D objTex;
uniform sampler2D normalTex;
uniform bool hasObjTex;
uniform bool hasNormalTex;
uniform bool needTangent;

//------------------------------------------------------------------------------------------
// in variables
in GS_OUT
{
    vec4 f_shadowCoord;
    vec3 f_normal;
    vec3 f_viewDir;
    vec2 f_texCoord;
    vec3 f_tangent;
    vec3 f_btangent;
};

//------------------------------------------------------------------------------------------
// out variables
out vec4 fragColor;

//------------------------------------------------------------------------------------------
// determine the normal belong to a specific plane
const vec3 planeNormal[6] = vec3[6](
    vec3(0, 1, 0), // floor
    vec3(0, -1, 0), // ceiling
    vec3(1, 0, 0), //left
    vec3(-1, 0, 0), //  right
    vec3(0, 0, -1), // front
    vec3(0, 0, 1) // back
);

void calculateNormal(in vec3 vertexNormal,  in vec3 normalIn, out vec3 normalOut)
{
    normalOut = vec3(0);
    for(int i=0; i < 6; ++i)
    {
        if(dot(vertexNormal, planeNormal[i]) > 0.5) // this is current plane
        {
            switch(i)
            {
                case 0:
                    normalOut = normalIn.xzy;
                    break;
                case 1:
                    normalOut = -normalIn.xzy;
                    break;
                case 2:
                    normalOut = normalIn.zxy;
                    break;
                case 3:
                    normalOut = -normalIn.zxy;
                    break;
                case 4:
                    normalOut = -normalIn.xyz;
                    break;
                case 5:
                    normalOut = normalIn.xyz;
                break;

            }

            break;
        }
    }
}

//------------------------------------------------------------------------------------------
// If an object uses texture, it must set "GL_TRUE" to hasObjTex
//------------------------------------------------------------------------------------------
void main()
{
    vec3 normal = normalize(f_normal);
    vec3 lightDir = -normalize(vec3(light.direction));
    vec3 viewDir = normalize(f_viewDir);

    if(hasNormalTex)
    {
        if(needTangent)
        {
            vec3 tangent = normalize(f_tangent);
            vec3 btangent = normalize(f_btangent);

            if(dot(cross(normal, tangent), btangent) < 0)
            {
                tangent = normalize(tangent - normal * dot(normal, tangent));
                btangent = -cross(normal, tangent);
            }
            else
            {
                tangent = normalize(tangent - normal * dot(normal, tangent));
                btangent = cross(normal, tangent);
            }

            vec3 bumpMapNormal = 2.0 * texture(normalTex, f_texCoord).xyz - vec3(1.0);
            mat3 TBN = mat3(tangent, btangent, normal);
            normal = TBN * bumpMapNormal;
        }
        else
        {
            calculateNormal(normalize(f_normal), 2.0 * texture(normalTex, f_texCoord).xyz - 1.0, normal);
        }
        normal = normalize(normal);
    }




    float alpha = 0.0f;
    vec3 surfaceColor = vec3(0.0f);

    if(hasObjTex)
    {
        vec4 texVal = texture(objTex, f_texCoord);

        surfaceColor = texVal.xyz;
        alpha = texVal.w;
    }

    surfaceColor = mix(vec3(material.diffuseColor), surfaceColor, alpha);

    vec3 ambient = ambientLight * surfaceColor;
    vec3 diffuse = vec3(max(dot(normal, lightDir), 0.0f)) * surfaceColor;

    vec3 specular = vec3(0.0f);

    vec3 halfDir = normalize(lightDir + viewDir);
    specular = pow(max(dot(halfDir, normal), 0.0f), material.shininess) * vec3(material.specularColor);

    /////////////////////////////////////////////////////////////////
    // output
    fragColor = vec4(ambient + light.intensity * (diffuse + specular), alpha);
}
