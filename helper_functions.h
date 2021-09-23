#ifndef HELPER_FUNCTIONS_H
#define HELPER_FUNCTIONS_H

#define GLM_FORCE_RADIANS
#define GLM_FORCE_SWIZZLE

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <fstream>
#include <vector>

#include "lodepng.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

void error_callback(int error, const char* description);
glm::vec3 calcDir(float kat_x, float kat_y);
GLuint readTexture(const char* filename);
GLuint loadCubemap(std::vector<std::string> faces);

class Model {
	public:
		int num_mesh = 0;
		std::vector<glm::vec4> verts;
		std::vector<glm::vec4> norms;
		std::vector<glm::vec2> texCoords;
		std::vector<unsigned int> indices;
			
		void loadModel(std::string plik);
};

#endif
