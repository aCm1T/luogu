#include <algorithm>
#include <cmath>
#include <iostream>
#include <set>
#include <utility>
#include <vector>

using namespace std;

const long double EPS = 1e-10L;

struct Point {
    long double x;
    long double y;
};

struct Box {
    long double minx, maxx, miny, maxy;
};

static bool eq(long double a, long double b) {
    return fabsl(a - b) <= EPS;
}

static vector<long double> unique_sorted(vector<long double> v) {
    sort(v.begin(), v.end());
    vector<long double> out;
    for (long double x : v) {
        if (out.empty() || !eq(out.back(), x)) out.push_back(x);
    }
    return out;
}

static Box bbox(const vector<Point> &p) {
    Box b{p[0].x, p[0].x, p[0].y, p[0].y};
    for (const Point &q : p) {
        b.minx = min(b.minx, q.x);
        b.maxx = max(b.maxx, q.x);
        b.miny = min(b.miny, q.y);
        b.maxy = max(b.maxy, q.y);
    }
    return b;
}

static bool inside_polygon(const vector<Point> &poly, long double x, long double y) {
    bool inside = false;
    int n = static_cast<int>(poly.size());
    for (int i = 0, j = n - 1; i < n; j = i++) {
        const Point &a = poly[j];
        const Point &b = poly[i];
        if ((a.y > y) != (b.y > y)) {
            long double x_intersect = a.x + (b.x - a.x) * (y - a.y) / (b.y - a.y);
            if (x_intersect > x) inside = !inside;
        }
    }
    return inside;
}

static bool translation_works(const vector<Point> &hole, const vector<Point> &cover,
                              const Box &hb, const Box &cb,
                              long double dx, long double dy) {
    if (hb.minx + dx < cb.minx - EPS || hb.maxx + dx > cb.maxx + EPS ||
        hb.miny + dy < cb.miny - EPS || hb.maxy + dy > cb.maxy + EPS) {
        return false;
    }

    vector<long double> xs, ys;
    xs.reserve(hole.size() + cover.size());
    ys.reserve(hole.size() + cover.size());
    for (const Point &p : cover) {
        xs.push_back(p.x);
        ys.push_back(p.y);
    }
    for (const Point &p : hole) {
        xs.push_back(p.x + dx);
        ys.push_back(p.y + dy);
    }
    xs = unique_sorted(xs);
    ys = unique_sorted(ys);

    for (size_t i = 0; i + 1 < xs.size(); ++i) {
        if (xs[i + 1] - xs[i] <= EPS) continue;
        long double x = (xs[i] + xs[i + 1]) / 2;
        for (size_t j = 0; j + 1 < ys.size(); ++j) {
            if (ys[j + 1] - ys[j] <= EPS) continue;
            long double y = (ys[j] + ys[j + 1]) / 2;
            if (inside_polygon(hole, x - dx, y - dy) &&
                !inside_polygon(cover, x, y)) {
                return false;
            }
        }
    }

    return true;
}

static void add_candidate(vector<pair<long double, long double>> &cand,
                          long double dx, long double dy) {
    cand.push_back({dx, dy});
}

static bool can_cover(const vector<Point> &hole, const vector<Point> &cover) {
    Box hb = bbox(hole);
    Box cb = bbox(cover);

    vector<long double> dx_values;
    vector<long double> dy_values;
    for (const Point &c : cover) {
        for (const Point &h : hole) {
            dx_values.push_back(c.x - h.x);
            dy_values.push_back(c.y - h.y);
        }
    }
    dx_values = unique_sorted(dx_values);
    dy_values = unique_sorted(dy_values);

    vector<pair<long double, long double>> candidates;
    candidates.reserve(cover.size() * hole.size() + 16);
    for (const Point &c : cover) {
        for (const Point &h : hole) {
            add_candidate(candidates, c.x - h.x, c.y - h.y);
        }
    }

    vector<long double> bbox_dx = {cb.minx - hb.minx, cb.maxx - hb.maxx};
    vector<long double> bbox_dy = {cb.miny - hb.miny, cb.maxy - hb.maxy};
    for (long double dx : bbox_dx) {
        for (long double dy : bbox_dy) add_candidate(candidates, dx, dy);
    }

    sort(candidates.begin(), candidates.end(), [](const auto &a, const auto &b) {
        if (!eq(a.first, b.first)) return a.first < b.first;
        return a.second + EPS < b.second;
    });
    candidates.erase(unique(candidates.begin(), candidates.end(), [](const auto &a, const auto &b) {
        return eq(a.first, b.first) && eq(a.second, b.second);
    }), candidates.end());

    for (const auto &p : candidates) {
        if (translation_works(hole, cover, hb, cb, p.first, p.second)) return true;
    }

    vector<long double> filtered_dx;
    vector<long double> filtered_dy;
    for (long double dx : dx_values) {
        if (dx + hb.minx >= cb.minx - EPS && dx + hb.maxx <= cb.maxx + EPS) {
            filtered_dx.push_back(dx);
        }
    }
    for (long double dy : dy_values) {
        if (dy + hb.miny >= cb.miny - EPS && dy + hb.maxy <= cb.maxy + EPS) {
            filtered_dy.push_back(dy);
        }
    }

    unsigned long long product = static_cast<unsigned long long>(filtered_dx.size()) *
                                 static_cast<unsigned long long>(filtered_dy.size());
    if (product <= 200000ULL) {
        for (long double dx : filtered_dx) {
            for (long double dy : filtered_dy) {
                if (translation_works(hole, cover, hb, cb, dx, dy)) return true;
            }
        }
    }

    return false;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int h, c;
    int tc = 1;
    while (cin >> h >> c && (h != 0 || c != 0)) {
        vector<Point> hole(h), cover(c);
        for (Point &p : hole) cin >> p.x >> p.y;
        for (Point &p : cover) cin >> p.x >> p.y;

        cout << "Hole " << tc++ << ": " << (can_cover(hole, cover) ? "Yes" : "No") << '\n';
    }

    return 0;
}
