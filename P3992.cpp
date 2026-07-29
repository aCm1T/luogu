#include <bits/stdc++.h>
using namespace std;

using ll = long long;

static const int B = 400;
static const int MAXP = 150005; // ≤ 3n points after discretization
static const int MAXBLK = MAXP / B + 5;

struct Block {
    int L = 0, R = 0, len = 0, tag = 0;
    // a: original order (value, length); b: sorted by value
    vector<pair<int, int>> a, b;
    vector<int> preLen; // prefix sum of lengths on sorted b
    ll ans = 0;

    void rebuild_sorted() {
        b = a;
        sort(b.begin(), b.end());
        preLen.assign(len + 1, 0);
        ans = 0;
        for (int i = 0; i < len; ++i) {
            preLen[i + 1] = preLen[i] + b[i].second;
            ans += 1LL * abs(b[i].first + tag) * b[i].second;
        }
    }

    void init(int l, int r, const vector<pair<int, int>> &src) {
        L = l;
        R = r;
        len = r - l + 1;
        tag = 0;
        a.assign(src.begin() + l, src.begin() + r + 1);
        rebuild_sorted();
    }

    // brute update on [l, r] (global indices), then rebuild
    void modify_range(int l, int r, int x) {
        for (int i = l; i <= r; ++i) a[i - L].first += x;
        rebuild_sorted();
    }

    // whole-block lazy ±1
    void modify_all(int x) {
        // lengths of positions that are currently negative / non-negative after tag
        // b sorted by raw value; effective = raw + tag
        if (x == 1) {
            // find last index with raw + tag < 0  i.e. raw < -tag
            int id = int(lower_bound(b.begin(), b.end(), make_pair(-tag, 0)) - b.begin());
            // [0, id) negative → become less negative or zero: |v| decreases by 1
            // [id, len) non-negative → |v| increases by 1
            ans -= preLen[id];
            ans += preLen[len] - preLen[id];
        } else { // x == -1
            // find last with raw + tag <= 0  i.e. raw <= -tag  → raw < -tag+1
            int id = int(lower_bound(b.begin(), b.end(), make_pair(-tag + 1, 0)) - b.begin());
            // [0, id] currently ≤ 0 → |v| increases by 1 when -=1
            // wait: values ≤ 0 become more negative or stay: |v| increases
            // values > 0 decrease by 1: |v| decreases
            // id = count of raw < -tag+1 = raw <= -tag = effective <= 0
            ans += preLen[id];
            ans -= preLen[len] - preLen[id];
        }
        tag += x;
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    cin >> n;
    vector<int> car(n + 1), station(n + 1);
    vector<int> coords;
    coords.reserve(3 * n + 5);
    for (int i = 1; i <= n; ++i) {
        cin >> car[i];
        coords.push_back(car[i]);
    }
    for (int i = 1; i <= n; ++i) {
        cin >> station[i];
        coords.push_back(station[i]);
    }
    int q;
    cin >> q;
    vector<pair<int, int>> qs(q);
    for (int i = 0; i < q; ++i) {
        cin >> qs[i].first >> qs[i].second;
        coords.push_back(qs[i].second);
    }

    sort(coords.begin(), coords.end());
    coords.erase(unique(coords.begin(), coords.end()), coords.end());
    auto id = [&](int x) {
        return int(lower_bound(coords.begin(), coords.end(), x) - coords.begin());
    };

    const int m = (int)coords.size(); // discrete points [0..m-1]
    // node[i] = (prefix, segment_len to next)
    vector<pair<int, int>> node(m);
    vector<int> diff(m + 1, 0);
    for (int i = 1; i <= n; ++i) {
        ++diff[id(car[i])];
        --diff[id(station[i])];
        car[i] = id(car[i]); // store discrete ids
    }
    for (auto &op : qs) op.second = id(op.second);

    int pref = 0;
    for (int i = 0; i < m; ++i) {
        pref += diff[i];
        node[i].first = pref;
        node[i].second = (i + 1 < m ? coords[i + 1] - coords[i] : 0);
    }

    const int nblk = (m + B - 1) / B;
    vector<Block> blk(nblk);
    for (int i = 0; i < nblk; ++i) {
        int l = i * B;
        int r = min(m - 1, (i + 1) * B - 1);
        blk[i].init(l, r, node);
    }

    auto total = [&]() {
        ll s = 0;
        for (int i = 0; i < nblk; ++i) s += blk[i].ans;
        return s;
    };

    auto range_add = [&](int l, int r, int x) {
        if (l > r) return;
        int bl = l / B, br = r / B;
        if (bl == br) {
            blk[bl].modify_range(l, r, x);
            return;
        }
        blk[bl].modify_range(l, blk[bl].R, x);
        blk[br].modify_range(blk[br].L, r, x);
        for (int i = bl + 1; i < br; ++i) blk[i].modify_all(x);
    };

    cout << total() << '\n';
    for (auto [idx, nx] : qs) {
        int old = car[idx];
        if (old < nx) range_add(old, nx - 1, -1);
        else if (old > nx) range_add(nx, old - 1, 1);
        car[idx] = nx;
        cout << total() << '\n';
    }
    return 0;
}
