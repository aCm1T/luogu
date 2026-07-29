#include <bits/stdc++.h>
using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    int caseNo = 1;
    while (cin >> n) {
        vector<pair<int, int>> plates;
        for (int stack = 0; stack < n; ++stack) {
            int h;
            cin >> h;
            for (int i = 0; i < h; ++i) {
                int x;
                cin >> x;
                plates.push_back({x, stack});
            }
        }

        sort(plates.begin(), plates.end());
        plates.erase(unique(plates.begin(), plates.end()), plates.end());

        vector<vector<int>> has;
        vector<int> groupSize;
        for (int i = 0; i < static_cast<int>(plates.size());) {
            int j = i;
            vector<int> row(n, 0);
            int count = 0;
            while (j < static_cast<int>(plates.size()) && plates[j].first == plates[i].first) {
                row[plates[j].second] = 1;
                ++count;
                ++j;
            }
            has.push_back(row);
            groupSize.push_back(count);
            i = j;
        }

        const int INF = 1'000'000'000;
        int m = static_cast<int>(has.size());
        vector<vector<int>> dp(m, vector<int>(n, INF));
        for (int s = 0; s < n; ++s) {
            if (has[0][s]) {
                dp[0][s] = groupSize[0];
            }
        }

        for (int i = 1; i < m; ++i) {
            for (int tail = 0; tail < n; ++tail) {
                if (!has[i][tail]) {
                    continue;
                }
                for (int prev = 0; prev < n; ++prev) {
                    if (dp[i - 1][prev] == INF) {
                        continue;
                    }
                    int add = groupSize[i];
                    if (has[i][prev] && (tail != prev || groupSize[i] == 1)) {
                        --add;
                    }
                    dp[i][tail] = min(dp[i][tail], dp[i - 1][prev] + add);
                }
            }
        }

        int best = *min_element(dp[m - 1].begin(), dp[m - 1].end());
        cout << "Case " << caseNo++ << ": " << best * 2 - n - 1 << '\n';
    }

    return 0;
}
