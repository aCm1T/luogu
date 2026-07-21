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

struct ChiCache {
    vector<long long> memo;
    long long k = 0, km1 = 0;
    void reset(long long kk, long long kk1, int cap) {
        k = kk;
        km1 = kk1;
        memo.assign(max(cap, 0) + 8, -1);
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

// Tiny dual-graph DC (only for very small q=0 cases).
struct ChromaticDC {
    long long k = 0;
    map<vector<vector<int>>, long long> memo;

    long long eval(vector<vector<int>> adj) {
        for (auto& row : adj) {
            sort(row.begin(), row.end());
            row.erase(unique(row.begin(), row.end()), row.end());
        }
        if (auto it = memo.find(adj); it != memo.end()) return it->second;

        int n = (int)adj.size();
        if (n == 0) return 0;
        if (n == 1) return k % MOD;

        int edges = 0;
        for (int i = 0; i < n; ++i)
            for (int j : adj[i])
                if (i < j) ++edges;
        if (edges == 0) return memo[adj] = mod_pow(k, n);
        if (edges == n - 1) return memo[adj] = k * mod_pow((k - 1 + MOD) % MOD, n - 1) % MOD;

        int a = -1, b = -1;
        for (int i = 0; i < n && a < 0; ++i)
            for (int j : adj[i])
                if (i < j) {
                    a = i;
                    b = j;
                    break;
                }

        auto g1 = adj;
        g1[a].erase(find(g1[a].begin(), g1[a].end(), b));
        g1[b].erase(find(g1[b].begin(), g1[b].end(), a));
        long long del = eval(g1);

        vector<int> mp(n);
        iota(mp.begin(), mp.end(), 0);
        mp[b] = a;
        for (int i = 0; i < n; ++i)
            while (mp[i] != mp[mp[i]]) mp[i] = mp[mp[i]];
        vector<int> roots;
        for (int i = 0; i < n; ++i)
            if (mp[i] == i) roots.push_back(i);
        sort(roots.begin(), roots.end());
        vector<int> rid(n, -1);
        for (int i = 0; i < (int)roots.size(); ++i) rid[roots[i]] = i;
        int rn = (int)roots.size();
        vector<vector<int>> g2(rn);
        for (int i = 0; i < n; ++i) {
            int iu = rid[mp[i]];
            for (int j : adj[i]) {
                int iv = rid[mp[j]];
                if (iu != iv) g2[iu].push_back(iv);
            }
        }
        for (auto& row : g2) {
            sort(row.begin(), row.end());
            row.erase(unique(row.begin(), row.end()), row.end());
        }
        long long contr = eval(move(g2));
        return memo[adj] = (del - contr + MOD) % MOD;
    }
};

struct Solver {
    int n = 0, m = 0, q = 0;
    long long k = 0, km1 = 0, inv_k = 0, inv_kk1 = 0;
    vector<int> parent;          // parent[v]>=1 real; 0 = true root / no parent
    vector<int> extra;           // e(v) virtual outer-ring attachments
    vector<char> under_virt;     // hung under virtual root -1 (uses non-root formula)
    vector<vector<int>> ch;
    vector<int> leaf_lo, leaf_hi;
    vector<int> postorder;
    vector<char> active;
    ChromaticDC dc;

    vector<int> del_stamp, vis_stamp, in_stamp;
    int del_id = 0, vis_id = 0, comp_id = 0;
    vector<int> comp_buf, stack_buf;

    bool is_leaf(int v) const { return ch[v].empty() && extra[v] == 0; }

    void rebuild_children_order() {
        ch.assign(n + 1, {});
        for (int i = 1; i <= n; ++i) {
            if (parent[i] >= 1) ch[parent[i]].push_back(i);
        }
        postorder.clear();
        postorder.reserve(n);
        vector<int> st;
        for (int v = 1; v <= n; ++v) {
            if (parent[v] < 1 && !under_virt[v]) st.push_back(v);
        }
        // also include under_virt nodes as forest roots
        for (int v = 1; v <= n; ++v) {
            if (under_virt[v]) st.push_back(v);
        }
        // fallback: any node with no alive parent link already covered
        vector<char> seen(n + 1, 0);
        vector<int> all_roots = st;
        st.clear();
        for (int r : all_roots) {
            if (seen[r]) continue;
            st.push_back(r);
            seen[r] = 1;
            while (!st.empty()) {
                int u = st.back();
                st.pop_back();
                postorder.push_back(u);
                for (int v : ch[u]) {
                    if (!seen[v]) {
                        seen[v] = 1;
                        st.push_back(v);
                    }
                }
            }
        }
        for (int v = 1; v <= n; ++v) {
            if (!seen[v]) {
                // disconnected safety
                st.push_back(v);
                seen[v] = 1;
                while (!st.empty()) {
                    int u = st.back();
                    st.pop_back();
                    postorder.push_back(u);
                    for (int c : ch[u])
                        if (!seen[c]) {
                            seen[c] = 1;
                            st.push_back(c);
                        }
                }
            }
        }
    }

    void build(const vector<int>& par) {
        parent = par;
        extra.assign(n + 1, 0);
        under_virt.assign(n + 1, 0);
        rebuild_children_order();

        // leaf ranges for star / tiny DC
        vector<int> leaves;
        leaves.reserve(n);
        for (int v = 1; v <= n; ++v)
            if (ch[v].empty() && !under_virt[v] && parent[v] != 0 || ch[v].empty()) {
                // structural leaves in original sense: no children
            }
        leaves.clear();
        vector<int> st = {1};
        vector<char> vis(n + 1, 0);
        if (n >= 1) {
            st = {1};
            vis[1] = 1;
            while (!st.empty()) {
                int u = st.back();
                st.pop_back();
                if (ch[u].empty()) leaves.push_back(u);
                else
                    for (int v : ch[u])
                        if (!vis[v]) {
                            vis[v] = 1;
                            st.push_back(v);
                        }
            }
        }
        m = (int)leaves.size();
        vector<int> leaf_index(n + 1, -1);
        for (int i = 0; i < m; ++i) leaf_index[leaves[i]] = i;
        leaf_lo.assign(n + 1, m);
        leaf_hi.assign(n + 1, -1);
        for (int i = (int)postorder.size() - 1; i >= 0; --i) {
            int u = postorder[i];
            if (ch[u].empty()) {
                int idx = leaf_index[u];
                if (idx >= 0) leaf_lo[u] = leaf_hi[u] = idx;
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
            bool has_par = (parent[v] >= 1 && !is_deleted(parent[v]) && active[v]) || under_virt[v];
            if (!has_par) {
                int cnt = extra[v];
                for (int c : ch[v])
                    if (!is_deleted(c) && active[c]) ++cnt;
                if (cnt == 1) del_stamp[v] = del_id;
            }
        }

        bool any = false;
        for (int v = 1; v <= n; ++v)
            if (!is_deleted(v) && !is_leaf(v)) {
                any = true;
                break;
            }
        if (!any) return k % MOD;

        // Fast path: single component, no under_virt forest.
        bool any_virt = false;
        for (int v = 1; v <= n; ++v)
            if (!is_deleted(v) && under_virt[v]) {
                any_virt = true;
                break;
            }
        if (!any_virt && !is_deleted(1)) {
            bool single = true;
            for (int v = 2; v <= n; ++v) {
                if (is_deleted(v)) continue;
                if (!(parent[v] >= 1 && !is_deleted(parent[v]) && active[v])) {
                    single = false;
                    break;
                }
            }
            if (single) {
                long long res = k % MOD;
                for (int v = 1; v <= n; ++v) {
                    if (is_deleted(v) || is_leaf(v)) continue;
                    int deg = extra[v];
                    for (int c : ch[v])
                        if (!is_deleted(c) && active[c]) ++deg;
                    bool has_par = v > 1 && parent[v] >= 1 && !is_deleted(parent[v]) && active[v];
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
                if (parent[u] >= 1 && !is_deleted(parent[u]) && active[u]) {
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
                bool has_par = parent[v] >= 1 && !is_deleted(parent[v]) && active[v] && in_stamp[parent[v]] == cid;
                if (under_virt[v]) {
                    has_par = true;
                    ++deg;
                } else if (has_par) {
                    ++deg;
                }
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
        sort(raw_edges.begin(), raw_edges.end());
        raw_edges.erase(unique(raw_edges.begin(), raw_edges.end()), raw_edges.end());

        if (ctot == 1) return k % MOD;
        if (ctot == 2 && (int)raw_edges.size() == 1) return k * km1 % MOD;

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
                for (int v : adj[u])
                    if (seen[v] == -1) {
                        seen[v] = 0;
                        bfs.push_back(v);
                        nodes.push_back(v);
                    }
            }
            int vn = (int)nodes.size();
            vector<int> remap(ctot, -1);
            for (int i = 0; i < vn; ++i) remap[nodes[i]] = i;
            vector<vector<int>> sub(vn);
            for (int u : nodes)
                for (int v : adj[u]) sub[remap[u]].push_back(remap[v]);
            for (auto& row : sub) {
                sort(row.begin(), row.end());
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
            bool all = active[2] && active[3];
            return all ? k * km1 % MOD : k % MOD;
        }
        if (q == 0 && n <= 7 && m <= 7 && k != 2) {
            int ctot = merged_components();
            if (ctot <= 7) return answer_dc();
        }
        return tree_formula();
    }
};

// ---------------------------------------------------------------------------
// Phase A compression (proven subset of #11):
// 1) Init e(leaf)=1
// 2) Constant deg-2 chains + fold constant nodes with e<=1 into parent.e
// 3) Predictable non-supports: const upward path to a support -> rehang under
//    virtual -1 with under_virt, e(parent)+=1
// ---------------------------------------------------------------------------
static Solver compress_interval(const Solver& src, const vector<char>& unstable) {
    int n = src.n;
    vector<int> parent = src.parent;
    vector<int> extra = src.extra;
    if ((int)extra.size() != n + 1) extra.assign(n + 1, 0);
    vector<char> under = src.under_virt;
    if ((int)under.size() != n + 1) under.assign(n + 1, 0);
    vector<char> alive(n + 1, 1);
    vector<char> act = src.active;
    vector<char> tog(n + 1, 0);
    for (int v = 2; v <= n; ++v)
        if (v < (int)unstable.size() && unstable[v]) tog[v] = 1;

    vector<vector<int>> ch(n + 1);
    auto rebuild = [&]() {
        for (int i = 1; i <= n; ++i) ch[i].clear();
        for (int i = 1; i <= n; ++i) {
            if (!alive[i]) continue;
            int p = parent[i];
            if (p >= 1 && alive[p]) ch[p].push_back(i);
        }
    };

    // Init e on structural leaves if not already set from prior compression.
    rebuild();
    for (int v = 1; v <= n; ++v) {
        if (!alive[v]) continue;
        if (ch[v].empty() && extra[v] == 0 && !under[v]) extra[v] = 1;
    }

    auto fold = [&]() {
        bool changed = true;
        while (changed) {
            changed = false;
            rebuild();
            for (int v = 2; v <= n; ++v) {
                if (!alive[v] || tog[v] || parent[v] < 1 || under[v]) continue;
                // compact kids
                int kn = 0;
                for (int c : ch[v])
                    if (alive[c]) ch[v][kn++] = c;
                ch[v].resize(kn);
                int cnt = (int)ch[v].size();
                if (cnt == 1 && extra[v] == 0) {
                    int only = ch[v][0];
                    int p = parent[v];
                    act[only] = act[v] && act[only];
                    parent[only] = p;
                    alive[v] = 0;
                    changed = true;
                } else if (cnt == 0 && extra[v] <= 1 && act[v]) {
                    int p = parent[v];
                    if (p >= 1 && alive[p]) {
                        extra[p] += 1;
                        alive[v] = 0;
                        extra[v] = 0;
                        changed = true;
                    }
                }
            }
        }
    };

    fold();
    rebuild();

    // minf along constant child edges
    vector<int> minf(n + 1, 0);
    vector<int> order;
    order.reserve(n);
    vector<char> seen(n + 1, 0);
    vector<int> st;
    for (int v = 1; v <= n; ++v) {
        if (!alive[v]) continue;
        if (parent[v] < 1 || !alive[parent[v]] || under[v]) st.push_back(v);
    }
    for (int r : st) {
        if (seen[r]) continue;
        vector<int> stack = {r};
        seen[r] = 1;
        while (!stack.empty()) {
            int u = stack.back();
            stack.pop_back();
            order.push_back(u);
            for (int c : ch[u])
                if (alive[c] && !seen[c]) {
                    seen[c] = 1;
                    stack.push_back(c);
                }
        }
    }
    for (int i = (int)order.size() - 1; i >= 0; --i) {
        int v = order[i];
        int s = extra[v];
        for (int c : ch[v])
            if (alive[c] && !tog[c]) s += minf[c];
        minf[v] = s;
    }

    vector<char> is_sup(n + 1, 0);
    for (int v = 1; v <= n; ++v)
        if (alive[v] && minf[v] >= 2) is_sup[v] = 1;

    // Predictable non-supports with constant upward path to a support.
    for (int v = 1; v <= n; ++v) {
        if (!alive[v] || minf[v] < 1 || is_sup[v]) continue;
        if (parent[v] < 1 || tog[v] || under[v]) continue;
        int cur = v;
        bool found = false;
        while (true) {
            int p = parent[cur];
            if (p < 1 || !alive[p]) break;
            if (tog[cur]) break;
            if (is_sup[p]) {
                found = true;
                break;
            }
            cur = p;
        }
        if (!found) continue;
        int p = parent[v];
        if (p >= 1 && alive[p]) extra[p] += 1;
        parent[v] = 0;
        under[v] = 1;
        act[v] = 1;
    }

    fold();
    rebuild();

    // Remap
    vector<int> id(n + 1, 0);
    int nn = 0;
    for (int v = 1; v <= n; ++v)
        if (alive[v]) id[v] = ++nn;

    Solver t;
    t.n = nn;
    t.q = src.q;
    t.k = src.k;
    t.km1 = src.km1;
    t.parent.assign(nn + 1, 0);
    t.extra.assign(nn + 1, 0);
    t.under_virt.assign(nn + 1, 0);
    t.active.assign(nn + 1, 1);
    for (int v = 1; v <= n; ++v) {
        if (!alive[v]) continue;
        int nv = id[v];
        t.extra[nv] = extra[v];
        t.under_virt[nv] = under[v];
        t.active[nv] = act[v];
        if (!under[v] && parent[v] >= 1 && alive[parent[v]])
            t.parent[nv] = id[parent[v]];
        else
            t.parent[nv] = 0;
    }
    t.init_mod_inverses();
    t.build(t.parent);
    // build overwrites extra/under — restore
    t.extra.assign(nn + 1, 0);
    t.under_virt.assign(nn + 1, 0);
    t.active.assign(nn + 1, 1);
    for (int v = 1; v <= n; ++v) {
        if (!alive[v]) continue;
        int nv = id[v];
        t.extra[nv] = extra[v];
        t.under_virt[nv] = under[v];
        t.active[nv] = act[v];
        if (!under[v] && parent[v] >= 1 && alive[parent[v]])
            t.parent[nv] = id[parent[v]];
        else
            t.parent[nv] = 0;
    }
    t.rebuild_children_order();
    t.del_stamp.assign(nn + 1, 0);
    t.vis_stamp.assign(nn + 1, 0);
    t.in_stamp.assign(nn + 1, 0);
    // keep id map in active unused slot? return via side channel
    t.comp_buf = id;  // stash old->new map in comp_buf temporarily
    return t;
}

static void dc_solve(Solver tr, vector<int> ops, vector<long long>& out, int base) {
    int q = (int)ops.size();
    vector<char> unstable(tr.n + 1, 0);
    for (int v : ops)
        if (v >= 2 && v <= tr.n) unstable[v] = 1;

    Solver ct = compress_interval(tr, unstable);
    vector<int> id = ct.comp_buf;  // size tr.n+1
    ct.comp_buf.clear();
    ct.comp_buf.reserve(ct.n);
    ct.stack_buf.reserve(ct.n);

    vector<int> cops(q);
    for (int i = 0; i < q; ++i) {
        int v = ops[i];
        cops[i] = (v >= 1 && v < (int)id.size()) ? id[v] : 0;
    }

    long long cost = 1LL * ct.n * (q + 1);
    if (q <= 1 || ct.n <= 48 || cost <= 2'000'000LL) {
        CHI.reset(ct.k, ct.km1, ct.n);
        out[base] = ct.tree_formula();
        for (int i = 0; i < q; ++i) {
            if (cops[i] >= 2) ct.active[cops[i]] ^= 1;
            out[base + i + 1] = ct.tree_formula();
        }
        return;
    }

    int mid = q / 2;
    vector<int> left(cops.begin(), cops.begin() + mid);
    vector<int> right(cops.begin() + mid, cops.end());
    dc_solve(ct, left, out, base);
    for (int i = 0; i < mid; ++i)
        if (cops[i] >= 2) ct.active[cops[i]] ^= 1;
    dc_solve(ct, right, out, base + mid);
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
            vector<int> kids;
            for (int i = 2; i <= n; ++i)
                if (parent[i] == 1) kids.push_back(i);
            int m = (int)kids.size();
            vector<int> idx(n + 1);
            for (int i = 0; i < m; ++i) idx[kids[i]] = i;
            vector<char> active(n + 1, 1);
            long long km1 = (k - 1 + MOD) % MOD;
            DSU dsu;
            auto count = [&]() {
                dsu.init(m);
                for (int v : kids)
                    if (!active[v]) dsu.unite(idx[v], (idx[v] + 1) % m);
                vector<int> seen(m, -1);
                int c = 0;
                for (int i = 0; i < m; ++i) {
                    int r = dsu.find(i);
                    if (seen[r] < 0) seen[r] = c++;
                }
                return c;
            };
            cout << cycle_chi(count(), k, km1) << '\n';
            for (int v : ops) {
                active[v] ^= 1;
                cout << cycle_chi(count(), k, km1) << '\n';
            }
            continue;
        }

        Solver sol;
        sol.n = n;
        sol.q = q;
        sol.k = k;
        sol.km1 = (k - 1 + MOD) % MOD;
        sol.init_mod_inverses();
        sol.parent = parent;
        sol.extra.assign(n + 1, 0);
        sol.under_virt.assign(n + 1, 0);
        sol.active.assign(n + 1, 1);
        sol.build(parent);
        CHI.reset(k, sol.km1, n);
        sol.dc.k = k;
        sol.dc.memo.clear();

        vector<char> unstable(n + 1, 0);
        for (int v : ops)
            if (v >= 2 && v <= n) unstable[v] = 1;

        Solver ct = compress_interval(sol, unstable);
        vector<int> id = ct.comp_buf;
        ct.comp_buf.clear();
        ct.comp_buf.reserve(ct.n);
        ct.stack_buf.reserve(ct.n);
        CHI.reset(k, ct.km1, ct.n);

        vector<int> cops(q);
        for (int i = 0; i < q; ++i) {
            int v = ops[i];
            cops[i] = (v >= 1 && v < (int)id.size()) ? id[v] : 0;
        }

        if (q > 0 && 1LL * ct.n * (q + 1) > 2'000'000LL) {
            vector<long long> answers(q + 1);
            dc_solve(ct, cops, answers, 0);
            for (long long x : answers) cout << x << '\n';
        } else {
            cout << ct.tree_formula() << '\n';
            for (int v : cops) {
                if (v >= 2) ct.active[v] ^= 1;
                cout << ct.tree_formula() << '\n';
            }
        }
    }
    return 0;
}
