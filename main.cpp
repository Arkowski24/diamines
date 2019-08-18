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
    uint8_t dir;
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
    cin.ignore(1);

    for (int i = 0; i < ySize; i++) {
        string boardLine;
        getline(cin, boardLine);
        boardLine.copy(board[i], xSize);
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
    Connection connection{};
    connection.dir = dir;
    connection.diamonds = set<int>();

    int altDir = (dir + 4) % 8;
    int y_delta = y + directions[altDir][0];
    int x_delta = x + directions[altDir][1];

    while (board[y_delta][x_delta] == SYM_SPCE && (stopDirection[y_delta][x_delta] & (1u << dir)) == 0) {
        if (diamID[y_delta][x_delta] > 0) {
            connection.diamonds.insert(diamID[y_delta][x_delta]);
        }

        if (nodeID[y_delta][x_delta] > 0) {
            Connection connection1 = connection;
            connection1.to = nodeID[y][x];
            graph[nodeID[y_delta][x_delta]].push_back(connection1);
        }

        y_delta += directions[altDir][0];
        x_delta += directions[altDir][1];
    }

    // Mine has always nodeID == 0
    if (nodeID[y_delta][x_delta] == 0) {
        return;
    }

    if (diamID[y_delta][x_delta] > 0) {
        connection.diamonds.insert(diamID[y_delta][x_delta]);
    }

    connection.to = nodeID[y][x];
    graph[nodeID[y_delta][x_delta]].push_back(connection);
}

void createNode(int y, int x) {
    int dirs[8] = {0, 1, 2, 3, 4, 5, 6, 7};

    for (int dir : dirs) {
        if (stopDirection[y][x] & (1u << dir)) {
            traverseLane(y, x, dir);
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

    if (nodeID[y_start][x_start] == 0) {
        nodeID[y_start][x_start] = nodeCount + 1;
        nodeCount++;
        graph.emplace_back(vector<Connection>());

        int dirs[8] = {0, 1, 2, 3, 4, 5, 6, 7};
        for (int dir : dirs) {
            Connection connection{};
            connection.dir = dir;
            connection.diamonds = set<int>();

            int y_delta = y_start + directions[dir][0];
            int x_delta = x_start + directions[dir][1];

            while (board[y_delta][x_delta] == SYM_SPCE && (stopDirection[y_delta][x_delta] & (1u << dir)) == 0) {
                if (diamID[y_delta][x_delta] > 0) {
                    connection.diamonds.insert(diamID[y_delta][x_delta]);
                }

                y_delta += directions[dir][0];
                x_delta += directions[dir][1];
            }

            // Mine has always nodeID == 0
            if (nodeID[y_delta][x_delta] == 0) {
                continue;
            }

            if (diamID[y_delta][x_delta] > 0) {
                connection.diamonds.insert(diamID[y_delta][x_delta]);
            }

            connection.to = nodeID[y_delta][x_delta];
            graph[nodeID[y_start][x_start]].push_back(connection);
        }
    }
}


int searchGraph(int node, int moveCount, uint8_t moveList[], set<int> diams) {
    if (moveCount > expectedSteps) {
        return -1;
    }

    if (diams.size() == diamCount) {
        return moveCount;
    }

    for (auto &con : graph[node]) {
        moveList[moveCount] = con.dir;
        diams.insert(con.diamonds.begin(), con.diamonds.end());
        int result = searchGraph(con.to, moveCount + 1, moveList, diams);
        if (result != -1) {
            return result;
        }
    }
    return -1;
}

void findAnswer() {
    uint8_t moveList[expectedSteps];
    int startNode = nodeID[y_start][x_start];

    int result = searchGraph(startNode, 0, moveList, set<int>());
    if (result == -1) {
        cout << "BRAK" << endl;
    } else {
        for (int i = 0; i < result; ++i) {
            cout << (int) moveList[i];
        }
    }
}

int main() {
    processInput();
    createGraph();
    findAnswer();

    return 0;
}