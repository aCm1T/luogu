#include <bits/stdc++.h>
using namespace std;

/*
 * Generalized multiple knapsack heuristic.
 *
 * A drawer price (per unit volume) turns an item/drawer choice into an
 * independent choice.  Repeated sub-gradient updates make those choices fit
 * the capacities.  Every round is then repaired into a feasible solution;
 * a best-improving relocation pass is applied before keeping the best one.
 */

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n, m;
    if (!(cin >> n >> m)) return 0;
    vector<long long> a(n), cap(m);
    for (auto &x : a) cin >> x;
    for (auto &x : cap) cin >> x;
    vector<vector<int>> w(n, vector<int>(m));
    for (auto &r : w) for (int &x : r) cin >> x;

    // One drawer is an ordinary 0/1 knapsack, for which an exact DP is both
    // faster and stronger than the general heuristic.
    if (m == 1 && cap[0] <= 200000) {
        int C = (int)cap[0];
        vector<vector<int>> dp(n + 1, vector<int>(C + 1));
        for (int i = 0; i < n; ++i) {
            int v = (int)a[i];
            for (int c = 0; c <= C; ++c) {
                dp[i + 1][c] = dp[i][c];
                if (c >= v) dp[i + 1][c] = max(dp[i + 1][c], dp[i][c - v] + w[i][0]);
            }
        }
        int at = int(max_element(dp[n].begin(), dp[n].end()) - dp[n].begin());
        vector<int> ans(n);
        for (int i = n - 1; i >= 0; --i) if (at >= a[i] &&
            dp[i + 1][at] == dp[i][at - a[i]] + w[i][0]) ans[i] = 1, at -= a[i];
        for (int x : ans) cout << x << '\n';
        return 0;
    }

    vector<int> best(n, 0), cur(n, 0);
    long long bestValue = -1;
    vector<double> price(m, 0.0);
    mt19937 rng(712367821);

    auto valueOf = [&](const vector<int> &sol) {
        long long ans = 0;
        for (int i = 0; i < n; ++i) if (sol[i]) ans += w[i][sol[i] - 1];
        return ans;
    };

    for (int round = 0; round < 90; ++round) {
        vector<int> ord(n);
        iota(ord.begin(), ord.end(), 0);
        // Alternating deterministic and shuffled orders avoids a systematic
        // bias when many items have near-identical values.
        if (round & 1) shuffle(ord.begin(), ord.end(), rng);
        else sort(ord.begin(), ord.end(), [&](int x, int y) {
            int bx = *max_element(w[x].begin(), w[x].end());
            int by = *max_element(w[y].begin(), w[y].end());
            return (long double)bx / a[x] > (long double)by / a[y];
        });

        vector<long long> used(m, 0);
        fill(cur.begin(), cur.end(), 0);
        // Lagrangian greedy construction, restricted to currently feasible
        // drawers, so each produced solution is valid without a lossy repair.
        for (int i : ord) {
            int take = -1;
            double score = 0.0;
            for (int j = 0; j < m; ++j) if (used[j] + a[i] <= cap[j]) {
                double s = w[i][j] - price[j] * (double)a[i];
                if (take < 0 || s > score || (s == score && w[i][j] > w[i][take])) {
                    take = j; score = s;
                }
            }
            if (take >= 0 && w[i][take] > 0) cur[i] = take + 1, used[take] += a[i];
        }

        // Best-improving one-item relocations (including inserting an
        // unselected item).  A few passes are inexpensive at these limits and
        // noticeably help the dense random cases.
        for (int pass = 0; pass < 3; ++pass) {
            bool changed = false;
            vector<int> seq(n); iota(seq.begin(), seq.end(), 0);
            shuffle(seq.begin(), seq.end(), rng);
            for (int i : seq) {
                int old = cur[i] - 1;
                long long oldv = old < 0 ? 0 : w[i][old];
                int go = old;
                long long gain = 0;
                for (int j = 0; j < m; ++j) {
                    if (j == old || used[j] + a[i] > cap[j]) continue;
                    long long g = (long long)w[i][j] - oldv;
                    if (g > gain) gain = g, go = j;
                }
                if (go != old) {
                    if (old >= 0) used[old] -= a[i];
                    used[go] += a[i]; cur[i] = go + 1; changed = true;
                }
            }
            if (!changed) break;
        }

        long long now = valueOf(cur);
        if (now > bestValue) bestValue = now, best = cur;

        // Price update uses the demand of the unconstrained Lagrangian
        // solution.  It guides subsequent feasible greedy rounds away from
        // scarce drawers without ever compromising output validity.
        vector<long long> demand(m, 0);
        for (int i = 0; i < n; ++i) {
            int at = -1; double mx = 0;
            for (int j = 0; j < m; ++j) {
                double s = w[i][j] - price[j] * (double)a[i];
                if (at < 0 || s > mx) at = j, mx = s;
            }
            if (mx > 0) demand[at] += a[i];
        }
        double step = 0.20 / sqrt(round + 1.0);
        for (int j = 0; j < m; ++j) {
            double scale = max(1LL, cap[j]);
            price[j] = max(0.0, price[j] + step * (demand[j] - cap[j]) / scale);
        }
    }

    for (int x : best) cout << x << '\n';
    return 0;
}
