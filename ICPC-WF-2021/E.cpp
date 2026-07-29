#include <algorithm>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <vector>

using namespace std;

namespace {

int k, m;
vector<long long> colorCount;
vector<int> spectrum;
long double successfulHands = 0;
long double visibleOrderFactor;

long double binomial(long long n, int r) {
    if (r < 0 || n < r) {
        return 0;
    }
    r = min(r, static_cast<int>(n - r));
    long double result = 1;
    for (int i = 1; i <= r; ++i) {
        result *= static_cast<long double>(n - r + i) / i;
    }
    return result;
}

void evaluateSpectrum() {
    long double hands = 1;
    for (int color = 0; color < m; ++color) {
        hands *= binomial(colorCount[color], spectrum[color]);
    }
    if (hands == 0) {
        return;
    }

    // An observation consists of the ordered visible cards and the marking
    // (color) of the hidden card.
    long double observations = 0;
    for (int hiddenColor = 0; hiddenColor < m; ++hiddenColor) {
        if (spectrum[hiddenColor] == 0) {
            continue;
        }

        long double choicesOfVisibleSet = 1;
        for (int color = 0; color < m; ++color) {
            int visible = spectrum[color] - (color == hiddenColor);
            choicesOfVisibleSet *= binomial(colorCount[color], visible);
        }
        observations += visibleOrderFactor * choicesOfVisibleSet;
    }

    // For a fixed spectrum the consistency graph has a matching saturating
    // its smaller side, so precisely min(|U|, |V|) hands can be successful.
    successfulHands += min(hands, observations);
}

void enumerateSpectra(int color, int remaining) {
    if (color + 1 == m) {
        spectrum[color] = remaining;
        evaluateSpectrum();
        return;
    }

    for (int take = 0; take <= remaining; ++take) {
        spectrum[color] = take;
        enumerateSpectra(color + 1, remaining - take);
    }
}

}  // namespace

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    cin >> k >> m;
    colorCount.resize(m);
    for (long long& count : colorCount) {
        cin >> count;
    }

    visibleOrderFactor = 1;
    for (int i = 2; i <= k - 1; ++i) {
        visibleOrderFactor *= i;
    }

    spectrum.assign(m, 0);
    enumerateSpectra(0, k);

    long long n = accumulate(colorCount.begin(), colorCount.end(), 0LL);
    long double totalHands = binomial(n, k);
    cout << fixed << setprecision(12) << successfulHands / totalHands << '\n';
    return 0;
}
