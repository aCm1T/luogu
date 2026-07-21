#include <bits/stdc++.h>
using namespace std;

static const int MOD = 1000000007;

struct Val {
    int x, y;          // connected: x=z; open: x=d, y=e
    unsigned char open;
};

struct State {
    int a, d, e;
    unsigned char con;
};

struct Trans {
    // type 0: always connected, x[0..2] is one row
    // type 1: preserves type, x[0]=p, x[1..4] is 2x2 block
    // type 2: always open, x[0..2] row d, x[3..5] row e
    int x[6];
    unsigned char type;
};

int K1, K2;

inline int addmod(long long x) { return int(x % MOD); }

inline State identityState() {
    return State{1, 0, 1, 1};
}

inline State toState(const Val &v) {
    if (!v.open) return State{v.x, 0, v.x, 1};
    return State{0, v.x, v.y, 0};
}

inline bool sameVal(const Val &a, const Val &b) {
    return a.open == b.open && a.x == b.x && (!a.open || a.y == b.y);
}

inline State seriesMerge(const State &L, const State &R) {
    long long a = 1LL * L.a * R.a + 1LL * K1 * L.d % MOD * R.d;
    long long d = 1LL * L.a * R.d + 1LL * L.d * R.a + 1LL * K2 * L.d % MOD * R.d;
    int e;
    if (L.con && R.con) {
        e = int(1LL * L.a * R.a % MOD);
    } else if (L.con && !R.con) {
        e = int(1LL * L.a * R.e % MOD);
    } else if (!L.con && R.con) {
        e = int(1LL * L.e * R.a % MOD);
    } else {
        e = int(a % MOD);
    }
    return State{int(a % MOD), int(d % MOD), e, (unsigned char)(L.con && R.con)};
}

inline Trans transIdentity() {
    Trans t{};
    t.type = 1;
    t.x[0] = 1;
    t.x[1] = 1;
    t.x[4] = 1;
    return t;
}

inline int mul2(int a, int b, int c, int d) {
    return int((1LL * a * b + 1LL * c * d) % MOD);
}

inline Trans compose(const Trans &A, const Trans &B) {
    // A after B
    Trans C{};
    if (A.type == 0) {
        C.type = 0;
        if (B.type == 0) {
            C.x[0] = int(1LL * A.x[0] * B.x[0] % MOD);
            C.x[1] = int(1LL * A.x[0] * B.x[1] % MOD);
            C.x[2] = int(1LL * A.x[0] * B.x[2] % MOD);
        } else if (B.type == 1) {
            C.x[0] = int(1LL * A.x[0] * B.x[0] % MOD);
            C.x[1] = mul2(A.x[1], B.x[1], A.x[2], B.x[3]);
            C.x[2] = mul2(A.x[1], B.x[2], A.x[2], B.x[4]);
        } else {
            for (int j = 0; j < 3; ++j)
                C.x[j] = mul2(A.x[1], B.x[j], A.x[2], B.x[j + 3]);
        }
    } else if (A.type == 1) {
        if (B.type == 0) {
            C.type = 0;
            C.x[0] = int(1LL * A.x[0] * B.x[0] % MOD);
            C.x[1] = int(1LL * A.x[0] * B.x[1] % MOD);
            C.x[2] = int(1LL * A.x[0] * B.x[2] % MOD);
        } else if (B.type == 1) {
            C.type = 1;
            C.x[0] = int(1LL * A.x[0] * B.x[0] % MOD);
            C.x[1] = mul2(A.x[1], B.x[1], A.x[2], B.x[3]);
            C.x[2] = mul2(A.x[1], B.x[2], A.x[2], B.x[4]);
            C.x[3] = mul2(A.x[3], B.x[1], A.x[4], B.x[3]);
            C.x[4] = mul2(A.x[3], B.x[2], A.x[4], B.x[4]);
        } else {
            C.type = 2;
            for (int j = 0; j < 3; ++j) {
                C.x[j] = mul2(A.x[1], B.x[j], A.x[2], B.x[j + 3]);
                C.x[j + 3] = mul2(A.x[3], B.x[j], A.x[4], B.x[j + 3]);
            }
        }
    } else {
        C.type = 2;
        if (B.type == 0) {
            for (int j = 0; j < 3; ++j) {
                C.x[j] = int(1LL * A.x[0] * B.x[j] % MOD);
                C.x[j + 3] = int(1LL * A.x[3] * B.x[j] % MOD);
            }
        } else if (B.type == 1) {
            C.x[0] = int(1LL * A.x[0] * B.x[0] % MOD);
            C.x[3] = int(1LL * A.x[3] * B.x[0] % MOD);
            C.x[1] = mul2(A.x[1], B.x[1], A.x[2], B.x[3]);
            C.x[2] = mul2(A.x[1], B.x[2], A.x[2], B.x[4]);
            C.x[4] = mul2(A.x[4], B.x[1], A.x[5], B.x[3]);
            C.x[5] = mul2(A.x[4], B.x[2], A.x[5], B.x[4]);
        } else {
            for (int j = 0; j < 3; ++j) {
                C.x[j] = mul2(A.x[1], B.x[j], A.x[2], B.x[j + 3]);
                C.x[j + 3] = mul2(A.x[4], B.x[j], A.x[5], B.x[j + 3]);
            }
        }
    }
    return C;
}

inline Val evalBase(const Trans &T) {
    if (T.type == 2) return Val{T.x[0], T.x[3], 1};
    return Val{T.x[0], 0, 0};
}

inline Trans makeLeaf(bool visible) {
    Trans t{};
    if (visible) {
        t.type = 2;
        t.x[0] = 1;
        t.x[3] = 1;
    } else {
        t.type = 0;
        t.x[0] = 1;
    }
    return t;
}

inline Trans makeInternal(bool visible, const State &L) {
    Trans t{};
    if (!visible) {
        t.type = 0;
        if (L.con) {
            t.x[0] = L.a;
            t.x[2] = L.a;
        } else {
            t.x[0] = L.e;
            t.x[1] = int(1LL * K1 * L.d % MOD);
        }
    } else {
        if (L.con) {
            t.type = 1;
            t.x[0] = L.a;
            t.x[1] = L.a;
            t.x[4] = L.a;
        } else {
            t.type = 2;
            t.x[0] = L.d;
            t.x[1] = int((L.a + 1LL * K2 * L.d) % MOD);
            t.x[3] = L.e;
            t.x[4] = int(1LL * K1 * L.d % MOD);
        }
    }
    return t;
}

inline Val staticNode(const State &children, bool visible) {
    if (!visible) return Val{children.e, 0, 0};
    if (children.con) return Val{children.a, 0, 0};
    return Val{children.d, children.e, 1};
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int T;
    cin >> T;
    while (T--) {
        int n, q;
        long long k;
        cin >> n >> k >> q;
        int kmod = int(k % MOD);
        K1 = int((k - 1) % MOD);
        K2 = int((k - 2) % MOD);

        vector<int> par(n + 1, 0), sz(n + 1, 1), heavy(n + 1, 0);
        for (int i = 2; i <= n; ++i) cin >> par[i];
        for (int i = n; i >= 2; --i) sz[par[i]] += sz[i];

        // Root is deliberately excluded from heavy chains.
        for (int u = 2; u <= n; ++u) {
            int best = 0;
            int end = u + sz[u];
            for (int v = u + 1; v < end; v += sz[v]) {
                if (!best || sz[v] > sz[best]) best = v;
            }
            heavy[u] = best;
        }

        vector<unsigned char> visible(n + 1, 1);
        vector<Val> dp(n + 1);

        // Initial DP, bottom-up.
        for (int u = n; u >= 2; --u) {
            if (heavy[u] == 0) {
                dp[u] = Val{1, 1, 1}; // visible atomic edge
            } else {
                State all = identityState();
                int end = u + sz[u];
                for (int v = u + 1; v < end; v += sz[v])
                    all = seriesMerge(all, toState(dp[v]));
                dp[u] = staticNode(all, true);
            }
        }

        // Build light-child aggregate trees. Root includes all children.
        vector<int> aggCnt(n + 1, 0), aggS(n + 1, 0), aggOff(n + 1, -1), aggPos(n + 1, -1);
        long long totalAgg = 0;
        for (int u = 1; u <= n; ++u) {
            int cnt = 0;
            int end = u + sz[u];
            for (int v = u + 1; v < end; v += sz[v]) {
                if (u == 1 || v != heavy[u]) ++cnt;
            }
            aggCnt[u] = cnt;
            if (cnt) {
                int s = 1;
                while (s < cnt) s <<= 1;
                aggS[u] = s;
                aggOff[u] = int(totalAgg);
                totalAgg += 2LL * s;
            }
        }
        vector<State> aggTree((size_t)totalAgg, identityState());
        for (int u = 1; u <= n; ++u) if (aggCnt[u]) {
            int idx = 0;
            int end = u + sz[u];
            int off = aggOff[u], s = aggS[u];
            for (int v = u + 1; v < end; v += sz[v]) {
                if (u == 1 || v != heavy[u]) {
                    aggPos[v] = idx;
                    aggTree[off + s + idx] = toState(dp[v]);
                    ++idx;
                }
            }
            for (int i = s - 1; i >= 1; --i)
                aggTree[off + i] = seriesMerge(aggTree[off + (i << 1)], aggTree[off + (i << 1 | 1)]);
        }

        auto getAgg = [&](int u) -> State {
            if (!aggCnt[u]) return identityState();
            return aggTree[aggOff[u] + 1];
        };

        auto updateAgg = [&](int child, const Val &nv) {
            int p = par[child];
            int off = aggOff[p], s = aggS[p];
            int i = s + aggPos[child];
            aggTree[off + i] = toState(nv);
            for (i >>= 1; i; i >>= 1)
                aggTree[off + i] = seriesMerge(aggTree[off + (i << 1)], aggTree[off + (i << 1 | 1)]);
        };

        // Heavy chains.
        vector<int> top(n + 1, 0), cpos(n + 1, 0), chainLen(n + 1, 0), chainS(n + 1, 0), chainOff(n + 1, -1);
        long long totalChain = 0;
        for (int u = 2; u <= n; ++u) {
            if (par[u] == 1 || heavy[par[u]] != u) {
                int len = 0;
                for (int x = u; x; x = heavy[x]) {
                    top[x] = u;
                    cpos[x] = len++;
                }
                chainLen[u] = len;
                int s = 1;
                while (s < len) s <<= 1;
                chainS[u] = s;
                chainOff[u] = int(totalChain);
                totalChain += 2LL * s;
            }
        }
        vector<Trans> chainTree((size_t)totalChain, transIdentity());

        vector<Trans> nodeMat(n + 1);
        for (int u = 2; u <= n; ++u) {
            if (heavy[u] == 0) nodeMat[u] = makeLeaf(true);
            else nodeMat[u] = makeInternal(true, getAgg(u));
            int h = top[u];
            chainTree[chainOff[h] + chainS[h] + cpos[u]] = nodeMat[u];
        }
        for (int h = 2; h <= n; ++h) if (chainLen[h]) {
            int off = chainOff[h], s = chainS[h];
            for (int i = s - 1; i >= 1; --i)
                chainTree[off + i] = compose(chainTree[off + (i << 1)], chainTree[off + (i << 1 | 1)]);
            dp[h] = evalBase(chainTree[off + 1]);
        }

        auto updateMatrix = [&](int u, const Trans &nm) {
            nodeMat[u] = nm;
            int h = top[u], off = chainOff[h], s = chainS[h];
            int i = s + cpos[u];
            chainTree[off + i] = nm;
            for (i >>= 1; i; i >>= 1)
                chainTree[off + i] = compose(chainTree[off + (i << 1)], chainTree[off + (i << 1 | 1)]);
        };

        auto headValue = [&](int h) -> Val {
            return evalBase(chainTree[chainOff[h] + 1]);
        };

        auto answer = [&]() -> int {
            State root = getAgg(1);
            return int(1LL * kmod * root.e % MOD);
        };

        cout << answer() << '\n';
        while (q--) {
            int v;
            cin >> v;
            visible[v] ^= 1;
            int cur = v;
            while (true) {
                int h = top[cur];
                Val oldHead = dp[h];
                Trans nm;
                if (heavy[cur] == 0) nm = makeLeaf(visible[cur]);
                else nm = makeInternal(visible[cur], getAgg(cur));
                updateMatrix(cur, nm);
                Val newHead = headValue(h);
                if (sameVal(oldHead, newHead)) break;
                dp[h] = newHead;
                updateAgg(h, newHead);
                int p = par[h];
                if (p == 1) break;
                cur = p;
            }
            cout << answer() << '\n';
        }
    }
    return 0;
}
