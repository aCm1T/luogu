#include <bits/stdc++.h>
using namespace std;

bool tryMergeFirst(const vector<long long>& takeover, const vector<long long>& buyout) {
    deque<long long> t(takeover.begin(), takeover.end());
    deque<long long> b(buyout.begin(), buyout.end());

    long long one = t.front();
    t.pop_front();
    long long two = 0;
    if (!t.empty()) {
        two = t.front();
        t.pop_front();
    }
    t.push_front(one + two);

    bool takeoverTurn = false;
    while (!t.empty() && !b.empty()) {
        if (takeoverTurn) {
            if (t.front() > b.front()) {
                b.pop_front();
            } else {
                one = t.front();
                t.pop_front();
                two = 0;
                if (!t.empty()) {
                    two = t.front();
                    t.pop_front();
                }
                t.push_front(one + two);
            }
        } else {
            if (b.front() > t.front()) {
                t.pop_front();
            } else {
                one = b.front();
                b.pop_front();
                two = 0;
                if (!b.empty()) {
                    two = b.front();
                    b.pop_front();
                }
                b.push_front(one + two);
            }
        }
        takeoverTurn = !takeoverTurn;
    }

    return !t.empty();
}

bool tryBuyFirst(const vector<long long>& takeover, const vector<long long>& buyout) {
    deque<long long> t(takeover.begin(), takeover.end());
    deque<long long> b(buyout.begin(), buyout.end());

    if (t.front() > b.front()) {
        b.pop_front();
    } else {
        return false;
    }

    bool takeoverTurn = false;
    while (!t.empty() && !b.empty()) {
        if (takeoverTurn) {
            if (t.front() > b.front()) {
                b.pop_front();
            } else {
                long long one = t.front();
                t.pop_front();
                long long two = 0;
                if (!t.empty()) {
                    two = t.front();
                    t.pop_front();
                }
                t.push_front(one + two);
            }
        } else {
            if (b.front() > t.front()) {
                t.pop_front();
            } else {
                long long one = b.front();
                b.pop_front();
                long long two = 0;
                if (!b.empty()) {
                    two = b.front();
                    b.pop_front();
                }
                b.push_front(one + two);
            }
        }
        takeoverTurn = !takeoverTurn;
    }

    return !t.empty();
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int m, n;
    int caseNo = 1;
    while (cin >> m >> n) {
        vector<long long> takeover(m), buyout(n);
        for (long long& x : takeover) {
            cin >> x;
        }
        for (long long& x : buyout) {
            cin >> x;
        }

        sort(takeover.begin(), takeover.end(), greater<long long>());
        sort(buyout.begin(), buyout.end(), greater<long long>());

        bool wins = tryMergeFirst(takeover, buyout) || tryBuyFirst(takeover, buyout);
        cout << "Case " << caseNo++ << ": "
             << (wins ? "Takeover Incorporated" : "Buyout Limited") << '\n';
    }

    return 0;
}
