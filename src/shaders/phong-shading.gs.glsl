#version 400 core
#extension GL_EXT_geometry_shader4: enable
//------------------------------------------------------------------------------------------
// geometry shader, phong shading
//------------------------------------------------------------------------------------------
layout(triangles) in;
layout(max_vertices = 3) out;
//------------------------------------------------------------------------------------------
// uniforms
layout(std140) uniform Matrices
{
    mat4 modelMatrix;
    mat4 normalMatrix;
    mat4 viewProjectionMatrix;
    mat4 shadowMatrix;
};

uniform bool needTangent;
//------------------------------------------------------------------------------------------
// in variables
in VS_OUT
{
    vec4 f_shadowCoord;
    vec3 f_normal;
    vec3 f_viewDir;
    vec2 f_texCoord;
} v_in[];


//------------------------------------------------------------------------------------------
// out variables
out GS_OUT
{
    vec4 f_shadowCoord;
    vec3 f_normal;
    vec3 f_viewDir;
    vec2 f_texCoord;
    vec3 f_tangent;
    vec3 f_btangent;
};

//------------------------------------------------------------------------------------------
void calculateTangents(in vec4 v1, in vec4 v2, in vec4 v3, in vec2 vt1, in vec2 vt2,
                       in vec2 vt3, out vec3 tangent, out vec3 btangent)
{
    float x1 = v2.x - v1.x;
    float x2 = v3.x - v1.x;
    float y1 = v2.y - v1.y;
    float y2 = v3.y - v1.y;
    float z1 = v2.z - v1.z;
    float z2 = v3.z - v1.z;

    float s1 = vt2.x - vt1.x;
    float s2 = vt3.x - vt1.x;
    float t1 = vt2.y - vt1.y;
    float t2 = vt3.y - vt1.y;

    float r = 1.0 / (s1 * t2 - s2 * t1);

    tangent = vec3((t2 * x1 - t1 * x2) * r, (t2 * y1 - t1 * y2) * r,
               (t2 * z1 - t1 * z2) * r);
    btangent = vec3((s1 * x2 - s2 * x1) * r, (s1 * y2 - s2 * y1) * r,
               (s1 * z2 - s2 * z1) * r);
    tangent = normalize(tangent);
    btangent = normalize(btangent);
}

//------------------------------------------------------------------------------------------
void main()
{
    vec3 tangent = vec3(1, 0, 0);
    vec3 btangent = vec3(0, 1, 0);
    if(needTangent)
    {
       calculateTangents(gl_in[0].gl_Position, gl_in[1].gl_Position, gl_in[2].gl_Position,
                         v_in[0].f_texCoord, v_in[1].f_texCoord, v_in[2].f_texCoord,
                         tangent, btangent);
    }

    for (int i = 0; i < 3; i++)
    {
        gl_Position = viewProjectionMatrix * gl_in[i].gl_Position;
        f_shadowCoord = v_in[i].f_shadowCoord;
        f_normal = v_in[i].f_normal;
        f_viewDir = v_in[i].f_viewDir;
        f_texCoord = v_in[i].f_texCoord;

        f_tangent = tangent;
        f_btangent = btangent;

        EmitVertex();
    }

}
