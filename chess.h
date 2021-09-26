#ifndef CHESS_H
#define CHESS_H

#include <iostream>
#include <fstream>
#include <string>

enum Color {white, black, none};
enum Piece {king, queen, rook, bishop, knight, pawn, e};

class Square {
	Color color;
	Piece piece;

	public:
		Square();
		Piece getPiece();
		int getPieceNum();
		Color getColor();
		int getColorNum();
		void setSquare(Color, Piece);
};

class Board : public Square {
	public:
		Square square[8][8];
		Board();
		void printBoard();
		int move(std::string path, int num);
};

#endif

/*
K = king	- król		 - 0
Q = queen	- hetman	 - 1
R = rook	- wie¿a		 - 2
B = bishop	- goniec	 - 3
N = knight	- skoczek	 - 4
P = pawn	- pion		 - 5
e = nutke 	- puste pole - 6

W - white
B - black
nutke - none - puste pole

*/