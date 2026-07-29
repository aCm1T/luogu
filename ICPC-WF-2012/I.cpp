#include <bits/stdc++.h>
using namespace std;

struct Interval {
    int a, b;
};

struct Segment {
    int fixed, a, b;
};

struct Trace {
    vector<Segment> horizontal;
    vector<Segment> vertical;
    bool opens = false;
};

struct Fenwick {
    int n = 0;
    vector<int> bit;

    Fenwick() = default;
    explicit Fenwick(int n_) { init(n_); }

    void init(int n_) {
        n = n_;
        bit.assign(n + 2, 0);
    }

    void add(int idx, int delta) {
        for (; idx <= n; idx += idx & -idx) bit[idx] += delta;
    }

    int sumPrefix(int idx) const {
        int res = 0;
        for (; idx > 0; idx -= idx & -idx) res += bit[idx];
        return res;
    }

    int rangeSum(int l, int r) const {
        if (l > r) return 0;
        return sumPrefix(r) - sumPrefix(l - 1);
    }

    int firstInRange(int l, int r) const {
        if (rangeSum(l, r) == 0) return -1;
        int before = sumPrefix(l - 1);
        int target = before + 1;
        int idx = 0;
        int mask = 1;
        while ((mask << 1) <= n) mask <<= 1;
        for (int step = mask; step; step >>= 1) {
            int nxt = idx + step;
            if (nxt <= n && bit[nxt] < target) {
                idx = nxt;
                target -= bit[nxt];
            }
        }
        return idx + 1;
    }
};

struct MirrorField {
    int rows, cols;
    map<pair<int, int>, char> mirrors;
    unordered_map<int, set<int>> byRow;
    unordered_map<int, set<int>> byCol;

    char at(int r, int c) const {
        auto it = mirrors.find({r, c});
        return it == mirrors.end() ? 0 : it->second;
    }

    void addMirror(int r, int c, char ch) {
        mirrors[{r, c}] = ch;
        byRow[r].insert(c);
        byCol[c].insert(r);
    }

    Trace trace(int startR, int startC, int dir, bool fromExit) const {
        Trace tr;
        int r = startR, c = startC;
        set<tuple<int, int, int>> seen;

        auto addHorizontal = [&](int row, int l, int rr) {
            l = max(l, 1);
            rr = min(rr, cols);
            if (l <= rr) tr.horizontal.push_back({row, l, rr});
        };
        auto addVertical = [&](int col, int lo, int hi) {
            lo = max(lo, 1);
            hi = min(hi, rows);
            if (lo <= hi) tr.vertical.push_back({col, lo, hi});
        };

        while (true) {
            if (!seen.insert({r, c, dir}).second) break;

            int nr = r, nc = c;
            bool hit = false;
            if (dir == 0) {
                auto itMap = byRow.find(r);
                if (itMap != byRow.end()) {
                    auto it = itMap->second.upper_bound(c);
                    if (it != itMap->second.end()) {
                        nc = *it;
                        hit = true;
                    }
                }
                if (hit) {
                    addHorizontal(r, c + 1, nc - 1);
                } else {
                    addHorizontal(r, c + 1, cols);
                    tr.opens = (!fromExit && r == rows);
                    break;
                }
            } else if (dir == 2) {
                auto itMap = byRow.find(r);
                if (itMap != byRow.end()) {
                    auto it = itMap->second.lower_bound(c);
                    if (it != itMap->second.begin()) {
                        --it;
                        nc = *it;
                        hit = true;
                    }
                }
                if (hit) {
                    addHorizontal(r, nc + 1, c - 1);
                } else {
                    addHorizontal(r, 1, c - 1);
                    tr.opens = (fromExit && r == 1);
                    break;
                }
            } else if (dir == 1) {
                auto itMap = byCol.find(c);
                if (itMap != byCol.end()) {
                    auto it = itMap->second.upper_bound(r);
                    if (it != itMap->second.end()) {
                        nr = *it;
                        hit = true;
                    }
                }
                if (hit) {
                    addVertical(c, r + 1, nr - 1);
                } else {
                    addVertical(c, r + 1, rows);
                    break;
                }
            } else {
                auto itMap = byCol.find(c);
                if (itMap != byCol.end()) {
                    auto it = itMap->second.lower_bound(r);
                    if (it != itMap->second.begin()) {
                        --it;
                        nr = *it;
                        hit = true;
                    }
                }
                if (hit) {
                    addVertical(c, nr + 1, r - 1);
                } else {
                    addVertical(c, 1, r - 1);
                    break;
                }
            }

            r = nr;
            c = nc;
            char ch = at(r, c);
            if (ch == '/') {
                if (dir == 0) dir = 3;
                else if (dir == 1) dir = 2;
                else if (dir == 2) dir = 1;
                else dir = 0;
            } else {
                if (dir == 0) dir = 1;
                else if (dir == 1) dir = 0;
                else if (dir == 2) dir = 3;
                else dir = 2;
            }
        }
        return tr;
    }
};

static vector<Segment> mergeSegments(const vector<Segment>& segs) {
    unordered_map<int, vector<Interval>> grouped;
    grouped.reserve(segs.size() * 2 + 1);
    for (const auto& s : segs) grouped[s.fixed].push_back({s.a, s.b});

    vector<Segment> res;
    for (auto& [fixed, intervals] : grouped) {
        sort(intervals.begin(), intervals.end(), [](const Interval& x, const Interval& y) {
            if (x.a != y.a) return x.a < y.a;
            return x.b < y.b;
        });
        int l = -1, r = -1;
        for (auto [a, b] : intervals) {
            if (l == -1) {
                l = a;
                r = b;
            } else if (a <= r + 1) {
                r = max(r, b);
            } else {
                res.push_back({fixed, l, r});
                l = a;
                r = b;
            }
        }
        if (l != -1) res.push_back({fixed, l, r});
    }
    return res;
}

static vector<Segment> intersectSameOrientation(vector<Segment> a, vector<Segment> b) {
    a = mergeSegments(a);
    b = mergeSegments(b);
    unordered_map<int, vector<Interval>> groupedA, groupedB;
    groupedA.reserve(a.size() * 2 + 1);
    groupedB.reserve(b.size() * 2 + 1);
    for (const auto& s : a) groupedA[s.fixed].push_back({s.a, s.b});
    for (const auto& s : b) groupedB[s.fixed].push_back({s.a, s.b});

    vector<Segment> res;
    for (auto& [fixed, va] : groupedA) {
        auto it = groupedB.find(fixed);
        if (it == groupedB.end()) continue;
        auto& vb = it->second;
        size_t i = 0, j = 0;
        while (i < va.size() && j < vb.size()) {
            int l = max(va[i].a, vb[j].a);
            int r = min(va[i].b, vb[j].b);
            if (l <= r) res.push_back({fixed, l, r});
            if (va[i].b < vb[j].b) ++i;
            else ++j;
        }
    }
    return res;
}

struct CountResult {
    long long count = 0;
    pair<int, int> best = {INT_MAX, INT_MAX};
};

static CountResult countIntersections(vector<Segment> H, vector<Segment> V, int rows, int cols) {
    H = mergeSegments(H);
    V = mergeSegments(V);

    struct Event {
        int x, row, delta;
        bool operator<(const Event& other) const {
            if (x != other.x) return x < other.x;
            return delta > other.delta;
        }
    };
    vector<Event> events;
    events.reserve(H.size() * 2);
    for (const auto& h : H) {
        events.push_back({h.a, h.fixed, 1});
        if (h.b + 1 <= cols) events.push_back({h.b + 1, h.fixed, -1});
    }
    sort(events.begin(), events.end());
    sort(V.begin(), V.end(), [](const Segment& x, const Segment& y) {
        if (x.fixed != y.fixed) return x.fixed < y.fixed;
        if (x.a != y.a) return x.a < y.a;
        return x.b < y.b;
    });

    Fenwick rowsBit(rows);
    size_t ei = 0;
    long long total = 0;
    for (const auto& v : V) {
        while (ei < events.size() && events[ei].x <= v.fixed) {
            rowsBit.add(events[ei].row, events[ei].delta);
            ++ei;
        }
        total += rowsBit.rangeSum(v.a, v.b);
    }

    struct RowEvent {
        int row, col, delta;
        bool operator<(const RowEvent& other) const {
            if (row != other.row) return row < other.row;
            return delta > other.delta;
        }
    };
    vector<RowEvent> rowEvents;
    rowEvents.reserve(V.size() * 2);
    for (const auto& v : V) {
        rowEvents.push_back({v.a, v.fixed, 1});
        if (v.b + 1 <= rows) rowEvents.push_back({v.b + 1, v.fixed, -1});
    }
    sort(rowEvents.begin(), rowEvents.end());
    sort(H.begin(), H.end(), [](const Segment& x, const Segment& y) {
        if (x.fixed != y.fixed) return x.fixed < y.fixed;
        if (x.a != y.a) return x.a < y.a;
        return x.b < y.b;
    });

    Fenwick colsBit(cols);
    ei = 0;
    pair<int, int> best = {INT_MAX, INT_MAX};
    for (size_t i = 0; i < H.size();) {
        int row = H[i].fixed;
        while (ei < rowEvents.size() && rowEvents[ei].row <= row) {
            colsBit.add(rowEvents[ei].col, rowEvents[ei].delta);
            ++ei;
        }
        int bestCol = INT_MAX;
        while (i < H.size() && H[i].fixed == row) {
            int col = colsBit.firstInRange(H[i].a, H[i].b);
            if (col != -1) bestCol = min(bestCol, col);
            ++i;
        }
        if (bestCol != INT_MAX) {
            best = {row, bestCol};
            break;
        }
    }

    return {total, best};
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int R, C, m, n;
    int tc = 1;
    while (cin >> R >> C >> m >> n) {
        MirrorField field;
        field.rows = R;
        field.cols = C;
        for (int i = 0; i < m; ++i) {
            int r, c;
            cin >> r >> c;
            field.addMirror(r, c, '/');
        }
        for (int i = 0; i < n; ++i) {
            int r, c;
            cin >> r >> c;
            field.addMirror(r, c, '\\');
        }

        Trace forward = field.trace(1, 0, 0, false);
        cout << "Case " << tc++ << ": ";
        if (forward.opens) {
            cout << 0 << '\n';
            continue;
        }

        Trace backward = field.trace(R, C + 1, 2, true);

        auto a = countIntersections(forward.horizontal, backward.vertical, R, C);
        auto b = countIntersections(forward.vertical, backward.horizontal, R, C);
        auto overlapH = intersectSameOrientation(mergeSegments(forward.horizontal), mergeSegments(backward.horizontal));
        auto overlapV = intersectSameOrientation(mergeSegments(forward.vertical), mergeSegments(backward.vertical));
        auto both = countIntersections(overlapH, overlapV, R, C);

        long long total = a.count + b.count - both.count;
        pair<int, int> best = min(a.best, b.best);
        if (total == 0) {
            cout << "impossible\n";
        } else {
            cout << total << ' ' << best.first << ' ' << best.second << '\n';
        }
    }

    return 0;
}
