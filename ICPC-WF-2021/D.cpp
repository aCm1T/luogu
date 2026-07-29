#include <algorithm>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <limits>
#include <queue>
#include <utility>
#include <vector>

using namespace std;

using Real = long double;

constexpr Real EPS = 1e-9L;
constexpr Real INF = numeric_limits<Real>::infinity();

struct Point {
    Real x = 0;
    Real y = 0;

    Point operator+(const Point& other) const {
        return {x + other.x, y + other.y};
    }

    Point operator-(const Point& other) const {
        return {x - other.x, y - other.y};
    }

    Point operator*(Real factor) const {
        return {x * factor, y * factor};
    }

    Point operator/(Real factor) const {
        return {x / factor, y / factor};
    }
};

Real dot(const Point& a, const Point& b) {
    return a.x * b.x + a.y * b.y;
}

Real cross(const Point& a, const Point& b) {
    return a.x * b.y - a.y * b.x;
}

Real length(const Point& a) {
    return hypotl(a.x, a.y);
}

/*
 * Intersects the ray origin + t * direction (direction has unit length)
 * with segment [a,b]. The returned value is t, or a negative value if the
 * segment does not meet the forward ray.
 *
 * sideMask records on which side(s) of the ray the segment has points:
 * bit 0 for one side and bit 1 for the other. An edge crossing the ray has
 * mask 3. Two edges tangent at the same ray can together also yield mask 3.
 */
Real raySegmentDistance(const Point& origin, const Point& direction,
                        const Point& a, const Point& b,
                        int* sideMask = nullptr) {
    const Real sideA = cross(a - origin, direction);
    const Real sideB = cross(b - origin, direction);
    const Real alongA = dot(a - origin, direction);
    const Real alongB = dot(b - origin, direction);

    if (sideMask != nullptr) {
        *sideMask = 0;
        if (sideA < -EPS || sideB < -EPS) {
            *sideMask |= 1;
        }
        if (sideA > EPS || sideB > EPS) {
            *sideMask |= 2;
        }
    }

    if ((sideA < -EPS && sideB < -EPS) ||
        (sideA > EPS && sideB > EPS)) {
        return -1;
    }

    if (abs(sideA) < EPS && abs(sideB) < EPS) {
        return min(alongA, alongB);
    }

    return (alongA * sideB - alongB * sideA) / (sideB - sideA);
}

bool pointOnSegment(const Point& a, const Point& b, const Point& p) {
    const Point ab = b - a;
    const Real len = length(ab);
    if (len < EPS) {
        return length(p - a) < EPS;
    }

    const Point ap = p - a;
    const Real projection = dot(ab, ap) / len;
    return abs(cross(ab, ap) / len) < EPS &&
           projection > -EPS && projection < len + EPS;
}

class Solver {
public:
    void run() {
        cin >> n;
        polygon.resize(n);
        for (Point& p : polygon) {
            cin >> p.x >> p.y;
        }
        cin >> guard.x >> guard.y;
        cin >> sculpture.x >> sculpture.y;

        buildTargets();
        cout << fixed << setprecision(15) << shortestPathToTarget() << '\n';
    }

private:
    int n = 0;
    Point guard;
    Point sculpture;
    vector<Point> polygon;

    // Indices [0,n) are polygon vertices and n is the guard. Every later
    // point is a terminal point at which at least half the art is visible.
    vector<Point> points;

    bool pointInsidePolygon(const Point& p) const {
        int winding = 0;
        for (int i = 0; i < n; ++i) {
            const Point& a = polygon[i];
            const Point& b = polygon[(i + 1) % n];
            const Real turn = cross(b - a, p - a);

            if (a.y <= p.y) {
                if (b.y > p.y && turn > 0) {
                    ++winding;
                }
            } else if (b.y <= p.y && turn < 0) {
                --winding;
            }
        }
        return winding != 0;
    }

    /*
     * Tests a straight movement from an original graph node to another point.
     * Any contact with a polygon vertex in the open segment is rejected; it
     * causes no loss, because a shortest path can be split at that vertex.
     */
    bool visibleSegment(int sourceIndex, const Point& destination) const {
        const Point& source = points[sourceIndex];
        const Real len = length(destination - source);
        if (len < EPS) {
            return true;
        }

        // Walking on either wall incident to the source vertex is allowed.
        if (sourceIndex < n) {
            if (pointOnSegment(polygon[sourceIndex],
                               polygon[(sourceIndex + 1) % n],
                               destination) ||
                pointOnSegment(polygon[sourceIndex],
                               polygon[(sourceIndex + n - 1) % n],
                               destination)) {
                return true;
            }
        }

        const Point direction = (destination - source) / len;
        for (int i = 0; i < n; ++i) {
            const Real distance = raySegmentDistance(
                source, direction, polygon[i], polygon[(i + 1) % n]);
            if (distance > EPS && distance < len - EPS) {
                return false;
            }
        }

        // With no boundary contact in its interior, the whole segment is on
        // the same side of the boundary as its midpoint.
        return pointInsidePolygon((source + destination) * 0.5L);
    }

    void addCriticalSegment(const Point& through) {
        const Point delta = through - sculpture;
        const Real deltaLength = length(delta);
        if (deltaLength < EPS) {
            points.push_back(through);
            return;
        }
        const Point direction = delta / deltaLength;

        vector<pair<Real, int>> events;
        events.reserve(n);
        for (int i = 0; i < n; ++i) {
            int sideMask = 0;
            const Real distance = raySegmentDistance(
                sculpture, direction, polygon[i],
                polygon[(i + 1) % n], &sideMask);
            if (distance >= 0) {
                events.push_back({distance, sideMask});
            }
        }
        sort(events.begin(), events.end());

        /*
         * A wall reaching the ray from only one side can obscure less than
         * half of a tiny circle. Half-visibility ends once walls have reached
         * the ray from both sides. A proper crossing contributes both sides
         * in one event.
         */
        Real endDistance = 0;
        int coveredSides = 0;
        for (const auto& [distance, sideMask] : events) {
            endDistance = distance;
            coveredSides |= sideMask;
            if (coveredSides == 3) {
                break;
            }
        }

        // One endpoint of the critical segment.
        points.push_back(sculpture + direction * endDistance);

        /*
         * For each possible last graph vertex, its closest point on this
         * segment is either the endpoint above, the sculpture (already a
         * target), or its orthogonal projection. Add every interior
         * projection. Cross-combining these candidates later also handles
         * visibility changes caused by a segment grazing another vertex.
         */
        for (int i = 0; i <= n; ++i) {
            const Real projection = dot(points[i] - sculpture, direction);
            if (projection > EPS && projection < endDistance - EPS) {
                points.push_back(sculpture + direction * projection);
            }
        }
    }

    void buildTargets() {
        points.reserve((n + 2) * (n + 2));
        for (const Point& p : polygon) {
            points.push_back(p);
        }
        points.push_back(guard);      // index n
        points.push_back(sculpture);  // index n+1, always a terminal point

        /*
         * The boundary of the half-visibility region changes only when a ray
         * from the sculpture passes through a polygon vertex. The guard's own
         * ray is added as well: if the guard already lies in the region, its
         * projection creates a zero-distance terminal point.
         */
        for (int i = 0; i <= n; ++i) {
            addCriticalSegment(points[i]);
        }
    }

    Real shortestPathToTarget() const {
        vector<Real> distance(points.size(), INF);
        using QueueEntry = pair<Real, int>;
        priority_queue<QueueEntry, vector<QueueEntry>, greater<QueueEntry>> pq;
        pq.push({0, n});

        while (!pq.empty()) {
            const auto [currentDistance, index] = pq.top();
            pq.pop();

            // Every point after the guard is a terminal candidate. Since the
            // heap is ordered, the first one removed has the optimal distance.
            if (index > n) {
                return currentDistance;
            }
            if (currentDistance >= distance[index]) {
                continue;
            }
            distance[index] = currentDistance;

            for (int next = 0; next < static_cast<int>(points.size()); ++next) {
                const Real edgeLength = length(points[next] - points[index]);
                const Real newDistance = currentDistance + edgeLength;
                if (newDistance >= distance[next]) {
                    continue;
                }
                if (visibleSegment(index, points[next])) {
                    pq.push({newDistance, next});
                }
            }
        }

        return 0;  // The sculpture itself is reachable inside a simple polygon.
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    Solver solver;
    solver.run();
    return 0;
}
