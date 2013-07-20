#ifndef SCENE_H
#define SCENE_H

#include "Renderable.hpp"
#include "Particles.hpp"
#include "Light.hpp"
#include "Shader.hpp"
#include "Camera.hpp"
#include "Mesh.hpp"

#include <glm.hpp>
#include <glut.h>
#include <SOIL.h>

#include <vector>
#include <set>

class DirLight;
class PointLight;

const float PI = 3.141592653589793238462f;

/* Scene
 * The Scene object handles all Element objects added to it, and renders them appropriately.
 * Destroying a Scene also calls the destructors of added objects.
 */
class Scene
{
public:
	Scene();
	~Scene();
	/* render() renders all renderables added to the scene.
	 * Opaque objects are rendered first, transparent second.
	 * Within these categories, renderables are rendered in the order added.
	 */
	void render();
	void update(int dTime);

	/* add() and remove() functions return a pointer to the element added/removed.
	 * e.g. Renderable* p = scene.add(new AdvectParticles(s, t1, t2));
	 * If the element could not be added/removed (e.g. maxPointLights is exceeded),
	 * nullptr is returned.
	 */
	Renderable* add(Renderable* r);
	Renderable* remove(Renderable* r);

	PhongLight* add(PhongLight* l);
	PhongLight* updateLight(PhongLight* l);
	PhongLight* remove(PhongLight* l);

	void setAmbLight(glm::vec4 _ambLight);
	
	static const int maxPhongLights = 50;
	
	static const int maxSHLights = 10;
	static const int nSHBands = 5;
	static const int sqrtSHSamples = 100;

	Camera* camera;
private:
	glm::vec4 ambLight;
	GLuint ambLight_u;

	std::set<Renderable*> opaque;
	std::set<Renderable*> translucent;

	std::set<Shader*> shaders;

	int nPhongLights;
	PhongLight* phongLights[maxPhongLights];
	glm::vec4 lightPos[maxPhongLights];
	glm::vec4 lightDiffuse[maxPhongLights];
	glm::vec4 lightSpecular[maxPhongLights];
	float lightAttenuation[maxPhongLights];
	void updatePhongLights();

	void updateCamera();
};

#endif
