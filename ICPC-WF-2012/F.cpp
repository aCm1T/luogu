#include <bits/stdc++.h>
using namespace std;

struct Cost {
    int key;
    int ring;
};

static constexpr int INF = 1'000'000'000;

bool better(const Cost& a, const Cost& b) {
    if (a.key != b.key) {
        return a.key < b.key;
    }
    return a.ring < b.ring;
}

Cost addCost(const Cost& a, const Cost& b) {
    if (a.key >= INF || b.key >= INF) {
        return {INF, INF};
    }
    return {a.key + b.key, a.ring + b.ring};
}

class Solver {
public:
    bool ringSeen[26]{};
    int keysOnRing[26][2]{};
    vector<pair<int, int>> rawEdges;

    Cost solve() {
        buildCompressedGraph();

        totalKeys[0] = 0;
        totalKeys[1] = 0;
        for (int i = 0; i < n; ++i) {
            totalKeys[0] += keyCnt[i][0];
            totalKeys[1] += keyCnt[i][1];
        }

        parent.assign(n, -1);
        vector<array<array<Cost, 4>, 3>> rootDp;
        for (int i = 0; i < n; ++i) {
            if (parent[i] == -1) {
                rootDp.push_back(dfs(i, -2));
            }
        }

        array<Cost, 4> global{};
        for (Cost& c : global) {
            c = {INF, INF};
        }
        global[0] = {0, 0};

        for (auto& root : rootDp) {
            array<Cost, 4> next{};
            for (Cost& c : next) {
                c = {INF, INF};
            }
            for (int mask = 0; mask < 4; ++mask) {
                if (global[mask].key >= INF) {
                    continue;
                }
                for (int state = 0; state < 3; ++state) {
                    for (int subMask = 0; subMask < 4; ++subMask) {
                        Cost cur = root[state][subMask];
                        if (cur.key >= INF) {
                            continue;
                        }
                        if (isOwner(state)) {
                            ++cur.ring;  // This root starts one kept component.
                        }
                        int combined = mask | subMask;
                        Cost candidate = addCost(global[mask], cur);
                        if (better(candidate, next[combined])) {
                            next[combined] = candidate;
                        }
                    }
                }
            }
            global = next;
        }

        int requiredMask = 0;
        int owners = 0;
        if (totalKeys[0] > 0) {
            requiredMask |= 1;
            ++owners;
        }
        if (totalKeys[1] > 0) {
            requiredMask |= 2;
            ++owners;
        }

        Cost answer = {INF, INF};
        for (int mask = 0; mask < 4; ++mask) {
            if ((mask & requiredMask) != requiredMask) {
                continue;
            }
            Cost cur = global[mask];
            if (cur.key >= INF) {
                continue;
            }
            cur.ring -= owners;
            if (better(cur, answer)) {
                answer = cur;
            }
        }
        return answer;
    }

private:
    int n = 0;
    int totalKeys[2]{};
    vector<array<int, 2>> keyCnt;
    vector<vector<int>> graph;
    vector<int> parent;

    static bool isOwner(int state) {
        return state == 1 || state == 2;
    }

    void buildCompressedGraph() {
        vector<int> id(26, -1);
        n = 0;
        for (int i = 0; i < 26; ++i) {
            if (ringSeen[i]) {
                id[i] = n++;
            }
        }

        keyCnt.assign(n, {0, 0});
        for (int i = 0; i < 26; ++i) {
            if (id[i] != -1) {
                keyCnt[id[i]][0] = keysOnRing[i][0];
                keyCnt[id[i]][1] = keysOnRing[i][1];
            }
        }

        graph.assign(n, {});
        for (auto [a, b] : rawEdges) {
            int u = id[a];
            int v = id[b];
            graph[u].push_back(v);
            graph[v].push_back(u);
        }
    }

    bool stateAllowed(int state) const {
        if (state == 1) {
            return totalKeys[0] > 0;
        }
        if (state == 2) {
            return totalKeys[1] > 0;
        }
        return true;
    }

    int keyCostForState(int u, int state) const {
        int adam = keyCnt[u][0];
        int brenda = keyCnt[u][1];
        if (state == 0) {
            return 2 * (adam + brenda);
        }
        if (state == 1) {
            return 2 * brenda;
        }
        return 2 * adam;
    }

    array<array<Cost, 4>, 3> dfs(int u, int p) {
        parent[u] = p;

        array<array<Cost, 4>, 3> dp{};
        for (auto& byMask : dp) {
            for (Cost& c : byMask) {
                c = {INF, INF};
            }
        }

        for (int state = 0; state < 3; ++state) {
            if (!stateAllowed(state)) {
                continue;
            }
            int mask = 0;
            if (state == 1) {
                mask = 1;
            } else if (state == 2) {
                mask = 2;
            }
            dp[state][mask] = {keyCostForState(u, state), 0};
        }

        for (int v : graph[u]) {
            if (v == p) {
                continue;
            }
            auto child = dfs(v, u);
            array<array<Cost, 4>, 3> next{};
            for (auto& byMask : next) {
                for (Cost& c : byMask) {
                    c = {INF, INF};
                }
            }

            for (int state = 0; state < 3; ++state) {
                for (int mask = 0; mask < 4; ++mask) {
                    if (dp[state][mask].key >= INF) {
                        continue;
                    }
                    for (int childState = 0; childState < 3; ++childState) {
                        for (int childMask = 0; childMask < 4; ++childMask) {
                            Cost sub = child[childState][childMask];
                            if (sub.key >= INF) {
                                continue;
                            }

                            if (childState != state) {
                                ++sub.ring;  // Disconnect this tree edge.
                            }
                            if (isOwner(childState) && childState != state) {
                                ++sub.ring;  // Child begins a new owner component.
                            }

                            int combined = mask | childMask;
                            Cost candidate = addCost(dp[state][mask], sub);
                            if (better(candidate, next[state][combined])) {
                                next[state][combined] = candidate;
                            }
                        }
                    }
                }
            }
            dp = next;
        }

        return dp;
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    string line;
    int caseNo = 1;
    Solver solver;

    while (cin >> line) {
        if (line == "0") {
            Cost answer = solver.solve();
            cout << "Case " << caseNo++ << ": ";
            if (answer.key >= INF) {
                cout << "impossible\n";
            } else {
                cout << answer.key << ' ' << answer.ring << '\n';
            }
            solver = Solver();
            continue;
        }

        char a = line[0];
        char b = line[1];
        bool lowerA = islower(static_cast<unsigned char>(a));
        bool lowerB = islower(static_cast<unsigned char>(b));

        if (lowerA && lowerB) {
            int u = a - 'a';
            int v = b - 'a';
            solver.ringSeen[u] = true;
            solver.ringSeen[v] = true;
            solver.rawEdges.push_back({u, v});
        } else {
            char ring = lowerA ? a : b;
            char key = lowerA ? b : a;
            int r = ring - 'a';
            int owner = key <= 'M' ? 0 : 1;
            solver.ringSeen[r] = true;
            ++solver.keysOnRing[r][owner];
        }
    }

    return 0;
}
