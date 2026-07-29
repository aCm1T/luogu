#include <cstdint>
#include <algorithm>
#include <iostream>
#include <vector>

using namespace std;

struct DynamicBitSet {
    int n = 0;
    vector<uint64_t> bits;

    DynamicBitSet() = default;
    explicit DynamicBitSet(int size) : n(size), bits((size + 63) / 64, 0) {}

    void flip(int idx) {
        bits[idx / 64] ^= (1ULL << (idx % 64));
    }

    void setBit(int idx, bool value) {
        if (value) {
            bits[idx / 64] |= (1ULL << (idx % 64));
        } else {
            bits[idx / 64] &= ~(1ULL << (idx % 64));
        }
    }

    bool get(int idx) const {
        return (bits[idx / 64] >> (idx % 64)) & 1ULL;
    }

    void clear() {
        for (uint64_t& block : bits) block = 0;
    }

    void orWith(const DynamicBitSet& other) {
        for (int i = 0; i < (int)bits.size(); ++i) bits[i] |= other.bits[i];
    }

    int cardinality() const {
        int ans = 0;
        for (uint64_t block : bits) ans += __builtin_popcountll(block);
        return ans;
    }
};

int n;
vector<DynamicBitSet> bits;

DynamicBitSet convertRow(int loc, const string& s) {
    DynamicBitSet res((int)s.size());
    res.flip(loc);
    for (int i = 0; i < (int)s.size(); ++i) {
        if (s[i] == '1') res.flip(i);
    }
    return res;
}

int solve(int curSet, int k, int maxItems, int last, const DynamicBitSet& allBits) {
    if (k == maxItems) {
        if (allBits.cardinality() == n) return curSet;
        return -1;
    }

    for (int i = last + 1; i < n; ++i) {
        DynamicBitSet newBits = allBits;
        newBits.orWith(bits[i]);
        int tmp = solve(curSet * 100 + i, k + 1, maxItems, i, newBits);
        if (tmp != -1) return tmp;
    }

    return -1;
}

int getBest(const DynamicBitSet& used, const DynamicBitSet& inSet) {
    int maxS = used.cardinality();
    int res = -1;

    for (int i = 0; i < n; ++i) {
        if (inSet.get(i)) continue;
        DynamicBitSet newBits = used;
        newBits.orWith(bits[i]);
        if (newBits.cardinality() > maxS) {
            res = i;
            maxS = newBits.cardinality();
        }
    }

    return res;
}

void solveWrapper(int caseNum) {
    for (int i = 1; i <= 4; ++i) {
        DynamicBitSet empty(n);
        int res = solve(0, 0, i, -1, empty);
        if (res != -1) {
            vector<int> list;
            for (int j = 0; j < i; ++j) {
                list.push_back(res % 100 + 1);
                res /= 100;
            }
            reverse(list.begin(), list.end());

            cout << "Case " << caseNum << ": " << i;
            for (int x : list) cout << ' ' << x;
            cout << '\n';
            return;
        }
    }

    DynamicBitSet used(n);
    DynamicBitSet inSet(n);
    for (int i = 0; i < 5; ++i) {
        int addItem = getBest(used, inSet);
        inSet.flip(addItem);
        used.orWith(bits[addItem]);

        for (int j = 0; j < n; ++j) {
            if (used.get(j)) {
                bits[j].clear();
                for (int k = 0; k < n; ++k) bits[k].setBit(j, false);
            }
        }
    }

    cout << "Case " << caseNum << ": 5";
    for (int i = 0; i < n; ++i) {
        if (inSet.get(i)) cout << ' ' << (i + 1);
    }
    cout << '\n';
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int loop = 1;
    while (cin >> n) {
        bits.assign(n, DynamicBitSet(n));
        for (int i = 0; i < n; ++i) {
            string s;
            cin >> s;
            bits[i] = convertRow(i, s);
        }

        solveWrapper(loop);
        ++loop;
    }

    return 0;
}
