#version 400 core

//attribute vec2 v_coord;
in vec3 v_coord;

//------------------------------------------------------------------------------------------
void main()
{
    gl_Position = vec4(v_coord, 1);
}
