#include <bits/stdc++.h>
using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    
    int day;
    cin >> day;
    
    if (day == 1 || day == 3 || day == 5) {
        cout << "NO";
    } else {
        cout << "YES";
    }
    
    return 0;
}
