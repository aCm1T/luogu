#include <bits/stdc++.h>
using namespace std;

static constexpr long long MOD = 1'000'000'007LL;

long long mod_pow(long long a, long long e) {
    a %= MOD;
    long long r = 1;
    while (e > 0) {
        if (e & 1) r = r * a % MOD;
        a = a * a % MOD;
        e >>= 1;
    }
    return r;
}

long long cycle_chi(int c, long long k, long long km1) {
    if (c <= 0) return 0;
    if (c == 1) return k % MOD;
    if (c == 2) return k * km1 % MOD;
    long long t = mod_pow(km1, c);
    if (c % 2 == 0) return (t + km1) % MOD;
    return (t - km1 + MOD) % MOD;
}

// Per-test chi cache for hot degrees.
struct ChiCache {
    long long k = 0, km1 = 0;
    vector<long long> memo;
    void reset(long long kk, long long kkm1, int n) {
        k = kk;
        km1 = kkm1;
        memo.assign(n + 5, -1);
    }
    long long get(int c) {
        if (c <= 0) return 0;
        if (c < (int)memo.size() && memo[c] >= 0) return memo[c];
        long long v = cycle_chi(c, k, km1);
        if (c < (int)memo.size()) memo[c] = v;
        return v;
    }
};

static ChiCache CHI;

struct Tree {
    int n = 0;
    long long k = 0, km1 = 0, inv_k = 0, inv_kk1 = 0;
    vector<int> parent, extra;
    vector<vector<int>> ch;
    vector<char> active;
    vector<int> order;
    vector<int> internals;  // non-leaf indices for faster scans

    vector<int> del_stamp, vis_stamp, in_stamp;
    int del_id = 0, vis_id = 0, comp_id = 0;
    vector<int> comp_buf, stack_buf;

    void init_mod() {
        km1 = (k - 1 + MOD) % MOD;
        inv_k = mod_pow(k % MOD, MOD - 2);
        inv_kk1 = mod_pow(k * km1 % MOD, MOD - 2);
    }

    bool is_leaf(int v) const { return ch[v].empty() && extra[v] == 0; }

    void rebuild() {
        ch.assign(n + 1, {});
        for (int i = 2; i <= n; ++i) ch[parent[i]].push_back(i);
        order.clear();
        order.reserve(n);
        internals.clear();
        vector<int> st = {1};
        while (!st.empty()) {
            int u = st.back();
            st.pop_back();
            order.push_back(u);
            if (!is_leaf(u)) internals.push_back(u);
            for (int v : ch[u]) st.push_back(v);
        }
        del_stamp.assign(n + 1, 0);
        vis_stamp.assign(n + 1, 0);
        in_stamp.assign(n + 1, 0);
        comp_buf.reserve(n);
        stack_buf.reserve(n);
    }

    bool is_deleted(int v) const { return del_stamp[v] == del_id; }

    long long formula() {
        ++del_id;
        for (int i = (int)order.size() - 1; i >= 0; --i) {
            int v = order[i];
            if (is_leaf(v)) continue;
            bool has = extra[v] > 0;
            if (!has) {
                for (int c : ch[v])
                    if (!is_deleted(c) && active[c]) {
                        has = true;
                        break;
                    }
            }
            if (!has) del_stamp[v] = del_id;
        }
        for (int v = 1; v <= n; ++v) {
            if (is_deleted(v)) continue;
            bool has_par = v > 1 && !is_deleted(parent[v]) && active[v];
            if (!has_par) {
                int cnt = extra[v];
                for (int c : ch[v])
                    if (!is_deleted(c) && active[c]) ++cnt;
                if (cnt == 1) del_stamp[v] = del_id;
            }
        }

        bool any = false;
        for (int v : internals)
            if (!is_deleted(v)) {
                any = true;
                break;
            }
        if (!any) return k % MOD;

        // Fast path: single component, compute degrees without BFS labeling.
        // Most queries keep the tree connected via active edges from root.
        bool rooted = !is_deleted(1);
        if (rooted) {
            bool single = true;
            for (int v = 2; v <= n; ++v) {
                if (is_deleted(v)) continue;
                if (!( !is_deleted(parent[v]) && active[v] )) {
                    single = false;
                    break;
                }
            }
            if (single) {
                long long res = k % MOD;
                for (int v : internals) {
                    if (is_deleted(v)) continue;
                    int deg = extra[v];
                    for (int c : ch[v])
                        if (!is_deleted(c) && active[c]) ++deg;
                    bool has_par = v > 1 && !is_deleted(parent[v]) && active[v];
                    if (has_par) ++deg;
                    if (deg <= 0) continue;
                    long long c = CHI.get(deg);
                    if (c == 0) return 0;
                    if (!has_par)
                        res = res * c % MOD * inv_k % MOD;
                    else
                        res = res * c % MOD * inv_kk1 % MOD;
                }
                return res;
            }
        }

        long long res = k % MOD;
        ++vis_id;
        int vis = vis_id;
        auto& comp = comp_buf;
        auto& stk = stack_buf;

        for (int s = 1; s <= n; ++s) {
            if (is_deleted(s) || vis_stamp[s] == vis) continue;
            comp.clear();
            stk.clear();
            stk.push_back(s);
            vis_stamp[s] = vis;
            while (!stk.empty()) {
                int u = stk.back();
                stk.pop_back();
                comp.push_back(u);
                if (u > 1 && !is_deleted(parent[u]) && active[u]) {
                    int p = parent[u];
                    if (vis_stamp[p] != vis) {
                        vis_stamp[p] = vis;
                        stk.push_back(p);
                    }
                }
                for (int c : ch[u])
                    if (!is_deleted(c) && active[c] && vis_stamp[c] != vis) {
                        vis_stamp[c] = vis;
                        stk.push_back(c);
                    }
            }
            ++comp_id;
            int cid = comp_id;
            for (int v : comp) in_stamp[v] = cid;

            for (int v : comp) {
                if (is_deleted(v) || is_leaf(v)) continue;
                int deg = extra[v];
                for (int c : ch[v])
                    if (!is_deleted(c) && active[c] && in_stamp[c] == cid) ++deg;
                bool has_par = v > 1 && !is_deleted(parent[v]) && active[v] && in_stamp[parent[v]] == cid;
                if (has_par) ++deg;
                if (deg <= 0) continue;
                long long c = CHI.get(deg);
                if (c == 0) return 0;
                if (!has_par)
                    res = res * c % MOD * inv_k % MOD;
                else
                    res = res * c % MOD * inv_kk1 % MOD;
            }
        }
        return res;
    }
};

// Safe compression (proven equivalent): O(n) bottom-up.
// - Fold constant plain leaves / extra<=1 no-child nodes into parent.extra += 1
// - Compress constant deg-2 chains with extra==0
static Tree compress_safe(const Tree& src, const vector<char>& unstable, vector<int>& id) {
    int n = src.n;
    vector<int> parent = src.parent;
    vector<int> extra = src.extra;
    if ((int)extra.size() != n + 1) extra.assign(n + 1, 0);
    vector<vector<int>> ch(n + 1);
    for (int i = 2; i <= n; ++i) ch[parent[i]].push_back(i);
    vector<char> alive(n + 1, 1);
    vector<char> tog(n + 1, 0);
    for (int v = 2; v <= n; ++v)
        if (unstable[v]) tog[v] = 1;

    // alive child counts
    vector<int> acnt(n + 1, 0);
    for (int v = 1; v <= n; ++v) acnt[v] = (int)ch[v].size();
    // single alive child pointer (valid only when acnt==1)
    vector<int> only_child(n + 1, -1);
    for (int v = 1; v <= n; ++v)
        if (acnt[v] == 1) only_child[v] = ch[v][0];

    for (int v = n; v >= 2; --v) {
        if (!alive[v] || tog[v]) continue;
        if (acnt[v] == 0 && extra[v] <= 1) {
            int p = parent[v];
            extra[p] += 1;
            alive[v] = 0;
            // remove v from p's child count
            --acnt[p];
            if (acnt[p] == 1) {
                // find remaining child
                only_child[p] = -1;
                for (int c : ch[p])
                    if (alive[c]) {
                        only_child[p] = c;
                        break;
                    }
            } else if (acnt[p] != 1) {
                only_child[p] = -1;
            }
        } else if (acnt[v] == 1 && extra[v] == 0) {
            int only = only_child[v];
            if (only < 0 || !alive[only]) continue;
            int p = parent[v];
            parent[only] = p;
            alive[v] = 0;
            // replace v with only in p's children conceptually:
            // acnt[p] unchanged (remove v, add only — but only was child of v not p)
            // Actually: p loses v, gains only. acnt same.
            for (int& x : ch[p])
                if (x == v) {
                    x = only;
                    break;
                }
            if (acnt[p] == 1) only_child[p] = only;
            ch[v].clear();
            acnt[v] = 0;
        }
    }

    id.assign(n + 1, 0);
    int nn = 0;
    for (int v = 1; v <= n; ++v)
        if (alive[v]) id[v] = ++nn;

    Tree t;
    t.n = nn;
    t.k = src.k;
    t.parent.assign(nn + 1, 0);
    t.extra.assign(nn + 1, 0);
    t.active.assign(nn + 1, 1);
    for (int v = 1; v <= n; ++v) {
        if (!alive[v]) continue;
        int nv = id[v];
        t.extra[nv] = extra[v];
        if (v > 1) t.parent[nv] = id[parent[v]];
        t.active[nv] = src.active[v];
    }
    t.init_mod();
    t.rebuild();
    return t;
}

static void solve_star(int n, long long k, const vector<int>& parent, const vector<int>& ops) {
    vector<int> kids;
    for (int i = 2; i <= n; ++i)
        if (parent[i] == 1) kids.push_back(i);
    int m = (int)kids.size();
    vector<int> idx(n + 1);
    for (int i = 0; i < m; ++i) idx[kids[i]] = i;
    vector<char> active(n + 1, 1);
    long long km1 = (k - 1 + MOD) % MOD;
    auto count = [&]() {
        vector<int> p(m);
        iota(p.begin(), p.end(), 0);
        auto find = [&](int x) {
            while (p[x] != x) {
                p[x] = p[p[x]];
                x = p[x];
            }
            return x;
        };
        for (int v : kids)
            if (!active[v]) {
                int a = find(idx[v]), b = find((idx[v] + 1) % m);
                if (a != b) p[b] = a;
            }
        vector<int> seen(m, -1);
        int c = 0;
        for (int i = 0; i < m; ++i) {
            int r = find(i);
            if (seen[r] < 0) seen[r] = c++;
        }
        return c;
    };
    cout << cycle_chi(count(), k, km1) << '\n';
    for (int v : ops) {
        active[v] ^= 1;
        cout << cycle_chi(count(), k, km1) << '\n';
    }
}

static void dc_rec(Tree tr, vector<int> ops, vector<long long>& out, int base) {
    int q = (int)ops.size();
    vector<char> unstable(tr.n + 1, 0);
    for (int v : ops)
        if (v >= 2 && v <= tr.n) unstable[v] = 1;
    vector<int> id;
    Tree ct = compress_safe(tr, unstable, id);
    for (int v = 1; v <= tr.n; ++v)
        if (id[v]) ct.active[id[v]] = tr.active[v];
    vector<int> cops(q);
    for (int i = 0; i < q; ++i) {
        int v = ops[i];
        cops[i] = (v >= 1 && v <= tr.n) ? id[v] : 0;
    }

    long long cost = 1LL * ct.n * (q + 1);
    if (q <= 1 || ct.n <= 20 || cost <= 1000000LL) {
        out[base] = ct.formula();
        for (int i = 0; i < q; ++i) {
            if (cops[i] >= 2) ct.active[cops[i]] ^= 1;
            out[base + i + 1] = ct.formula();
        }
        return;
    }

    int mid = q / 2;
    vector<int> left(cops.begin(), cops.begin() + mid);
    vector<int> right(cops.begin() + mid, cops.end());
    dc_rec(ct, left, out, base);
    for (int i = 0; i < mid; ++i)
        if (cops[i] >= 2) ct.active[cops[i]] ^= 1;
    dc_rec(ct, right, out, base + mid);
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
        vector<int> parent(n + 1);
        for (int i = 2; i <= n; ++i) cin >> parent[i];
        vector<int> ops(q);
        for (int i = 0; i < q; ++i) cin >> ops[i];

        if (n == 3) {
            vector<char> active(4, 1);
            long long km1 = (k - 1 + MOD) % MOD;
            auto ans = [&]() {
                return (active[2] && active[3]) ? k * km1 % MOD : k % MOD;
            };
            cout << ans() << '\n';
            for (int v : ops) {
                active[v] ^= 1;
                cout << ans() << '\n';
            }
            continue;
        }

        bool is_star = true;
        for (int i = 2; i <= n; ++i)
            if (parent[i] != 1) {
                is_star = false;
                break;
            }
        if (is_star) {
            solve_star(n, k, parent, ops);
            continue;
        }

        Tree tr;
        tr.n = n;
        tr.k = k;
        tr.parent = parent;
        tr.extra.assign(n + 1, 0);
        tr.active.assign(n + 1, 1);
        tr.init_mod();
        tr.rebuild();
        CHI.reset(k, tr.km1, n);

        vector<char> unstable(n + 1, 0);
        for (int v : ops)
            if (v >= 2 && v <= n) unstable[v] = 1;
        vector<int> id;
        Tree ct = compress_safe(tr, unstable, id);
        vector<int> cops(q);
        for (int i = 0; i < q; ++i) {
            int v = ops[i];
            cops[i] = (v >= 1 && v <= n) ? id[v] : 0;
        }

        if (q > 0 && 1LL * ct.n * (q + 1) > 1000000LL) {
            vector<long long> answers(q + 1);
            dc_rec(ct, cops, answers, 0);
            for (long long x : answers) cout << x << '\n';
        } else {
            cout << ct.formula() << '\n';
            for (int v : cops) {
                if (v >= 2) ct.active[v] ^= 1;
                cout << ct.formula() << '\n';
            }
        }
    }
    return 0;
}
