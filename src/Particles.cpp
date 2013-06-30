#include "Particles.hpp"

template<int maxParticles>
AdvectParticles<maxParticles>::AdvectParticles(Shader* _renderShader, 
	Texture* _bbTex, Texture* _decayTex)
	:ParticleSystem(_renderShader),
	 bbTex(_bbTex), decayTex(_decayTex),
	 avgLifetime(1500), varLifetime(200),
	 perturbChance(10),
	 initAcn(glm::vec4(0.0, 0.00000001, 0.0, 0.0)),
	 initVel(glm::vec4(0.0, 0.0, 0.0, 0.0)),
	 perturbRadius(0.000005f),
	 centerForce(0.00001f),
	 baseRadius(0.0005f),
	 bbHeight(0.002f), bbWidth(0.001f),
	 perturb_on(true), init_perturb(false)
{
	cameraPos = glm::vec3(0.0, 0.0, -1.0);
	renderShader->use();

	// Set up particles.
	for(int i = 0; i < maxParticles; ++i)
	{
		spawnParticle(i);
	}

	init(bbTex, decayTex);

	glUseProgram(0);
}

template<int maxParticles>
AdvectParticles<maxParticles>::AdvectParticles(Shader* _renderShader, 
	Texture* _bbTex, Texture* _decayTex,
	int _avgLifetime, int _varLifetime, 
	glm::vec4 _initAcn, glm::vec4 _initVel,
	int _perturbChance, float _perturbRadius,
	float _baseRadius, float _centerForce,
	float _bbHeight, float _bbWidth,
	bool _perturb_on, bool _init_perturb)
	:ParticleSystem(_renderShader),
     bbTex(_bbTex), decayTex(_decayTex),
	 avgLifetime(_avgLifetime), varLifetime(_varLifetime),
	 perturbChance(_perturbChance),
	 initAcn(_initAcn),
	 initVel(_initVel),
	 perturbRadius(_perturbRadius),
	 centerForce(_centerForce),
	 baseRadius(_baseRadius),
	 bbHeight(_bbHeight), bbWidth(_bbWidth),
	 perturb_on(_perturb_on), init_perturb(_init_perturb)
{
	cameraPos = glm::vec3(0.0, 0.0, -1.0);
	renderShader->use();

	// Set up particles.
	for(int i = 0; i < maxParticles; ++i)
	{
		spawnParticle(i);
		if(init_perturb) vel[i] = perturb(vel[i]);
	}

	init(bbTex, decayTex);

	glUseProgram(0);
}

template <int maxParticles>
void AdvectParticles<maxParticles>::render()
{
	if(!scene) return;

	renderShader->setModelToWorld(modelToWorld);

	renderShader->setBBTexUnit(bbTex->getTexUnit());
	renderShader->setDecayTexUnit(decayTex->getTexUnit());

	glm::mat4 cam = scene->camera->getMat();
	cameraPos = glm::vec3(cam[3][0], cam[3][1], cam[3][2]);
	renderShader->setCameraPos(cameraPos);

	glBindBuffer(GL_ARRAY_BUFFER, pos_vbo);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(pos), &(pos));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);
	
	glBindBuffer(GL_ARRAY_BUFFER, decay_vbo);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(decay), &(decay));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	
	glDrawArrays(GL_POINTS, 0, MaxParticles);

	glUseProgram(0);
}

template <int maxParticles>
void AdvectParticles<maxParticles>::update(int dTime)
{
	for(int i = 0; i < MaxParticles; ++i)
	{
		updateParticle(i, dTime);
	}
}

template <int maxParticles>
void AdvectParticles<maxParticles>::updateParticle(int index, int dTime)
{
	time[index] += dTime;
	if(time[index] > lifeTime[index]) spawnParticle(index);
	decay[index] = ((float) time[index]) / ((float) lifeTime[index]);

	if(time[index] % perturbChance == 1 && perturb_on) vel[index] = perturb(vel[index]);

	vel[index] += (float) dTime * (acn[index] + (glm::vec4(-pos[index].x, 0.0, -pos[index].z, 0.0) * centerForce));
	pos[index] += (float) dTime * vel[index];
}

template <int maxParticles>
void AdvectParticles<maxParticles>::spawnParticle(int index)
{
	time[index] = 0;
	lifeTime[index] = avgLifetime + randi(-varLifetime, +varLifetime);
	decay[index] = 0.0;
	acn[index] = initAcn;
	vel[index] = initVel;
	pos[index] = randInitPos();
}

template <int maxParticles>
glm::vec4 AdvectParticles<maxParticles>::randInitPos()
{
	float theta = randf(0.0f, 2.0f * PI);
	float radius = randf(0.0f, baseRadius);
	return glm::vec4(radius*cos(theta), 0.0, radius*sin(theta), 1.0);
}

template <int maxParticles>
glm::vec4 AdvectParticles<maxParticles>::perturb(glm::vec4 input)
{
	float theta = randf(0.0f, 2.0f * PI);
	float radius = randf(0.0f, perturbRadius);
	return input + glm::vec4(radius * cos(theta), 0.0, radius * sin(theta), 0.0);
}

template <int maxParticles>
float AdvectParticles<maxParticles>::randf(float low, float high)
{
	float r = (float) rand() / (float) RAND_MAX;
	return low + ((high - low) * r);
}

template <int maxParticles>
int AdvectParticles<maxParticles>::randi(int low, int high)
{
	int r = rand() % (high - low);
	return r + low;
}

template <int maxParticles>
void AdvectParticles<maxParticles>::init(Texture* bbTex, Texture* decayTex)
{
	renderShader->setBBTexUnit(bbTex->getTexUnit());
	renderShader->setDecayTexUnit(decayTex->getTexUnit());

	// Set up vertex buffer objects.
	glGenBuffers(1, &pos_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, pos_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(pos), &(pos), GL_DYNAMIC_DRAW);

	glGenBuffers(1, &decay_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, decay_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(decay), &decay, GL_DYNAMIC_DRAW);

	// Set up uniforms.
	renderShader->setBBWidth(bbWidth);
	renderShader->setBBHeight(bbHeight);
	renderShader->setCameraPos(cameraPos);

	pos_attrib = renderShader->getAttribLoc("vPos");
	decay_attrib = renderShader->getAttribLoc("vDecay");
}

template <int maxParticles>
AdvectParticlesRandLights::AdvectParticlesRandLights(int _nLights, 
	Shader* _renderShader, Texture* _bbTex, Texture* _decayTex)
		:AdvectParticles(Shader* _renderShader, 
			Texture* _bbTex, Texture* _decayTex),
		nLights(_nLights)
{
	// Set up vector of lights.
	for(int i = 0; i < nLights; ++i)
	{
		lights.push_back(new PointLight(getOrigin(), 0.2));
	}
}
