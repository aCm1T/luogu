#include <bits/stdc++.h>
using namespace std;

typedef long long ll;
typedef unsigned long long ull;
typedef __int128_t i128;

using pii = pair<int, int>;
using pll = pair<ll, ll>;

const int SIZE = 100005;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    ll l[SIZE];

    cin>>n;

    for(int i=0;i<n;i++)
        cin>>l[i];

    sort(l,l+n);

    for (int i=0;i<n;i++)
        cout<<l[i]<<' ';

    return 0;
}
