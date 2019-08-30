// Arkadiusz Placha
#include <iostream>
#include <vector>
#include <set>
#include <queue>
#include <tuple>
#include <string>
#include <cstring>
#include <algorithm>

#define BOARD_MSIZE 200

#define SYM_WALL '#'
#define SYM_DIAM '+'
#define SYM_MINE '*'
#define SYM_HOLE 'O'
#define SYM_PCAR '.'
#define SYM_SPCE ' '

using namespace std;

struct Connection {
    int64_t id;

    int32_t to;
    uint8_t dir;
    set<int32_t> diamonds;
};


struct FinalNode;

struct FinalConnection {
    int64_t to;
    string path;
};

struct FinalPath {
    int64_t to;
    string path;
    set<int32_t> diams;
};

struct FinalNode {
    uint8_t dir;
    set<int32_t> diamonds;
    vector<FinalConnection> connections;

    int64_t maxPathFrom;
    vector<set<int64_t>> pathsMap;
    vector<FinalPath> pathsToDiams;
};

char board[BOARD_MSIZE][BOARD_MSIZE]; //Y, X
int32_t ySize, xSize;
int64_t expectedSteps;
queue<pair<Connection *, int64_t>> nodeQueue;

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
int32_t nodeID[BOARD_MSIZE][BOARD_MSIZE] = {0};
int32_t diamID[BOARD_MSIZE][BOARD_MSIZE] = {0};

int32_t y_start, x_start;
int32_t nodeCount = 0;
int32_t diamCount = 0;
vector<vector<Connection>> graph;
int64_t connectionCount = 0;


Connection startGuardian;
vector<FinalNode> finalGraph;
vector<int64_t> tMap;

// Initial Graph
// ===============================================================
void readInput() {
    cin >> ySize >> xSize;
    cin >> expectedSteps;
    cin.ignore(1);

    for (int32_t i = 0; i < ySize; i++) {
        string boardLine;
        getline(cin, boardLine);
        boardLine.copy(board[i], xSize);
    }
}

void processPoint(const int32_t &y, const int32_t &x) {
    int8_t dirs[8] = {0, 1, 2, 3, 4, 5, 6, 7};

    switch (board[y][x]) {
        case SYM_HOLE:
            stopDirection[y][x] = 0xFF;
            return;
        case SYM_SPCE:
            for (int8_t dir : dirs) {
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

    for (int32_t y = 0; y < ySize; ++y) {
        for (int32_t x = 0; x < xSize; ++x) {
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

void traverseLane(const int32_t &y, const int32_t &x, const uint8_t dir) {
    Connection connection{};
    connection.id = connectionCount++;
    connection.dir = dir;
    connection.diamonds = set<int32_t>();

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

void createNode(const int32_t &y, const int32_t &x) {
    int8_t dirs[8] = {0, 1, 2, 3, 4, 5, 6, 7};

    for (int8_t dir : dirs) {
        if (stopDirection[y][x] & (1u << dir)) {
            traverseLane(y, x, dir);
        }
    }
}

void createGraph() {
    for (int32_t y = 0; y < ySize; ++y) {
        for (int32_t x = 0; x < xSize; ++x) {
            createNode(y, x);
        }
    }
}

void createNewFinalNode(const Connection &con) {
    vector<set<int64_t>> diaToVector;
    for (int32_t i = 0; i <= diamCount; ++i) {
        diaToVector.emplace_back(set<int64_t>());
    }

    int64_t index = finalGraph.size();
    finalGraph.push_back({con.dir, con.diamonds, vector<FinalConnection>(),
                          0, diaToVector, vector<FinalPath>()});
    tMap[con.id] = index;
}

// Final Graph
// ===============================================================
void fillFinalGraph() {
    startGuardian.id = connectionCount++;
    startGuardian.to = nodeID[y_start][x_start];
    startGuardian.dir = 0;
    startGuardian.diamonds = set<int32_t>();
    tMap = vector<int64_t>(connectionCount + 1);

    createNewFinalNode(startGuardian);
    for (auto &conVec : graph) {
        for (auto &con : conVec) {
            if (!con.diamonds.empty()) {
                createNewFinalNode(con);
            }
        }
    }
}

void findDiaConnections(const Connection *outCon, const int64_t &maxLen) {
    int32_t node = outCon->to;
    set<int32_t> visited;

    //ID, path
    queue<pair<int32_t, string>> q;
    q.push(make_pair(node, ""));

    while (!q.empty()) {
        pair<int32_t, string> entry = q.front();
        q.pop();

        int nid = entry.first;
        string path = entry.second;

        auto insRes = visited.insert(nid);
        if (!insRes.second)
            continue;

        int64_t nodeOutID = tMap[outCon->id];
        for (auto &con : graph[nid]) {
            if (con.diamonds.empty()) {
                if (path.length() < maxLen) {
                    string newPath = path + (char) (con.dir + '0');
                    q.push(make_pair(con.to, newPath));
                }
            } else {
                int64_t nodeToID = tMap[con.id];
                FinalConnection connection = {nodeToID, path};
                finalGraph[nodeOutID].connections.push_back(connection);

                if (path.length() < maxLen) {
                    int64_t newMaxPath = max(finalGraph[nodeOutID].maxPathFrom, maxLen - (int64_t) path.length() - 1);
                    finalGraph[nodeOutID].maxPathFrom = newMaxPath;
                    nodeQueue.push(make_pair(&con, newMaxPath));
                }
            }
        }

    }
}

void buildFinalGraph() {
    fillFinalGraph();
    set<int64_t> visited;

    int64_t startId = tMap[startGuardian.id];
    finalGraph[startId].maxPathFrom = expectedSteps;
    nodeQueue.push(make_pair(&startGuardian, finalGraph[startId].maxPathFrom));
    while (!nodeQueue.empty()) {
        auto element = nodeQueue.front();
        nodeQueue.pop();

        auto insRes = visited.insert(element.first->id);
        if (!insRes.second)
            continue;

        findDiaConnections(element.first, element.second);
    }
}

// Min paths
// ===============================================================
void findMinPathsForNode(const int64_t &finalID) {
    auto compare = [](const pair<int64_t, int64_t> &lhs, const pair<int64_t, int64_t> &rhs) {
        return lhs.first >= rhs.first;
    };

    vector<FinalPath> pathsVec;
    priority_queue<pair<int64_t, int64_t>, vector<pair<int64_t, int64_t>>, decltype(compare)> queue(compare);
    set<int64_t> visited;

    pathsVec.push_back({finalID, "", set<int32_t>()});
    queue.push(make_pair(0, 0));
    while (!queue.empty()) {
        auto elemQ = queue.top();
        queue.pop();

        int64_t pathToDateLen = pathsVec[elemQ.second].path.length();

        auto insRes = visited.insert(pathsVec[elemQ.second].to);
        if (!insRes.second)
            continue;

        if (pathToDateLen > finalGraph[finalID].maxPathFrom)
            continue;


        auto &diamondsSet = finalGraph[pathsVec[elemQ.second].to].diamonds;
        set<int32_t> newDiams;
        set_difference(diamondsSet.begin(), diamondsSet.end(), pathsVec[elemQ.second].diams.begin(),
                       pathsVec[elemQ.second].diams.end(), inserter(newDiams, newDiams.end()));

        if (!newDiams.empty()) {
            pathsVec[elemQ.second].diams.insert(newDiams.begin(), newDiams.end());
            int64_t index = finalGraph[finalID].pathsToDiams.size();
            finalGraph[finalID].pathsToDiams.push_back(pathsVec[elemQ.second]);

            for (int dia : newDiams) {
                finalGraph[finalID].pathsMap[dia].insert(index);
            }
        }

        for (auto &con: finalGraph[pathsVec[elemQ.second].to].connections) {
            int64_t newPathLen = pathToDateLen + con.path.length() + 1;
            if (newPathLen <= finalGraph[finalID].maxPathFrom) {
                char dir = (char) (finalGraph[con.to].dir + '0');

                string newP = pathsVec[elemQ.second].path + con.path + dir;
                pathsVec.push_back({con.to, newP, pathsVec[elemQ.second].diams});
                queue.push(make_pair(newP.size(), pathsVec.size() - 1));
            }
        }
    }
}

void calculateMinPaths() {
    for (int64_t i = 0; i < finalGraph.size(); ++i) {
        findMinPathsForNode(i);
    }
}


// Find Answer
// ===============================================================
int64_t searchGraph(const int64_t &finalNodeID, char *currentPath, const int64_t &currentPathSize,
                    const set<int32_t> &diamsToFind, const int64_t &maxSteps) {
    if (currentPathSize >= maxSteps) {
        return 0;
    }
    if (diamsToFind.empty()) {
        return currentPathSize;
    }


    FinalNode &finalNode = finalGraph[finalNodeID];
    set<int32_t> pathsToCheck;
    for (auto &diam : diamsToFind) {
        bool inserted = false;

        for (auto &con : finalNode.pathsMap[diam]) {
            auto &finPath = finalNode.pathsToDiams[con];
            if (currentPathSize + finPath.path.length() <= maxSteps) {
                pathsToCheck.insert(con);
                inserted = true;
            }
        }

        if (!inserted) {
            return 0;
        }
    }

    for (int64_t pathID : pathsToCheck) {
        auto &finPath = finalNode.pathsToDiams[pathID];
        set<int32_t> newDiams;
        set_difference(diamsToFind.begin(), diamsToFind.end(), finPath.diams.begin(),
                       finPath.diams.end(), inserter(newDiams, newDiams.end()));

        strcpy((currentPath + currentPathSize), finPath.path.c_str());
        if (newDiams.empty()) {
            return currentPathSize + finPath.path.length();
        }

        int64_t res = searchGraph(finPath.to, currentPath, currentPathSize + finPath.path.length(), newDiams, maxSteps);
        if (res > 0) {
            return res;
        }
    }

    return 0;
}

void findAnswer() {
    set<int32_t> toFindSet;
    for (int32_t i = 1; i <= diamCount; ++i) {
        toFindSet.insert(i);
    }

    char finalPath[expectedSteps + 1];
    int64_t path = searchGraph(0, finalPath, 0, toFindSet, expectedSteps);
    if (path == 0) {
        cout << "BRAK" << endl;
    } else {
        for (int64_t i = 0; i < path; ++i) {
            cout << finalPath[i];
        }
    }
}

int main() {
    ios_base::sync_with_stdio(false);

    processInput();
    createGraph();
    buildFinalGraph();
    calculateMinPaths();
    findAnswer();

    return 0;
}
