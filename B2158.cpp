#include <bits/stdc++.h>
using namespace std;

typedef long long ll;
typedef unsigned long long ull;
typedef __int128_t i128;

using pii = pair<int, int>;
using pll = pair<ll, ll>;

constexpr int MOD = 1'000'000'007;

struct Student {
    string id;
    double score;
};

bool cmp(const Student& a, const Student& b) {
    return a.score > b.score;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n, k;
    Student student[101];

    cin >> n >> k;
    for (int i = 1; i <= n; i++)
        cin >> student[i].id >> student[i].score;
    sort(student + 1, student + n + 1, cmp);
    cout << student[k].id << ' ' << student[k].score << '\n';

    return 0;
}
