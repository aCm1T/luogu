#include <bits/stdc++.h>
using namespace std;

const double EARTH_RADIUS = 6370.0;
const double PI = acos(-1.0);
const double EPS = 1e-10;
const double INF = 1e100;

struct Vec {
    double x, y, z;

    Vec operator+(const Vec& o) const { return {x + o.x, y + o.y, z + o.z}; }
    Vec operator-(const Vec& o) const { return {x - o.x, y - o.y, z - o.z}; }
    Vec operator*(double k) const { return {x * k, y * k, z * k}; }
    Vec operator/(double k) const { return {x / k, y / k, z / k}; }
};

static double dot(Vec a, Vec b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

static Vec cross(Vec a, Vec b) {
    return {
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    };
}

static double norm(Vec a) {
    return sqrt(max(0.0, dot(a, a)));
}

static Vec normalize(Vec a) {
    double n = norm(a);
    return a / n;
}

static double clampDouble(double x, double lo, double hi) {
    return max(lo, min(hi, x));
}

static double angleBetween(Vec a, Vec b) {
    return acos(clampDouble(dot(a, b), -1.0, 1.0));
}

static Vec fromLonLat(double lonDeg, double latDeg) {
    double lon = lonDeg * PI / 180.0;
    double lat = latDeg * PI / 180.0;
    double clat = cos(lat);
    return {clat * cos(lon), clat * sin(lon), sin(lat)};
}

static bool samePoint(Vec a, Vec b) {
    return angleBetween(a, b) < 1e-8;
}

static vector<Vec> circleIntersections(Vec a, Vec b, double capAngle) {
    vector<Vec> res;
    double d = dot(a, b);
    Vec n = cross(a, b);
    double nn = norm(n);
    if (nn < 1e-12) return res;

    double target = cos(capAngle);
    double denom = 1.0 + d;
    if (fabs(denom) < 1e-12) return res;

    Vec base = (a + b) * (target / denom);
    double h2 = 1.0 - dot(base, base);
    if (h2 < -1e-9) return res;
    h2 = max(0.0, h2);
    Vec unitN = n / nn;
    double h = sqrt(h2);
    res.push_back(normalize(base + unitN * h));
    if (h > 1e-9) res.push_back(normalize(base - unitN * h));
    return res;
}

static bool addUnique(vector<Vec>& nodes, Vec p) {
    for (const Vec& q : nodes) {
        if (samePoint(p, q)) return false;
    }
    nodes.push_back(p);
    return true;
}

static bool arcCovered(Vec a, Vec b, const vector<Vec>& airports, double capAngle) {
    double theta = angleBetween(a, b);
    if (theta < 1e-12) return true;
    if (PI - theta < 1e-9) return false;

    Vec e1 = a;
    Vec e2 = normalize(b - a * cos(theta));
    double target = cos(capAngle);
    vector<pair<double, double>> intervals;

    for (Vec c : airports) {
        double u = dot(c, e1);
        double v = dot(c, e2);
        double amp = hypot(u, v);
        if (amp + 1e-12 < target) continue;
        double ratio = clampDouble(target / max(amp, 1e-300), -1.0, 1.0);
        double delta = acos(ratio);
        double center = atan2(v, u);
        for (int k = -2; k <= 2; ++k) {
            double l = center - delta + 2.0 * PI * k;
            double r = center + delta + 2.0 * PI * k;
            double lo = max(0.0, l);
            double hi = min(theta, r);
            if (lo <= hi + 1e-10) intervals.push_back({lo, hi});
        }
    }

    if (intervals.empty()) return false;
    sort(intervals.begin(), intervals.end());
    double covered = 0.0;
    for (auto [l, r] : intervals) {
        if (l > covered + 1e-8) return false;
        covered = max(covered, r);
        if (covered >= theta - 1e-8) return true;
    }
    return covered >= theta - 1e-8;
}

static vector<double> dijkstraDense(const vector<vector<double>>& graph, int src) {
    int n = (int)graph.size();
    vector<double> dist(n, INF);
    vector<char> used(n, false);
    dist[src] = 0.0;
    for (int it = 0; it < n; ++it) {
        int v = -1;
        for (int i = 0; i < n; ++i) {
            if (!used[i] && (v == -1 || dist[i] < dist[v])) v = i;
        }
        if (v == -1 || dist[v] >= INF / 2) break;
        used[v] = true;
        for (int to = 0; to < n; ++to) {
            if (graph[v][to] < INF / 2 && dist[to] > dist[v] + graph[v][to]) {
                dist[to] = dist[v] + graph[v][to];
            }
        }
    }
    return dist;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int N, R;
    int tc = 1;
    while (cin >> N >> R) {
        vector<Vec> airports;
        airports.reserve(N);
        for (int i = 0; i < N; ++i) {
            double lon, lat;
            cin >> lon >> lat;
            airports.push_back(fromLonLat(lon, lat));
        }

        double capAngle = R / EARTH_RADIUS;
        vector<Vec> nodes = airports;
        for (int i = 0; i < N; ++i) {
            for (int j = i + 1; j < N; ++j) {
                for (Vec p : circleIntersections(airports[i], airports[j], capAngle)) {
                    bool insideSomeCap = false;
                    for (Vec a : airports) {
                        if (angleBetween(p, a) <= capAngle + 1e-8) {
                            insideSomeCap = true;
                            break;
                        }
                    }
                    if (insideSomeCap) addUnique(nodes, p);
                }
            }
        }

        int M = (int)nodes.size();
        vector<vector<double>> visibility(M, vector<double>(M, INF));
        for (int i = 0; i < M; ++i) visibility[i][i] = 0.0;
        for (int i = 0; i < M; ++i) {
            for (int j = i + 1; j < M; ++j) {
                double ang = angleBetween(nodes[i], nodes[j]);
                if (PI - ang < 1e-9) continue;
                if (arcCovered(nodes[i], nodes[j], airports, capAngle)) {
                    double d = EARTH_RADIUS * ang;
                    visibility[i][j] = visibility[j][i] = d;
                }
            }
        }

        vector<vector<double>> regionDist(N, vector<double>(N, INF));
        for (int i = 0; i < N; ++i) {
            vector<double> d = dijkstraDense(visibility, i);
            for (int j = 0; j < N; ++j) regionDist[i][j] = d[j];
        }

        int Q;
        cin >> Q;
        cout << "Case " << tc++ << ":\n";
        cout.setf(ios::fixed);
        cout << setprecision(3);
        while (Q--) {
            int s, t;
            double c;
            cin >> s >> t >> c;
            --s;
            --t;

            vector<vector<double>> airportGraph(N, vector<double>(N, INF));
            for (int i = 0; i < N; ++i) airportGraph[i][i] = 0.0;
            for (int i = 0; i < N; ++i) {
                for (int j = 0; j < N; ++j) {
                    if (i != j && regionDist[i][j] <= c + 1e-7) {
                        airportGraph[i][j] = regionDist[i][j];
                    }
                }
            }
            vector<double> ans = dijkstraDense(airportGraph, s);
            if (ans[t] >= INF / 2) {
                cout << "impossible\n";
            } else {
                cout << ans[t] + 0.0 << '\n';
            }
        }
    }

    return 0;
}
