#include<bits/stdc++.h>
#define N 25 
using namespace std;
bool cmp (string a, string b){
    return (a+b>b+a);
}
int main(){
    int n;
    string s[N];
    cin >>n;
    for (int i=0;i<n;i++)
        cin>>s[i];
    sort(s,s+n,cmp);
    for (int i=0;i<n;i++)
        cout<<s[i];
    return 0;
}
