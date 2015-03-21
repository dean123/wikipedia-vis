#version 330

///////////////////////////////////////////////////////////////////

// vertex attribute layout
layout( points ) in;
layout( triangle_strip, max_vertices = 4 ) out;

///////////////////////////////////////////////////////////////////

// input/output definitions

uniform mat4 Projection;
uniform mat4 View;
uniform mat4 Model;

// Color input
in vData
{
    vec4 color;
}vertices[];


// Color output
out fData
{
    vec4 color;
}frag;

///////////////////////////////////////////////////////////////////

// implementation

void main(void)
{
  vec4 pos = gl_in[0].gl_Position;

  pos = Projection * View * Model * pos;

  gl_Position = pos + vec4(-0.002, 0.002, 0.0, 0.0);
  frag.color = vertices[0].color;
  EmitVertex();

  gl_Position = pos + vec4(-0.002, -0.002, 0.0, 0.0);
  frag.color = vertices[0].color;
  EmitVertex();

  gl_Position = pos + vec4(0.002, 0.002, 0.0, 0.0);
  frag.color = vertices[0].color;
  EmitVertex();

  gl_Position = pos + vec4(0.002, -0.002, 0.0, 0.0);
  frag.color = vertices[0].color;
  EmitVertex();


  EndPrimitive();
}
