#include <bits/stdc++.h>
#define SIZE 5005
using namespace std;

typedef long long ll;
typedef unsigned long long ull;
typedef __int128_t i128;
constexpr int MOD = 1'000'000'007;

ll n,m,a[SIZE][SIZE];

ll com(ull n,ull m){
    ll ans;
    for (int i =1;i<=n+1;i++)
        for (int j = 1;j<=i; j++)
            if(j==1 || i==j)
                a[i][j]=1;
            else {
                a[i][j] = a[i-1][j]+a[i-1][j-1];
                a[i][j] %= MOD;
            }
    return a[n+1][m+1];
}
void solve() {
    // Solution Code
    cin>>n>>m;
    cout<<com(n,m)%MOD<<endl;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    solve();

    return 0;
}
