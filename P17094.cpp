#include <bits/stdc++.h>
using namespace std;

// P17094 [ICPC 2017 Qingdao R] Collecting Cents
//
// Each year: multiples of d compound for 1 year; residue r is peeled into
// lengths {2,3} and (when a*3 < d) length d. One-length peels may be partial;
// two-length peels fully split r. Pending stores pre-applied payouts.
//
// Dense generation-stamp DP + dense transition cache for rem >= d.

using u64 = unsigned long long;
using u128 = __uint128_t;
using u32 = unsigned int;

static inline u64 adep(u64 c, int t, int d) {
    return c + (u64)((u128)c * (u128)t / (u128)d);
}

namespace {

constexpr int PMAX = 5;
constexpr int PBASE = 16;
constexpr int RMAX = 6;
constexpr int CODES = 1048576;  // 16^5
constexpr int SMAX = RMAX * CODES;
constexpr int SLOTMAX = 1 << 16;

u64 best_q[SMAX];
u32 stamp[SMAX];
u32 gen_nxt = 1;

u32 slot_cur[SLOTMAX], slot_nxt[SLOTMAX];
u64 q_cur[SLOTMAX];
int nslot_cur = 0, nslot_nxt = 0;

struct Trans {
    int X;
    int pcode;
};

vector<Trans> arena;
u32 trans_off[SMAX];
uint16_t trans_len[SMAX];
uint16_t trans_built[SMAX];
uint16_t trans_epoch = 1;
int cache_d = -1;

inline int pack_state(int r, const int *p, int P) {
    int code = 0;
    for (int i = P - 1; i >= 0; --i) code = code * PBASE + p[i];
    return r + RMAX * code;
}

inline void unpack_state(int id, int &r, int *p, int P) {
    r = id % RMAX;
    int code = id / RMAX;
    for (int i = 0; i < P; ++i) {
        p[i] = code % PBASE;
        code /= PBASE;
    }
    for (int i = P; i < PMAX; ++i) p[i] = 0;
}

inline int pend_code_of(const int *p, int P) {
    int code = 0;
    for (int i = P - 1; i >= 0; --i) code = code * PBASE + p[i];
    return code;
}

inline void upsert_nxt(int id, u64 q) {
    if (stamp[id] != gen_nxt) {
        stamp[id] = gen_nxt;
        best_q[id] = q;
        slot_nxt[nslot_nxt++] = (u32)id;
    } else if (q > best_q[id]) {
        best_q[id] = q;
    }
}

void reset_trans_cache(int d) {
    if (cache_d == d) return;
    cache_d = d;
    ++trans_epoch;
    if (trans_epoch == 0) {
        memset(trans_built, 0, sizeof(trans_built));
        trans_epoch = 1;
    }
    arena.clear();
}

void ensure_trans(int id, int d, int P) {
    if (trans_built[id] == trans_epoch) return;
    trans_built[id] = trans_epoch;

    int r, pend[PMAX], np[PMAX];
    unpack_state(id, r, pend, P);

    u32 start = (u32)arena.size();

    auto add_trans = [&](int peel, int L1, int a1, int L2, int a2) {
        for (int i = 0; i < P - 1; ++i) np[i] = pend[i + 1];
        if (P) np[P - 1] = 0;
        if (a1) np[L1 - 2] += (int)adep((u64)a1, L1, d);
        if (a2) np[L2 - 2] += (int)adep((u64)a2, L2, d);
        arena.push_back({r - peel + pend[0], pend_code_of(np, P)});
    };

    add_trans(0, 0, 0, 0, 0);

    auto fill_lens = [&](int a, int *out) -> int {
        int m = 0;
        auto add = [&](int L) {
            if (L < 2 || L > d) return;
            for (int i = 0; i < m; ++i)
                if (out[i] == L) return;
            out[m++] = L;
        };
        add(2);
        add(3);
        if (a * 3 < d) add(d);
        return m;
    };

    for (int a1 = 1; a1 <= r; ++a1) {
        int L1s[4];
        int m1 = fill_lens(a1, L1s);
        for (int i = 0; i < m1; ++i) add_trans(a1, L1s[i], a1, 0, 0);
    }
    for (int a1 = 1; a1 < r; ++a1) {
        int a2 = r - a1;
        int L1s[4], L2s[4];
        int m1 = fill_lens(a1, L1s);
        int m2 = fill_lens(a2, L2s);
        for (int i = 0; i < m1; ++i)
            for (int j = 0; j < m2; ++j)
                if (L1s[i] < L2s[j])
                    add_trans(a1 + a2, L1s[i], a1, L2s[j], a2);
    }

    trans_off[id] = start;
    trans_len[id] = (uint16_t)(arena.size() - start);
}

u64 solve_one(u64 n, int d, int y) {
    if (y <= 0) return n;
    if (d == 1) return n << y;

    const int P = d - 1;
    reset_trans_cache(d);

    if (gen_nxt > 0xFFFFF000u) {
        memset(stamp, 0, sizeof(u32) * (size_t)SMAX);
        gen_nxt = 1;
    }

    nslot_cur = 0;
    int zp[PMAX] = {};
    slot_cur[0] = (u32)pack_state((int)(n % (u64)d), zp, P);
    q_cur[0] = n / (u64)d;
    nslot_cur = 1;

    u64 answer = 0;
    int pend[PMAX], np[PMAX];

    for (int year = 0; year < y; ++year) {
        const int rem = y - year;
        nslot_nxt = 0;
        ++gen_nxt;
        if (gen_nxt == 0) {
            memset(stamp, 0, sizeof(u32) * (size_t)SMAX);
            gen_nxt = 1;
        }

        const bool fast = (rem >= d);

        for (int si = 0; si < nslot_cur; ++si) {
            const int id = (int)slot_cur[si];
            const u64 q = q_cur[si];

            if (fast) {
                ensure_trans(id, d, P);
                const Trans *tr = arena.data() + trans_off[id];
                const int tlen = trans_len[id];
                for (int k = 0; k < tlen; ++k) {
                    const u64 nf = q * (u64)(d + 1) + (u64)(u32)tr[k].X;
                    const int nid = (int)(nf % (u64)d) + RMAX * tr[k].pcode;
                    upsert_nxt(nid, nf / (u64)d);
                }
                continue;
            }

            int r;
            unpack_state(id, r, pend, P);

            auto emit = [&](int L1, int a1, int L2, int a2) {
                const int peel = a1 + a2;
                const u64 nf =
                    (u64)((u128)q * (u128)(d + 1) + (u128)(r - peel) +
                          (u128)pend[0]);
                if (rem == 1) {
                    if (nf > answer) answer = nf;
                    return;
                }
                for (int i = 0; i < P - 1; ++i) np[i] = pend[i + 1];
                if (P) np[P - 1] = 0;
                if (a1) np[L1 - 2] += (int)adep((u64)a1, L1, d);
                if (a2) np[L2 - 2] += (int)adep((u64)a2, L2, d);
                upsert_nxt(pack_state((int)(nf % (u64)d), np, P),
                           nf / (u64)d);
            };

            emit(0, 0, 0, 0);

            auto fill_lens = [&](int a, int *out) -> int {
                int m = 0;
                auto add = [&](int L) {
                    if (L < 2 || L > min(d, rem)) return;
                    for (int i = 0; i < m; ++i)
                        if (out[i] == L) return;
                    out[m++] = L;
                };
                add(2);
                add(3);
                if (a * 3 < d) add(d);
                return m;
            };

            for (int a1 = 1; a1 <= r; ++a1) {
                int L1s[4];
                int m1 = fill_lens(a1, L1s);
                for (int i = 0; i < m1; ++i) emit(L1s[i], a1, 0, 0);
            }
            for (int a1 = 1; a1 < r; ++a1) {
                const int a2 = r - a1;
                int L1s[4], L2s[4];
                int m1 = fill_lens(a1, L1s);
                int m2 = fill_lens(a2, L2s);
                for (int i = 0; i < m1; ++i)
                    for (int j = 0; j < m2; ++j)
                        if (L1s[i] < L2s[j]) emit(L1s[i], a1, L2s[j], a2);
            }
        }

        if (rem == 1) return answer;

        nslot_cur = nslot_nxt;
        for (int i = 0; i < nslot_cur; ++i) {
            slot_cur[i] = slot_nxt[i];
            q_cur[i] = best_q[slot_nxt[i]];
        }
    }

    for (int si = 0; si < nslot_cur; ++si) {
        int id = (int)slot_cur[si];
        int r;
        unpack_state(id, r, pend, P);
        u64 tot = q_cur[si] * (u64)d + (u64)r;
        for (int i = 0; i < P; ++i) tot += (u64)pend[i];
        if (tot > answer) answer = tot;
    }
    return answer;
}

}  // namespace

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int T;
    if (!(cin >> T)) return 0;
    while (T--) {
        u64 n;
        int d, y;
        cin >> n >> d >> y;
        cout << solve_one(n, d, y) << '\n';
    }
    return 0;
}
