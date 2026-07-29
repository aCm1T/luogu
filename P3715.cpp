#include <bits/stdc++.h>
using namespace std;

static const int MOD = 1e9 + 7;
static const int MAXS = 105; // AC nodes: sum of forbidden lengths + 1

struct AC {
    int tot = 1;
    int ch[MAXS][26]{}, fail[MAXS]{}, danger[MAXS]{};

    void insert(const string &s) {
        int u = 1;
        for (char c : s) {
            int x = c - 'a';
            if (!ch[u][x]) ch[u][x] = ++tot;
            u = ch[u][x];
        }
        danger[u] = 1;
    }

    void build() {
        queue<int> q;
        for (int c = 0; c < 26; ++c) {
            if (ch[1][c]) {
                fail[ch[1][c]] = 1;
                q.push(ch[1][c]);
            } else {
                ch[1][c] = 1;
            }
        }
        while (!q.empty()) {
            int u = q.front();
            q.pop();
            danger[u] |= danger[fail[u]];
            for (int c = 0; c < 26; ++c) {
                if (ch[u][c]) {
                    fail[ch[u][c]] = ch[fail[u]][c];
                    q.push(ch[u][c]);
                } else {
                    ch[u][c] = ch[fail[u]][c];
                }
            }
        }
    }

    // Walk word from state u; return end state, or -1 if any danger is hit.
    int go(int u, const string &w) const {
        if (danger[u]) return -1;
        for (char c : w) {
            u = ch[u][c - 'a'];
            if (danger[u]) return -1;
        }
        return u;
    }
};

struct Mat {
    int n = 0;
    vector<vector<int>> a;
    explicit Mat(int n = 0, bool ident = false) : n(n), a(n, vector<int>(n, 0)) {
        if (ident) for (int i = 0; i < n; ++i) a[i][i] = 1;
    }
};

static Mat operator*(const Mat &A, const Mat &B) {
    Mat C(A.n);
    for (int i = 0; i < A.n; ++i)
        for (int k = 0; k < A.n; ++k) if (A.a[i][k])
            for (int j = 0; j < A.n; ++j) if (B.a[k][j])
                C.a[i][j] = (C.a[i][j] + 1LL * A.a[i][k] * B.a[k][j]) % MOD;
    return C;
}

static Mat mpow(Mat base, long long e) {
    Mat res(base.n, true);
    while (e > 0) {
        if (e & 1) res = res * base;
        base = base * base;
        e >>= 1;
    }
    return res;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n, m;
    long long L;
    cin >> n >> m >> L;
    vector<string> words(n);
    for (int i = 0; i < n; ++i) cin >> words[i];
    AC ac;
    for (int i = 0; i < m; ++i) {
        string t;
        cin >> t;
        ac.insert(t);
    }
    ac.build();

    const int S = ac.tot;
    // to[u][i] = end state after appending words[i] from u, or -1
    vector<vector<int>> to(S + 1, vector<int>(n, -1));
    for (int u = 1; u <= S; ++u) {
        if (ac.danger[u]) continue;
        for (int i = 0; i < n; ++i) to[u][i] = ac.go(u, words[i]);
    }

    auto solveDP = [&]() -> int {
        int LL = (int)L;
        vector<vector<int>> dp(LL + 1, vector<int>(S + 1, 0));
        dp[0][1] = 1;
        for (int len = 0; len < LL; ++len) {
            for (int u = 1; u <= S; ++u) if (dp[len][u]) {
                for (int i = 0; i < n; ++i) {
                    int v = to[u][i];
                    int wlen = (int)words[i].size();
                    if (v != -1 && len + wlen <= LL)
                        dp[len + wlen][v] = (dp[len + wlen][v] + dp[len][u]) % MOD;
                }
            }
        }
        int ans = 0;
        for (int u = 1; u <= S; ++u) ans = (ans + dp[LL][u]) % MOD;
        return ans;
    };

    auto solveMat = [&]() -> int {
        // Companion matrix on (f[i-1][1..S], f[i][1..S]).
        // Index: 0..S-1 -> f[i-1][u+1], S..2S-1 -> f[i][u+1]
        int N = 2 * S;
        Mat M(N);
        // shift: f[i] -> f[i] in next first half
        for (int u = 0; u < S; ++u) M.a[S + u][u] = 1;
        for (int u = 1; u <= S; ++u) {
            if (ac.danger[u]) continue;
            for (int i = 0; i < n; ++i) {
                int v = to[u][i];
                if (v == -1) continue;
                int wlen = (int)words[i].size();
                if (wlen == 1) {
                    // f[i][u] -> f[i+1][v]  => second -> second
                    M.a[S + (u - 1)][S + (v - 1)] = (M.a[S + (u - 1)][S + (v - 1)] + 1) % MOD;
                } else if (wlen == 2) {
                    // f[i-1][u] -> f[i+1][v] => first -> second
                    M.a[u - 1][S + (v - 1)] = (M.a[u - 1][S + (v - 1)] + 1) % MOD;
                }
            }
        }
        // start = (f[-1]=0, f[0] with root=1)
        vector<int> cur(N, 0);
        cur[S + 0] = 1; // f[0][1] = 1  (root is node 1 -> index 0 in block)
        Mat P = mpow(M, L);
        int ans = 0;
        for (int j = 0; j < N; ++j) if (cur[j]) {
            for (int u = 0; u < S; ++u) {
                // after L steps, second half is f[L]
                ans = (ans + 1LL * cur[j] * P.a[j][S + u]) % MOD;
            }
        }
        return ans;
    };

    // Points 1–6: L ≤ 100 → DP. Points 7–10: basic words length ≤ 2 → matrix expo.
    if (L <= 100) {
        cout << solveDP() << '\n';
    } else {
        cout << solveMat() << '\n';
    }
    return 0;
}
