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
bool end_game = false;
int move_code = 0;

ShaderProgram* spSkybox;
ShaderProgram* spBoard;
ShaderProgram* spPiece;

GLuint texBoard[8];
GLuint texPiece[4];
GLuint cubemapTexture;
GLuint sky;

Board board;

Model piece_models[6];

float speed_x = 0;//[radiany/s]
float speed_y = 0;
float speed_x_1 = 0;
float speed_y_1 = 0;
float aspectRatio = 1;
float walk_speed = 0;

glm::vec3 pos = glm::vec3(0, 2, -4);
float angle_x = 0; 
float angle_y = 0;
float angle_x_1 = 0.3;
float angle_y_1 = 0;

void windowResizeCallback(GLFWwindow* window, int width, int height) {
	if (height == 0) return;
	aspectRatio = (float)width / (float)height;
	glViewport(0, 0, width, height);
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mod) {
	if (action == GLFW_PRESS) {
		if (key == GLFW_KEY_LEFT) speed_y = 2;
		if (key == GLFW_KEY_RIGHT) speed_y = -2;
		if (key == GLFW_KEY_PAGE_UP) speed_x = -2;
		if (key == GLFW_KEY_PAGE_DOWN) speed_x = 2;
		if (key == GLFW_KEY_UP) walk_speed = 6;
		if (key == GLFW_KEY_DOWN) walk_speed = -6;
		if (key == GLFW_KEY_W) speed_x_1 = PI;
		if (key == GLFW_KEY_S) speed_x_1 = -PI;
		if (key == GLFW_KEY_D) speed_y_1 = PI;
		if (key == GLFW_KEY_A) speed_y_1 = -PI;
		if (key == GLFW_KEY_N) n_bool = true;
		if (key == GLFW_KEY_ESCAPE) end_game = true;
		if (key == GLFW_KEY_1) {
			pos = glm::vec3(0, 2.7, -2.7);
			angle_x = 0;
			angle_y = 0;
			angle_x_1 = 0.6;
			angle_y_1 = 0;
		}
		if (key == GLFW_KEY_2) {
			pos = glm::vec3(0, 1.3, 2.6);
			angle_x = 0;
			angle_y = 0;
			angle_x_1 = 0.3;
			angle_y_1 = PI;
		}

		if (key == GLFW_KEY_3) {
			pos = glm::vec3(-2.5, 2.5, 0);
			angle_x = 0;
			angle_y = 0;
			angle_x_1 = 0.75;
			angle_y_1 = PI/2;
		}
		if (key == GLFW_KEY_4) {
			pos = glm::vec3(3.31, 1.4, 0);
			angle_x = 0;
			angle_y = 0;
			angle_x_1 = 0.086;
			angle_y_1 = PI*1.5;
		}
		if (key == GLFW_KEY_5) {
			pos = glm::vec3(0, 3, -1);
			angle_x = 0;
			angle_y = 0;
			angle_x_1 = 1.28;
			angle_y_1 = 0;
		}

	}
	if (action == GLFW_RELEASE) {
		if (key == GLFW_KEY_LEFT) speed_y = 0;
		if (key == GLFW_KEY_RIGHT) speed_y = 0;
		if (key == GLFW_KEY_PAGE_UP) speed_x = 0;
		if (key == GLFW_KEY_PAGE_DOWN) speed_x = 0;
		if (key == GLFW_KEY_UP) walk_speed = 0;
		if (key == GLFW_KEY_DOWN) walk_speed = 0;
		if (key == GLFW_KEY_W) speed_x_1 = 0;
		if (key == GLFW_KEY_S) speed_x_1 = 0;
		if (key == GLFW_KEY_D) speed_y_1 = 0;
		if (key == GLFW_KEY_A) speed_y_1 = 0;
	}
}

//Procedura inicjuj??ca
void initOpenGLProgram(GLFWwindow* window) {
	glClearColor(1,1,1,1); //Ustaw kolor czyszczenia bufora kolor??w
	glEnable(GL_DEPTH_TEST); //W????cz test g????boko??ci na pikselach
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

	texPiece[0] = readTexture("textures\\piece\\White_Marble_004_COLOR.png");
	texPiece[1] = readTexture("textures\\piece\\unsplash.png");
	texPiece[2] = readTexture("textures\\piece\\Marble_Red_003_COLOR.png");
	texPiece[3] = readTexture("textures\\piece\\green.png");

	sky = readTexture("textures\\skybox\\top.png");

	std::vector<std::string> faces {
			"textures\\skybox\\right.png",
			"textures\\skybox\\left.png",
			"textures\\skybox\\top.png",
			"textures\\skybox\\bottom.png",
			"textures\\skybox\\front.png",
			"textures\\skybox\\back.png"
	};
	cubemapTexture = loadCubemap(faces);

	piece_models[0].loadModel(std::string("models\\KingNew.fbx"));
	piece_models[1].loadModel(std::string("models\\QueenNew.fbx"));
	piece_models[2].loadModel(std::string("models\\RookNew.fbx"));
	for (int i = 0; i < piece_models[2].num_verts; i++) {
		piece_models[2].norms[i].x = piece_models[2].norms[i].x * -1;
		piece_models[2].norms[i].y = piece_models[2].norms[i].y * -1;
		piece_models[2].norms[i].z = piece_models[2].norms[i].z * -1;
	}
	piece_models[3].loadModel(std::string("models\\BishopNew.fbx"));
	for (int i = 0; i < piece_models[2].num_verts; i++) {
		piece_models[3].norms[i].x = piece_models[3].norms[i].x * -1;
		piece_models[3].norms[i].y = piece_models[3].norms[i].y * -1;
		piece_models[3].norms[i].z = piece_models[3].norms[i].z * -1;
	}
	piece_models[4].loadModel(std::string("models\\KnightNew.fbx"));
	piece_models[5].loadModel(std::string("models\\PawnNew.fbx"));
}

//Zwolnienie zasob??w zaj??tych przez program
void freeOpenGLProgram(GLFWwindow* window) {
	delete spSkybox;
	delete spBoard;
	delete spPiece;

	glDeleteTextures(8, &texBoard[0]);
	glDeleteTextures(4, &texPiece[0]);
	glDeleteTextures(1, &cubemapTexture);
	glDeleteTextures(1, &sky);
}

//Procedura rysuj??ca zawarto???? sceny
void drawScene(GLFWwindow* window,float angle_x,float angle_y, float angle_x_1, float angle_y_1) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //Wyczy???? bufor koloru i bufor g????boko??ci
	
	bool draw_board = true;
	bool draw_piece = false;
	int piece_num = 2;
	bool draw_all_pieces = true;
	bool draw_skybox = true;

	glm::vec4 lp = glm::vec4(4, 2, -4, 1); // Po??o??enie ??r??d??a ??wiat??a
	glm::vec4 lp1 = glm::vec4(-4, 2, 4, 1); // Po??o??enie ??r??d??a ??wiat??a 1

	glm::mat4 V = glm::lookAt(pos, pos + calcDir(angle_x_1, angle_y_1), glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 P = glm::perspective(glm::radians(50.0f), aspectRatio, 0.01f, 50.0f);

	glm::mat4 M = glm::mat4(1.0f);
	M = glm::rotate(M, angle_y, glm::vec3(0.0f, 1.0f, 0.0f));
	M = glm::rotate(M, angle_x, glm::vec3(1.0f, 0.0f, 0.0f));
	
	if (draw_board) {
		spBoard->use();
		glUniformMatrix4fv(spBoard->u("P"), 1, false, glm::value_ptr(P));
		glUniformMatrix4fv(spBoard->u("V"), 1, false, glm::value_ptr(V));
		glm::mat4 Plansza = glm::scale(M, glm::vec3(1, 0.3, 1));
		glUniformMatrix4fv(spBoard->u("M"), 1, false, glm::value_ptr(Plansza));
		glUniform4fv(spBoard->u("lp"), 1, glm::value_ptr(lp));
		glUniform4fv(spBoard->u("lp1"), 1, glm::value_ptr(lp1));

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

		glUniform1i(spBoard->u("textureMap4"), 3);
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, sky);

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

		glUniform1i(spBoard->u("textureMap4"), 3);
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, sky);

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
		glUniform4fv(spPiece->u("lp1"), 1, glm::value_ptr(lp1));

		glEnableVertexAttribArray(spPiece->a("vertex"));
		glVertexAttribPointer(spPiece->a("vertex"), 4, GL_FLOAT, false, 0, piece_models[piece_num].verts.data());

		glEnableVertexAttribArray(spPiece->a("normal"));
		glVertexAttribPointer(spPiece->a("normal"), 4, GL_FLOAT, false, 0, piece_models[piece_num].norms.data());

		glEnableVertexAttribArray(spPiece->a("texCoord0"));
		glVertexAttribPointer(spPiece->a("texCoord0"), 2, GL_FLOAT, false, 0, piece_models[piece_num].texCoords.data());

		glUniform1i(spPiece->u("textureMap0"), 0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texPiece[0]);

		glUniform1i(spPiece->u("textureMap1"), 1);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, sky);

		glDrawElements(GL_TRIANGLES, piece_models[piece_num].indices.size(), GL_UNSIGNED_INT, piece_models[piece_num].indices.data());

		glDisableVertexAttribArray(spPiece->a("vertex"));
		glDisableVertexAttribArray(spPiece->a("normal"));
		glDisableVertexAttribArray(spPiece->a("texCoord"));
	}

	if (draw_all_pieces) {
		for (int i = 0; i < 8; i++) {
			for (int j = 0; j < 8; j++) {
				piece_num = board.square[i][j].getPieceNum();
				if (piece_num != 6) {
					int c_num = board.square[i][j].getColorNum();
					spPiece->use();
					glUniformMatrix4fv(spPiece->u("P"), 1, false, glm::value_ptr(P));
					glUniformMatrix4fv(spPiece->u("V"), 1, false, glm::value_ptr(V));
					
					glm::mat4 M_p = glm::scale(M, glm::vec3(0.04, 0.04, 0.04));
					M_p = glm::translate(M_p, glm::vec3(22 - j*6.2857, 7.5, -22 + i*6.2857));
					M_p = glm::rotate(M_p, (float)-0.5 * PI, glm::vec3(1, 0, 0));

					if (piece_num == 0) {
						M_p = glm::rotate(M_p, (float)-0.5 * PI, glm::vec3(0, 0, 1));
					}
					else if (piece_num == 4 && c_num == 0) {
						M_p = glm::rotate(M_p, (float) PI, glm::vec3(0, 0, 1));
					}

					if (move_code == 3 && piece_num == 0 && c_num == 0) c_num = 2;
					else if (move_code == 4 && piece_num == 0 && c_num == 1) c_num = 2;
					else if (move_code == 5) {
						if (c_num == 0) c_num = 3;
						else c_num = 2;
					}
					else if (move_code == 6) {
						if (c_num == 0) c_num = 3;
						else c_num = 2;
					}
					else if (move_code == 7) c_num = 3;

					glUniformMatrix4fv(spPiece->u("M"), 1, false, glm::value_ptr(M_p));
					glUniform4fv(spPiece->u("lp"), 1, glm::value_ptr(lp));
					glUniform4fv(spPiece->u("lp1"), 1, glm::value_ptr(lp1));

					glEnableVertexAttribArray(spPiece->a("vertex"));
					glVertexAttribPointer(spPiece->a("vertex"), 4, GL_FLOAT, false, 0, piece_models[piece_num].verts.data());

					glEnableVertexAttribArray(spPiece->a("normal"));
					glVertexAttribPointer(spPiece->a("normal"), 4, GL_FLOAT, false, 0, piece_models[piece_num].norms.data());

					glEnableVertexAttribArray(spPiece->a("texCoord0"));
					glVertexAttribPointer(spPiece->a("texCoord0"), 2, GL_FLOAT, false, 0, piece_models[piece_num].texCoords.data());

					glUniform1i(spPiece->u("textureMap0"), 0);
					glActiveTexture(GL_TEXTURE0);
					glBindTexture(GL_TEXTURE_2D, texPiece[c_num]);

					glUniform1i(spPiece->u("textureMap1"), 1);
					glActiveTexture(GL_TEXTURE1);
					glBindTexture(GL_TEXTURE_2D, sky);
					
					glDrawElements(GL_TRIANGLES, piece_models[piece_num].indices.size(), GL_UNSIGNED_INT, piece_models[piece_num].indices.data());

					glDisableVertexAttribArray(spPiece->a("vertex"));
					glDisableVertexAttribArray(spPiece->a("normal"));
					glDisableVertexAttribArray(spPiece->a("texCoord"));
				}
			}
		}
	}

	if (draw_skybox) {
		spSkybox->use();
		glm::mat4 view = glm::mat4(glm::mat3(V));
		glUniformMatrix4fv(spSkybox->u("V"), 1, false, glm::value_ptr(view));
		glUniformMatrix4fv(spSkybox->u("P"), 1, false, glm::value_ptr(P));

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

	glfwSwapBuffers(window);
}


int main(void) {
	GLFWwindow* window;

	glfwSetErrorCallback(error_callback);

	if (!glfwInit()) { //Zainicjuj bibliotek?? GLFW
		fprintf(stderr, "Nie mo??na zainicjowa?? GLFW.\n");
		exit(EXIT_FAILURE);
	}

	window = glfwCreateWindow(900, 900, "Gkiw_chess", NULL, NULL);

	if (!window) //Je??eli okna nie uda??o si?? utworzy??, to zamknij program
	{
		fprintf(stderr, "Nie mo??na utworzy?? okna.\n");
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);

	if (glewInit() != GLEW_OK) { //Zainicjuj bibliotek?? GLEW
		fprintf(stderr, "Nie mo??na zainicjowa?? GLEW.\n");
		exit(EXIT_FAILURE);
	}

	initOpenGLProgram(window);

	glfwSetTime(0);

	std::string file_chess = "move.txt";
	int num_move = 0;
	board.printBoard();
	
	//G????wna p??tla
	while (!glfwWindowShouldClose(window))
	{
		if (n_bool) {
			move_code = board.move(file_chess, num_move++);
			if (move_code == 2) {
				board.move(file_chess, num_move++);
				board.move(file_chess, num_move++);
			}
			else if (move_code == 3 || move_code == 4 || move_code == 5 || move_code == 6 || move_code == 7 || move_code == 8) {
				board.move(file_chess, num_move++);
			}
			else if (move_code == 1) break;

			if (move_code != 5 && move_code != 6 && move_code != 7) board.printBoard();
			n_bool = false;
		}

		if (end_game == true) break;

		angle_x_1 += speed_x * glfwGetTime();
		angle_y_1 += speed_y * glfwGetTime();

		angle_x += speed_x_1 * glfwGetTime();
		angle_y += speed_y_1 * glfwGetTime();

		pos += (float)(walk_speed * glfwGetTime()) * calcDir(angle_x_1, angle_y_1);

		glfwSetTime(0);
		drawScene(window, angle_x, angle_y, angle_x_1, angle_y_1);
		glfwPollEvents();
	}

	freeOpenGLProgram(window);

	glfwDestroyWindow(window);
	glfwTerminate();
	exit(EXIT_SUCCESS);
}
