#include <algorithm>
#include <cmath>
#include <iostream>
#include <numeric>
#include <set>
#include <vector>

using namespace std;

struct Point {
    double p[3]{};
    double v[3]{};

    double distanceTo(const Point& other, double t) const {
        double sumsq = 0.0;
        for (int i = 0; i < 3; ++i) {
            double delta = p[i] + t * v[i] - (other.p[i] + t * other.v[i]);
            sumsq += delta * delta;
        }
        return sqrt(sumsq);
    }

    Point diff(const Point& other) const {
        Point ans;
        for (int i = 0; i < 3; ++i) {
            ans.p[i] = p[i] - other.p[i];
            ans.v[i] = v[i] - other.v[i];
        }
        return ans;
    }

    vector<double> crossings(const Point& other) const {
        double a = 0.0, b = 0.0, c = 0.0;
        for (int i = 0; i < 3; ++i) {
            a += v[i] * v[i] - other.v[i] * other.v[i];
            b += 2.0 * (p[i] * v[i] - other.p[i] * other.v[i]);
            c += p[i] * p[i] - other.p[i] * other.p[i];
        }

        vector<double> ans;
        if (a == 0.0) {
            if (b == 0.0) return ans;
            double t1 = -c / b;
            if (t1 > 0.0) ans.push_back(t1 + 5e-7);
            return ans;
        }

        double disc = b * b - 4.0 * a * c;
        if (disc < 0.0) return ans;

        double root = sqrt(disc);
        double t1 = (-b + root) / (2.0 * a);
        double t2 = (-b - root) / (2.0 * a);
        if (t1 > 0.0) ans.push_back(t1 + 5e-7);
        if (t2 > 0.0) ans.push_back(t2 + 5e-7);
        return ans;
    }
};

struct Edge {
    int v1 = 0;
    int v2 = 0;
    int hash = 0;
    double distance = 0.0;

    Edge() = default;
    Edge(int a, int b) {
        v1 = min(a, b);
        v2 = max(a, b);
        hash = 100 * v1 + v2;
    }
};

struct State {
    Edge e1;
    Edge e2;
    double t = 0.0;
};

struct StateLess {
    bool operator()(const State& a, const State& b) const {
        if (a.t < b.t - 1e-9) return true;
        if (a.t > b.t + 1e-9) return false;
        if (a.e1.hash != b.e1.hash) return a.e1.hash < b.e1.hash;
        return a.e2.hash < b.e2.hash;
    }
};

struct DSU {
    vector<int> parent;
    vector<int> height;

    explicit DSU(int n) : parent(n), height(n, 0) {
        iota(parent.begin(), parent.end(), 0);
    }

    int find(int child) {
        while (parent[child] != child) child = parent[child];
        return child;
    }

    bool unite(int a, int b) {
        int root1 = find(a);
        int root2 = find(b);
        if (root1 == root2) return false;

        if (height[root1] > height[root2]) {
            parent[root2] = root1;
        } else if (height[root1] == height[root2]) {
            parent[root2] = root1;
            ++height[root1];
        } else {
            parent[root1] = root2;
        }
        return true;
    }
};

void setDistance(vector<Edge>& edges, const vector<Point>& points, double t) {
    for (Edge& edge : edges) {
        edge.distance = points[edge.v1].distanceTo(points[edge.v2], t);
    }
}

vector<Edge> getMST(const vector<Edge>& edges, int n, set<int>& nextMST) {
    vector<Edge> sorted = edges;
    sort(sorted.begin(), sorted.end(), [](const Edge& a, const Edge& b) {
        if (a.distance < b.distance - 1e-9) return true;
        if (a.distance > b.distance + 1e-9) return false;
        return a.hash < b.hash;
    });

    DSU dsu(n);
    vector<Edge> ans;
    nextMST.clear();
    for (const Edge& edge : sorted) {
        if (dsu.unite(edge.v1, edge.v2)) {
            ans.push_back(edge);
            nextMST.insert(edge.hash);
            if ((int)ans.size() == n - 1) break;
        }
    }
    return ans;
}

bool differentMST(const set<int>& a, const set<int>& b) {
    for (int x : a) {
        if (!b.count(x)) return true;
    }
    return false;
}

vector<Edge> getAllEdges(const vector<Point>& points) {
    vector<Edge> edges;
    for (int i = 0; i < (int)points.size(); ++i) {
        for (int j = i + 1; j < (int)points.size(); ++j) {
            edges.emplace_back(i, j);
        }
    }
    return edges;
}

set<State, StateLess> getCriticalPoints(const vector<Edge>& allEdges, const vector<Point>& points) {
    set<State, StateLess> critPts;
    for (int i = 0; i < (int)allEdges.size(); ++i) {
        for (int j = i + 1; j < (int)allEdges.size(); ++j) {
            const Edge& e1 = allEdges[i];
            const Edge& e2 = allEdges[j];
            Point first = points[e1.v1].diff(points[e1.v2]);
            Point second = points[e2.v1].diff(points[e2.v2]);
            for (double t : first.crossings(second)) {
                critPts.insert(State{e1, e2, t});
            }
        }
    }
    return critPts;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    int loop = 1;
    while (cin >> n) {
        vector<Point> points(n);
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < 3; ++j) cin >> points[i].p[j];
            for (int j = 0; j < 3; ++j) cin >> points[i].v[j];
        }

        vector<Edge> allEdges = getAllEdges(points);
        set<State, StateLess> critPts = getCriticalPoints(allEdges, points);

        setDistance(allEdges, points, 0.0);
        set<int> curMST, nextMST;
        vector<Edge> mst = getMST(allEdges, n, nextMST);
        curMST = nextMST;

        int res = 1;
        while (!critPts.empty()) {
            State next = *critPts.begin();
            critPts.erase(critPts.begin());

            bool status1 = curMST.count(next.e1.hash) != 0;
            bool status2 = curMST.count(next.e2.hash) != 0;
            if ((status1 && status2) || (!status1 && !status2)) continue;

            if (!status1) mst.push_back(next.e1);
            if (!status2) mst.push_back(next.e2);

            setDistance(mst, points, next.t);
            mst = getMST(mst, n, nextMST);

            if (differentMST(curMST, nextMST)) {
                ++res;
                curMST = nextMST;
            }
        }

        cout << "Case " << loop << ": " << res << '\n';
        ++loop;
    }

    return 0;
}
