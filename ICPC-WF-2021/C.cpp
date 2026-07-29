#include <cstdint>
#include <iostream>
#include <numeric>

using namespace std;

using i128 = __int128_t;
using int64 = long long;

// Returns base^exponent, or limit+1 if it is larger than limit.
static i128 powerLimited(int64 base, int exponent, int64 limit) {
    i128 result = 1;
    i128 factor = base;
    const i128 cap = static_cast<i128>(limit) + 1;

    while (exponent > 0) {
        if (exponent & 1) {
            result *= factor;
            if (result > limit) {
                return cap;
            }
        }
        exponent >>= 1;
        if (exponent > 0) {
            factor *= factor;
            if (factor > limit) {
                factor = cap;
            }
        }
    }
    return result;
}

static i128 integerPower(i128 base, int exponent) {
    i128 result = 1;
    while (exponent > 0) {
        if (exponent & 1) {
            result *= base;
        }
        exponent >>= 1;
        if (exponent > 0) {
            base *= base;
        }
    }
    return result;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    int64 m;
    cin >> n >> m;

    // If g=a/b is the passed fraction, integrality is equivalent to
    // S=(b^n-a^n)/(b-a) dividing m.  Also S >= b^(n-1).
    for (int64 b = 2; powerLimited(b, n - 1, m) <= m; ++b) {
        const i128 bPower = integerPower(b, n);

        // f=(b-a)/b, so descending a minimizes p for this denominator.
        for (int64 a = b - 1; a >= 1; --a) {
            if (gcd(a, b) != 1) {
                continue;
            }

            const i128 aPower = integerPower(a, n);
            const i128 sum = (bPower - aPower) / (b - a);
            if (sum <= m && m % static_cast<int64>(sum) == 0) {
                cout << b - a << ' ' << b << '\n';
                return 0;
            }
        }
    }

    cout << "impossible\n";
    return 0;
}
