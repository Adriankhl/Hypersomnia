#version 330

uniform mat4 projection_matrix;
layout(location = 0) in vec2 position;
layout(location = 1) in vec2 texcoord;
layout(location = 2) in vec4 color;
layout(location = 3) in vec4 special;

smooth out vec4 theColor;
out vec2 startingAngleVec;
out vec2 endingAngleVec;
out vec2 startingAngleInsideVec;
out vec2 endingAngleInsideVec;
out vec2 theTexcoord;

#define DEG_TO_RAD 0.0174532925
#define TO_RADIANS 180*0.0174532925

void main() 
{
	vec4 output_vert;
	output_vert.x = position.x;		
	output_vert.y = position.y;				
	output_vert.z = 0.0f;						
	output_vert.w = 1.0f;

	startingAngleVec = vec2(cos(special.x * TO_RADIANS), sin(special.x * TO_RADIANS));
	endingAngleVec = vec2(cos(special.y * TO_RADIANS), sin(special.y * TO_RADIANS));

	startingAngleInsideVec = vec2(cos(special.z * TO_RADIANS), sin(special.z * TO_RADIANS));
	endingAngleInsideVec = vec2(cos(special.w * TO_RADIANS), sin(special.w * TO_RADIANS));
	
	normalize(startingAngleVec);
	normalize(endingAngleVec);

	gl_Position = projection_matrix*output_vert;
	theColor = color;
	theTexcoord = texcoord;
}