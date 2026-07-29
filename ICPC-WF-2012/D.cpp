#include <iostream>
#include <string>
#include <vector>

using namespace std;

long long bruteForce(const string& s, const string& pattern) {
    size_t pos = s.find(pattern);
    if (pos == string::npos) return 0;

    long long cnt = 0;
    while (pos != string::npos) {
        ++cnt;
        pos = s.find(pattern, pos + 1);
    }
    return cnt;
}

long long inGap(const string& s, const string& pattern) {
    string two = s + s;
    int startIndex = (int)s.size() - (int)pattern.size() + 1;
    if (startIndex < 0) startIndex = 0;
    int endIndex = (int)s.size();

    long long cnt = 0;
    size_t first = two.find(pattern, (size_t)startIndex);
    while (first != string::npos && (int)first < endIndex) {
        ++cnt;
        first = two.find(pattern, first + 1);
    }

    return cnt;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    vector<string> fibs(30);
    fibs[0] = "0";
    fibs[1] = "1";
    for (int i = 2; i < (int)fibs.size(); ++i) {
        fibs[i] = fibs[i - 1] + fibs[i - 2];
    }

    int n;
    string s;
    int loop = 1;
    while (cin >> n >> s) {
        if (n < (int)fibs.size()) {
            cout << "Case " << loop << ": " << bruteForce(fibs[n], s) << '\n';
        } else {
            vector<long long> numTimes(n + 1, 0);
            for (int i = 0; i < (int)fibs.size(); ++i) {
                numTimes[i] = bruteForce(fibs[i], s);
            }

            long long gap1 = inGap(fibs[(int)fibs.size() - 3], s);
            long long gap2 = inGap(fibs[(int)fibs.size() - 2], s);

            for (int i = (int)fibs.size(); i <= n; ++i) {
                if ((i - (int)fibs.size()) % 2 == 0) {
                    numTimes[i] = numTimes[i - 1] + numTimes[i - 2] + gap1;
                } else {
                    numTimes[i] = numTimes[i - 1] + numTimes[i - 2] + gap2;
                }
            }

            cout << "Case " << loop << ": " << numTimes[n] << '\n';
        }

        ++loop;
    }

    return 0;
}
