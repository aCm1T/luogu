#include <bits/stdc++.h>
#define SIZE 105
using namespace std;

typedef long long ll;
typedef unsigned long long ull;
typedef __int128_t i128;

int n,m;
int a[SIZE][SIZE];
void solve() {
    // Solution Code
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    cin >>n>>m;
    for(int i =0;i<n;i++)
        for(int j=0;j<m;j++)
            cin>>a[i][j];
    for(int j=0;j<m;j++){
        for (int i=n-1;i>=0;i--){
            cout<<a[i][j]<<' ';   
        }
        cout<<'\n';
    }      


    return 0;
}
