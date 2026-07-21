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

struct DSU {
    vector<int> p, r;
    void init(int n) {
        p.resize(n);
        r.assign(n, 0);
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
};

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
        roots.reserve(n);
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

struct Solver {
    int n, m, q;
    long long k, km1, inv_k, inv_kk1;
    vector<int> parent;
    vector<vector<int>> ch;
    vector<int> leaf_lo, leaf_hi;
    vector<int> postorder;
    vector<char> active;
    ChromaticDC dc;

    vector<int> del_stamp, vis_stamp, in_stamp;
    int del_id = 0, vis_id = 0, comp_id = 0;

    vector<int> comp_buf, stack_buf;

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
        postorder.clear();
        postorder.reserve(n);
        st = {1};
        while (!st.empty()) {
            int u = st.back();
            st.pop_back();
            postorder.push_back(u);
            for (int v : ch[u]) st.push_back(v);
        }
        for (int i = (int)postorder.size() - 1; i >= 0; --i) {
            int u = postorder[i];
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

        del_stamp.assign(n + 1, 0);
        vis_stamp.assign(n + 1, 0);
        in_stamp.assign(n + 1, 0);
        comp_buf.reserve(n);
        stack_buf.reserve(n);
    }

    void init_mod_inverses() {
        inv_k = mod_pow(k % MOD, MOD - 2);
        inv_kk1 = mod_pow(k * km1 % MOD, MOD - 2);
    }

    bool is_deleted(int v) const { return del_stamp[v] == del_id; }

    long long tree_formula() {
        ++del_id;
        for (int i = (int)postorder.size() - 1; i >= 0; --i) {
            int v = postorder[i];
            if (ch[v].empty()) continue;
            bool has = false;
            for (int c : ch[v])
                if (!is_deleted(c) && active[c]) {
                    has = true;
                    break;
                }
            if (!has) del_stamp[v] = del_id;
        }
        // Pass 2: delete component roots that have exactly one active son.
        for (int v = 1; v <= n; ++v) {
            if (is_deleted(v)) continue;
            bool has_par = v > 1 && !is_deleted(parent[v]) && active[v];
            if (!has_par) {
                int cnt = 0;
                for (int c : ch[v])
                    if (!is_deleted(c) && active[c]) ++cnt;
                if (cnt == 1) del_stamp[v] = del_id;
            }
        }

        bool any = false;
        for (int v = 1; v <= n; ++v)
            if (!is_deleted(v) && !ch[v].empty()) any = true;
        if (!any) return k % MOD;

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
                if (is_deleted(v) || ch[v].empty()) continue;
                int deg = 0;
                for (int c : ch[v])
                    if (!is_deleted(c) && active[c] && in_stamp[c] == cid) ++deg;
                bool has_par = v > 1 && !is_deleted(parent[v]) && active[v] && in_stamp[parent[v]] == cid;
                if (has_par) ++deg;
                if (deg <= 0) continue;
                long long c = cycle_chi(deg, k, km1);
                if (c == 0) return 0;
                if (!has_par)
                    res = res * c % MOD * inv_k % MOD;
                else
                    res = res * c % MOD * inv_kk1 % MOD;
            }
        }
        return res;
    }

    int merged_components() {
        DSU dsu;
        dsu.init(m);
        for (int v = 2; v <= n; ++v)
            if (!active[v]) dsu.unite(leaf_lo[v], (leaf_hi[v] + 1) % m);
        vector<int> seen(m, -1);
        int ctot = 0;
        for (int i = 0; i < m; ++i) {
            int r = dsu.find(i);
            if (seen[r] < 0) seen[r] = ctot++;
        }
        return ctot;
    }

    long long answer_k2() {
        DSU dsu;
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

        vector<vector<int>> adj(ctot);
        auto add = [&](int a, int b) {
            if (a == b) return;
            adj[a].push_back(b);
            adj[b].push_back(a);
        };
        for (int i = 0; i < m; ++i) add(comp[i], comp[(i + 1) % m]);
        for (int v = 2; v <= n; ++v) {
            if (!active[v]) continue;
            add(comp[leaf_lo[v]], comp[(leaf_hi[v] + 1) % m]);
        }
        for (auto& row : adj) {
            ranges::sort(row);
            row.erase(unique(row.begin(), row.end()), row.end());
        }
        return k2_chi(adj);
    }

    long long answer_dc() {
        DSU dsu;
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

        if (ctot == 1) return k % MOD;
        if (ctot == 2 && (int)raw_edges.size() == 1) return k * km1 % MOD;
        if ((int)raw_edges.size() == ctot) {
            bool ok = true;
            vector<int> deg(ctot, 0);
            for (auto [u, v] : raw_edges) {
                ++deg[u];
                ++deg[v];
            }
            for (int d : deg)
                if (d != 2) ok = false;
            if (ok) return cycle_chi(ctot, k, km1);
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
                for (int v : adj[u]) sub[iu].push_back(remap[v]);
            }
            for (auto& row : sub) {
                ranges::sort(row);
                row.erase(unique(row.begin(), row.end()), row.end());
            }

            int sn = (int)sub.size();
            if (sn == 1) {
                res = res * (k % MOD) % MOD;
                continue;
            }
            int edges = 0;
            for (int i = 0; i < sn; ++i)
                for (int j : sub[i])
                    if (i < j) ++edges;
            if (edges == 0)
                res = res * mod_pow(k, sn) % MOD;
            else if (edges == sn - 1)
                res = res * (k * mod_pow(km1, sn - 1) % MOD) % MOD;
            else {
                bool all2 = true;
                for (int i = 0; i < sn; ++i)
                    if ((int)sub[i].size() != 2) all2 = false;
                if (edges == sn && all2)
                    res = res * cycle_chi(sn, k, km1) % MOD;
                else
                    res = res * dc.eval(move(sub)) % MOD;
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

        // Brute dual DC only for tiny q=0 cases (subtasks 4–5 style).
        // With queries, or larger graphs, use hanging-vertex formula (subtasks 7–10).
        if (q == 0 && n <= 7 && m <= 7 && k != 2) {
            int ctot = merged_components();
            if (ctot <= 7) return answer_dc();
        }
        return tree_formula();
    }
};

// Compress always-active degree-2 chains (edge to parent never toggled).
// Intermediate vertices contribute χ(2)/(k(k-1))=1, so dropping them is safe.
static void compress_constant_chains(int& n, vector<int>& parent, vector<int>& ops) {
    vector<char> toggled(n + 1, 0);
    for (int v : ops) toggled[v] = 1;

    vector<vector<int>> ch(n + 1);
    for (int i = 2; i <= n; ++i) ch[parent[i]].push_back(i);

    vector<char> alive(n + 1, 1);
    bool changed = true;
    while (changed) {
        changed = false;
        for (int v = 2; v <= n; ++v) {
            if (!alive[v] || toggled[v]) continue;
            // exactly one alive child?
            int only = -1, cnt = 0;
            for (int c : ch[v])
                if (alive[c]) {
                    only = c;
                    ++cnt;
                    if (cnt > 1) break;
                }
            if (cnt != 1) continue;
            // reparent only -> parent[v]; edge v->parent always active
            int p = parent[v];
            parent[only] = p;
            // update ch[p]: replace v with only
            for (int& x : ch[p])
                if (x == v) {
                    x = only;
                    break;
                }
            ch[v].clear();
            alive[v] = 0;
            changed = true;
        }
    }

    vector<int> id(n + 1, 0);
    int nn = 0;
    for (int v = 1; v <= n; ++v)
        if (alive[v]) id[v] = ++nn;

    vector<int> new_parent(nn + 1, 0);
    for (int v = 2; v <= n; ++v) {
        if (!alive[v]) continue;
        new_parent[id[v]] = id[parent[v]];
    }
    for (int& v : ops) v = id[v];

    n = nn;
    parent.swap(new_parent);
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int t;
    cin >> t;
    while (t--) {
        int n, q;
        long long k;
        cin >> n >> k >> q;

        vector<int> parent(n + 1);
        for (int i = 2; i <= n; ++i) cin >> parent[i];

        vector<int> ops(q);
        for (int i = 0; i < q; ++i) cin >> ops[i];

        if (q > 0 && n > 64) compress_constant_chains(n, parent, ops);

        Solver sol;
        sol.n = n;
        sol.q = q;
        sol.k = k;
        sol.km1 = (k - 1 + MOD) % MOD;
        sol.init_mod_inverses();
        sol.dc.k = k;
        sol.dc.memo.clear();
        sol.build(parent);
        sol.active.assign(n + 1, 1);

        bool is_star = true;
        for (int i = 2; i <= n; ++i)
            if (parent[i] != 1) is_star = false;

        if (is_star) {
            DSU dsu;
            vector<int> seen;
            dsu.init(1);
            auto count_comp = [&]() -> int {
                dsu.init(sol.m);
                for (int v = 2; v <= n; ++v)
                    if (!sol.active[v])
                        dsu.unite(sol.leaf_lo[v], (sol.leaf_hi[v] + 1) % sol.m);
                seen.assign(sol.m, -1);
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
