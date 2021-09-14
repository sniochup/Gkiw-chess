#ifndef CHESS_H
#define CHESS_H

#include <iostream>
#include <fstream>
#include <string>

enum Color {white, black, none};
enum Piece {king, queen, rook, bishop, knight, pawn, e};

class Square {
	int x, y;
	Color color;
	Piece piece;

	public:
		Square();
		Piece getPiece();
		Color getColor();
		void setSquare(Color, Piece);
};

class Board : public Square {
	public:
		Square square[8][8];
		Board();
		void printBoard();
		void move(std::string path, int num);
};

#endif

/*
K = king	- król
Q = queen	- hetman
R = rook	- wie¿a
B = bishop	- goniec
N = knight	- skoczek
P = pawn	- pion
nutke = e	- puste pole

W - white
B - black
nutke - none - puste pole

*/