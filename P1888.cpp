#include <bits/stdc++.h>
using namespace std;

typedef long long ll;
typedef unsigned long long ull;
typedef __int128_t i128;

ll l[3];

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    cin>>l[0]>>l[1]>>l[2];
    sort(l,l+3);
    cout<<l[0]/gcd(l[0],l[2])<<'/'<<l[2]/gcd(l[0],l[2])<<endl;
    
    return 0;
}
