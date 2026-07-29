#include <algorithm>
#include <iostream>
#include <vector>

using namespace std;

const int INF = 1000000;

void floyd(vector<vector<int>>& adj) {
    int n = (int)adj.size();
    for (int k = 0; k < n; ++k) {
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < n; ++j) {
                if (adj[i][k] + adj[k][j] < adj[i][j]) {
                    adj[i][j] = adj[i][k] + adj[k][j];
                }
            }
        }
    }
}

int countBits(int n) {
    int cnt = 0;
    while (n > 0) {
        if (n & 1) ++cnt;
        n >>= 1;
    }
    return cnt;
}

vector<vector<int>> getTSP(const vector<vector<int>>& adj, int start) {
    int n = (int)adj.size() - 2;
    int maxBits = (n + 1) / 2;
    vector<vector<int>> table(1 << n, vector<int>(n, INF));

    for (int i = 0; i < n; ++i) {
        table[1 << i][i] = adj[start][i + 1];
    }

    for (int mask = 0; mask < (int)table.size(); ++mask) {
        int bits = countBits(mask);
        if (bits > maxBits) continue;

        int saveRow = mask;
        int j = 0;
        while (saveRow > 0) {
            if (saveRow & 1) {
                int rest = mask - (1 << j);
                int best = INF;
                for (int k = 0; k < n; ++k) {
                    if (table[rest][k] + adj[k + 1][j + 1] < best) {
                        best = table[rest][k] + adj[k + 1][j + 1];
                    }
                }
                if (best < table[mask][j]) table[mask][j] = best;
            }
            ++j;
            saveRow >>= 1;
        }
    }

    return table;
}

int getAns(const vector<vector<int>>& adj) {
    vector<vector<int>> startFirst = getTSP(adj, 0);
    vector<vector<int>> startLast = getTSP(adj, (int)adj.size() - 1);

    int n = (int)adj.size() - 2;
    int limit = (n % 2 == 0) ? (int)startFirst.size() / 2 : (int)startFirst.size();
    int sizeHalf = n / 2;
    int best = INF;

    for (int mask = 0; mask < limit; ++mask) {
        if (countBits(mask) != sizeHalf) continue;

        int other = (int)startFirst.size() - 1 - mask;

        int bestThere = INF;
        for (int j = 0; j < n; ++j) {
            for (int k = 0; k < n; ++k) {
                int curPath = startFirst[mask][j] + startLast[other][k] + adj[j + 1][k + 1];
                if (curPath < bestThere) bestThere = curPath;
            }
        }

        int bestBack = INF;
        for (int j = 0; j < n; ++j) {
            for (int k = 0; k < n; ++k) {
                int curPath = startLast[mask][j] + startFirst[other][k] + adj[k + 1][j + 1];
                if (curPath < bestBack) bestBack = curPath;
            }
        }

        int cur = bestThere + bestBack;
        if (cur < best) best = cur;
    }

    return best;
}

int solve(const vector<vector<int>>& adj) {
    if ((int)adj.size() == 3) {
        return 2 * (adj[0][1] + adj[1][2]);
    }
    return getAns(adj);
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n, m;
    int loop = 1;
    while (cin >> n >> m) {
        vector<vector<int>> adj(n, vector<int>(n, INF));
        for (int i = 0; i < n; ++i) adj[i][i] = 0;

        for (int i = 0; i < m; ++i) {
            int v1, v2, dist;
            cin >> v1 >> v2 >> dist;
            adj[v1][v2] = dist;
            adj[v2][v1] = dist;
        }

        floyd(adj);
        cout << "Case " << loop << ": " << solve(adj) << '\n';
        ++loop;
    }

    return 0;
}
