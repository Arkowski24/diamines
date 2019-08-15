#include <iostream>

#define BOARD_MSIZE 200

#define SYM_WALL '#'
#define SYM_DIAM '+'
#define SYM_MINE '*'
#define SYM_HOLE 'O'
#define SYM_PCAR '.'
#define SYM_SPCE ' '


using namespace std;

char board[BOARD_MSIZE][BOARD_MSIZE];
int x, y;
int expectedSteps;


void processInput() {
    cin >> y >> x;
    cin >> expectedSteps;

    for (int i = 0; i < y; i++) {
        string boardLine;
        cin >> boardLine;
        boardLine.copy(board[i], boardLine.length());
    }
}

int main() {
    processInput();

    return 0;
}