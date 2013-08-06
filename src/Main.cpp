#include "Scene.hpp"
#include "Texture.hpp"
#include "Particles.hpp"
#include "Mesh.hpp"
#include "AOMesh.hpp"
#include "SH.hpp"
#include "SHMat.hpp"
#include "SphereFunc.hpp"

#include <glm.hpp>
#include <GL/glut.h>

/* This file will contain the construction and rendering of the scene
 * I am working on right now. 
 */

int init();
void display();
void reshape (int, int);
void keyboard(unsigned char, int, int);

void addSHArray(Scene* scene, glm::vec3 pos, int nBands, float scale, float spacing);

const int nSwirls = 400;
const int nSparks = 5;

float theta = 0.0f;
float phi = 0.0f;

Scene* scene;
SHLight* light;
SpherePlot* plot;

const int k = 5;

int eTime;
int deTime;

const float delta = 0.4f;

int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	eTime = glutGet(GLUT_ELAPSED_TIME);
    glutInitDisplayMode(GLUT_DOUBLE);
    glutInitWindowSize(500, 500);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("API Demo");
    glewInit();
    int good = init();
    if(!good) return 0;
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
	glutKeyboardFunc(keyboard);
    glutMainLoop();

	delete scene;
}

// Called by glutInit().
int init()
{
	glClearColor(1.0, 1.0, 1.0, 1.0);

	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);

	scene = new Scene();

	ParticleShader* pShader = new ParticleShader(true, "ScrollTexFire");
	Texture* flameTex = new Texture("bigFlame.png");
	Texture* decayTex = new Texture("decay2.png");
	AdvectParticlesCentroidLights* centreParticles = 
		new AdvectParticlesCentroidLights(nSwirls, 10, 10, 1000, pShader, flameTex, decayTex);
	centreParticles->translate(glm::vec3(0.0, -1.0, 1.5));
	scene->add(centreParticles);

	AdvectParticlesRandLights* randParticles = new AdvectParticlesRandLights(nSwirls, 10, 2000, pShader, flameTex, decayTex);
	randParticles->translate(glm::vec3(0.0, -1.0, -3.0));
	
	scene->add(randParticles);

	Material greenMat;

	greenMat.ambient = glm::vec4(0.0f, 0.1f, 0.0f, 1.0f);
	greenMat.diffuse = glm::vec4(0.0f, 0.7f, 0.0f, 1.0f);
	greenMat.specular = glm::vec4(0.2f, 0.2f, 0.2f, 1.0f);
	greenMat.exponent = 1.0f;

	AOShader* aoShader = new AOShader(false, "AOSolid");
	LightShader* lightShader = new LightShader(false, "Solid");

	Mesh* bunny = new Mesh("bunny.obj", greenMat, lightShader);
	scene->add(bunny);

	AOMesh* bunnyAO = new AOMesh("bunny.obj", greenMat, aoShader, 10);
	bunnyAO->translate(glm::vec3(1.0f, 0.0f, 0.0f));
	scene->add(bunnyAO);

	light = new SHLight(
		[] (double theta, double phi) -> glm::vec3 
		{
			//float val = 0.2f;
			float val = pulse(theta, phi, glm::vec3(1.0, 0.0, 0.0), 4.0f, 1.0f);

			return glm::vec3(val, val, val);
		}
	);
	scene->add(light);

	return 1;
}

// Perform rendering and updates here.
void display()
{
	deTime = glutGet(GLUT_ELAPSED_TIME) - eTime;
	eTime = glutGet(GLUT_ELAPSED_TIME);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	scene->update(deTime);
	glm::mat4 rotation(1.0f);
	//Look up/down
	rotation = glm::rotate(glm::mat4(1.0), phi, glm::vec3(1.0, 0.0, 0.0));
	//Spin around
	rotation = glm::rotate(rotation,     theta, glm::vec3(0.0, 1.0, 0.0));
	light->rotateCoeffts(rotation);
	scene->render();
	glutSwapBuffers();
	glutPostRedisplay();
}

// Called when window size changes.
void reshape (int w, int h)
{
	float aspect = (float) w / (float) h;
	scene->camera->setAspect(aspect);
	glViewport(0, 0, (GLsizei) w, (GLsizei) h);
}

// Called when a key is pressed.
void keyboard(unsigned char key, int x, int y)
{
	scene->camera->keyboardInput(key, x, y);

    switch (key)
    {
	case 't':
		theta += 1.6f;
		break;
	case 'g':
		theta -= 1.6f;
		break;
	case 'f':
		phi -= 1.6f;
		break;
	case 'h':
		phi += 1.6f;
		break;

    case 27:
        exit(0);
        return;
    }
}
