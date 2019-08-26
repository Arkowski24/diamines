// Arkadiusz Placha
#include <iostream>
#include <vector>
#include <set>
#include <queue>
#include <tuple>
#include <cstring>
#include <unordered_map>

#define BOARD_MSIZE 200

#define SYM_WALL '#'
#define SYM_DIAM '+'
#define SYM_MINE '*'
#define SYM_HOLE 'O'
#define SYM_PCAR '.'
#define SYM_SPCE ' '

using namespace std;

struct Connection {
    int id;

    int to;
    uint8_t dir;
    set<int> diamonds;
};

struct FinalNode;

struct FinalConnection {
    int to;
    string path;
};

struct FinalNode {
    int dir;
    set<int> diamonds;

    long maxPathFrom;
    vector<FinalConnection> connections;
};


char board[BOARD_MSIZE][BOARD_MSIZE]; //Y, X
unsigned int ySize, xSize;
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
u_int32_t nodeCount = 0;
u_int32_t diamCount = 0;
vector<vector<Connection>> graph;
u_int32_t connectionCount = 0;


Connection startGuardian;
vector<FinalNode> finalGraph;
unordered_map<int, int> tMap;

// Initial Graph
// ===============================================================
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
    const int dirs[8] = {0, 1, 2, 3, 4, 5, 6, 7};

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

    if (nodeID[y_start][x_start] == 0) {
        nodeID[y_start][x_start] = nodeCount + 1;
        nodeCount++;
    }
    graph = vector<vector<Connection>>(nodeCount + 1);
}

void traverseLane(int y, int x, uint8_t dir) {
    Connection connection{};
    connection.id = connectionCount++;
    connection.dir = dir;
    connection.diamonds = set<int>();

    int altDir = (dir + 4) % 8;
    int y_delta = y + directions[altDir][0];
    int x_delta = x + directions[altDir][1];

    if (diamID[y][x] > 0) {
        connection.diamonds.insert(diamID[y][x]);
    }

    while (board[y_delta][x_delta] == SYM_SPCE && (stopDirection[y_delta][x_delta] & (1u << dir)) == 0) {
        if (diamID[y_delta][x_delta] > 0) {
            connection.diamonds.insert(diamID[y_delta][x_delta]);
        }

        if (nodeID[y_delta][x_delta] > 0) {
            Connection connection1 = connection;
            connection1.id = connectionCount++;
            connection1.to = nodeID[y][x];
            graph[nodeID[y_delta][x_delta]].push_back(connection1);
        }

        y_delta += directions[altDir][0];
        x_delta += directions[altDir][1];
    }

    if (nodeID[y_delta][x_delta] == 0) {
        return;
    }

    if (diamID[y_delta][x_delta] > 0) {
        connection.diamonds.insert(diamID[y_delta][x_delta]);
    }

    connection.id = connectionCount++;
    connection.to = nodeID[y][x];
    graph[nodeID[y_delta][x_delta]].push_back(connection);
}

void createNode(int y, int x) {
    uint8_t dirs[8] = {0, 1, 2, 3, 4, 5, 6, 7};

    for (uint8_t dir : dirs) {
        if (stopDirection[y][x] & (1u << dir)) {
            traverseLane(y, x, dir);
        }
    }
}

void createGraph() {
    for (int y = 0; y < ySize; ++y) {
        for (int x = 0; x < xSize; ++x) {
            createNode(y, x);
        }
    }
}

// Final Graph
// ===============================================================
void fillFinalGraph() {
    startGuardian.id = connectionCount++;
    startGuardian.to = nodeID[y_start][x_start];
    startGuardian.dir = 0;
    startGuardian.diamonds = set<int>();

    finalGraph.push_back({startGuardian.dir, startGuardian.diamonds, 0, vector<FinalConnection>()});
    tMap.insert(make_pair(startGuardian.id, finalGraph.size() - 1));

    for (auto &conVec : graph) {
        for (auto &con : conVec) {
            if (!con.diamonds.empty()) {
                finalGraph.push_back({con.dir, con.diamonds, 0, vector<FinalConnection>()});
                tMap.insert(make_pair(con.id, finalGraph.size() - 1));
            }
        }
    }
}

void findDiaConnections(queue<pair<Connection, int>> *nodeQueue, const Connection &outCon, int maxLen) {
    int node = outCon.to;

    bool visited[nodeCount];
    for (int i = 0; i < nodeCount; ++i) { visited[i] = false; }

    //ID, path
    queue<pair<int, string>> q;
    q.push(make_pair(node, ""));

    while (!q.empty()) {
        pair<int, string> entry = q.front();
        q.pop();

        int nid = entry.first;
        string path = entry.second;

        if (visited[nid])
            continue;

        visited[nid] = true;
        for (auto &con : graph[nid]) {
            if (con.diamonds.empty()) {
                if (path.length() < maxLen) {
                    string newPath = path + (char) (con.dir + '0');
                    q.push(make_pair(con.to, newPath));
                }
            } else {
                int nodeToID = tMap[con.id];
                FinalConnection connection = {nodeToID, path};
                finalGraph[tMap[outCon.id]].connections.push_back(connection);

                int newMaxPath = max(finalGraph[tMap[outCon.id]].maxPathFrom, maxLen - (long) path.length() - 1);
                finalGraph[tMap[outCon.id]].maxPathFrom = newMaxPath;

                if (maxLen > path.length()) {
                    nodeQueue->push(make_pair(con, maxLen - path.length() - 1));
                }
            }
        }
    }
}

void buildFinalGraph() {
    fillFinalGraph();
    queue<pair<Connection, int>> nodeQueue;
    set<int> visited;

    finalGraph[tMap[startGuardian.id]].maxPathFrom = expectedSteps;
    nodeQueue.push(make_pair(startGuardian, expectedSteps));
    while (!nodeQueue.empty()) {
        auto element = nodeQueue.front();
        nodeQueue.pop();

        auto insRes = visited.insert(element.first.id);
        if (!insRes.second)
            continue;

        findDiaConnections(&nodeQueue, element.first, element.second);
    }
}

// Find Answer
// ===============================================================
string searchGraph(int finalNodeID, string currentPath, set<int> diams, const int maxSteps) {
    if (currentPath.length() >= maxSteps) {
        return "";
    }

    currentPath += (char) (finalGraph[finalNodeID].dir + '0');
    diams.insert(finalGraph[finalNodeID].diamonds.begin(), finalGraph[finalNodeID].diamonds.end());

    if (diams.size() == diamCount) {
        return currentPath;
    }

    for (auto &con : finalGraph[finalNodeID].connections) {
        string result = searchGraph(con.to, currentPath + con.path, diams, maxSteps);
        if (!result.empty()) {
            return result;
        }
    }

    return "";
}

void findAnswer() {
    set<int> startSet = set<int>();
    string path = searchGraph(0, "", startSet, expectedSteps + 1);
    if (path.empty()) {
        cout << "BRAK" << endl;
    } else {
        cout << path.substr(1) << endl;
    }
}

int main() {
    ios_base::sync_with_stdio(false);

    processInput();
    createGraph();
    buildFinalGraph();
    findAnswer();

    return 0;
}
