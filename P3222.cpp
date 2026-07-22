#include <bits/stdc++.h>
using namespace std;

/*
 * P3222 [HNOI2012] 射箭
 *
 * Parabola through the origin: y = a x^2 + b x with a < 0, b > 0.
 * Target (x, [y1, y2]) becomes
 *   y1/x <= a x + b <= y2/x,
 * i.e. two half-planes in the (a, b)-plane.
 *
 * Feasibility is monotone in the prefix length, so binary-search the answer
 * and test each mid with half-plane intersection. All candidate half-planes
 * (plus a box clipping a < 0, b > 0) are polar-sorted once; each check only
 * keeps planes whose target index is <= mid.
 */

using LD = long double;

static constexpr int N = 200010;
static constexpr LD INF = 1e12L;
static constexpr LD EPS = 1e-12L;

struct Vec {
    LD x, y;
    Vec() : x(0), y(0) {}
    Vec(LD x, LD y) : x(x), y(y) {}
    Vec operator+(const Vec &o) const { return {x + o.x, y + o.y}; }
    Vec operator-(const Vec &o) const { return {x - o.x, y - o.y}; }
    Vec operator*(LD k) const { return {x * k, y * k}; }
    LD operator^(const Vec &o) const { return x * o.y - y * o.x; }
};

struct Line {
    Vec p, v;
    LD ang;
    int id;
    Line() : ang(0), id(0) {}
    Line(Vec a, Vec b, int id) : p(a), v(b - a), ang(atan2((double)v.y, (double)v.x)), id(id) {}
    bool operator<(const Line &o) const { return ang < o.ang; }
    // Strictly to the right of the directed line (on-line points are kept).
    bool right(const Vec &q) const { return (v ^ (q - p)) < -EPS; }
};

static Vec intersect(const Line &a, const Line &b) {
    LD t = (b.v ^ (b.p - a.p)) / (b.v ^ a.v);
    return a.p + a.v * t;
}

static Line lines[N], que[N];
static Vec pts[N];
static int midAns;

static bool halfPlane(int tot) {
    int i = 0;
    while (i < tot && lines[i].id > midAns) ++i;
    if (i >= tot) return false;

    int h = 0, t = 0;
    que[0] = lines[i++];
    for (; i < tot; ++i) {
        if (lines[i].id > midAns) continue;
        while (h < t && lines[i].right(pts[t - 1])) --t;
        while (h < t && lines[i].right(pts[h])) ++h;
        // Parallel: keep the more restrictive one.
        if (fabsl(lines[i].ang - que[t].ang) > EPS) {
            que[++t] = lines[i];
        } else if (lines[i].right(que[t].p)) {
            que[t] = lines[i];
        }
        if (h < t) pts[t - 1] = intersect(que[t - 1], que[t]);
    }
    while (h < t && que[h].right(pts[t - 1])) --t;
    return t - h > 1;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    if (!(cin >> n)) return 0;

    int tot = 0;
    for (int i = 1; i <= n; ++i) {
        LD x, y1, y2;
        cin >> x >> y1 >> y2;
        // b >= -x a + y1/x  (above the line, left of left->right directed edge)
        lines[tot++] = Line(Vec(0, y1 / x), Vec(1, y1 / x - x), i);
        // b <= -x a + y2/x  (below the line, left of right->left directed edge)
        lines[tot++] = Line(Vec(1, y2 / x - x), Vec(0, y2 / x), i);
    }
    // Clip to a < 0, b > 0 with a large open box (EPS so a,b never hit 0).
    lines[tot++] = Line(Vec(-INF, EPS), Vec(-EPS, EPS), 0);
    lines[tot++] = Line(Vec(-EPS, EPS), Vec(-EPS, INF), 0);
    lines[tot++] = Line(Vec(-EPS, INF), Vec(-INF, INF), 0);
    lines[tot++] = Line(Vec(-INF, INF), Vec(-INF, EPS), 0);

    sort(lines, lines + tot);

    int lo = 1, hi = n;
    while (lo < hi) {
        midAns = (lo + hi + 1) >> 1;
        if (halfPlane(tot)) lo = midAns;
        else hi = midAns - 1;
    }
    cout << lo << '\n';
    return 0;
}
