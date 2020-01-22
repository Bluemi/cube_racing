VISUALIZER_SHADER_STRINGIFY(
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;

out vec3 sheepColor;
out vec3 vertex_position;

uniform mat4 model;
uniform mat4 proj_view;

void main()
{
	gl_Position = proj_view * model * vec4(aPos, 1.0);
	sheepColor = aColor;
	vertex_position = (model * vec4(aPos, 1.0)).xyz;
}
)
