#include "Scene.hpp"

Scene::~Scene()
{
	for(std::set<Renderable*>::iterator i = renderables.begin(); i != renderables.end(); ++i)
	{
		delete (*i);
	}

	for(std::set<Shader*>::iterator i = shaders.begin();
		i != shaders.end(); ++i)
	{
		delete (*i);
	}

	delete camera;
	camera = nullptr;
}

void Scene::render()
{
	updateCamera();
	for(std::set<Renderable*>::iterator i = renderables.begin(); i != renderables.end(); ++i)
	{
		(*i)->render();
	}
}

void Scene::update(int dTime)
{
	for(std::set<Renderable*>::iterator i = renderables.begin(); i != renderables.end(); ++i)
	{
		(*i)->update(dTime);
	}
}

void Scene::add(Renderable* const r)
{
	if(r == nullptr) return;
	renderables.insert(r);
	r->scene = this;
	shaders.insert(r->renderShader);

	
	setAmbLight(ambLight);
	updateDirLights();
	updatePointLights();
}

void Scene::remove(Renderable* r)
{
	renderables.erase(r);
	//TODO Remove shaders if appropriate (not needed by another renderable).
}

void Scene::add(DirLight* d)
{
	if(nDirLights >= MaxDirLights) return;
	dirLights[nDirLights] = d;
	dirLightOn[nDirLights] = d->on ? 1 : 0;
	dirLightDir[nDirLights] = d->dir;
	dirIntensity[nDirLights] = d->intensity;
	d->index = nDirLights;
	d->scene = this;
	++nDirLights;
	updateDirLights();
}

void Scene::updateLight(DirLight* d)
{
	if(d->scene != this || d->index == -1 || dirLights[d->index] != d) return;
	dirLightOn[d->index] = d->on ? 1 : 0;
	dirLightDir[d->index] = d->dir;
	dirIntensity[d->index] = d->intensity;
	updateDirLights(); 
}

void Scene::remove(DirLight* d)
{
	if(nDirLights <= 0 || d->index == -1) return;
	for(int i = d->index; i < nDirLights-1; ++i)
	{
		dirLights[i] = dirLights[i+1];
		dirLightOn[i] = dirLightOn[i+1];
		dirLights[i]->index = i;
		dirLightDir[i] = dirLightDir[i+1];
		dirIntensity[i] = dirIntensity[i+1];
	}
	dirLights[nDirLights-1] = nullptr;
	dirLightOn[nDirLights-1] = 0;
	dirLightDir[nDirLights-1] = glm::vec3(0.0);
	dirIntensity[nDirLights-1] = -1.0;
	--nDirLights;
	d->index = -1;
	updateDirLights();
}

void Scene::add(PointLight* p)
{
	if(nPointLights >= MaxPointLights) return;
	pointLightOn[p->index] = p->on ? 1 : 0;
	pointLights[nPointLights] = p;
	pointLightPos[nPointLights] = p->pos;
	pointIntensity[nPointLights] = p->intensity;
	p->index = nPointLights;
	p->scene = this;
	++nPointLights;
	updatePointLights();
}

void Scene::updateLight(PointLight* p)
{
	if(p->scene != this || p->index == -1) return;
	pointLightOn[p->index] = p->on ? 1 : 0;
	pointLightPos[p->index] = p->pos;
	pointIntensity[p->index] = p->intensity;
	updatePointLights();
}

void Scene::remove(PointLight* p)
{
	if(nPointLights <= 0 || p->index == -1) return;
	for(int i = p->index; i < nPointLights-1; ++i)
	{
		pointLights[i] = pointLights[i+1];
		pointLights[i]->index = i;
		pointLightOn[i] = pointLightOn[i+1];
		pointLightPos[i] = pointLightPos[i+1];
		pointIntensity[i] = pointIntensity[i+1];
	}
	pointLights[nPointLights-1] = nullptr;
	pointLightOn[nPointLights-1] = 0;
	pointLightPos[nPointLights-1] = glm::vec4(0.0, 0.0, 0.0, 1.0);
	pointIntensity[nPointLights-1] = -1.0;
	--nDirLights;
	p->index = -1;
	updatePointLights();
}

//TODO replace with checking by type of shader.
void Scene::setAmbLight(float _ambLight)
{
	ambLight = _ambLight;
	for(std::set<Shader*>::iterator i = shaders.begin();
		i != shaders.end(); ++i)
	{
		if((*i)->hasAmbLight)
			((LightShader*)(*i))->setAmbLight(ambLight);
	}
}

void Scene::updateDirLights()
{
	for(std::set<Shader*>::iterator i = shaders.begin();
		i != shaders.end(); ++i)
	{
		if((*i)->hasDirLights)
			((LightShader*)(*i))->setDirLights(dirLightOn, 
				dirLightDir, dirIntensity, MaxDirLights);
	}
}

void Scene::updatePointLights()
{
	for(std::set<Shader*>::iterator i = shaders.begin();
		i != shaders.end(); ++i)
	{
		if((*i)->hasPointLights)
			((LightShader*)(*i))->setPointLights(pointLightOn, pointLightPos, 
				pointIntensity, MaxPointLights);
	}
}

void Scene::updateCamera()
{
	for(std::set<Shader*>::iterator i = shaders.begin();
		i != shaders.end(); ++i)
	{
		(*i)->setWorldToCamera(camera->getMat());
	}
}
