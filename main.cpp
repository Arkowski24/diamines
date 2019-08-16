#include <iostream>
#include <vector>
#include <set>

#define BOARD_MSIZE 200

#define SYM_WALL '#'
#define SYM_DIAM '+'
#define SYM_MINE '*'
#define SYM_HOLE 'O'
#define SYM_PCAR '.'
#define SYM_SPCE ' '

using namespace std;

struct Connection {
    int to;
    int dir;
    set<int> diamonds;
};

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
u_int32_t diamID[BOARD_MSIZE][BOARD_MSIZE] = {0};

int y_start, x_start;
int nodeCount = 0;
int diamCount = 0;
vector<vector<Connection>> graph;

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
        case SYM_SPCE:
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

    for (int y = 0; y < ySize; ++y) {
        for (int x = 0; x < xSize; ++x) {
            if (board[y][x] == SYM_PCAR) {
                y_start = y;
                x_start = x;
                board[y][x] = SYM_SPCE;
            } else if (board[y][x] == SYM_DIAM) {
                diamID[y][x] = diamCount + 1;
                diamCount++;
                board[y][x] = SYM_SPCE;
            }

            processPoint(y, x);
            if (stopDirection[y][x] > 0) {
                nodeID[y][x] = nodeCount + 1;
                nodeCount++;
            }
        }
    }
    graph = vector<vector<Connection>>(nodeCount + 1);
}

void traverseLane(int y, int x, int dir) {
    Connection connection;
    connection.diamonds = set<int>();

    int y_delta = y + directions[dir][0];
    int x_delta = x + directions[dir][1];

    while (board[y_delta][x_delta] == SYM_SPCE && stopDirection[y_delta][x_delta] == 0) {
        if (diamID[y_delta][x_delta] > 0) {
            connection.diamonds.insert(nodeID[y_delta][x_delta]);
        }

        y_delta += directions[dir][0];
        x_delta += directions[dir][1];
    }

    if (board[y_delta][x_delta] == SYM_MINE || nodeID[y_delta][x_delta] == 0) {
        return;
    }

    if (diamID[y_delta][x_delta] > 0) {
        connection.diamonds.insert(nodeID[y_delta][x_delta]);
    }

    connection.to = nodeID[y_delta][x_delta];
    connection.dir = dir;
    graph[nodeID[y_delta][x_delta]].push_back(connection);
}

void createNode(int y, int x) {
    int dirs[8] = {0, 1, 2, 3, 4, 5, 6, 7};

    for (int dir : dirs) {
        if (stopDirection[y][x] & (1u << dir)) {
            int altDir = (dir + 4) % 8;
            traverseLane(y, x, altDir);
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