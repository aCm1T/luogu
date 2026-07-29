#include <cmath>
#include <iomanip>
#include <iostream>
#include <vector>

using namespace std;

double evalPoly(const vector<double>& poly, double x) {
    double ans = 0.0;
    for (int i = 0; i < (int)poly.size(); ++i) {
        ans += poly[i] * pow(x, i);
    }
    return ans;
}

double volume(const vector<double>& integral, double low, double high) {
    return acos(-1.0) * (evalPoly(integral, high) - evalPoly(integral, low));
}

double searchMark(const vector<double>& integral, double target, double lowInt,
                  double lowSearch, double highSearch) {
    double mid = (lowSearch + highSearch) / 2.0;
    while (highSearch - lowSearch > 1e-4) {
        mid = (lowSearch + highSearch) / 2.0;
        double val = volume(integral, lowInt, mid);
        if (val < target) {
            lowSearch = mid;
        } else if (val > target) {
            highSearch = mid;
        } else {
            return mid;
        }
    }
    return mid;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int degree;
    int loop = 1;
    cout << fixed << setprecision(2);

    while (cin >> degree) {
        vector<double> poly(degree + 1);
        for (double& coeff : poly) cin >> coeff;

        vector<double> polysq(2 * degree + 1, 0.0);
        for (int i = 0; i < (int)poly.size(); ++i) {
            for (int j = 0; j < (int)poly.size(); ++j) {
                polysq[i + j] += poly[i] * poly[j];
            }
        }

        vector<double> integral(2 * degree + 2, 0.0);
        for (int i = 0; i < (int)polysq.size(); ++i) {
            integral[i + 1] = polysq[i] / (i + 1);
        }

        double low, high, inc;
        cin >> low >> high >> inc;

        double total = volume(integral, low, high);
        cout << "Case " << loop << ": " << total << '\n';

        if (total < inc) {
            cout << "insufficient volume\n";
        } else {
            int numMarks = (int)(total / inc);
            if (numMarks > 8) numMarks = 8;

            double newX = low;
            for (int i = 0; i < numMarks; ++i) {
                double target = (i + 1) * inc;
                newX = searchMark(integral, target, low, newX, high);
                cout << newX - low << ' ';
            }
            cout << '\n';
        }

        ++loop;
    }

    return 0;
}
