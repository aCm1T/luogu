#include <bits/stdc++.h>
using namespace std;

typedef long long ll;
typedef unsigned long long ull;
typedef __int128_t i128;

using pii = pair<int, int>;
using pll = pair<ll, ll>;

struct pos{
    double x;
    double y;
}a,b,c;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    
    double e,f,g;
    double s;

    cin>>a.x>>a.y;
    cin>>b.x>>b.y;
    cin>>c.x>>c.y;

    e = sqrt (pow(a.x - b.x , 2) + pow(a.y - b.y , 2));
    f = sqrt (pow(a.x - c.x , 2) + pow(a.y - c.y , 2));
    g = sqrt (pow(c.x - b.x , 2) + pow(c.y - b.y , 2));

    s = 0.5 * (e+f+g);
    cout<<(int)round(sqrt(s*(s-e)*(s-f)*(s-g)));

    return 0;
}
