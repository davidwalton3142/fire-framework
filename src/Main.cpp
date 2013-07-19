#include "Scene.hpp"
#include "SH.hpp"
#include "SHMat.hpp"

#include <glm.hpp>

/* This file will contain the construction and rendering of the scene
 * I am working on right now. 
 */

int init();
void display();
void reshape (int, int);
void keyboard(unsigned char, int, int);

const int nSwirls = 400;
const int nSparks = 5;

Scene* scene;

LightShader* lShader;
ParticleShader* pShader;
AdvectParticlesRandLights<nSwirls>* swirl;
AdvectParticles<nSwirls>* swirl2;
AdvectParticles<nSwirls>* swirl3;
AdvectParticles<nSparks>* sparks;
DirLight* d;
PointLight* p;
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
	glClearColor(0.0, 0.0, 0.0, 1.0);
    glutCreateWindow("API Demo");
    glewInit();
    int good = init();
    if(!good) return 0;
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
	glutKeyboardFunc(keyboard);
    glutMainLoop();

	delete lShader;
	delete pShader;
	delete scene;
}

// Called by glutInit().
int init()
{
	std::vector<float> c = SH::shProject(10, 3, [] (double x, double y) -> double {return SH::realSH(1,1,x,y) + SH::realSH(1,-1,x,y) + SH::realSH(2,1,x,y) + SH::realSH(2,-1,x,y)+ SH::realSH(2,2,x,y) + SH::realSH(2,-2,x,y) ;});
	for(std::vector<float>::iterator i = c.begin(); i != c.end(); ++i)
		std::cout << (*i) << "\n";
	SHMat rot(glm::rotate(glm::mat4(1.0f), 0.0f, glm::vec3(1.0f, 0.0f, 0.0f)), 3);
	c = rot * c;
	std::cout << "Rotated:\n";
	for(std::vector<float>::iterator i = c.begin(); i != c.end(); ++i)
		std::cout << (*i) << "\n";
	glEnable(GL_CULL_FACE);
	glEnable(GL_BLEND); 
	glEnable(GL_DEPTH_TEST);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	scene = new Scene();

	pShader = new ParticleShader(true, "ScrollTexFire");
	Texture* flameTex = new Texture("bigFlame.png");
	Texture* decayTex = new Texture("decay2.png");
	AdvectParticlesCentroidLights<nSwirls>* centreParticles = new AdvectParticlesCentroidLights<nSwirls>(10, 10, 1000, pShader, flameTex, decayTex);
	centreParticles->translate(glm::vec3(0.0, -1.0, 3.0));
	scene->add(centreParticles);

	swirl = new AdvectParticlesRandLights<nSwirls>(10, 2000, pShader, flameTex, decayTex);
	swirl->translate(glm::vec3(0.0, -1.0, -3.0));
	scene->add(swirl);

	lShader = new LightShader(false, "Solid", true, true, true);
	/*
	ArrSolid<36>* cube;
	for(int i = -k; i <= k; ++i)
		for(int j = -k; j <= k; ++j)
		{
			cube = Solid::Cube(lShader);
			cube->translate(glm::vec3(3.0*i, 3.0*j, 0.0));
			scene->add(cube);
		}
	*/

	ArrSolid<36>* cube = Solid::Cube(lShader);
	cube->translate(glm::vec3(2.0f, 0.0f, 0.0f));
	scene->add(cube);

	std::vector<Mesh*> loaded = Mesh::loadFile("teapot.obj", lShader);

	for(std::vector<Mesh*>::iterator i = loaded.begin();
		i != loaded.end(); ++i)
	{
		(*i)->uniformScale(0.08f);
		scene->add(*i);
		std::cout << "Adding a mesh to scene.\n";
	}

	loaded = Mesh::loadFile("Rabbit.obj", lShader);

	for(std::vector<Mesh*>::iterator i = loaded.begin();
		i != loaded.end(); ++i)
	{
		(*i)->uniformScale(2.0f);
		(*i)->translate(glm::vec3(-1.0, -0.8, 0.0));
		scene->add(*i);
		std::cout << "Adding a mesh to scene.\n";
	}

	scene->setAmbLight(0.1f);
	return 1;
}

// Perform rendering and updates here.
void display()
{
	deTime = glutGet(GLUT_ELAPSED_TIME) - eTime;
	eTime = glutGet(GLUT_ELAPSED_TIME);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	scene->update(deTime);
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
			d->on = !(d->on);
			scene->updateLight(d);
			break;
		case 'y':
			p->on = !(p->on);
			scene->updateLight(p);
			break;
      	case 27:
            exit(0);
            return;
    }
}
