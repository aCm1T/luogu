#include <bits/stdc++.h>
using namespace std;

typedef long long ll;
typedef unsigned long long ull;
typedef __int128_t i128;

using pii = pair<int, int>;
using pll = pair<ll, ll>;

constexpr int MOD = 1'000'000'007;

int l,m;
int u,v;
const int SIZE = 10005;
int road[SIZE];
int cnt;

void solve() {
    // Solution Code
    cin >>l>>m;
    for (int i=0;i<m;i++) {
        cin>>u>>v;
        for (int j=u;j<=v;j++)
            road[j] = 1;
    }
    for (int i =0;i<SIZE;i++)
        if(road[i]==1) cnt++;
    cout<< (l-cnt+1);
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    solve();
    return 0;
}
