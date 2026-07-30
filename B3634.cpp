#include <bits/stdc++.h>
using namespace std;

typedef long long ll;
typedef unsigned long long ull;
typedef __int128_t i128;

using pii = pair<int, int>;
using pll = pair<ll, ll>;

constexpr int MOD = 1'000'000'007;

int a,b;

ll lcm(int a,int b){
    return a*b/gcd(a,b);
}
void solve() {
    // Solution Code
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    cin >> a >>b;
    cout << gcd(a,b) <<' '<< lcm(a,b);

    return 0;
}
