VISUALIZER_SHADER_STRINGIFY(
layout (location = 0) in vec2 aPos;

uniform mat4 projection;
uniform mat4 model;

void main()
{
	gl_Position = projection * model * vec4(aPos.xy, 0.0, 1.0);
}
)
