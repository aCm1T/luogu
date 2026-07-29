#include <algorithm>
#include <cstdint>
#include <iostream>
#include <vector>

using namespace std;

using int64 = long long;

struct Edge {
    int to;
    int64 weight;
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n, q;
    cin >> n >> q;

    vector<vector<Edge>> graph(n);
    int64 totalEdgeWeight = 0;
    for (int i = 0; i < n - 1; ++i) {
        int u, v;
        int64 w;
        cin >> u >> v >> w;
        --u;
        --v;
        graph[u].push_back({v, w});
        graph[v].push_back({u, w});
        totalEdgeWeight += w;
    }

    // LCA data for one fixed root. An LCA under any query root can then be
    // obtained from three fixed-root LCAs.
    constexpr int LOG = 12;
    vector<vector<int>> up(LOG, vector<int>(n));
    vector<int> level(n);
    vector<int64> distanceFromZero(n);
    vector<int> traversal;
    traversal.reserve(n);
    traversal.push_back(0);
    up[0][0] = 0;

    for (int i = 0; i < n; ++i) {
        const int u = traversal[i];
        for (const Edge& edge : graph[u]) {
            if (edge.to == up[0][u]) {
                continue;
            }
            up[0][edge.to] = u;
            level[edge.to] = level[u] + 1;
            distanceFromZero[edge.to] = distanceFromZero[u] + edge.weight;
            traversal.push_back(edge.to);
        }
    }

    for (int bit = 1; bit < LOG; ++bit) {
        for (int u = 0; u < n; ++u) {
            up[bit][u] = up[bit - 1][up[bit - 1][u]];
        }
    }

    auto lca = [&](int u, int v) {
        if (level[u] < level[v]) {
            swap(u, v);
        }
        int difference = level[u] - level[v];
        for (int bit = 0; bit < LOG; ++bit) {
            if ((difference >> bit) & 1) {
                u = up[bit][u];
            }
        }
        if (u == v) {
            return u;
        }
        for (int bit = LOG - 1; bit >= 0; --bit) {
            if (up[bit][u] != up[bit][v]) {
                u = up[bit][u];
                v = up[bit][v];
            }
        }
        return up[0][u];
    };

    auto distance = [&](int u, int v) {
        const int ancestor = lca(u, v);
        return distanceFromZero[u] + distanceFromZero[v]
            - 2 * distanceFromZero[ancestor];
    };

    /*
     * parentTowardRoot[r][u] is the first neighbour on the path u -> r.
     * farthestOutside[r][u] is the greatest distance from r to a node
     * outside u's subtree when the tree is rooted at r.
     *
     * For each root, every subtree is a contiguous preorder interval.
     * Prefix and suffix maxima therefore compute all outside maxima in O(n).
     */
    const size_t matrixSize = static_cast<size_t>(n) * n;
    vector<uint16_t> parentTowardRoot(matrixSize);
    vector<int64> farthestOutside(matrixSize);
    vector<int64> eccentricity(n);

    vector<int> order;
    vector<int> preorderIndex(n);
    vector<int> subtreeSize(n);
    vector<int64> rootedDistance(n);
    vector<int64> prefixMaximum(n + 1);
    vector<int64> suffixMaximum(n + 1);
    order.reserve(n);
    vector<int> dfsStack;
    dfsStack.reserve(n);

    for (int root = 0; root < n; ++root) {
        const size_t row = static_cast<size_t>(root) * n;
        order.clear();
        dfsStack.clear();
        dfsStack.push_back(root);
        parentTowardRoot[row + root] = static_cast<uint16_t>(root);
        rootedDistance[root] = 0;

        while (!dfsStack.empty()) {
            const int u = dfsStack.back();
            dfsStack.pop_back();
            preorderIndex[u] = static_cast<int>(order.size());
            order.push_back(u);
            subtreeSize[u] = 1;
            const int parent = parentTowardRoot[row + u];
            for (const Edge& edge : graph[u]) {
                if (edge.to == parent) {
                    continue;
                }
                parentTowardRoot[row + edge.to] =
                    static_cast<uint16_t>(u);
                rootedDistance[edge.to] = rootedDistance[u] + edge.weight;
                dfsStack.push_back(edge.to);
            }
        }

        for (int i = n - 1; i > 0; --i) {
            const int u = order[i];
            subtreeSize[parentTowardRoot[row + u]] += subtreeSize[u];
        }

        constexpr int64 NONE = -1;
        prefixMaximum[0] = NONE;
        for (int i = 0; i < n; ++i) {
            prefixMaximum[i + 1] =
                max(prefixMaximum[i], rootedDistance[order[i]]);
        }
        suffixMaximum[n] = NONE;
        for (int i = n - 1; i >= 0; --i) {
            suffixMaximum[i] =
                max(suffixMaximum[i + 1], rootedDistance[order[i]]);
        }
        eccentricity[root] = prefixMaximum[n];

        for (int u = 0; u < n; ++u) {
            const int left = preorderIndex[u];
            const int right = left + subtreeSize[u];
            farthestOutside[row + u] =
                max(prefixMaximum[left], suffixMaximum[right]);
        }
    }

    const int64 twiceAllEdges = 2 * totalEdgeWeight;

    while (q--) {
        int start, key, trap;
        cin >> start >> key >> trap;
        --start;
        --key;
        --trap;

        // L is LCA(start, trap) when the tree is rooted at key.
        const int x = lca(start, trap);
        const int y = lca(start, key);
        const int z = lca(trap, key);
        int l;
        if (x == y) {
            l = z;
        } else if (x == z) {
            l = y;
        } else {
            l = x;
        }

        // Then trap is on the only path from start to key.
        if (l == trap) {
            cout << "impossible\n";
            continue;
        }

        /*
         * A walk ending at E normally costs 2R-dist(start,E). If E is in
         * subtree_key(l), the key-before-trap order adds nothing. Otherwise,
         * simplifying the necessary extra traversal gives
         * 2R-dist(E,key)+dist(l,key)-dist(l,start).
         */
        int64 farthestEndpointInLSubtree;
        if (l == key) {
            farthestEndpointInLSubtree = eccentricity[start];
        } else {
            const int towardKey =
                parentTowardRoot[static_cast<size_t>(key) * n + l];

            // Since start is in subtree_key(l), rooting at start makes the
            // complement of subtree_key(l) exactly subtree_start(towardKey).
            farthestEndpointInLSubtree =
                farthestOutside[static_cast<size_t>(start) * n + towardKey];
        }

        int64 answer = twiceAllEdges - farthestEndpointInLSubtree;

        if (l != key) {
            const int64 farthestEndpointOutsideLSubtree =
                farthestOutside[static_cast<size_t>(key) * n + l];
            const int64 endingOutsideCost =
                twiceAllEdges - farthestEndpointOutsideLSubtree
                + distance(l, key) - distance(l, start);
            answer = min(answer, endingOutsideCost);
        }

        cout << answer << '\n';
    }

    return 0;
}
