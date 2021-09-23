#define GLM_FORCE_RADIANS
#define GLM_FORCE_SWIZZLE

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <stdlib.h>
#include <stdio.h>
#include "constants.h"
#include "shaderprogram.h"
#include "myCube.h"
#include "chess.h"
#include "helper_functions.h"

bool n_bool = false;

glm::vec3 pos = glm::vec3(0, 0, -5);

ShaderProgram* spSkybox;
ShaderProgram* spBoard;
ShaderProgram* spPiece;

GLuint texBoard[8];
GLuint cubemapTexture;

Board board;
Square square[8][8];

Model King;
Model Queen;
Model Rook;
Model Bishop;
Model Knight;
Model Pawn;

float speed_x = 0;//[radiany/s]
float speed_y = 0;//[radiany/s]
float speed_x_1 = 0;//[radiany/s]
float speed_y_1 = 0;//[radiany/s]
float aspectRatio = 1;
float walk_speed = 0;

void windowResizeCallback(GLFWwindow* window, int width, int height) {
	if (height == 0) return;
	aspectRatio = (float)width / (float)height;
	glViewport(0, 0, width, height);
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mod) {
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

//Procedura inicjująca
void initOpenGLProgram(GLFWwindow* window) {
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

	std::vector<std::string> faces {
			"textures\\skybox\\right.png",
			"textures\\skybox\\left.png",
			"textures\\skybox\\top.png",
			"textures\\skybox\\bottom.png",
			"textures\\skybox\\front.png",
			"textures\\skybox\\back.png"
	};
	cubemapTexture = loadCubemap(faces);
	
	King.loadModel(std::string("models\\King.fbx"));
	Queen.loadModel(std::string("models\\Queen.fbx"));
	Rook.loadModel(std::string("models\\Rook.fbx"));
	Bishop.loadModel(std::string("models\\Bishop.fbx"));
	Knight.loadModel(std::string("models\\Knight.fbx"));
	Pawn.loadModel(std::string("models\\Pawn.fbx"));
}

//Zwolnienie zasobów zajętych przez program
void freeOpenGLProgram(GLFWwindow* window) {
	delete spSkybox;
	delete spBoard;
	delete spPiece;

	glDeleteTextures(8, &texBoard[0]);
	glDeleteTextures(1, &cubemapTexture);
}

//Procedura rysująca zawartość sceny
void drawScene(GLFWwindow* window,float angle_x,float angle_y, float angle_x_1, float angle_y_1) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //Wyczyść bufor koloru i bufor głębokości
	
	bool draw_board = false;
	bool draw_piece = true;
	bool draw_skybox = false;
	//bool draw_all_pieces = false;

	bool draw_king = false;
	bool draw_queen = false;
	bool draw_bishop = false;
	bool draw_knight = true;
	bool draw_rook = false;
	bool draw_pawn = false;

	glm::vec4 lp = glm::vec4(0, 0, -10, 1); // Położenie źródła światła

	glm::mat4 V = glm::lookAt(pos, pos + calcDir(angle_x_1, angle_y_1), glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 P = glm::perspective(glm::radians(50.0f), aspectRatio, 0.01f, 50.0f);

	glm::mat4 M = glm::mat4(1.0f); //Zainicjuj macierz modelu macierzą jednostkową
	M = glm::rotate(M, angle_y, glm::vec3(0.0f, 1.0f, 0.0f)); //Pomnóż macierz modelu razy macierz obrotu o kąt angle wokół osi Y
	M = glm::rotate(M, angle_x, glm::vec3(1.0f, 0.0f, 0.0f)); //Pomnóż macierz modelu razy macierz obrotu o kąt angle wokół osi X
	
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
		spPiece->use();
		glUniformMatrix4fv(spPiece->u("P"), 1, false, glm::value_ptr(P));
		glUniformMatrix4fv(spPiece->u("V"), 1, false, glm::value_ptr(V));
		glUniformMatrix4fv(spPiece->u("M"), 1, false, glm::value_ptr(M));
		glUniform4fv(spPiece->u("lp"), 1, glm::value_ptr(lp));
		
		if (draw_king) {
			glEnableVertexAttribArray(spPiece->a("vertex"));
			glVertexAttribPointer(spPiece->a("vertex"), 4, GL_FLOAT, false, 0, King.verts.data());

			glEnableVertexAttribArray(spPiece->a("normal"));
			glVertexAttribPointer(spPiece->a("normal"), 4, GL_FLOAT, false, 0, King.norms.data());

			glDrawElements(GL_TRIANGLES, King.indices.size(), GL_UNSIGNED_INT, King.indices.data());
		}

		if (draw_queen) {
			glEnableVertexAttribArray(spPiece->a("vertex"));
			glVertexAttribPointer(spPiece->a("vertex"), 4, GL_FLOAT, false, 0, Queen.verts.data());

			glEnableVertexAttribArray(spPiece->a("normal"));
			glVertexAttribPointer(spPiece->a("normal"), 4, GL_FLOAT, false, 0, Queen.norms.data());

			glDrawElements(GL_TRIANGLES, Queen.indices.size(), GL_UNSIGNED_INT, Queen.indices.data());
		}

		if (draw_bishop) {
			glEnableVertexAttribArray(spPiece->a("vertex"));
			glVertexAttribPointer(spPiece->a("vertex"), 4, GL_FLOAT, false, 0, Bishop.verts.data());

			glEnableVertexAttribArray(spPiece->a("normal"));
			glVertexAttribPointer(spPiece->a("normal"), 4, GL_FLOAT, false, 0, Bishop.norms.data());

			glDrawElements(GL_TRIANGLES, Bishop.indices.size(), GL_UNSIGNED_INT, Bishop.indices.data());
		}

		if (draw_knight) {
			glEnableVertexAttribArray(spPiece->a("vertex"));
			glVertexAttribPointer(spPiece->a("vertex"), 4, GL_FLOAT, false, 0, Knight.verts.data());

			glEnableVertexAttribArray(spPiece->a("normal"));
			glVertexAttribPointer(spPiece->a("normal"), 4, GL_FLOAT, false, 0, Knight.norms.data());

			glDrawElements(GL_TRIANGLES, Knight.indices.size(), GL_UNSIGNED_INT, Knight.indices.data());
		}

		if (draw_rook) {
			glEnableVertexAttribArray(spPiece->a("vertex"));
			glVertexAttribPointer(spPiece->a("vertex"), 4, GL_FLOAT, false, 0, Rook.verts.data());

			glEnableVertexAttribArray(spPiece->a("normal"));
			glVertexAttribPointer(spPiece->a("normal"), 4, GL_FLOAT, false, 0, Rook.norms.data());

			glDrawElements(GL_TRIANGLES, Rook.indices.size(), GL_UNSIGNED_INT, Rook.indices.data());
		}

		if (draw_pawn) {
			glEnableVertexAttribArray(spPiece->a("vertex"));
			glVertexAttribPointer(spPiece->a("vertex"), 4, GL_FLOAT, false, 0, Pawn.verts.data());

			glEnableVertexAttribArray(spPiece->a("normal"));
			glVertexAttribPointer(spPiece->a("normal"), 4, GL_FLOAT, false, 0, Pawn.norms.data());

			glDrawElements(GL_TRIANGLES, Pawn.indices.size(), GL_UNSIGNED_INT, Pawn.indices.data());
		}

		glDisableVertexAttribArray(spPiece->a("vertex"));
		glDisableVertexAttribArray(spPiece->a("normal"));
	}

	/*if (draw_all_pieces) {
		for (int i = 0; i < 8; i++) {
			for (int j = 0; j < 8; j++) {
				Piece p = square[i][j].getPiece;
				switch (p)
				{
					case king:
						spPiece->use();
						glUniformMatrix4fv(spPiece->u("P"), 1, false, glm::value_ptr(P));
						glUniformMatrix4fv(spPiece->u("V"), 1, false, glm::value_ptr(V));
						glUniformMatrix4fv(spPiece->u("M"), 1, false, glm::value_ptr(M));
						glUniform4fv(spPiece->u("lp"), 1, glm::value_ptr(lp));
						glEnableVertexAttribArray(spPiece->a("vertex"));
						glVertexAttribPointer(spPiece->a("vertex"), 4, GL_FLOAT, false, 0, King.verts.data());
						glEnableVertexAttribArray(spPiece->a("normal"));
						glVertexAttribPointer(spPiece->a("normal"), 4, GL_FLOAT, false, 0, King.norms.data());
						glDrawElements(GL_TRIANGLES, King.indices.size(), GL_UNSIGNED_INT, King.indices.data());
						glDisableVertexAttribArray(spPiece->a("vertex"));
						glDisableVertexAttribArray(spPiece->a("normal"));
						break;
					case queen:
						spPiece->use();
						glUniformMatrix4fv(spPiece->u("P"), 1, false, glm::value_ptr(P));
						glUniformMatrix4fv(spPiece->u("V"), 1, false, glm::value_ptr(V));
						glUniformMatrix4fv(spPiece->u("M"), 1, false, glm::value_ptr(M));
						glUniform4fv(spPiece->u("lp"), 1, glm::value_ptr(lp));
						glEnableVertexAttribArray(spPiece->a("vertex"));
						glVertexAttribPointer(spPiece->a("vertex"), 4, GL_FLOAT, false, 0, Queen.verts.data());
						glEnableVertexAttribArray(spPiece->a("normal"));
						glVertexAttribPointer(spPiece->a("normal"), 4, GL_FLOAT, false, 0, Queen.norms.data());
						glDrawElements(GL_TRIANGLES, Queen.indices.size(), GL_UNSIGNED_INT, Queen.indices.data());
						glDisableVertexAttribArray(spPiece->a("vertex"));
						glDisableVertexAttribArray(spPiece->a("normal"));
						break;
					case bishop:
						spPiece->use();
						glUniformMatrix4fv(spPiece->u("P"), 1, false, glm::value_ptr(P));
						glUniformMatrix4fv(spPiece->u("V"), 1, false, glm::value_ptr(V));
						glUniformMatrix4fv(spPiece->u("M"), 1, false, glm::value_ptr(M));
						glUniform4fv(spPiece->u("lp"), 1, glm::value_ptr(lp));
						glEnableVertexAttribArray(spPiece->a("vertex"));
						glVertexAttribPointer(spPiece->a("vertex"), 4, GL_FLOAT, false, 0, Bishop.verts.data());
						glEnableVertexAttribArray(spPiece->a("normal"));
						glVertexAttribPointer(spPiece->a("normal"), 4, GL_FLOAT, false, 0, Bishop.norms.data());
						glDrawElements(GL_TRIANGLES, Bishop.indices.size(), GL_UNSIGNED_INT, Bishop.indices.data());
						glDisableVertexAttribArray(spPiece->a("vertex"));
						glDisableVertexAttribArray(spPiece->a("normal"));
						break;
					case knight:
						spPiece->use();
						glUniformMatrix4fv(spPiece->u("P"), 1, false, glm::value_ptr(P));
						glUniformMatrix4fv(spPiece->u("V"), 1, false, glm::value_ptr(V));
						glUniformMatrix4fv(spPiece->u("M"), 1, false, glm::value_ptr(M));
						glUniform4fv(spPiece->u("lp"), 1, glm::value_ptr(lp));
						glEnableVertexAttribArray(spPiece->a("vertex"));
						glVertexAttribPointer(spPiece->a("vertex"), 4, GL_FLOAT, false, 0, Knight.verts.data());
						glEnableVertexAttribArray(spPiece->a("normal"));
						glVertexAttribPointer(spPiece->a("normal"), 4, GL_FLOAT, false, 0, Knight.norms.data());
						glDrawElements(GL_TRIANGLES, Knight.indices.size(), GL_UNSIGNED_INT, Knight.indices.data());
						glDisableVertexAttribArray(spPiece->a("vertex"));
						glDisableVertexAttribArray(spPiece->a("normal"));
						break;
					case rook:
						spPiece->use();
						glUniformMatrix4fv(spPiece->u("P"), 1, false, glm::value_ptr(P));
						glUniformMatrix4fv(spPiece->u("V"), 1, false, glm::value_ptr(V));
						glUniformMatrix4fv(spPiece->u("M"), 1, false, glm::value_ptr(M));
						glUniform4fv(spPiece->u("lp"), 1, glm::value_ptr(lp));
						glEnableVertexAttribArray(spPiece->a("vertex"));
						glVertexAttribPointer(spPiece->a("vertex"), 4, GL_FLOAT, false, 0, Rook.verts.data());
						glEnableVertexAttribArray(spPiece->a("normal"));
						glVertexAttribPointer(spPiece->a("normal"), 4, GL_FLOAT, false, 0, Rook.norms.data());
						glDrawElements(GL_TRIANGLES, Rook.indices.size(), GL_UNSIGNED_INT, Rook.indices.data());
						glDisableVertexAttribArray(spPiece->a("vertex"));
						glDisableVertexAttribArray(spPiece->a("normal"));
						break;
					case pawn:
						spPiece->use();
						glUniformMatrix4fv(spPiece->u("P"), 1, false, glm::value_ptr(P));
						glUniformMatrix4fv(spPiece->u("V"), 1, false, glm::value_ptr(V));
						glUniformMatrix4fv(spPiece->u("M"), 1, false, glm::value_ptr(M));
						glUniform4fv(spPiece->u("lp"), 1, glm::value_ptr(lp));
						glEnableVertexAttribArray(spPiece->a("vertex"));
						glVertexAttribPointer(spPiece->a("vertex"), 4, GL_FLOAT, false, 0, Pawn.verts.data());
						glEnableVertexAttribArray(spPiece->a("normal"));
						glVertexAttribPointer(spPiece->a("normal"), 4, GL_FLOAT, false, 0, Pawn.norms.data());
						glDrawElements(GL_TRIANGLES, Pawn.indices.size(), GL_UNSIGNED_INT, Pawn.indices.data());
						glDisableVertexAttribArray(spPiece->a("vertex"));
						glDisableVertexAttribArray(spPiece->a("normal"));
						break;
					case e:
						break;
				}
			}
		}
	}*/

	if (draw_skybox) {
		spSkybox->use();
		glm::mat4 view = glm::mat4(glm::mat3(V));
		glUniformMatrix4fv(spSkybox->u("view"), 1, false, glm::value_ptr(view));
		glUniformMatrix4fv(spSkybox->u("projection"), 1, false, glm::value_ptr(P));

		glDepthFunc(GL_LEQUAL);
		glEnableVertexAttribArray(spSkybox->a("aPos"));
		glVertexAttribPointer(spSkybox->a("aPos"), 3, GL_FLOAT, false, 0, skyboxVertices);
		glUniform1i(spSkybox->u("skybox"), 0);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);
		glDepthFunc(GL_LESS);
	}

	glfwSwapBuffers(window); //Skopiuj bufor tylny do bufora przedniego
}


int main(void) {
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

	//Board board; - zainicjowalam globalnie
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
