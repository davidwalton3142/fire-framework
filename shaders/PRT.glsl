-- Vertex
#version 420

in vec4 vPos;
in vec3 transferCoeffts[9];

uniform mat4 modelToWorld;

layout(std140) uniform cameraBlock
{
	mat4 worldToCamera;
	vec3 cameraPos;
	vec3 cameraDir;
};

layout(std140) uniform SHBlock
{
	vec3 lightCoeffts[9 * 10];
	uint nLights;
};

out vec3 color;

void main()
{
	gl_Position = worldToCamera * modelToWorld * vPos;

	color = vec3(0.0, 0.0, 0.0);

	for(int l = 0; l < nLights; ++l)
	{
		for(int c = 0; c < 9; ++c)
		{
			color += transferCoeffts[c] * lightCoeffts[c + (l * 9)];
		}
	}
}

-- Fragment
#version 420

in vec3 color;

out vec4 fragColor;

void main()
{
	fragColor = vec4(color, 1.0);
}
