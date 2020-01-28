#include "global.inc"

layout(binding = 0) uniform sampler2D texture0;		// camera feed
//layout(binding = 1) uniform sampler2D texture1;	// noise

in vs_output
{
  vec4 color;
  vec2 texcoord;
} frag;

out vec4 result;

// TO-DO: Make some CRT filter stuff

void main(void)
{
	vec2 trueCoord = frag.texcoord;
	trueCoord.y = 1.0 - trueCoord.y; // flip image vertically, else it'll appear flipped in-game

	vec4 col = texture2D( texture0, trueCoord );
	col = (vec4(col.r) + vec4(col.g) + vec4(col.b)) / 3.0; // grayscale

	result = col;
}