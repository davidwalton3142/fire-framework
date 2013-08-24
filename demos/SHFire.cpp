#include "Scene.hpp"
#include "Camera.hpp"
#include "Texture.hpp"
#include "Particles.hpp"
#include "PRTMesh.hpp"
#include "SpherePlot.hpp"

#include <glm.hpp>
#include <GL/glut.h>
#include <gtc/matrix_transform.hpp>

#include <fstream>

/* Phong Fire Demo
 * Demo displays a textured mesh illuminated by a fire using Phong lighting
 * with centroid lights.
 */

int init();
void display();
void reshape (int, int);
void keyboard(unsigned char, int, int);

ParticleShader* tShader; // Scrolling texture shader.
ParticleShader* pShader; // Procedural texture shader.
ParticleShader* sShader; // Spark shader.

AdvectParticlesCentroidSHLights* flame;
AdvectParticles*                 sparks;
AdvectParticles*                 smoke;

SpherePlot* plot;

PRTMesh* bunny;

Scene* scene;
SHLight* light;

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
	const float flameIntensity = 2.0f;
	const int nFlameLights = 5;
	const int lightClumpSize = 4;
	const int hopInterval = -1; // Never hop. Set to +ve ms value to hop.

	/* Spark Properties */
	const int nSparkParticles = 5;
	const int sparkLifetime = 2000;
	const int sparkVarLifetime = 200;
	const glm::vec4 sparkInitAcn(0.0f, -0.0000004f, 0.0f, 0.0f);
	const glm::vec4 sparkInitVel(0.0f, 0.0008f, 0.0f, 0.0f);
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

	/* Edit mesh & texture filenames and PRT type here. */
	/* mode should be UNSHADOWED, SHADOWED or INTERREFLECTED */
	const std::string filename = "stanford.obj";
	const std::string diffTexture = "stanford.png";
	const PRTMode mode = SHADOWED;

	/* Check if baked file exists. If not, make one. */
	const std::string bakedFilename = filename + ".prt" + 
		(mode == UNSHADOWED ? "u" : mode == SHADOWED ? "s" : "i")
		+ "5";
	std::ifstream temp(bakedFilename);
	if(!temp)
		PRTMesh::bake(mode, filename, diffTexture, 40, 5, 1);

	SHShader* bunnyShader = new SHShader(false, "diffPRT");

	bunny = new PRTMesh(
		bakedFilename,bunnyShader);

	bunny->uniformScale(0.2f);

	scene->add(bunny);

	pShader = new ParticleShader(true, false, "ProceduralFire");
	tShader = new ParticleShader(true, true , "ScrollTexFire" );
	sShader = new ParticleShader(true, true , "Sparks");

	Texture* flameAlphaTex = new Texture("flameAlpha.png");
	Texture* flameDecayTex = new Texture("flameDecay.png");

	Texture* sparkAlphaTex = new Texture("sparkAlpha.png");
	Texture* sparkDecayTex = new Texture("sparkDecay.png");

	Texture* smokeAlphaTex = new Texture("smokeAlpha.png");
	Texture* smokeDecayTex = new Texture("smokeDecay.png");

	flame = new AdvectParticlesCentroidSHLights(
		bunny, flameIntensity,
		nFlameParticles, nFlameLights, lightClumpSize, hopInterval,
		pShader, flameAlphaTex, flameDecayTex);

	sparks = new AdvectParticles(
		nSparkParticles, sShader, sparkAlphaTex, sparkDecayTex,
		sparkLifetime, sparkVarLifetime, 
		sparkInitAcn, sparkInitVel,
		avgSparkPerturb, varSparkPerturb, sparkPerturbRadius,
		sparkBaseRadius, sparkCenterForce,
		sparkBBHeight, sparkBBWidth,
		true, true, false);

	smoke = new AdvectParticles(
		nSmokeParticles, pShader, smokeAlphaTex, smokeDecayTex, true, false);

	flame->translate(glm::vec3(0.0f, 0.0f, 1.0f));
	sparks->translate(glm::vec3(0.0f, 0.0f, 1.0f));
	smoke->translate(glm::vec3(0.0f, 1.0f, 1.0f));

	scene->add(flame);
	scene->add(sparks);
	scene->add(smoke);

	Shader* plotShader = new Shader(false, "SpherePlot");

	plot = new SpherePlot(
		[] (float theta, float phi) -> 
		float {
			std::vector<glm::vec3> allLights(GC::nSHCoeffts);
			std::fill(allLights.begin(), allLights.end(), glm::vec3(0.0f));
			for(size_t l = 0; l < flame->lights.size(); ++l)
				for(size_t c = 0; c < GC::nSHCoeffts; ++c)
					allLights[c] += flame->lights[l]->getCoeffts()[c];
			return SH::evaluate(allLights, theta, phi).x / flame->getIntensity();
			},
		40, plotShader);

	plot->translate(glm::vec3(-1.0f, 0.0f, 0.0f));

	scene->add(plot);

	plot->uniformScale(0.2f);

	return 1;
}

// Perform rendering and updates here.
void display()
{
	deTime = glutGet(GLUT_ELAPSED_TIME) - eTime;
	eTime = glutGet(GLUT_ELAPSED_TIME);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	scene->update(deTime);

	plot->replot(
		[] (float theta, float phi) -> 
		float {
			std::vector<glm::vec3> allLights(GC::nSHCoeffts);
			std::fill(allLights.begin(), allLights.end(), glm::vec3(0.0f));
			for(size_t l = 0; l < flame->lights.size(); ++l)
				for(size_t c = 0; c < GC::nSHCoeffts; ++c)
					allLights[c] += flame->lights[l]->getCoeffts()[c];
			return SH::evaluate(allLights, theta, phi).x / flame->getIntensity();
			},
			40);

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
    case 'f':
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

    case 27:
        exit(0);
        return;
    }
}
