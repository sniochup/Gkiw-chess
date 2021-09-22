#include "chess.h"

using namespace std;

Square::Square() {
	color = none;
	piece = e;
}

Piece Square::getPiece()
{
	return piece;
}

Color Square::getColor()
{
	return color;
}

void Square::setSquare(Color c, Piece p) {
	color = c;
	piece = p;
}

Board::Board() {
	square[0][0].setSquare(white, rook);
	square[0][1].setSquare(white, knight);
	square[0][2].setSquare(white, bishop);
	square[0][3].setSquare(white, queen);
	square[0][4].setSquare(white, king);
	square[0][5].setSquare(white, bishop);
	square[0][6].setSquare(white, knight);
	square[0][7].setSquare(white, rook);

	square[7][0].setSquare(black, rook);
	square[7][1].setSquare(black, knight);
	square[7][2].setSquare(black, bishop);
	square[7][3].setSquare(black, queen);
	square[7][4].setSquare(black, king);
	square[7][5].setSquare(black, bishop);
	square[7][6].setSquare(black, knight);
	square[7][7].setSquare(black, rook);

	for (int i = 0; i < 8; i++) {
		square[1][i].setSquare(white, pawn);
		square[6][i].setSquare(black, pawn);
	}
}



void Board::printBoard() {
	for (int i = 7; i >= 0; i--) {
		cout << i+1 << "  ";
		for (int j = 0; j < 8; j++) {
			Piece p = square[i][j].getPiece();
			Color c = square[i][j].getColor();

			switch (p)
			{
				case king: (c == white) ? cout << "WK " : cout << "BK ";
					break;
				case queen: (c == white) ? cout << "WQ " : cout << "BQ ";
					break;
				case bishop:(c == white) ? cout << "WB " : cout << "BB ";
					break;
				case knight:(c == white) ? cout << "WH " : cout << "BH ";
					break;
				case rook: (c == white) ? cout << "WR " : cout << "BR ";
					break;
				case pawn: (c == white) ? cout << "WP " : cout << "BP ";
					break;
				case e: cout << "\16  ";
					break;
			}
		}
		cout << endl;
	}
	cout << endl;
	cout << "   a  b  c  d  e  f  g  h" << endl;
	cout << endl;
}

void Board::move(std::string path, int num) {
	fstream file;
	string b, f, line;
	char a;
	int int_bf[2];

	file.open(path, ios::in);

	for (int i = 0; i < num; i++) {
		getline(file, line);
	}

	if (file >> b) {
		file >> f;

		for (int i = 0; i < 2; i++) {
			if (i == 0) a = b[0];
			else a = f[0];

			switch (a) {
			case 'a':
				int_bf[i] = 0;
				break;
			case 'b':
				int_bf[i] = 1;
				break;
			case 'c':
				int_bf[i] = 2;
				break;
			case 'd':
				int_bf[i] = 3;
				break;
			case 'e':
				int_bf[i] = 4;
				break;
			case 'f':
				int_bf[i] = 5;
				break;
			case 'g':
				int_bf[i] = 6;
				break;
			case 'h':
				int_bf[i] = 7;
				break;
			}
		}
		square[int(f[1]) - 49][int_bf[1]].setSquare(square[int(b[1]) - 49][int_bf[0]].getColor(), square[int(b[1]) - 49][int_bf[0]].getPiece());
		square[int(b[1]) - 49][int_bf[0]].setSquare(none, e);

		file.close();

	}
	else cout << "KONIEC PLIKU - KONIEC GRY" << endl;
}