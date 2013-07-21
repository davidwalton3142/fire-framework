template<int maxParticles>
AdvectParticles<maxParticles>::AdvectParticles(ParticleShader* _shader, 
	Texture* _bbTex, Texture* _decayTex)
	:ParticleSystem(_shader),
	 bbTex(_bbTex), decayTex(_decayTex),
	 avgLifetime(1500), varLifetime(200),
	 perturbChance(10),
	 initAcn(glm::vec4(0.0, 0.000001, 0.0, 0.0)),
	 initVel(glm::vec4(0.0, 0.0, 0.0, 0.0)),
	 perturbRadius(0.0005f),
	 centerForce(0.00001f),
	 baseRadius(0.2f),
	 bbHeight(0.2f), bbWidth(0.2f),
	 perturb_on(true), init_perturb(false),
	 cameraDir(glm::vec3(0.0, 0.0, -1.0))
{
	shader->use();

	// Set up particles.
	for(int i = 0; i < maxParticles; ++i)
	{
		spawnParticle(i);
	}

	init(bbTex, decayTex);

	glUseProgram(0);
}

template<int maxParticles>
AdvectParticles<maxParticles>::AdvectParticles(ParticleShader* _shader, 
	Texture* _bbTex, Texture* _decayTex,
	int _avgLifetime, int _varLifetime, 
	glm::vec4 _initAcn, glm::vec4 _initVel,
	int _perturbChance, float _perturbRadius,
	float _baseRadius, float _centerForce,
	float _bbHeight, float _bbWidth,
	bool _perturb_on, bool _init_perturb)
	:ParticleSystem(_shader),
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
	shader->use();

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
	glDepthMask(GL_FALSE);
	if(!scene) return;

	shader->setModelToWorld(modelToWorld);

	shader->setBBTexUnit(bbTex->getTexUnit());
	shader->setDecayTexUnit(decayTex->getTexUnit());

	shader->use();

	glBindBuffer(GL_ARRAY_BUFFER, pos_vbo);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(pos), &(pos));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);
	
	glBindBuffer(GL_ARRAY_BUFFER, decay_vbo);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(decay), &(decay));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	
	glDrawArrays(GL_POINTS, 0, maxParticles);

	glUseProgram(0);
	glDepthMask(GL_TRUE);
}

template <int maxParticles>
void AdvectParticles<maxParticles>::update(int dTime)
{
	for(int i = 0; i < maxParticles; ++i)
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
	shader->setBBTexUnit(bbTex->getTexUnit());
	shader->setDecayTexUnit(decayTex->getTexUnit());

	// Set up vertex buffer objects.
	glGenBuffers(1, &pos_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, pos_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(pos), &(pos), GL_DYNAMIC_DRAW);

	glGenBuffers(1, &decay_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, decay_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(decay), &decay, GL_DYNAMIC_DRAW);

	// Set up uniforms.
	shader->setBBWidth(bbWidth);
	shader->setBBHeight(bbHeight);

	pos_attrib = shader->getAttribLoc("vPos");
	decay_attrib = shader->getAttribLoc("vDecay");
}

template <int maxParticles>
AdvectParticlesLights<maxParticles>::AdvectParticlesLights(int _nLights, 
	ParticleShader* _shader, Texture* _bbTex, Texture* _decayTex)
	:AdvectParticles<maxParticles>(_shader, _bbTex, _decayTex),
	 nLights(_nLights)
{
	// Set up vector of lights.
	for(int i = 0; i < nLights; ++i)
	{
		lights.push_back(new PhongLight(getOrigin()));
	}
}

template <int maxParticles>
AdvectParticlesLights<maxParticles>::AdvectParticlesLights(
	int nLights, ParticleShader* _shader, 
	Texture* _bbTex, Texture* _decayTex,
	int avgLifetime, int varLifetime, 
	glm::vec4 initAcn, glm::vec4 initVel,
	int perturbChance, float perturbRadius,
	float baseRadius, float centerForce,
	float bbHeight, float bbWidth,
	bool perturb_on, bool _init_perturb)
	:AdvectParticles<maxParticles>(_shader, _bbTex, _decayTex,
	 avgLifetime, varLifetime, 
	 initAcn, initVel,
	 perturbChance, perturbRadius,
	 baseRadius, centerForce,
	 bbHeight, bbWidth,
	 perturb_on, _init_perturb),
	 nLights(_nLights)
{
	// Set up vector of lights.
	for(int i = 0; i < nLights; ++i)
	{
		lights.push_back(new PhongLight(getOrigin()));
	}
}

template <int maxParticles>
void AdvectParticlesLights<maxParticles>::onAdd()
{
	PhongLight* p;
	// Add lights
	for(int i = 0; i < nLights; ++i)
	{
		p = scene->add(lights[i]);
		if(p == nullptr) //Light not added correctly (too many lights?).
		{
			std::cout << "Warning: Not all particle lights could be added.\n";
			break;
		}
	}
}

template <int maxParticles>
void AdvectParticlesLights<maxParticles>::onRemove()
{
	for (int i = 0; i < nLights; ++i)
	{
		scene->remove(lights[i]);
	}
}

template <int maxParticles>
void AdvectParticlesLights<maxParticles>::update(int dTime)
{
	AdvectParticles<maxParticles>::update(dTime);
	updateLights();
}

template <int maxParticles>
AdvectParticlesRandLights<maxParticles>::AdvectParticlesRandLights(int _nLights, int _interval, ParticleShader* _shader, 
		Texture* _bbTex, Texture* _decayTex)
		:AdvectParticlesLights(_nLights, _shader, 
		 _bbTex, _decayTex),
		 interval(_interval), counter(0), particles(std::vector<int>(nLights)) {init();}

template <int maxParticles>
AdvectParticlesRandLights<maxParticles>::AdvectParticlesRandLights(int _nLights, int _interval, ParticleShader* _shader, 
		Texture* _bbTex, Texture* _decayTex,
		int avgLifetime, int varLifetime, 
		glm::vec4 initAcn, glm::vec4 initVel,
		int perturbChance, float perturbRadius,
		float baseRadius, float centerForce,
		float bbHeight, float bbWidth,
		bool perturb_on, bool _init_perturb)
		:AdvectParticlesLights(nLights, _shader, 
		_bbTex, _decayTex,
		avgLifetime, varLifetime, 
		initAcn, initVel,
		perturbChance, perturbRadius,
		baseRadius, centerForce,
		bbHeight, bbWidth,
		perturb_on, _init_perturb),
		interval(_interval), counter(0), particles(std::vector<int>(nLights)) {init();}

template <int maxParticles>
void AdvectParticlesRandLights<maxParticles>::init()
{
	for(int i = 0; i < nLights; ++i)
		particles[i] = 0;
	randomizeLights();
}

template <int maxParticles>
void AdvectParticlesRandLights<maxParticles>::updateLights()
{
	if(interval == 0)
		randomizeLights();
	else if(interval > 0)
	{
		counter += glutGet(GLUT_ELAPSED_TIME);
		if(counter > interval)
		{
			randomizeLights();
			counter = 0;
		}
	}

	for(int i = 0; i < nLights; ++i)
	{
		lights[i]->setPos(modelToWorld * pos[particles[i]]);
	}
}

template <int maxParticles>
void AdvectParticlesRandLights<maxParticles>::randomizeLights()
{
	//Move each light to the location of a randomly selected particle.
	int randIndex;
	for(int i = 0; i < nLights; ++i)
	{
		randIndex = randi(0, maxParticles);
		particles[i] = randIndex;
	}
}

template <int maxParticles>
AdvectParticlesCentroidLights<maxParticles>::AdvectParticlesCentroidLights(
		int _nLights, int _clumpSize,
		int _interval, ParticleShader* _shader, 
		Texture* _bbTex, Texture* _decayTex)
		:AdvectParticlesLights(_nLights, _shader, 
		 _bbTex, _decayTex),
		 interval(_interval), counter(0),
		 clumpSize(_clumpSize)
{init();}

template <int maxParticles>
AdvectParticlesCentroidLights<maxParticles>::AdvectParticlesCentroidLights(
		int _nLights, int _clumpSize,
		int _interval, ParticleShader* _shader, 
		Texture* _bbTex, Texture* _decayTex,
		int avgLifetime, int varLifetime, 
		glm::vec4 initAcn, glm::vec4 initVel,
		int perturbChance, float perturbRadius,
		float baseRadius, float centerForce,
		float bbHeight, float bbWidth,
		bool perturb_on, bool _init_perturb)
		:AdvectParticlesLights(nLights, _shader, 
		_bbTex, _decayTex,
		avgLifetime, varLifetime, 
		initAcn, initVel,
		perturbChance, perturbRadius,
		baseRadius, centerForce,
		bbHeight, bbWidth,
		perturb_on, _init_perturb),
		interval(_interval), counter(0),
		clumpSize(_clumpSize)
{init();}

template <int maxParticles>
void AdvectParticlesCentroidLights<maxParticles>::init()
{
	for(int i = 0; i < nLights; ++i)
	{
		std::vector<int> clump;
		for(int j = 0; j < clumpSize; ++j)
			clump.push_back(randi(0, maxParticles));
		clumps.push_back(clump);
	}
}

template <int maxParticles>
void AdvectParticlesCentroidLights<maxParticles>::randomizeClumps()
{
	for(std::vector<std::vector<int>>::iterator i = clumps.begin(); i != clumps.end(); ++i)
		for(std::vector<int>::iterator j = i->begin(); j != i->end(); ++j)
			(*j) = randi(0, maxParticles);
}

template <int maxParticles>
void AdvectParticlesCentroidLights<maxParticles>::updateLights()
{
	if(interval == 0)
		randomizeClumps();
	else if(interval > 0)
	{
		counter += glutGet(GLUT_ELAPSED_TIME);
		if(counter > interval)
		{
			randomizeClumps();
			counter = 0;
		}
	}

	for(int i = 0; i < nLights; ++i)
	{
		lights[i]->setPos(modelToWorld * getParticleCentroid(clumps[i]));
	}
}

template <int maxParticles>
glm::vec4 AdvectParticlesCentroidLights<maxParticles>::getParticleCentroid(const std::vector<int>& clump)
{
	glm::vec4 sum;
	for(std::vector<int>::const_iterator i = clump.begin(); i != clump.end(); ++i)
		sum += pos[(*i)];
	return sum / (float) clump.size();
}
