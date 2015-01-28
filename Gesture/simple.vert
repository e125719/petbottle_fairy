#version 120



void main(void)
{

  gl_Position = ftransform();
    
    gl_FrontColor.rgb = 0.5 * gl_Normal.xyz + 0.5;
}
