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

long long tree_chi(int c, long long k, long long km1) {
    if (c == 1) return k % MOD;
    return k * mod_pow(km1, c - 1) % MOD;
}

long long cycle_chi(int c, long long k, long long km1) {
    if (c == 1) return k % MOD;
    if (c == 2) return k * km1 % MOD;
    long long t = mod_pow(km1, c);
    if (c % 2 == 0) return (t + km1) % MOD;
    return (t - km1 + MOD) % MOD;
}

long long binary_tree_formula(int c, long long k, long long km1, long long km2) {
    if (c == 1) return k % MOD;
    return k * km1 % MOD * mod_pow(km2, c - 2) % MOD;
}

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

        auto [a, b] = edges[0];
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

struct BlockData {
    vector<int> verts;
    vector<pair<int, int>> edges;
};

long long block_chi(const BlockData& blk, int n, long long k, long long km1, long long km2,
                    bool use_binary_formula, ChromaticDC& dc) {
    int vcnt = (int)blk.verts.size();
    int ecnt = (int)blk.edges.size();
    if (vcnt == 1) return k % MOD;
    if (ecnt == vcnt - 1) return tree_chi(vcnt, k, km1);
    if (ecnt == vcnt) {
        bool all2 = true;
        vector<int> deg(n, 0);
        for (auto [a, b] : blk.edges) {
            deg[a]++;
            deg[b]++;
        }
        for (int x : blk.verts)
            if (deg[x] != 2) all2 = false;
        if (all2) return cycle_chi(vcnt, k, km1);
    }
    if (use_binary_formula && ecnt == 2 * vcnt - 3)
        return binary_tree_formula(vcnt, k, km1, km2);

    vector<int> old_to_new(n, -1);
    for (int i = 0; i < vcnt; ++i) old_to_new[blk.verts[i]] = i;

    vector<vector<int>> sub(vcnt);
    for (auto [a, b] : blk.edges) {
        int u = old_to_new[a], v = old_to_new[b];
        sub[u].push_back(v);
        sub[v].push_back(u);
    }
    return dc.eval(move(sub));
}

long long connected_chi(vector<vector<int>>& adj, long long k, long long km1, long long km2,
                        bool use_binary_formula, ChromaticDC& dc) {
    int n = (int)adj.size();
    if (n == 0) return 0;
    if (n == 1) return k % MOD;
    if (k == 2) return k2_chi(adj);

    int edges = 0;
    for (int i = 0; i < n; ++i)
        for (int j : adj[i]) if (i < j) ++edges;
    if (edges == 0) return mod_pow(k, n);
    if (edges == n - 1) return tree_chi(n, k, km1);
    bool all2 = true;
    for (auto& row : adj) if ((int)row.size() != 2) all2 = false;
    if (edges == n && all2) return cycle_chi(n, k, km1);

    vector<int> disc(n, -1), low(n, -1), parent(n, -1);
    int timer = 0;
    vector<int> edge_u, edge_v;
    vector<BlockData> blocks;
    vector<int> art_deg(n, 0);

    function<void(int)> tarjan = [&](int u) {
        disc[u] = low[u] = timer++;
        for (int v : adj[u]) {
            if (disc[v] == -1) {
                parent[v] = u;
                edge_u.push_back(u);
                edge_v.push_back(v);
                tarjan(v);
                low[u] = min(low[u], low[v]);
                if (low[v] >= disc[u]) {
                    BlockData blk;
                    while (true) {
                        int a = edge_u.back();
                        int b = edge_v.back();
                        edge_u.pop_back();
                        edge_v.pop_back();
                        blk.verts.push_back(a);
                        blk.verts.push_back(b);
                        blk.edges.emplace_back(a, b);
                        if (a == u && b == v) break;
                    }
                    sort(blk.verts.begin(), blk.verts.end());
                    blk.verts.erase(unique(blk.verts.begin(), blk.verts.end()), blk.verts.end());
                    for (int x : blk.verts) ++art_deg[x];
                    blocks.push_back(move(blk));
                }
            } else if (v != parent[u] && disc[v] < disc[u]) {
                edge_u.push_back(u);
                edge_v.push_back(v);
                low[u] = min(low[u], disc[v]);
            }
        }
    };
    tarjan(0);

    long long res = 1;
    int extra = 0;
    for (int v = 0; v < n; ++v)
        if (art_deg[v] > 1) extra += art_deg[v] - 1;

    if (blocks.size() == 1) {
        return block_chi(blocks[0], n, k, km1, km2, use_binary_formula, dc);
    }

    for (const auto& blk : blocks)
        res = res * block_chi(blk, n, k, km1, km2, use_binary_formula, dc) % MOD;

    if (extra > 0) {
        long long exp = (MOD - 1 - (extra % (MOD - 1))) % (MOD - 1);
        res = res * mod_pow(k % MOD, exp) % MOD;
    }
    return res;
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

bool is_special_binary(const vector<int>& parent, int n) {
    for (int i = 2; i <= n; ++i) {
        int want = 2 * (i / 2) - 1;
        if (parent[i] != want) return false;
    }
    return true;
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
        const long long km1 = (k - 1 + MOD) % MOD;
        const long long km2 = (k - 2 + MOD) % MOD;

        vector<int> parent(n + 1);
        vector<vector<int>> ch(n + 1);
        for (int i = 2; i <= n; ++i) {
            cin >> parent[i];
            ch[parent[i]].push_back(i);
        }
        const bool special_tree = is_special_binary(parent, n);
        bool is_star = true;
        for (int i = 2; i <= n; ++i)
            if (parent[i] != 1) is_star = false;

        vector<int> ops(q);
        for (int i = 0; i < q; ++i) cin >> ops[i];

        vector<int> leaves;
        leaves.reserve(n);
        vector<int> st;
        st.push_back(1);
        while (!st.empty()) {
            int u = st.back();
            st.pop_back();
            if (ch[u].empty()) leaves.push_back(u);
            else
                for (int v : ch[u]) st.push_back(v);
        }
        const int m = (int)leaves.size();

        vector<int> leaf_index(n + 1, -1);
        for (int i = 0; i < m; ++i) leaf_index[leaves[i]] = i;

        vector<int> leaf_lo(n + 1, m), leaf_hi(n + 1, -1);
        vector<int> order;
        order.reserve(n);
        st.assign(1, 1);
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

        vector<pair<int, int>> tree_pair(n + 1);
        for (int v = 2; v <= n; ++v) {
            int a = leaf_lo[v];
            int b = (leaf_hi[v] + 1) % m;
            tree_pair[v] = {a, b};
        }

        vector<char> active(n + 1, 1);
        vector<int> inactive;
        inactive.reserve(n);

        DSU dsu;
        vector<int> comp(m);
        vector<int> label(m);
        vector<pair<int, int>> raw_edges;
        vector<vector<int>> adj;
        vector<int> comp_id;
        vector<int> bfs_st;
        vector<int> nodes;
        vector<int> remap;
        vector<vector<int>> sub;
        ChromaticDC dc;
        dc.k = k;

        auto count_components = [&]() -> int {
            dsu.init(m);
            for (int v : inactive) {
                auto [a, b] = tree_pair[v];
                dsu.unite(a, b);
            }
            fill(label.begin(), label.end(), -1);
            int ctot = 0;
            for (int i = 0; i < m; ++i) {
                int r = dsu.find(i);
                if (label[r] < 0) label[r] = ctot++;
            }
            return ctot;
        };

        if (is_star) {
            cout << cycle_chi(count_components(), k, km1) << '\n';
            for (int v : ops) {
                active[v] ^= 1;
                if (active[v])
                    inactive.erase(ranges::find(inactive, v));
                else
                    inactive.push_back(v);
                cout << cycle_chi(count_components(), k, km1) << '\n';
            }
            continue;
        }

        auto answer = [&]() -> long long {
            dsu.init(m);
            for (int v : inactive) {
                auto [a, b] = tree_pair[v];
                dsu.unite(a, b);
            }

            fill(label.begin(), label.end(), -1);
            int ctot = 0;
            for (int i = 0; i < m; ++i) {
                int r = dsu.find(i);
                if (label[r] < 0) label[r] = ctot++;
                comp[i] = label[r];
            }

            raw_edges.clear();
            raw_edges.reserve(m + n);
            auto add_edge = [&](int x, int y) {
                if (x == y) return;
                if (x > y) swap(x, y);
                raw_edges.emplace_back(x, y);
            };
            for (int i = 0; i < m; ++i) add_edge(comp[i], comp[(i + 1) % m]);
            for (int v = 2; v <= n; ++v) {
                if (!active[v]) continue;
                auto [a, b] = tree_pair[v];
                add_edge(comp[a], comp[b]);
            }
            ranges::sort(raw_edges);
            raw_edges.erase(unique(raw_edges.begin(), raw_edges.end()), raw_edges.end());

            adj.assign(ctot, {});
            for (auto [u, v] : raw_edges) {
                adj[u].push_back(v);
                adj[v].push_back(u);
            }

            comp_id.assign(ctot, -1);
            long long res = 1;
            int cid = 0;

            for (int s = 0; s < ctot; ++s) {
                if (comp_id[s] != -1) continue;
                nodes.clear();
                bfs_st.assign(1, s);
                comp_id[s] = cid;
                nodes.push_back(s);
                for (size_t qi = 0; qi < bfs_st.size(); ++qi) {
                    int u = bfs_st[qi];
                    for (int v : adj[u]) {
                        if (comp_id[v] == -1) {
                            comp_id[v] = cid;
                            bfs_st.push_back(v);
                            nodes.push_back(v);
                        }
                    }
                }

                int verts = (int)nodes.size();
                remap.assign(ctot, -1);
                for (int i = 0; i < verts; ++i) remap[nodes[i]] = i;

                sub.assign(verts, {});
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

                res = res * connected_chi(sub, k, km1, km2, special_tree, dc) % MOD;
                ++cid;
            }
            return res;
        };

        cout << answer() << '\n';
        for (int v : ops) {
            active[v] ^= 1;
            if (active[v])
                inactive.erase(ranges::find(inactive, v));
            else
                inactive.push_back(v);
            cout << answer() << '\n';
        }
    }
    return 0;
}
