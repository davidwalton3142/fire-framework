-- Vertex
#version 420

in vec4 vPos;
in vec4 transferCoeffts[9];

uniform mat4 modelToWorld;

layout(std140) uniform cameraBlock
{
	mat4 worldToCamera;
	vec3 cameraPos;
	vec3 cameraDir;
};

layout(std140) uniform SHBlock
{
	vec4 lightCoeffts[$nSHCoeffts$ * $maxSHLights$];	
	int nLights;
};

out vec3 color;

void main()
{
	gl_Position = worldToCamera * modelToWorld * vPos;

	color = vec3(0.0, 0.0, 0.0);

	for(int l = 0; l < $maxSHLights$; ++l)
	{
		for(int c = 0; c < $nSHCoeffts$; ++c)
		{
			color += transferCoeffts[c].xyz * lightCoeffts[c + (l * $nSHCoeffts$)].xyz;
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
