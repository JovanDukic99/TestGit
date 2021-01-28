#version 330

// the vertex shader operates on each vertex

// input data (in) from the VBO
// Each vertex contains of 2 floats (x, y)
// every in... has an index, it counts from 0
// that index we use in our program to fill inputs

in vec2 vertexPosition;
in vec4 vertexColor;

// flat for no interpolation
// ignors color of some vertices
// must be set on same variable in vertex and fragment shader
/* flat out vec4 fragmentColor; */

// output data (out) to fragment shader
// smooth means linear interpolation
out vec2 fragmentPosition;
smooth out vec4 fragmentColor;

uniform float time;

void main() { 
    // set the x,y coordinates of vertex

    float x = (cos(time) - 1) * 0.5f;
    float y = (sin(time) - 1) * 0.5f;

    gl_Position.xy = vertexPosition + vec2(x, y);

    // z coordinate is 0, bcs we are in 2D space
    gl_Position.z = 0.0;

    // indicate that the coordinates are normalized
    gl_Position.w = 1.0;

    // setting up fragment position
    fragmentPosition = vertexPosition + vec2(x, y);

    // setting up fragment color
    fragmentColor = vertexColor;
}

