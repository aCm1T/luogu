#include <bits/stdc++.h>
using namespace std;

const double INF = 1e100;
const double EPS = 1e-9;

struct Point {
    double x, y;

    Point operator+(const Point& o) const { return {x + o.x, y + o.y}; }
    Point operator-(const Point& o) const { return {x - o.x, y - o.y}; }
    Point operator*(double k) const { return {x * k, y * k}; }
    Point operator/(double k) const { return {x / k, y / k}; }
};

static double dot(Point a, Point b) {
    return a.x * b.x + a.y * b.y;
}

static double cross(Point a, Point b) {
    return a.x * b.y - a.y * b.x;
}

static double norm(Point a) {
    return sqrt(dot(a, a));
}

static Point reflectPoint(Point p, Point a, Point b) {
    Point d = b - a;
    double t = dot(p - a, d) / dot(d, d);
    Point proj = a + d * t;
    return proj * 2.0 - p;
}

struct Edge {
    Point a, b;
};

static double bounceCost(Point start, Point goal, const vector<Edge>& edges, int l, int r) {
    if (l > r) return norm(goal - start);

    vector<Edge> unfolded;
    unfolded.reserve(r - l + 1);
    Point target = goal;
    vector<Edge> cur = edges;

    for (int i = l; i <= r; ++i) {
        unfolded.push_back(cur[i]);
        Point a = cur[i].a;
        Point b = cur[i].b;
        for (int j = i + 1; j <= r; ++j) {
            cur[j].a = reflectPoint(cur[j].a, a, b);
            cur[j].b = reflectPoint(cur[j].b, a, b);
        }
        target = reflectPoint(target, a, b);
    }

    Point dir = target - start;
    double len = norm(dir);
    if (len < EPS) return INF;

    double lastT = -1e-8;
    for (const Edge& e : unfolded) {
        Point s = e.a;
        Point ed = e.b;
        Point w = ed - s;
        double den = cross(dir, w);
        if (fabs(den) < 1e-10) return INF;
        Point rhs = s - start;
        double t = cross(rhs, w) / den;
        double u = cross(rhs, dir) / den;
        if (t < -1e-8 || t > 1.0 + 1e-8) return INF;
        if (u < -1e-8 || u > 1.0 + 1e-8) return INF;
        if (t + 1e-8 < lastT) return INF;
        lastT = t;
    }

    return len;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    double px, py;
    int tc = 1;
    cout.setf(ios::fixed);
    cout << setprecision(2);

    while (cin >> n >> px >> py) {
        Point start{px, py};
        vector<Point> poly(n);
        for (int i = 0; i < n; ++i) cin >> poly[i].x >> poly[i].y;

        double answer = INF;
        for (int first = 0; first < n; ++first) {
            vector<Point> v(n + 1);
            for (int i = 0; i <= n; ++i) v[i] = poly[(first + i) % n];

            vector<Edge> edges(n);
            for (int i = 0; i < n; ++i) edges[i] = {v[i], v[i + 1]};

            vector<double> dp(n + 1, INF);
            dp[0] = 0.0;

            for (int pos = 0; pos <= n; ++pos) {
                if (dp[pos] >= INF / 2) continue;
                Point here = (pos == 0 ? start : v[pos - 1]);

                double finish = bounceCost(here, start, edges, pos, n - 1);
                answer = min(answer, dp[pos] + finish);

                for (int j = pos; j <= n; ++j) {
                    int nextPos = min(n, j + 1);
                    if (nextPos <= pos) continue;
                    Point there = v[j % n];
                    int l = pos;
                    int r = j - 2;
                    double cost = bounceCost(here, there, edges, l, r);
                    if (cost >= INF / 2) continue;
                    dp[nextPos] = min(dp[nextPos], dp[pos] + cost);
                }
            }
        }

        cout << "Case " << tc++ << ": " << answer + 1e-9 << '\n';
    }

    return 0;
}
