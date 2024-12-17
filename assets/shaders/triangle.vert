precision mediump float;

in vec3 position;

void main()
{
    gl_Position = vec4(vec3(position), 1.0);
    //vec4(position, 1.0);
}
