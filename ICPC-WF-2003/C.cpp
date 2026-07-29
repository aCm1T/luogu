#include <algorithm>
#include <array>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <limits>
#include <vector>

using namespace std;

struct Transform {
    long long ox;
    long long oy;
    int sx;  // +1: x = ox + local x, -1: x = ox + width - local x
};

struct Candidate {
    long double d2;
    long long x;
    long long y;
    unsigned long long idx;
};

static const array<int, 9> CX = {0, 1, 2, 2, 1, 0, 0, 1, 2};
static const array<int, 9> CY = {0, 0, 0, 1, 1, 1, 2, 2, 2};
static const array<bool, 9> REV = {false, true, false, false, true, false, false, true, false};
static const long double EPS = 1e-12L;

static array<unsigned long long, 9> pow9v;
static array<long long, 9> widthv;

static long long map_x(const Transform &t, long long width, long long local_x) {
    return t.sx == 1 ? t.ox + local_x : t.ox + width - local_x;
}

static bool better(const Candidate &a, const Candidate &b) {
    if (a.d2 + EPS < b.d2) return true;
    if (b.d2 + EPS < a.d2) return false;
    if (a.x != b.x) return a.x < b.x;
    if (a.y != b.y) return a.y < b.y;
    return a.idx < b.idx;
}

static Candidate search_curve(int order, const Transform &t, long double tx, long double ty) {
    long long width = widthv[order];
    Candidate best{numeric_limits<long double>::infinity(), 0, 0, 0};

    if (order == 1) {
        for (int i = 0; i < 9; ++i) {
            long long gx = map_x(t, width, CX[i]);
            long long gy = t.oy + CY[i];
            long double dx = tx - gx;
            long double dy = ty - gy;
            Candidate cur{dx * dx + dy * dy, gx, gy, static_cast<unsigned long long>(i)};
            if (better(cur, best)) best = cur;
        }
        return best;
    }

    long long child_width = widthv[order - 1];
    long long step = child_width + 1;

    struct Child {
        long double lb;
        int high_idx;
        Transform tr;
    };

    vector<Child> children;
    children.reserve(9);

    for (int i = 0; i < 9; ++i) {
        long long px0 = CX[i] * step + ((i % 2 == 0) ? 0 : child_width);
        long long px1 = CX[i] * step + ((i % 2 == 0) ? child_width : 0);
        long long gx0 = map_x(t, width, px0);
        long long gx1 = map_x(t, width, px1);

        Transform child;
        child.ox = min(gx0, gx1);
        child.oy = t.oy + CY[i] * step;
        child.sx = (gx1 > gx0) ? 1 : -1;

        long double dx = 0;
        if (tx < child.ox) dx = child.ox - tx;
        else if (tx > child.ox + child_width) dx = tx - (child.ox + child_width);

        long double dy = 0;
        if (ty < child.oy) dy = child.oy - ty;
        else if (ty > child.oy + child_width) dy = ty - (child.oy + child_width);

        children.push_back({dx * dx + dy * dy, i, child});
    }

    sort(children.begin(), children.end(), [](const Child &a, const Child &b) {
        if (fabsl(a.lb - b.lb) > EPS) return a.lb < b.lb;
        return a.high_idx < b.high_idx;
    });

    unsigned long long child_count = pow9v[order - 1];
    for (const Child &child : children) {
        if (child.lb > best.d2 + EPS) continue;
        Candidate cur = search_curve(order - 1, child.tr, tx, ty);
        if (REV[child.high_idx]) {
            cur.idx = static_cast<unsigned long long>(child.high_idx) * child_count
                    + (child_count - 1 - cur.idx);
        } else {
            cur.idx = static_cast<unsigned long long>(child.high_idx) * child_count + cur.idx;
        }
        if (better(cur, best)) best = cur;
    }

    return best;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    pow9v[0] = 1;
    long long p3 = 1;
    widthv[0] = 0;
    for (int i = 1; i <= 8; ++i) {
        pow9v[i] = pow9v[i - 1] * 9ULL;
        p3 *= 3;
        widthv[i] = p3 - 1;
    }

    int order;
    int tc = 1;
    while (cin >> order && order != 0) {
        long double x1, y1, x2, y2;
        cin >> x1 >> y1 >> x2 >> y2;

        long double width = static_cast<long double>(widthv[order]);
        Transform root{0, 0, 1};
        Candidate a = search_curve(order, root, x1 * width, y1 * width);
        Candidate b = search_curve(order, root, x2 * width, y2 * width);

        long double stub1 = sqrtl(a.d2) / width;
        long double stub2 = sqrtl(b.d2) / width;
        long double bus = static_cast<long double>(
            a.idx > b.idx ? a.idx - b.idx : b.idx - a.idx) / width;
        long double ans = stub1 + stub2 + bus;

        if (tc > 1) cout << '\n';
        cout << "Case " << tc++ << ".  Distance is "
             << fixed << setprecision(4) << static_cast<double>(ans) << '\n';
    }

    return 0;
}
