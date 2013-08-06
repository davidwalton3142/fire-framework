#ifndef PRTMESH_HPP
#define PRTMESH_HPP

#include <vector>
#include <string>

#include <omp.h>

#include <glm.hpp>

#include "Mesh.hpp"
#include "Scene.hpp"
#include "Intersect.hpp"
#include "SH.hpp"

enum PRTMode : char {UNSHADOWED, SHADOWED, INTERREFLECTED, NONE};

class PRTMesh : public Renderable
{
public:
	PRTMesh(
		const std::string& filename,
		const Material& material,
		PRTMode mode, int sqrtNSamples,
		int nBands,
		Shader* shader,
		int nBounces = 3);

	PRTMesh(
		const std::vector<std::string>& filenames,
		const std::vector<Material>& materials,
		PRTMode mode, int sqrtNSamples,
		int nBands,
		Shader* shader,
		int nBounces = 3);

	void render();
	void update(int dTime) {};
	Shader* getShader() {return shader;};
private:
	std::string genPrebakedFilename(
		const std::string& filename,
		PRTMode mode,
		int nBands
		);

	void writePrebakedFile(
		const std::vector<glm::vec4>& verts,
		const std::vector<GLushort>& elems,
	 	const std::vector<std::vector<glm::vec3>>& transfer,
	 	const std::string& filename);

	void readPrebakedFile(
		std::vector<glm::vec4>& verts,
		std::vector<GLushort>& elems,
	 	std::vector<std::vector<glm::vec3>>& transfer,
	 	int nCoeffts,
	 	const std::string& filename);

	void init();

	void bake(const MeshData& data,
		PRTMode mode, int nBands, int sqrtNSamples,
		std::vector<glm::vec4>& verts,
		std::vector<std::vector<glm::vec3>>& transfer,
		int nBounces);

	void interreflect(
		const MeshData& data,
		int nBands, int sqrtNSamples, int nBounces,
		const std::vector<glm::vec4>& verts,
		std::vector<std::vector<glm::vec3>>& transfer);

	Shader* shader;

	std::vector<glm::vec4> verts;
	std::vector<GLushort>  elems;

	std::vector<std::vector<glm::vec3>> transfer;
	std::vector<glm::vec3> colors;

	GLuint verts_vbo;
	GLuint elems_vbo;
	GLuint colors_vbo;

	GLuint vert_attrib;
	GLuint color_attrib;
};

#endif
