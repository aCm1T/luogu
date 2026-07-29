#include <algorithm>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <string>
#include <vector>

using namespace std;

struct Start {
    int x;
    int y;
    vector<int> markerTimes;
    int needed = 0;
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int width, height;
    cin >> width >> height;

    vector<pair<int, int>> markers;
    for (int row = 0; row < height; ++row) {
        string line;
        cin >> line;
        const int y = height - row;
        for (int x = 1; x <= width; ++x) {
            if (line[x - 1] == 'X') {
                markers.push_back({x, y});
            }
        }
    }

    constexpr int radius = 100;
    constexpr int side = 2 * radius + 1;
    vector<vector<int>> spiralTime(side, vector<int>(side, -1));

    int x = 0;
    int y = 0;
    int dx = 0;
    int dy = 1;
    int segmentLength = 1;
    int usedInSegment = 0;
    int turns = 0;
    for (int step = 0; step <= side * side; ++step) {
        if (abs(x) <= radius && abs(y) <= radius) {
            spiralTime[y + radius][x + radius] = step;
        }

        x += dx;
        y += dy;
        if (++usedInSegment == segmentLength) {
            const int nextDx = dy;
            const int nextDy = -dx;
            dx = nextDx;
            dy = nextDy;
            usedInSegment = 0;
            if (++turns % 2 == 0) {
                ++segmentLength;
            }
        }
    }

    vector<Start> starts;
    starts.reserve(width * height);
    for (int startY = 1; startY <= height; ++startY) {
        for (int startX = 1; startX <= width; ++startX) {
            Start start;
            start.x = startX;
            start.y = startY;
            start.markerTimes.reserve(markers.size());
            for (auto [markerX, markerY] : markers) {
                start.markerTimes.push_back(
                    spiralTime[markerY - startY + radius]
                              [markerX - startX + radius]);
            }
            sort(start.markerTimes.begin(), start.markerTimes.end());
            starts.push_back(move(start));
        }
    }

    vector<int> order(starts.size());
    iota(order.begin(), order.end(), 0);
    sort(order.begin(), order.end(), [&](int lhs, int rhs) {
        return starts[lhs].markerTimes < starts[rhs].markerTimes;
    });

    for (size_t i = 1; i < order.size(); ++i) {
        Start& previous = starts[order[i - 1]];
        Start& current = starts[order[i]];
        for (size_t marker = 0; marker < markers.size(); ++marker) {
            if (previous.markerTimes[marker] != current.markerTimes[marker]) {
                const int distinguishAt =
                    min(previous.markerTimes[marker],
                        current.markerTimes[marker]);
                previous.needed = max(previous.needed, distinguishAt);
                current.needed = max(current.needed, distinguishAt);
                break;
            }
        }
    }

    long long total = 0;
    int maximum = 0;
    for (const Start& start : starts) {
        total += start.needed;
        maximum = max(maximum, start.needed);
    }

    cout << fixed << setprecision(3)
         << static_cast<long double>(total) / starts.size() << '\n';
    cout << maximum << '\n';

    bool firstOutput = true;
    for (const Start& start : starts) {
        if (start.needed == maximum) {
            if (!firstOutput) {
                cout << ' ';
            }
            firstOutput = false;
            cout << '(' << start.x << ',' << start.y << ')';
        }
    }
    cout << '\n';
    return 0;
}
