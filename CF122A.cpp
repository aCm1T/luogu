#include <bits/stdc++.h>
using namespace std;

typedef long long ll;
typedef unsigned long long ull;
typedef __int128_t i128;

using pii = pair<int, int>;
using pll = pair<ll, ll>;

int solve(int x) {
    vector<int> divlist={4,7,44,47,74,77,444,447,474,477,744,747,774,777};
    for (int i=0;i<divlist.size();i++) {
        if (x==divlist[i])
            return true;
        else if (x%divlist[i]==0)
            return true;
    }
    return false;
}


int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int x;
    cin>>x;

    if (solve(x))
        cout<<"YES";
    else
        cout<<"NO";

    return 0;
}
