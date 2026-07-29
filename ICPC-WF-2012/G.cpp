#include <bits/stdc++.h>
using namespace std;

struct Junction {
    int x, y, z, holes, id;
};

struct State {
    double total;
    int vertex;
    int slot;
    bool canLeave;

    bool operator<(const State& other) const {
        return total > other.total;
    }
};

static constexpr double INF = 1e100;
static constexpr double EPS = 1e-10;

int n, m;
vector<Junction> pts;
vector<vector<int>> adjList;
vector<vector<double>> allDist;

double dist3(const Junction& a, const Junction& b) {
    long long dx = a.x - b.x;
    long long dy = a.y - b.y;
    long long dz = a.z - b.z;
    return sqrt(static_cast<double>(dx * dx + dy * dy + dz * dz));
}

void dfsComponent(int u, int comp, int limit, vector<int>& component) {
    component[u] = comp;
    for (int v : adjList[u]) {
        if (v < limit && component[v] == -1) {
            dfsComponent(v, comp, limit, component);
        }
    }
}

double evaluateLevel(int start, int finish, int limit) {
    vector<int> component(limit, -1);
    int compCount = 0;
    for (int i = 0; i < limit; ++i) {
        if (component[i] == -1) {
            dfsComponent(i, compCount++, limit, component);
        }
    }

    vector<vector<int>> compVertices(compCount);
    vector<int> compHoles(compCount, 0);
    for (int i = 0; i < limit; ++i) {
        compVertices[component[i]].push_back(i);
        compHoles[component[i]] += pts[i].holes;
    }

    vector<array<double, 2>> best(limit);
    for (int i = 0; i < limit; ++i) {
        best[i] = {INF, INF};
    }

    priority_queue<State> pq;
    int startSlot = pts[start].holes > 0 ? 1 : 0;
    double startCost = compHoles[component[start]] / 2.0;
    best[start][startSlot] = startCost;
    pq.push({startCost, start, startSlot, pts[start].holes > 0});

    while (!pq.empty()) {
        State cur = pq.top();
        pq.pop();
        if (cur.total > best[cur.vertex][cur.slot] + EPS) {
            continue;
        }

        if (cur.vertex == finish) {
            return cur.total;
        }

        int myComp = component[cur.vertex];

        for (int v : compVertices[myComp]) {
            int slot = pts[v].holes > 0 ? 1 : 0;
            double nextTotal = cur.total;
            if (nextTotal + EPS < best[v][slot]) {
                best[v][slot] = nextTotal;
                pq.push({nextTotal, v, slot, pts[v].holes > 0});
            }
        }

        if (!cur.canLeave) {
            continue;
        }

        for (int v = 0; v < limit; ++v) {
            if (component[v] == myComp || pts[v].holes == 0) {
                continue;
            }
            double nextTotal = cur.total + allDist[cur.vertex][v] - 1.0
                               + compHoles[component[v]] / 2.0;
            int slot = 1;
            if (nextTotal + EPS < best[v][slot]) {
                best[v][slot] = nextTotal;
                pq.push({nextTotal, v, slot, pts[v].holes > 1});
            }
        }
    }

    return INF;
}

double solveCase(int start, int finish, int minZ) {
    int limit = 0;
    while (limit < n && pts[limit].z <= minZ) {
        ++limit;
    }

    double answer = INF;
    while (true) {
        answer = min(answer, evaluateLevel(start, finish, limit));
        if (limit == n) {
            break;
        }
        int nextZ = pts[limit].z;
        while (limit < n && pts[limit].z == nextZ) {
            ++limit;
        }
    }
    return answer;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int caseNo = 1;
    while (cin >> n >> m) {
        pts.resize(n);
        int minZ = -10000;
        for (int i = 0; i < n; ++i) {
            cin >> pts[i].x >> pts[i].y >> pts[i].z >> pts[i].holes;
            pts[i].id = i + 1;
            if ((i == 0 || i == n - 1) && pts[i].z > minZ) {
                minZ = pts[i].z;
            }
        }

        sort(pts.begin(), pts.end(), [](const Junction& a, const Junction& b) {
            return a.z < b.z;
        });

        vector<int> lookup(n + 1);
        for (int i = 0; i < n; ++i) {
            lookup[pts[i].id] = i;
        }

        allDist.assign(n, vector<double>(n, 0.0));
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < n; ++j) {
                allDist[i][j] = dist3(pts[i], pts[j]);
            }
        }

        adjList.assign(n, {});
        for (int i = 0; i < m; ++i) {
            int a, b;
            cin >> a >> b;
            a = lookup[a];
            b = lookup[b];
            adjList[a].push_back(b);
            adjList[b].push_back(a);
        }

        double answer = solveCase(lookup[1], lookup[n], minZ);
        cout << "Case " << caseNo++ << ": ";
        if (answer >= INF / 2) {
            cout << "impossible\n";
        } else {
            cout << fixed << setprecision(4) << answer + 0.0 << '\n';
        }
    }

    return 0;
}
