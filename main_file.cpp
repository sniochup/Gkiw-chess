#define GLM_FORCE_RADIANS

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <stdlib.h>
#include <stdio.h>
#include "constants.h"
#include "allmodels.h"
#include "lodepng.h"
#include "shaderprogram.h"
#include <iostream>
#include <fstream>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <myCube.h>

#include "chess.h"
bool n_bool = false;


GLuint tex; //Uchwyt – deklaracja globalna
int num_mesh = 0;
std::vector<glm::vec4> verts[60];
std::vector<glm::vec4> norms[60];
std::vector<glm::vec2> texCoords[60];
std::vector<unsigned int> indices[60];

float speed_x = 0;//[radiany/s]
float speed_y = 0;//[radiany/s]
float spee_x = 0;//[radiany/s]
float spee_y = 0;//[radiany/s]
float aspectRatio = 1;
float walk_speed = 0;

glm::vec3 pos = glm::vec3(0, 2, -11);

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
	//Wczytaj obrazek
	unsigned error = lodepng::decode(image, width, height, filename);

	//Import do pamięci karty graficznej
	glGenTextures(1, &tex); //Zainicjuj jeden uchwyt
	glBindTexture(GL_TEXTURE_2D, tex); //Uaktywnij uchwyt
	//Wczytaj obrazek do pamięci KG skojarzonej z uchwytem
	glTexImage2D(GL_TEXTURE_2D, 0, 4, width, height, 0,
		GL_RGBA, GL_UNSIGNED_BYTE, (unsigned char*)image.data());

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	return tex;
}

//Procedura obsługi błędów
void error_callback(int error, const char* description) {
	fputs(description, stderr);
}

void key_callback(
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
		if (key == GLFW_KEY_UP) walk_speed = 2;
		if (key == GLFW_KEY_DOWN) walk_speed = -2;
		if (key == GLFW_KEY_LEFT) speed_y = 1;
		if (key == GLFW_KEY_W) spee_x = PI;
		if (key == GLFW_KEY_S) spee_y = -PI;
		if (key == GLFW_KEY_N) n_bool = true;

	}
	if (action == GLFW_RELEASE) {
		if (key == GLFW_KEY_LEFT) speed_y = 0;
		if (key == GLFW_KEY_RIGHT) speed_y = 0;
		if (key == GLFW_KEY_PAGE_UP) speed_x = 0;
		if (key == GLFW_KEY_PAGE_DOWN) speed_x = 0;
		if (key == GLFW_KEY_UP) walk_speed = 0;
		if (key == GLFW_KEY_DOWN) walk_speed = 0;
		if (key == GLFW_KEY_W) spee_x = 0;
		if (key == GLFW_KEY_S) spee_y = 0;
	}
}

void windowResizeCallback(GLFWwindow* window, int width, int height) {
	if (height == 0) return;
	aspectRatio = (float)width / (float)height;
	glViewport(0, 0, width, height);
}

void loadModel(std::string plik) {

	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(plik, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenSmoothNormals);
	std::cout << importer.GetErrorString() << std::endl;

	std::cout << scene->mNumMeshes << std::endl;
	num_mesh = scene->mNumMeshes;
	//aiMesh* mesh = scene->mMeshes[51];
	aiMesh* mesh[60];
	aiMaterial* material[60];

	if (scene->HasMeshes()) {
		for (int it = 0; it < scene->mNumMeshes; it++) {
			mesh[it] = scene->mMeshes[it];

			for (int i = 0; i < mesh[it]->mNumVertices; i++) {
				aiVector3D vertex = mesh[it]->mVertices[i];
				verts[it].push_back(glm::vec4(vertex.x, vertex.y, vertex.z, 1));
				aiVector3D normal = mesh[it]->mNormals[i];
				norms[it].push_back(glm::vec4(normal.x, normal.y, normal.z, 1));
				aiVector3D texCoord = mesh[it]->mTextureCoords[0][i];
				texCoords[it].push_back(glm::vec2(texCoord.x, texCoord.y));
			}

			for (int i = 0; i < mesh[it]->mNumFaces; i++) {
				aiFace& face = mesh[it]->mFaces[i];
				for (int j = 0; j < face.mNumIndices; j++) {
					indices[it].push_back(face.mIndices[j]);
				}
			}

			material[it] = scene->mMaterials[mesh[it]->mMaterialIndex];

			for (int i = 0; i < 19; i++) {
				std::cout << i << " " << material[it]->GetTextureCount((aiTextureType)i) << std::endl;
			}

			for (int i = 0; i < material[it]->GetTextureCount(aiTextureType_DIFFUSE); i++) {
				aiString str;

				material[it]->GetTexture(aiTextureType_DIFFUSE, i, &str);
				std::cout << str.C_Str() << std::endl;
			}
		}
	}
}


//Procedura inicjująca
void initOpenGLProgram(GLFWwindow* window) {
    initShaders();
	//************Tutaj umieszczaj kod, który należy wykonać raz, na początku programu************
	glClearColor(1,1,1,1); //Ustaw kolor czyszczenia bufora kolorów
	glEnable(GL_DEPTH_TEST); //Włącz test głębokości na pikselach
	glfwSetWindowSizeCallback(window, windowResizeCallback);
	glfwSetKeyCallback(window, key_callback);

	//Wczytanie i import obrazka – w initOpenGLProgram
	
	tex = readTexture("bricks.png");
	loadModel(std::string("models\\LowPolyChess.fbx"));
}


//Zwolnienie zasobów zajętych przez program
void freeOpenGLProgram(GLFWwindow* window) {
    freeShaders();
    //************Tutaj umieszczaj kod, który należy wykonać po zakończeniu pętli głównej************
	
	//Usunięcie tekstury z pamięci karty graficznej – w freeOpenGLProgram
	glDeleteTextures(1, &tex);
}

//Procedura rysująca zawartość sceny
void drawScene(GLFWwindow* window,float angle_x,float angle_y, float kat_x, float kat_y) {
	//************Tutaj umieszczaj kod rysujący obraz******************l
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //Wyczyść bufor koloru i bufor głębokości
	
	glm::mat4 M = glm::mat4(1.0f); //Zainicjuj macierz modelu macierzą jednostkową
	M = glm::rotate(M, angle_y, glm::vec3(0.0f, 1.0f, 0.0f)); //Pomnóż macierz modelu razy macierz obrotu o kąt angle wokół osi Y
	M = glm::rotate(M, angle_x, glm::vec3(1.0f, 0.0f, 0.0f)); //Pomnóż macierz modelu razy macierz obrotu o kąt angle wokół osi X
	glm::mat4 V = glm::lookAt(pos, pos + calcDir(kat_x, kat_y), glm::vec3(0.0f, 1.0f, 0.0f)); //Wylicz macierz widoku
	glm::mat4 P = glm::perspective(glm::radians(50.0f), 1.0f, 0.1f, 50.0f);

	spTextured->use();

	glUniformMatrix4fv(spTextured->u("P"), 1, false, glm::value_ptr(P));
	glUniformMatrix4fv(spTextured->u("V"), 1, false, glm::value_ptr(V));

	glm::mat4 Plansza = M;
	glm::mat4 Plansza1 = glm::scale(Plansza, glm::vec3(2, 0.1, 2));
	glUniformMatrix4fv(spTextured->u("M"), 1, false, glm::value_ptr(Plansza1));
	glUniform4f(spTextured->u("color"), 0.2,0.4,0.7, 1);
	Models::cube.drawSolid();
	Plansza = glm::scale(Plansza, glm::vec3(2.5, 0.3, 2.5));
	Plansza = glm::translate(Plansza, glm::vec3(0,-1,0));
	glUniformMatrix4fv(spTextured->u("M"), 1, false, glm::value_ptr(Plansza));
	glUniform4f(spTextured->u("color"), 0.6, 0.4, 0.2, 1);
	Models::cube.drawSolid();


	//glm::mat4 M1 = M;
	////M1 = glm::scale(M1, glm::vec3(2,2,2));
	//M1 = glm::rotate(M1, -90*PI/180, glm::vec3(1.0f, 0.0f, 0.0f));


	//for (int i = 0; i < 1; i++) {
	//	spTextured->use();
	//	glUniformMatrix4fv(spTextured->u("P"), 1, false, glm::value_ptr(P));
	//	glUniformMatrix4fv(spTextured->u("V"), 1, false, glm::value_ptr(V));
	//	glUniformMatrix4fv(spTextured->u("M"), 1, false, glm::value_ptr(M1));
	//	glUniform4f(spTextured->u("color"), 0, 1, 0, 1);

	//	glEnableVertexAttribArray(spTextured->a("vertex"));
	//	glVertexAttribPointer(spTextured->a("vertex"), 4, GL_FLOAT, false, 0, verts[i].data());

	//	glEnableVertexAttribArray(spTextured->a("normal"));
	//	glVertexAttribPointer(spTextured->a("normal"), 4, GL_FLOAT, false, 0, norms[i].data());

	//	glEnableVertexAttribArray(spTextured->a("texCoord"));
	//	glVertexAttribPointer(spTextured->a("texCoord"), 2, GL_FLOAT, false, 0, verts[i].data());

	//	glActiveTexture(GL_TEXTURE0);
	//	glBindTexture(GL_TEXTURE_2D, tex);
	//	glUniform1i(spTextured->u("tex"), 0);
	//	glDrawElements(GL_TRIANGLES, indices[i].size(), GL_UNSIGNED_INT, indices[i].data());

	//	glDisableVertexAttribArray(spTextured->a("vertex"));
	//	glDisableVertexAttribArray(spTextured->a("normal"));
	//	glDisableVertexAttribArray(spTextured->a("texCoord"));
	//}



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
	float kat_x = 0;
	float kat_y = 0;
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

		kat_x += speed_x * glfwGetTime();
		kat_y += speed_y * glfwGetTime();

		angle_x += spee_x * glfwGetTime(); //Oblicz kąt o jaki obiekt obrócił się podczas poprzedniej klatki
		angle_y += spee_y * glfwGetTime(); //Oblicz kąt o jaki obiekt obrócił się podczas poprzedniej klatki
		pos += (float)(walk_speed * glfwGetTime()) * calcDir(kat_x, kat_y);

		glfwSetTime(0); //Wyzeruj licznik czasu
		drawScene(window,angle_x,angle_y, kat_x, kat_y); //Wykonaj procedurę rysującą
		glfwPollEvents(); //Wykonaj procedury callback w zalezności od zdarzeń jakie zaszły.
	}

	freeOpenGLProgram(window);

	glfwDestroyWindow(window); //Usuń kontekst OpenGL i okno
	glfwTerminate(); //Zwolnij zasoby zajęte przez GLFW
	exit(EXIT_SUCCESS);
	
}
