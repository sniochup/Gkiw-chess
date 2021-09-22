#define GLM_FORCE_RADIANS
#define GLM_FORCE_SWIZZLE

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include "constants.h"
#include "lodepng.h"
#include "shaderprogram.h"
#include "myCube.h"

#include "chess.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

ShaderProgram* spSkybox;
ShaderProgram* spBoard;
ShaderProgram* spPiece;

bool n_bool = false;

GLuint texBoard[8];

int num_mesh = 0;
std::vector<glm::vec4> verts[60];
std::vector<glm::vec4> norms[60];
std::vector<glm::vec2> texCoords[60];
std::vector<unsigned int> indices[60];

float speed_x = 0;//[radiany/s]
float speed_y = 0;//[radiany/s]
float speed_x_1 = 0;//[radiany/s]
float speed_y_1 = 0;//[radiany/s]
float aspectRatio = 1;
float walk_speed = 0;

glm::vec3 pos = glm::vec3(0, 0, -5);

glm::vec3 calcDir(float kat_x, float kat_y) {
	glm::vec4 dir = glm::vec4(0, 0, 1, 0);
	glm::mat4 M = glm::rotate(glm::mat4(1.0f), kat_y, glm::vec3(0, 1, 0));
	M = glm::rotate(M, kat_x, glm::vec3(1, 0, 0));
	dir = M * dir;
	return glm::vec3(dir);
}

GLuint readTexture(const char* filename) {
	GLuint tex;
	glActiveTexture(GL_TEXTURE0);

	//Wczytanie do pamięci komputera
	std::vector<unsigned char> image;   //Alokuj wektor do wczytania obrazka
	unsigned width, height;   //Zmienne do których wczytamy wymiary obrazka
	unsigned error = lodepng::decode(image, width, height, filename); //Wczytaj obrazek

	//Import do pamięci karty graficznej
	glGenTextures(1, &tex); //Zainicjuj jeden uchwyt
	glBindTexture(GL_TEXTURE_2D, tex); //Uaktywnij uchwyt

	//Wczytaj obrazek do pamięci KG skojarzonej z uchwytem
	glTexImage2D(GL_TEXTURE_2D, 0, 4, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, (unsigned char*)image.data());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	return tex;
}

//Procedura obsługi błędów
void error_callback(int error, const char* description) {
	fputs(description, stderr);
}

void keyCallback(
	GLFWwindow* window,
	int key,
	int scancode,
	int action,
	int mod
) {
	if (action == GLFW_PRESS) {
		if (key == GLFW_KEY_LEFT) speed_y = 1;
		if (key == GLFW_KEY_RIGHT) speed_y = -1;
		if (key == GLFW_KEY_PAGE_UP) speed_x = 1;
		if (key == GLFW_KEY_PAGE_DOWN) speed_x = -1;
		if (key == GLFW_KEY_UP) walk_speed = 10;
		if (key == GLFW_KEY_DOWN) walk_speed = -10;
		if (key == GLFW_KEY_W) speed_x_1 = PI;
		if (key == GLFW_KEY_S) speed_y_1 = -PI;
		if (key == GLFW_KEY_N) n_bool = true;

	}
	if (action == GLFW_RELEASE) {
		if (key == GLFW_KEY_LEFT) speed_y = 0;
		if (key == GLFW_KEY_RIGHT) speed_y = 0;
		if (key == GLFW_KEY_PAGE_UP) speed_x = 0;
		if (key == GLFW_KEY_PAGE_DOWN) speed_x = 0;
		if (key == GLFW_KEY_UP) walk_speed = 0;
		if (key == GLFW_KEY_DOWN) walk_speed = 0;
		if (key == GLFW_KEY_W) speed_x_1 = 0;
		if (key == GLFW_KEY_S) speed_y_1 = 0;
	}
}

void windowResizeCallback(GLFWwindow* window, int width, int height) {
	if (height == 0) return;
	aspectRatio = (float)width / (float)height;
	glViewport(0, 0, width, height);
}

unsigned int loadCubemap(std::vector<std::string> faces) {
	unsigned int textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	unsigned width, height;
	for (unsigned int i = 0; i < faces.size(); i++)
	{
		std::vector<unsigned char> image;
		unsigned error = lodepng::decode(image, width, height, faces[i].c_str());
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, &image);
		
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	return textureID;
}

std::vector<std::string> faces {
		"textures\\skybox\\right.png",
		"textures\\skybox\\left.png",
		"textures\\skybox\\top.png",
		"textures\\skybox\\bottom.png",
		"textures\\skybox\\front.png",
		"textures\\skybox\\back.png"
};
unsigned int cubemapTexture = loadCubemap(faces);

void loadModel(std::string plik) {

    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(plik, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenSmoothNormals);
    std::cout << importer.GetErrorString() << std::endl;

    std::cout << scene->mNumMeshes << std::endl;
    num_mesh = scene->mNumMeshes;
    //aiMesh* mesh = scene->mMeshes[51];

    //aiMesh* mesh[60];
    //aiMaterial* material[60];

    if (scene->HasMeshes()) {
        for (int it = 0; it < scene->mNumMeshes; it++) {

            aiMesh* mesh = scene->mMeshes[it];

            for (int i = 0; i < mesh->mNumVertices; i++) {
                aiVector3D vertex = mesh->mVertices[i];
                verts[it].push_back(glm::vec4(vertex.x, vertex.y, vertex.z, 1));
                aiVector3D normal = mesh->mNormals[i];
                norms[it].push_back(glm::vec4(normal.x, normal.y, normal.z, 1));
                //aiVector3D texCoord = mesh->mTextureCoords[0][i];
                //texCoords[it].push_back(glm::vec2(texCoord.x, texCoord.y));
				//std::cout << vertex.x << " " << vertex.y << " " << vertex.z << "  ->  " << normal.x << " " << normal.y << " " << normal.z << std::endl;
            }

            for (int i = 0; i < mesh->mNumFaces; i++) {
                aiFace& face = mesh->mFaces[i];
                for (int j = 0; j < face.mNumIndices; j++) {
                    indices[it].push_back(face.mIndices[j]);
                }
            }

            aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

            /*for (int i = 0; i < 19; i++) {
                std::cout << i << " " << material->GetTextureCount((aiTextureType)i) << std::endl;
            }*/

            for (int i = 0; i < material->GetTextureCount(aiTextureType_DIFFUSE); i++) {
                aiString str;

                material->GetTexture(aiTextureType_DIFFUSE, i, &str);
                std::cout << str.C_Str() << std::endl;
            }
        }
    }
}


//Procedura inicjująca
void initOpenGLProgram(GLFWwindow* window) {
    initShaders();

	glClearColor(1,1,1,1); //Ustaw kolor czyszczenia bufora kolorów
	glEnable(GL_DEPTH_TEST); //Włącz test głębokości na pikselach
	glfwSetWindowSizeCallback(window, windowResizeCallback);
	glfwSetKeyCallback(window, keyCallback);

	spSkybox = new ShaderProgram("v_skybox.glsl", NULL, "f_skybox.glsl");
	spBoard = new ShaderProgram("v_board.glsl", NULL, "f_board.glsl");
	spPiece = new ShaderProgram("v_piece.glsl", NULL, "f_piece.glsl");
	
	texBoard[0] = readTexture("textures\\board\\512-diffuse.png");
	texBoard[1] = readTexture("textures\\board\\512-normal.png");
	texBoard[2] = readTexture("textures\\board\\512-bump.png");
	texBoard[3] = readTexture("textures\\board\\512-spec.png");
	texBoard[4] = readTexture("textures\\board_concrete\\Concrete_Wall_008_basecolor.png");
	texBoard[5] = readTexture("textures\\board_concrete\\Concrete_Wall_008_normal.png");
	texBoard[6] = readTexture("textures\\board_concrete\\Concrete_Wall_008_height.png");
	texBoard[7] = readTexture("textures\\board_concrete\\Concrete_Wall_008_roughness.png");

	loadModel(std::string("models\\King.fbx"));
}


//Zwolnienie zasobów zajętych przez program
void freeOpenGLProgram(GLFWwindow* window) {
    freeShaders();
	delete spSkybox;
	delete spBoard;
	delete spPiece;

	glDeleteTextures(8, &texBoard[0]);
}

//Procedura rysująca zawartość sceny
void drawScene(GLFWwindow* window,float angle_x,float angle_y, float angle_x_1, float angle_y_1) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //Wyczyść bufor koloru i bufor głębokości
	
	bool draw_skybox = true;
	bool draw_board = false;
	bool draw_piece = false;

	glm::vec4 lp = glm::vec4(0, 0, -10, 1); // Położenie źródła światła

	glm::mat4 V = glm::lookAt(pos, pos + calcDir(angle_x_1, angle_y_1), glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 P = glm::perspective(glm::radians(50.0f), aspectRatio, 0.01f, 50.0f);

	glm::mat4 M = glm::mat4(1.0f); //Zainicjuj macierz modelu macierzą jednostkową
	M = glm::rotate(M, angle_y, glm::vec3(0.0f, 1.0f, 0.0f)); //Pomnóż macierz modelu razy macierz obrotu o kąt angle wokół osi Y
	M = glm::rotate(M, angle_x, glm::vec3(1.0f, 0.0f, 0.0f)); //Pomnóż macierz modelu razy macierz obrotu o kąt angle wokół osi X
	
	if (draw_skybox) {
		spSkybox->use();
		glUniformMatrix4fv(spSkybox->u("projection"), 1, false, glm::value_ptr(P));
		glUniformMatrix4fv(spSkybox->u("view"), 1, false, glm::value_ptr(V));
		//glUniformMatrix4fv(spSkybox->u("model"), 1, false, glm::value_ptr(M));
		//glUniformMatrix3fv(spSkybox->u("aPos"), 1, false, glm::value_ptr(pos));
		//glEnableVertexAttribArray(spSkybox->a("skybox"));
		//glVertexAttribPointer(spSkybox->a("skybox"), 3, GL_FLOAT, false, 0, skyboxVertices);
		//glBindVertexArray(*skyboxVertices);
		glDepthFunc(GL_LEQUAL);

		unsigned int skyboxVAO, skyboxVBO;
		glGenVertexArrays(1, &skyboxVAO);
		glGenBuffers(1, &skyboxVBO);
		glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

		glBindVertexArray(skyboxVAO);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);
		glDepthFunc(GL_LESS);
				
	}

	if (draw_board) {
		spBoard->use();
		glUniformMatrix4fv(spBoard->u("P"), 1, false, glm::value_ptr(P));
		glUniformMatrix4fv(spBoard->u("V"), 1, false, glm::value_ptr(V));
		glm::mat4 Plansza = glm::scale(M, glm::vec3(1, 0.3, 1));
		glUniformMatrix4fv(spBoard->u("M"), 1, false, glm::value_ptr(Plansza));
		glUniform4fv(spBoard->u("lp"), 1, glm::value_ptr(lp));

		glEnableVertexAttribArray(spBoard->a("vertex"));
		glVertexAttribPointer(spBoard->a("vertex"), 4, GL_FLOAT, false, 0, myCubeVertices);

		glEnableVertexAttribArray(spBoard->a("c1"));
		glVertexAttribPointer(spBoard->a("c1"), 4, GL_FLOAT, false, 0, myCubeC1);

		glEnableVertexAttribArray(spBoard->a("c2"));
		glVertexAttribPointer(spBoard->a("c2"), 4, GL_FLOAT, false, 0, myCubeC2);

		glEnableVertexAttribArray(spBoard->a("c3"));
		glVertexAttribPointer(spBoard->a("c3"), 4, GL_FLOAT, false, 0, myCubeC3);

		glEnableVertexAttribArray(spBoard->a("texCoord0"));
		glVertexAttribPointer(spBoard->a("texCoord0"), 2, GL_FLOAT, false, 0, myCubeTexCoords);

		glUniform1i(spBoard->u("textureMap0"), 0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texBoard[0]);

		glUniform1i(spBoard->u("textureMap1"), 1);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, texBoard[1]);

		glUniform1i(spBoard->u("textureMap2"), 2);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, texBoard[2]);

		glUniform1i(spBoard->u("textureMap3"), 3);
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, texBoard[3]);

		glDrawArrays(GL_TRIANGLES, 0, myCubeVertexCount);
		
		Plansza = glm::scale(Plansza, glm::vec3(1.3, 1, 1.3));
		Plansza = glm::translate(Plansza, glm::vec3(0, -0.2, 0));
		glUniformMatrix4fv(spBoard->u("M"), 1, false, glm::value_ptr(Plansza));

		glUniform1i(spBoard->u("textureMap0"), 0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texBoard[4]);

		glUniform1i(spBoard->u("textureMap1"), 1);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, texBoard[5]);

		glUniform1i(spBoard->u("textureMap2"), 2);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, texBoard[6]);

		glUniform1i(spBoard->u("textureMap3"), 3);
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, texBoard[7]);

		glDrawArrays(GL_TRIANGLES, 0, myCubeVertexCount);

		glDisableVertexAttribArray(spBoard->a("vertex"));
		glDisableVertexAttribArray(spBoard->a("c1"));
		glDisableVertexAttribArray(spBoard->a("c2"));
		glDisableVertexAttribArray(spBoard->a("c3"));
		glDisableVertexAttribArray(spBoard->a("texCoord0"));
	}
	
	if (draw_piece) {
		for (int i = 0; i < 1; i++) {
			spPiece->use();
			glUniformMatrix4fv(spPiece->u("P"), 1, false, glm::value_ptr(P));
			glUniformMatrix4fv(spPiece->u("V"), 1, false, glm::value_ptr(V));
			glUniformMatrix4fv(spPiece->u("M"), 1, false, glm::value_ptr(M));
			glUniform4fv(spPiece->u("lp"), 1, glm::value_ptr(lp));

			glEnableVertexAttribArray(spPiece->a("vertex"));
			glVertexAttribPointer(spPiece->a("vertex"), 4, GL_FLOAT, false, 0, verts[i].data());

			glEnableVertexAttribArray(spPiece->a("normal"));
			glVertexAttribPointer(spPiece->a("normal"), 4, GL_FLOAT, false, 0, norms[i].data());

			//glEnableVertexAttribArray(spPiece->a("color"));
			//glVertexAttribPointer(spPiece->a("color"), 4, GL_FLOAT, false, 0, );

			glDrawElements(GL_TRIANGLES, indices[i].size(), GL_UNSIGNED_INT, indices[i].data());

			glDisableVertexAttribArray(spPiece->a("vertex"));
			glDisableVertexAttribArray(spPiece->a("normal"));
			//glDisableVertexAttribArray(spPiece->a("color"));
		}
	}

	glfwSwapBuffers(window); //Skopiuj bufor tylny do bufora przedniego
}


int main(void)
{

	GLFWwindow* window; //Wskaźnik na obiekt reprezentujący okno

	glfwSetErrorCallback(error_callback);//Zarejestruj procedurę obsługi błędów

	if (!glfwInit()) { //Zainicjuj bibliotekę GLFW
		fprintf(stderr, "Nie można zainicjować GLFW.\n");
		exit(EXIT_FAILURE);
	}

	window = glfwCreateWindow(500, 500, "OpenGL", NULL, NULL);  //Utwórz okno 500x500 o tytule "OpenGL" i kontekst OpenGL.

	if (!window) //Jeżeli okna nie udało się utworzyć, to zamknij program
	{
		fprintf(stderr, "Nie można utworzyć okna.\n");
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	glfwMakeContextCurrent(window); //Od tego momentu kontekst okna staje się aktywny i polecenia OpenGL będą dotyczyć właśnie jego.
	glfwSwapInterval(1); //Czekaj na 1 powrót plamki przed pokazaniem ukrytego bufora

	if (glewInit() != GLEW_OK) { //Zainicjuj bibliotekę GLEW
		fprintf(stderr, "Nie można zainicjować GLEW.\n");
		exit(EXIT_FAILURE);
	}

	initOpenGLProgram(window); //Operacje inicjujące

	//Główna pętla
	float angle_x = 0; //zadeklaruj zmienną przechowującą aktualny kąt obrotu
	float angle_y = 0; //zadeklaruj zmienną przechowującą aktualny kąt obrotu
	float angle_x_1 = 0;
	float angle_y_1 = 0;
	glfwSetTime(0); //Wyzeruj licznik czasu

	Board board;
	board.printBoard();
	int num_move = 0;

	while (!glfwWindowShouldClose(window)) //Tak długo jak okno nie powinno zostać zamknięte
	{
		if (n_bool) {
			board.move("move.txt", num_move++);
			board.printBoard();
			n_bool = false;
		}

		angle_x_1 += speed_x * glfwGetTime();
		angle_y_1 += speed_y * glfwGetTime();

		angle_x += speed_x_1 * glfwGetTime(); //Oblicz kąt o jaki obiekt obrócił się podczas poprzedniej klatki
		angle_y += speed_y_1 * glfwGetTime(); //Oblicz kąt o jaki obiekt obrócił się podczas poprzedniej klatki
		pos += (float)(walk_speed * glfwGetTime()) * calcDir(angle_x_1, angle_y_1);

		glfwSetTime(0); //Wyzeruj licznik czasu
		drawScene(window, angle_x, angle_y, angle_x_1, angle_y_1); //Wykonaj procedurę rysującą
		glfwPollEvents(); //Wykonaj procedury callback w zalezności od zdarzeń jakie zaszły.
	}

	freeOpenGLProgram(window);

	glfwDestroyWindow(window); //Usuń kontekst OpenGL i okno
	glfwTerminate(); //Zwolnij zasoby zajęte przez GLFW
	exit(EXIT_SUCCESS);
}
