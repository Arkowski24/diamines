#include <iostream>
#include <vector>

#define BOARD_MSIZE 200

#define SYM_WALL '#'
#define SYM_DIAM '+'
#define SYM_MINE '*'
#define SYM_HOLE 'O'
#define SYM_PCAR '.'
#define SYM_SPCE ' '

using namespace std;

char board[BOARD_MSIZE][BOARD_MSIZE]; //Y, X
int ySize, xSize;
int expectedSteps;

int8_t directions[][2] = {
        {-1, 0},
        {-1, 1},
        {0,  1},
        {1,  1},
        {1,  0},
        {1,  -1},
        {0,  -1},
        {-1, -1}
};
uint8_t stopDirection[BOARD_MSIZE][BOARD_MSIZE] = {0};
u_int32_t nodeID[BOARD_MSIZE][BOARD_MSIZE] = {0};

int y_start, x_start;
int diaCount = 0;

void readInput() {
    cin >> ySize >> xSize;
    cin >> expectedSteps;

    for (int i = 0; i < ySize; i++) {
        string boardLine;
        cin >> boardLine;
        boardLine.copy(board[i], boardLine.length());
    }
}

void processPoint(int y, int x) {
    int dirs[8] = {0, 1, 2, 3, 4, 5, 6, 7};

    switch (board[y][x]) {
        case SYM_HOLE:
            stopDirection[y][x] = 0xFF;
            return;
        case SYM_DIAM:
            diaCount++;
        case SYM_SPCE:
        case SYM_PCAR:
            for (int dir : dirs) {
                int y_i = y + directions[dir][0];
                int x_i = x + directions[dir][1];
                if (board[y_i][x_i] == SYM_WALL)
                    stopDirection[y][x] |= (1u << dir);
            }
        default:
            return;
    }
}

void processInput() {
    readInput();

    int nodes = 0;
    for (int y = 0; y < ySize; ++y) {
        for (int x = 0; x < xSize; ++x) {
            processPoint(y, x);

            if (stopDirection[y][x] > 0) {
                nodeID[y][x] = nodes + 1;
                nodes++;
            }

            if (board[y][x] == SYM_PCAR) {
                y_start = y;
                x_start = x;
            }
        }
    }
}

void createNode(int y, int x) {
    int dirs[8] = {0, 1, 2, 3, 4, 5, 6, 7};

    for (int dir : dirs) {
        if (stopDirection[y][x] & (1u << dir)) {
            int altDir = (dir + 4) % 8;

            vector<bool> diamonds(diaCount);
        }
    }
}

void createGraph() {
    for (int y = 0; y < ySize; ++y) {
        for (int x = 0; x < xSize; ++x) {
            if (nodeID[y][x] > 0) {
                createNode(y, x);
            }
        }
    }
}

int main() {
    processInput();
    createGraph();


    return 0;
}