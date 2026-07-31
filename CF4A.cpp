#include <bits/stdc++.h>
using namespace std;

typedef long long ll;
typedef unsigned long long ull;
typedef __int128_t i128;

using pii = pair<int, int>;
using pll = pair<ll, ll>;


int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int x;
    cin>>x;
    if (x==2)
        cout<<"NO";
    else if (x%2==0)
        cout<<"YES";
    else
        cout<<"NO";

    return 0;
}
