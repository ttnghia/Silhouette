#version 400 core
//------------------------------------------------------------------------------------------
// vertex shader, silhouette rendering
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
// uniforms
layout(std140) uniform Matrices
{
    mat4 modelMatrix;
    mat4 normalMatrix;
    mat4 viewProjectionMatrix;
    mat4 shadowMatrix;
};

uniform float offset;
//------------------------------------------------------------------------------------------
in vec3 v_coord;
in vec3 v_normal;

//------------------------------------------------------------------------------------------
void main()
{
    vec4 worldCoord = modelMatrix * vec4(v_coord + offset*mat3(normalMatrix) * v_normal, 1.0f);

    /////////////////////////////////////////////////////////////////
    // output
    gl_Position = viewProjectionMatrix * worldCoord;
}
