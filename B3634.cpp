#include <bits/stdc++.h>
using namespace std;

typedef long long ll;
typedef unsigned long long ull;
typedef __int128_t i128;

using pii = pair<int, int>;
using pll = pair<ll, ll>;

constexpr int MOD = 1'000'000'007;

ll a,b;

ll lcm(ll a,ll b){
    return a*b/gcd(a,b);
}
void solve() {
    cin >> a >> b;
    cout << gcd(a,b) <<' '<< lcm(a,b);
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    solve();
    return 0;
}
