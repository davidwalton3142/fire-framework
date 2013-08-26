#include "Scene.hpp"
#include "Camera.hpp"
#include "Texture.hpp"
#include "Particles.hpp"
#include "Mesh.hpp"

#include <glm.hpp>
#include <GL/glut.h>
#include <gtc/matrix_transform.hpp>

/* Phong Fire Demo
 * Demo displays a textured mesh illuminated by a fire, with and
 *   without AO. 
 */

int init();
void display();
void reshape (int, int);
void keyboard(unsigned char, int, int);

void rotateRenderable(Renderable*, float, float);
void rotateRenderables();

ParticleShader* tShader; // Scrolling texture shader.
ParticleShader* pShader; // Procedural texture shader.
ParticleShader* sShader; // Spark shader.

AdvectParticlesCentroidLights* flame;
AdvectParticles*               sparks;
AdvectParticles*               smoke;

Mesh* bunny;
Mesh* bunnyAO;

Scene* scene;
SHLight* light;

const int k = 5;

int eTime;
int deTime;

const float delta = 0.4f;

float theta = 0.0f;
float phi = 0.0f;
float rotateDelta = 1.6f;

int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	eTime = glutGet(GLUT_ELAPSED_TIME);
    glutInitDisplayMode(GLUT_DOUBLE);
    glutInitWindowSize(500, 500);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Fire and Phong Lighting Demo");
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
	/* Ambient Light Properties */
	const float ambIntensity = 0.5f;
	const glm::vec4 ambColor(0.7f, 0.7f, 0.9f, 1.0f); //light blue

	/* Flame Properties */
	const int nFlameParticles = 400;
	const int nFlameLights = 10;
	const int lightClumpSize = 10;
	const int hopInterval = -1; // Never hop. Set to +ve ms value to hop.
	const float flameLightIntensity = 0.0018f;

	/* Spark Properties */
	const int nSparkParticles = 5;
	const int sparkLifetime = 2000;
	const int sparkVarLifetime = 200;
	const glm::vec4 sparkInitAcn(0.0f, -0.0000004f, 0.0f, 0.0f);
	const float sparkInitVel = 0.0f;
	const float sparkInitUpVel = 0.0008f;
	const int avgSparkPerturb = 1000;
	const int varSparkPerturb = 100;
	const float sparkPerturbRadius = 0.0004f;
	const float sparkCenterForce = 0.0f;
	const float sparkBaseRadius = 0.2f;
	const float sparkBBHeight = 0.03f;
	const float sparkBBWidth = 0.03f;

	/* Smoke Properties */
	const int nSmokeParticles = 20;

	/* Bunny Properties */
	const float bunnySpecExp = 1.0f;

	glClearColor(ambColor.x, ambColor.y, ambColor.z, ambColor.w);

	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);

	scene = new Scene();
	scene->setAmbLight(ambColor * ambIntensity);

	pShader = new ParticleShader(true, false, "ProceduralFire");
	tShader = new ParticleShader(true, true , "ScrollTexFire" );
	sShader = new ParticleShader(true, true , "Sparks");

	Texture* flameAlphaTex = new Texture("flameAlpha.png");
	Texture* flameDecayTex = new Texture("flameDecay.png");

	Texture* sparkAlphaTex = new Texture("sparkAlpha.png");
	Texture* sparkDecayTex = new Texture("sparkDecay.png");

	Texture* smokeAlphaTex = new Texture("smokeAlpha.png");
	Texture* smokeDecayTex = new Texture("smokeDecay.png");

	flame = new AdvectParticlesCentroidLights(
		nFlameParticles, nFlameLights, lightClumpSize, hopInterval,
		pShader, flameAlphaTex, flameDecayTex);

	flame->setLightIntensity(flameLightIntensity);

	sparks = new AdvectParticles(
		nSparkParticles, sShader, sparkAlphaTex, sparkDecayTex, 
		false, true);

	sparks->avgLifetime = sparkLifetime;
	sparks->varLifetime = sparkVarLifetime;
	sparks->initAcn = sparkInitAcn;
	sparks->initVel = sparkInitVel;
	sparks->initUpVel = sparkInitUpVel;
	sparks->avgPerturbTime = avgSparkPerturb;
	sparks->varPerturbTime = varSparkPerturb;
	sparks->baseRadius = sparkBaseRadius;
	sparks->bbHeight = sparkBBHeight;
	sparks->bbWidth = sparkBBWidth;

	smoke = new AdvectParticles(
		nSmokeParticles, pShader, smokeAlphaTex, smokeDecayTex, true, false);

	flame->translate(glm::vec3(0.0f, 0.0f, 1.0f));
	sparks->translate(glm::vec3(0.0f, 0.0f, 1.0f));
	smoke->translate(glm::vec3(0.0f, 1.0f, 1.0f));

	scene->add(flame);
	scene->add(sparks);
	scene->add(smoke);

	LightShader* bunnyShader = new LightShader(false, "BlinnPhong");

	Texture* bunnyDiffTex = new Texture("stanfordDiff.png");
	Texture* bunnyAOTex = new Texture("stanfordAO.png");
	Texture* bunnySpecTex = new Texture("stanfordSpec.png");

	bunny = new Mesh(
		"stanford.obj",
		bunnyDiffTex, bunnyDiffTex, bunnySpecTex,
		bunnySpecExp, bunnyShader);

	bunnyAO = new Mesh(
		"stanford.obj",
		bunnyAOTex, bunnyAOTex, bunnySpecTex,
		bunnySpecExp, bunnyShader);

	bunny->uniformScale(0.2f);
	bunnyAO->uniformScale(0.2f);

	bunny->translate(glm::vec3(-0.5f, 0.0f, 0.0f));
	bunnyAO->translate(glm::vec3(0.5f, 0.0f, 0.0f));

	scene->add(bunny);
	scene->add(bunnyAO);

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
    case 'p':
    	//Switch fire mode.
    	if(flame->getShader() == tShader)
    	{
    		flame->setShader(pShader);
    		smoke->setShader(pShader);
    	}
    	else
    	{
    		flame->setShader(tShader);
    		smoke->setShader(tShader);
    	}
    	break;

	case 't':
		phi -= rotateDelta;
		rotateRenderables();
		break;
	case 'g':
		phi += rotateDelta;
		rotateRenderables();
		break;
	case 'f':
		theta += rotateDelta; 
		rotateRenderables();
		break;
	case 'h':
		theta -= rotateDelta;
		rotateRenderables();
		break;
    case 27:
        exit(0);
        return;
    }
}

void rotateRenderable(Renderable* renderable, float theta, float phi)
{
	glm::mat4 rotation(1.0f);
	rotation = glm::rotate(rotation, phi, glm::vec3(1.0, 0.0, 0.0));
	rotation = glm::rotate(rotation, theta, glm::vec3(0.0, 1.0, 0.0));
	renderable->setRotation(rotation);
}

void rotateRenderables()
{
	rotateRenderable(bunny, theta, phi);
	rotateRenderable(bunnyAO, theta, phi);
}
