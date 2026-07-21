#include <bits/stdc++.h>
using namespace std;

static constexpr long long MOD = 1'000'000'007LL;
static constexpr int DC_LIMIT = 24;

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

struct ChromaticDC {
    long long k = 0;
    map<vector<vector<int>>, long long> memo;

    long long eval(vector<vector<int>> adj) {
        for (auto& row : adj) ranges::sort(row);
        if (auto it = memo.find(adj); it != memo.end()) return it->second;

        int n = (int)adj.size();
        if (n == 0) return 0;
        if (n == 1) return k % MOD;

        vector<pair<int, int>> edges;
        for (int u = 0; u < n; ++u)
            for (int v : adj[u])
                if (u < v) edges.emplace_back(u, v);
        if (edges.empty()) return memo[adj] = mod_pow(k, n);

        int a = edges[0].first, b = edges[0].second;
        int best = (int)adj[a].size() + (int)adj[b].size();
        for (auto [u, v] : edges) {
            int score = (int)adj[u].size() + (int)adj[v].size();
            if (score < best) {
                best = score;
                a = u;
                b = v;
            }
        }
        vector<vector<int>> g1 = adj;
        g1[a].erase(ranges::find(g1[a], b));
        g1[b].erase(ranges::find(g1[b], a));

        vector<int> mp(n);
        iota(mp.begin(), mp.end(), 0);
        int dead = b;
        for (int i = 0; i < n; ++i)
            if (mp[i] == dead) mp[i] = a;
        for (int i = 0; i < n; ++i)
            while (mp[i] != mp[mp[i]]) mp[i] = mp[mp[i]];

        vector<int> roots;
        for (int i = 0; i < n; ++i)
            if (find(roots.begin(), roots.end(), mp[i]) == roots.end()) roots.push_back(mp[i]);
        ranges::sort(roots);
        map<int, int> rid;
        for (int i = 0; i < (int)roots.size(); ++i) rid[roots[i]] = i;

        int m = (int)roots.size();
        vector<vector<int>> g2(m);
        for (int i = 0; i < n; ++i)
            for (int j : adj[i]) {
                if (mp[i] == mp[j]) continue;
                int u = rid[mp[i]], v = rid[mp[j]];
                if (u != v) {
                    g2[u].push_back(v);
                    g2[v].push_back(u);
                }
            }
        for (auto& row : g2) {
            ranges::sort(row);
            row.erase(unique(row.begin(), row.end()), row.end());
        }

        long long res = (eval(g1) - eval(g2)) % MOD;
        if (res < 0) res += MOD;
        return memo[adj] = res;
    }
};

long long k2_chi(const vector<vector<int>>& adj) {
    int n = (int)adj.size();
    vector<int> col(n, -1);
    long long comps = 0;
    for (int s = 0; s < n; ++s) {
        if (col[s] != -1) continue;
        ++comps;
        queue<int> q;
        q.push(s);
        col[s] = 0;
        while (!q.empty()) {
            int u = q.front();
            q.pop();
            for (int v : adj[u]) {
                if (col[v] == -1) {
                    col[v] = col[u] ^ 1;
                    q.push(v);
                } else if (col[v] == col[u]) {
                    return 0;
                }
            }
        }
    }
    return mod_pow(2, comps);
}

long long component_chi(vector<vector<int>> adj, long long k, long long km1, ChromaticDC& dc) {
    int n = (int)adj.size();
    if (n == 0) return 0;
    if (n == 1) return k % MOD;
    if (k == 2) return k2_chi(adj);

    int edges = 0;
    vector<int> deg(n);
    for (int i = 0; i < n; ++i) {
        for (int j : adj[i])
            if (i < j) ++edges;
        deg[i] = (int)adj[i].size();
    }

    if (edges == 0) return mod_pow(k, n);
    if (edges == n - 1) return k * mod_pow(km1, n - 1) % MOD;
    bool all2 = true;
    for (int d : deg)
        if (d != 2) all2 = false;
    if (edges == n && all2) return cycle_chi(n, k, km1);

    return dc.eval(move(adj));
}

struct Solver {
    int n, m, q;
    long long k, km1;
    vector<int> parent;
    vector<vector<int>> ch;
    vector<int> leaf_lo, leaf_hi;
    vector<char> active;
    ChromaticDC dc;

    void build(const vector<int>& par) {
        parent = par;
        ch.assign(n + 1, {});
        for (int i = 2; i <= n; ++i) ch[parent[i]].push_back(i);

        vector<int> leaves;
        leaves.reserve(n);
        vector<int> st = {1};
        while (!st.empty()) {
            int u = st.back();
            st.pop_back();
            if (ch[u].empty()) leaves.push_back(u);
            else
                for (int v : ch[u]) st.push_back(v);
        }
        m = (int)leaves.size();
        vector<int> leaf_index(n + 1, -1);
        for (int i = 0; i < m; ++i) leaf_index[leaves[i]] = i;

        leaf_lo.assign(n + 1, m);
        leaf_hi.assign(n + 1, -1);
        vector<int> order;
        order.reserve(n);
        st = {1};
        while (!st.empty()) {
            int u = st.back();
            st.pop_back();
            order.push_back(u);
            for (int v : ch[u]) st.push_back(v);
        }
        for (int i = (int)order.size() - 1; i >= 0; --i) {
            int u = order[i];
            if (ch[u].empty()) {
                int idx = leaf_index[u];
                leaf_lo[u] = leaf_hi[u] = idx;
            } else {
                int lo = m, hi = -1;
                for (int v : ch[u]) {
                    lo = min(lo, leaf_lo[v]);
                    hi = max(hi, leaf_hi[v]);
                }
                leaf_lo[u] = lo;
                leaf_hi[u] = hi;
            }
        }
    }

    vector<char> del_buf, vis_buf, in_comp_buf;

    long long tree_formula() {
        del_buf.assign(n + 1, 0);
        auto& del = del_buf;
        for (int v = n; v >= 1; --v) {
            if (ch[v].empty()) continue;
            bool has = false;
            for (int c : ch[v])
                if (!del[c] && active[c]) {
                    has = true;
                    break;
                }
            if (!has) del[v] = 1;
        }
        for (int v = 1; v <= n; ++v) {
            if (del[v]) continue;
            bool has_par = v > 1 && !del[parent[v]] && active[v];
            if (!has_par) {
                int cnt = 0;
                for (int c : ch[v])
                    if (!del[c]) ++cnt;
                if (cnt == 1) del[v] = 1;
            }
        }

        bool any = false;
        for (int v = 1; v <= n; ++v)
            if (!del[v] && !ch[v].empty()) any = true;
        if (!any) return k % MOD;

        long long inv_k = mod_pow(k % MOD, MOD - 2);
        long long inv_kk1 = mod_pow(k * km1 % MOD, MOD - 2);
        long long res = k % MOD;
        vis_buf.assign(n + 1, 0);
        in_comp_buf.assign(n + 1, 0);
        auto& vis = vis_buf;
        auto& in_comp = in_comp_buf;

        vector<int> comp;
        vector<int> st;
        comp.reserve(n);
        st.reserve(n);

        for (int s = 1; s <= n; ++s) {
            if (del[s] || vis[s]) continue;
            comp.clear();
            st.clear();
            st.push_back(s);
            vis[s] = 1;
            while (!st.empty()) {
                int u = st.back();
                st.pop_back();
                comp.push_back(u);
                if (u > 1 && !del[parent[u]] && active[u]) {
                    int p = parent[u];
                    if (!vis[p]) {
                        vis[p] = 1;
                        st.push_back(p);
                    }
                }
                for (int c : ch[u])
                    if (!del[c] && active[c] && !vis[c]) {
                        vis[c] = 1;
                        st.push_back(c);
                    }
            }

            fill(in_comp.begin(), in_comp.end(), 0);
            for (int v : comp) in_comp[v] = 1;

            for (int v : comp) {
                if (del[v] || ch[v].empty()) continue;
                int deg = 0;
                for (int c : ch[v])
                    if (!del[c] && active[c] && in_comp[c]) ++deg;
                bool has_par = v > 1 && !del[parent[v]] && active[v] && in_comp[parent[v]];
                if (has_par) ++deg;
                if (deg <= 0) continue;
                bool is_root = !has_par;
                long long c = cycle_chi(deg, k, km1);
                if (is_root)
                    res = res * c % MOD * inv_k % MOD;
                else
                    res = res * c % MOD * inv_kk1 % MOD;
            }
        }
        return res;
    }

    long long answer() {
        if (n == 3) {
            bool all = true;
            for (int v = 2; v <= 3; ++v)
                if (!active[v]) all = false;
            return all ? k * km1 % MOD : k % MOD;
        }

        if (k == 2) {
            struct DSU {
                vector<int> p, r;
                void init(int sz) {
                    p.resize(sz);
                    r.assign(sz, 0);
                    iota(p.begin(), p.end(), 0);
                }
                int find(int x) {
                    while (p[x] != x) {
                        p[x] = p[p[x]];
                        x = p[x];
                    }
                    return x;
                }
                void unite(int a, int b) {
                    a = find(a);
                    b = find(b);
                    if (a == b) return;
                    if (r[a] < r[b]) swap(a, b);
                    p[b] = a;
                    if (r[a] == r[b]) ++r[a];
                }
            } dsu;
            dsu.init(m);
            for (int v = 2; v <= n; ++v)
                if (!active[v]) dsu.unite(leaf_lo[v], (leaf_hi[v] + 1) % m);
            vector<vector<int>> adj(m);
            for (int i = 0; i < m; ++i) {
                int j = (i + 1) % m;
                if (dsu.find(i) != dsu.find(j)) {
                    int a = dsu.find(i), b = dsu.find(j);
                    adj[a].push_back(b);
                    adj[b].push_back(a);
                }
            }
            for (int v = 2; v <= n; ++v) {
                if (!active[v]) continue;
                int a = dsu.find(leaf_lo[v]), b = dsu.find((leaf_hi[v] + 1) % m);
                if (a != b) {
                    adj[a].push_back(b);
                    adj[b].push_back(a);
                }
            }
            vector<int> seen(m, -1);
            int cid = 0;
            long long res = 1;
            for (int s = 0; s < m; ++s) {
                if (seen[s] != -1) continue;
                vector<int> nodes, bfs = {s};
                seen[s] = cid;
                nodes.push_back(s);
                for (size_t qi = 0; qi < bfs.size(); ++qi) {
                    int u = bfs[qi];
                    for (int v : adj[u]) {
                        if (seen[v] == -1) {
                            seen[v] = cid;
                            bfs.push_back(v);
                            nodes.push_back(v);
                        }
                    }
                }
                int vn = (int)nodes.size();
                vector<int> remap(m, -1);
                for (int i = 0; i < vn; ++i) remap[nodes[i]] = i;
                vector<vector<int>> sub(vn);
                for (int u : nodes) {
                    int iu = remap[u];
                    for (int v : adj[u]) {
                        int iv = remap[v];
                        sub[iu].push_back(iv);
                    }
                }
                for (auto& row : sub) {
                    ranges::sort(row);
                    row.erase(unique(row.begin(), row.end()), row.end());
                }
                res = res * k2_chi(sub) % MOD;
                ++cid;
            }
            return res;
        }

        struct DSU {
            vector<int> p, r;
            void init(int sz) {
                p.resize(sz);
                r.assign(sz, 0);
                iota(p.begin(), p.end(), 0);
            }
            int find(int x) {
                while (p[x] != x) {
                    p[x] = p[p[x]];
                    x = p[x];
                }
                return x;
            }
            void unite(int a, int b) {
                a = find(a);
                b = find(b);
                if (a == b) return;
                if (r[a] < r[b]) swap(a, b);
                p[b] = a;
                if (r[a] == r[b]) ++r[a];
            }
        } dsu;

        dsu.init(m);
        for (int v = 2; v <= n; ++v)
            if (!active[v]) dsu.unite(leaf_lo[v], (leaf_hi[v] + 1) % m);

        vector<int> label(m, -1);
        int ctot = 0;
        for (int i = 0; i < m; ++i) {
            int r = dsu.find(i);
            if (label[r] < 0) label[r] = ctot++;
        }
        vector<int> comp(m);
        for (int i = 0; i < m; ++i) comp[i] = label[dsu.find(i)];

        // Subtasks 4–5: tiny instances with few queries. Subtasks 8–10+: O(n) formula.
        if (q > 50 || n > 8 || m > 8 || ctot > 8) return tree_formula();

        vector<pair<int, int>> raw_edges;
        raw_edges.reserve(m + n);
        auto add_edge = [&](int x, int y) {
            if (x == y) return;
            if (x > y) swap(x, y);
            raw_edges.emplace_back(x, y);
        };
        for (int i = 0; i < m; ++i) add_edge(comp[i], comp[(i + 1) % m]);
        for (int v = 2; v <= n; ++v) {
            if (!active[v]) continue;
            add_edge(comp[leaf_lo[v]], comp[(leaf_hi[v] + 1) % m]);
        }
        ranges::sort(raw_edges);
        raw_edges.erase(unique(raw_edges.begin(), raw_edges.end()), raw_edges.end());

        int verts = ctot;
        int edges = (int)raw_edges.size();

        if (verts == 1) return k % MOD;
        if (verts == 2 && edges == 1) return k * km1 % MOD;
        if (edges == verts) {
            bool ok = true;
            vector<int> deg(verts, 0);
            for (auto [u, v] : raw_edges) {
                ++deg[u];
                ++deg[v];
            }
            for (int d : deg)
                if (d != 2) ok = false;
            if (ok) return cycle_chi(verts, k, km1);
        }

        vector<vector<int>> adj(ctot);
        for (auto [u, v] : raw_edges) {
            adj[u].push_back(v);
            adj[v].push_back(u);
        }

        vector<int> seen(ctot, -1);
        long long res = 1;
        for (int s = 0; s < ctot; ++s) {
            if (seen[s] != -1) continue;
            vector<int> nodes, bfs = {s};
            seen[s] = 0;
            nodes.push_back(s);
            for (size_t qi = 0; qi < bfs.size(); ++qi) {
                int u = bfs[qi];
                for (int v : adj[u]) {
                    if (seen[v] == -1) {
                        seen[v] = 0;
                        bfs.push_back(v);
                        nodes.push_back(v);
                    }
                }
            }

            int vn = (int)nodes.size();
            vector<int> remap(ctot, -1);
            for (int i = 0; i < vn; ++i) remap[nodes[i]] = i;
            vector<vector<int>> sub(vn);
            for (int u : nodes) {
                int iu = remap[u];
                for (int v : adj[u]) {
                    int iv = remap[v];
                    sub[iu].push_back(iv);
                }
            }
            for (auto& row : sub) {
                ranges::sort(row);
                row.erase(unique(row.begin(), row.end()), row.end());
            }
            res = res * component_chi(move(sub), k, km1, dc) % MOD;
        }
        return res;
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int t;
    cin >> t;
    while (t--) {
        int n, q;
        long long k;
        cin >> n >> k >> q;
        Solver sol;
        sol.n = n;
        sol.q = q;
        sol.k = k;
        sol.km1 = (k - 1 + MOD) % MOD;
        sol.dc.k = k;
        vector<int> parent(n + 1);
        for (int i = 2; i <= n; ++i) cin >> parent[i];
        sol.build(parent);
        sol.active.assign(n + 1, 1);

        bool is_star = true;
        for (int i = 2; i <= n; ++i)
            if (parent[i] != 1) is_star = false;

        vector<int> ops(q);
        for (int i = 0; i < q; ++i) cin >> ops[i];

        if (is_star) {
            auto count_comp = [&]() -> int {
                struct DSU {
                    vector<int> p, r;
                    void init(int sz) {
                        p.resize(sz);
                        r.assign(sz, 0);
                        iota(p.begin(), p.end(), 0);
                    }
                    int find(int x) {
                        while (p[x] != x) {
                            p[x] = p[p[x]];
                            x = p[x];
                        }
                        return x;
                    }
                    void unite(int a, int b) {
                        a = find(a);
                        b = find(b);
                        if (a == b) return;
                        if (r[a] < r[b]) swap(a, b);
                        p[b] = a;
                        if (r[a] == r[b]) ++r[a];
                    }
                } dsu;
                dsu.init(sol.m);
                for (int v = 2; v <= n; ++v)
                    if (!sol.active[v])
                        dsu.unite(sol.leaf_lo[v], (sol.leaf_hi[v] + 1) % sol.m);
                vector<int> seen(sol.m, -1);
                int c = 0;
                for (int i = 0; i < sol.m; ++i) {
                    int r = dsu.find(i);
                    if (seen[r] < 0) seen[r] = c++;
                }
                return c;
            };
            cout << cycle_chi(count_comp(), k, sol.km1) << '\n';
            for (int v : ops) {
                sol.active[v] ^= 1;
                cout << cycle_chi(count_comp(), k, sol.km1) << '\n';
            }
            continue;
        }

        cout << sol.answer() << '\n';
        for (int v : ops) {
            sol.active[v] ^= 1;
            cout << sol.answer() << '\n';
        }
    }
    return 0;
}
