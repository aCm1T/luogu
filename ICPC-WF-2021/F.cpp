#include <algorithm>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <limits>
#include <vector>

using namespace std;

struct Point {
    long long x;
    long long y;
};

struct Flight {
    Point from;
    Point to;
    long long zFrom;
    long long zTo;
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int islandCount, flightCount;
    cin >> islandCount >> flightCount;

    vector<vector<Point>> islands(islandCount);
    for (vector<Point>& island : islands) {
        int vertexCount;
        cin >> vertexCount;
        island.resize(vertexCount);
        for (Point& vertex : island) {
            cin >> vertex.x >> vertex.y;
        }
    }

    vector<Flight> flights(flightCount);
    for (Flight& flight : flights) {
        cin >> flight.from.x >> flight.from.y >> flight.zFrom
            >> flight.to.x >> flight.to.y >> flight.zTo;
    }

    long double answerRatio = 0;
    for (const vector<Point>& island : islands) {
        long double islandBest = numeric_limits<long double>::infinity();

        for (const Flight& flight : flights) {
            const long long dx = flight.to.x - flight.from.x;
            const long long dy = flight.to.y - flight.from.y;
            const long long lengthSquared = dx * dx + dy * dy;
            long double requiredRatio = 0;
            bool possible = true;

            for (const Point& vertex : island) {
                const long long vx = vertex.x - flight.from.x;
                const long long vy = vertex.y - flight.from.y;
                const long long projection = vx * dx + vy * dy;

                // A line-scan camera never photographs beyond either endpoint.
                if (projection < 0 || projection > lengthSquared) {
                    possible = false;
                    break;
                }

                const long long cross = dx * vy - dy * vx;
                const long double distance =
                    fabsl(static_cast<long double>(cross)) /
                    sqrtl(static_cast<long double>(lengthSquared));
                const long double t =
                    static_cast<long double>(projection) / lengthSquared;
                const long double altitude =
                    flight.zFrom +
                    t * static_cast<long double>(flight.zTo - flight.zFrom);
                requiredRatio = max(requiredRatio, distance / altitude);
            }

            if (possible) {
                islandBest = min(islandBest, requiredRatio);
            }
        }

        if (!isfinite(islandBest)) {
            cout << "impossible\n";
            return 0;
        }
        answerRatio = max(answerRatio, islandBest);
    }

    const long double pi = acosl(-1.0L);
    const long double answer = atanl(answerRatio) * 180 / pi;
    cout << fixed << setprecision(10) << answer << '\n';
    return 0;
}
