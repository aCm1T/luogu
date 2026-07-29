#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

struct Tablet {
    int balance;
    int minPrefix;
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    cin >> n;

    vector<Tablet> tablets(n);
    vector<int> nonnegative;
    vector<int> negative;
    nonnegative.reserve(n);
    negative.reserve(n);

    long long totalBalance = 0;
    for (int id = 0; id < n; ++id) {
        string text;
        cin >> text;

        int balance = 0;
        int minPrefix = 0;
        for (char symbol : text) {
            balance += (symbol == '(' ? 1 : -1);
            minPrefix = min(minPrefix, balance);
        }

        tablets[id] = {balance, minPrefix};
        totalBalance += balance;
        if (balance >= 0) {
            nonnegative.push_back(id);
        } else {
            negative.push_back(id);
        }
    }

    if (totalBalance != 0) {
        cout << "impossible\n";
        return 0;
    }

    // Nonnegative tablets with a smaller required starting balance go first.
    sort(nonnegative.begin(), nonnegative.end(), [&](int lhs, int rhs) {
        if (tablets[lhs].minPrefix != tablets[rhs].minPrefix) {
            return tablets[lhs].minPrefix > tablets[rhs].minPrefix;
        }
        return lhs < rhs;
    });

    // This is the analogous greedy order after reversing and swapping
    // parentheses in all negative-balance tablets.
    sort(negative.begin(), negative.end(), [&](int lhs, int rhs) {
        const int lhsKey =
            tablets[lhs].minPrefix - tablets[lhs].balance;
        const int rhsKey =
            tablets[rhs].minPrefix - tablets[rhs].balance;
        if (lhsKey != rhsKey) {
            return lhsKey < rhsKey;
        }
        return lhs < rhs;
    });

    long long currentBalance = 0;
    auto append = [&](int id) {
        if (currentBalance + tablets[id].minPrefix < 0) {
            return false;
        }
        currentBalance += tablets[id].balance;
        return true;
    };

    for (int id : nonnegative) {
        if (!append(id)) {
            cout << "impossible\n";
            return 0;
        }
    }
    for (int id : negative) {
        if (!append(id)) {
            cout << "impossible\n";
            return 0;
        }
    }

    for (int id : nonnegative) {
        cout << id + 1 << '\n';
    }
    for (int id : negative) {
        cout << id + 1 << '\n';
    }

    return 0;
}
