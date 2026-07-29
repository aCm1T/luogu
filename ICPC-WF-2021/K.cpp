#include <algorithm>
#include <iostream>
#include <utility>
#include <vector>

using namespace std;

struct Point {
    long long x = 0;
    long long y = 0;

    Point operator+(const Point& other) const {
        return {x + other.x, y + other.y};
    }
    Point operator-(const Point& other) const {
        return {x - other.x, y - other.y};
    }
    bool operator==(const Point& other) const {
        return x == other.x && y == other.y;
    }
};

static long long dot(const Point& a, const Point& b) {
    return a.x * b.x + a.y * b.y;
}

static long long quality(const Point& point) {
    return dot(point, point);
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int nodeCount;
    cin >> nodeCount;

    vector<vector<int>> children(nodeCount);
    vector<Point> leaf(nodeCount);
    for (int node = 0; node < nodeCount; ++node) {
        int childCount;
        cin >> childCount;
        if (childCount == 0) {
            cin >> leaf[node].x >> leaf[node].y;
        } else {
            children[node].resize(childCount);
            for (int& child : children[node]) {
                cin >> child;
                --child;
            }
        }
    }

    vector<Point> minimum(nodeCount), maximum(nodeCount);
    long long answer = 0;

    auto extremePoints = [&](const Point& direction) {
        for (int node = nodeCount - 1; node >= 0; --node) {
            if (children[node].empty()) {
                minimum[node] = maximum[node] = leaf[node];
                continue;
            }

            Point sumMinimum;
            Point sumMaximum;
            Point bestMinimumPair;
            Point bestMaximumPair;
            bool firstChild = true;

            for (int child : children[node]) {
                sumMinimum = sumMinimum + minimum[child];
                sumMaximum = sumMaximum + maximum[child];
                const Point pair = minimum[child] + maximum[child];
                if (firstChild || dot(pair, direction) <
                                      dot(bestMinimumPair, direction)) {
                    bestMinimumPair = pair;
                }
                if (firstChild || dot(pair, direction) >
                                      dot(bestMaximumPair, direction)) {
                    bestMaximumPair = pair;
                }
                firstChild = false;
            }

            // If child i wins, the result is child_i minus all other
            // children. Extremes of different child subtrees are independent.
            minimum[node] = bestMinimumPair - sumMaximum;
            maximum[node] = bestMaximumPair - sumMinimum;
        }

        answer = max(answer, quality(minimum[0]));
        answer = max(answer, quality(maximum[0]));
        return pair<Point, Point>{minimum[0], maximum[0]};
    };

    auto [first, last] = extremePoints({1, 0});
    if (first == last) {
        tie(first, last) = extremePoints({0, 1});
    }

    // Reconstruct both convex-hull chains. For a directed chord A->B,
    // maximizing in its left normal either finds a new hull vertex on that
    // side or proves that the chord is a hull edge.
    vector<pair<Point, Point>> pending;
    if (!(first == last)) {
        pending.push_back({first, last});
        pending.push_back({last, first});
    }

    while (!pending.empty()) {
        const auto [from, to] = pending.back();
        pending.pop_back();

        const Point edge = to - from;
        const Point normal = {-edge.y, edge.x};
        const Point middle = extremePoints(normal).second;

        if (dot(middle, normal) > dot(from, normal)) {
            pending.push_back({middle, to});
            pending.push_back({from, middle});
        }
    }

    cout << answer << '\n';
    return 0;
}
