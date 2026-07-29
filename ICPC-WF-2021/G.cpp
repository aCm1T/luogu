#include <algorithm>
#include <cmath>
#include <complex>
#include <iostream>
#include <utility>
#include <vector>

using namespace std;

namespace {

using Complex = complex<double>;
const double PI = acos(-1.0);

void fftLine(Complex* values, int size, bool inverse) {
    for (int i = 1, j = 0; i < size; ++i) {
        int bit = size >> 1;
        while (j & bit) {
            j ^= bit;
            bit >>= 1;
        }
        j ^= bit;
        if (i < j) {
            swap(values[i], values[j]);
        }
    }

    for (int length = 2; length <= size; length <<= 1) {
        double angle = 2.0 * PI / length * (inverse ? 1.0 : -1.0);
        Complex step(cos(angle), sin(angle));
        int half = length >> 1;
        for (int start = 0; start < size; start += length) {
            Complex root(1.0, 0.0);
            for (int offset = 0; offset < half; ++offset) {
                Complex even = values[start + offset];
                Complex odd = values[start + offset + half] * root;
                values[start + offset] = even + odd;
                values[start + offset + half] = even - odd;
                root *= step;
            }
        }
    }

    if (inverse) {
        for (int i = 0; i < size; ++i) {
            values[i] /= size;
        }
    }
}

void fftRows(vector<Complex>& values, int rows, int columns, bool inverse) {
    for (int row = 0; row < rows; ++row) {
        fftLine(values.data() + static_cast<size_t>(row) * columns,
                columns, inverse);
    }
}

void transpose(const vector<Complex>& source, vector<Complex>& destination,
               int rows, int columns) {
    constexpr int BLOCK = 32;
    for (int rowBlock = 0; rowBlock < rows; rowBlock += BLOCK) {
        for (int columnBlock = 0; columnBlock < columns;
             columnBlock += BLOCK) {
            int rowEnd = min(rows, rowBlock + BLOCK);
            int columnEnd = min(columns, columnBlock + BLOCK);
            for (int row = rowBlock; row < rowEnd; ++row) {
                for (int column = columnBlock; column < columnEnd; ++column) {
                    destination[static_cast<size_t>(column) * rows + row] =
                        source[static_cast<size_t>(row) * columns + column];
                }
            }
        }
    }
}

}  // namespace

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int motifRows, motifColumns;
    cin >> motifRows >> motifColumns;
    vector<int> motif(static_cast<size_t>(motifRows) * motifColumns);
    for (int& color : motif) {
        cin >> color;
    }

    int mosaicRows, mosaicColumns;
    cin >> mosaicRows >> mosaicColumns;
    vector<int> mosaic(static_cast<size_t>(mosaicRows) * mosaicColumns);
    for (int& color : mosaic) {
        cin >> color;
    }

    if (motifRows > mosaicRows || motifColumns > mosaicColumns) {
        cout << "0\n";
        return 0;
    }

    int fftRowsCount = 1;
    while (fftRowsCount < motifRows + mosaicRows - 1) {
        fftRowsCount <<= 1;
    }
    int fftColumnsCount = 1;
    while (fftColumnsCount < motifColumns + mosaicColumns - 1) {
        fftColumnsCount <<= 1;
    }

    size_t transformSize =
        static_cast<size_t>(fftRowsCount) * fftColumnsCount;
    vector<Complex> first(transformSize);
    vector<Complex> second(transformSize);
    vector<Complex> spare(transformSize);

    long long motifSquareSum = 0;
    for (int row = 0; row < motifRows; ++row) {
        for (int column = 0; column < motifColumns; ++column) {
            int color =
                motif[static_cast<size_t>(row) * motifColumns + column];
            if (color == 0) {
                continue;
            }
            motifSquareSum += 1LL * color * color;

            // Reverse the motif so convolution indices represent alignments.
            int reversedRow = motifRows - 1 - row;
            int reversedColumn = motifColumns - 1 - column;
            first[static_cast<size_t>(reversedRow) * fftColumnsCount +
                  reversedColumn] = Complex(color, 1);
        }
    }
    for (int row = 0; row < mosaicRows; ++row) {
        for (int column = 0; column < mosaicColumns; ++column) {
            int color =
                mosaic[static_cast<size_t>(row) * mosaicColumns + column];
            second[static_cast<size_t>(row) * fftColumnsCount + column] =
                Complex(color, color * color);
        }
    }
    motif.clear();
    motif.shrink_to_fit();
    mosaic.clear();
    mosaic.shrink_to_fit();

    // Transform the packed (motif colors, motif mask) arrays. Keeping the
    // result transposed makes the second transform cache-friendly.
    fftRows(first, fftRowsCount, fftColumnsCount, false);
    transpose(first, spare, fftRowsCount, fftColumnsCount);
    fftRows(spare, fftColumnsCount, fftRowsCount, false);

    // Transform the packed (mosaic colors, squared mosaic colors) arrays.
    fftRows(second, fftRowsCount, fftColumnsCount, false);
    transpose(second, first, fftRowsCount, fftColumnsCount);
    fftRows(first, fftColumnsCount, fftRowsCount, false);

    // If F is the transform of a+i*b for real arrays a and b, then
    // transform(a)[x] = (F[x] + conjugate(F[-x])) / 2. Extract both packed
    // transforms and put the two required convolutions into one inverse FFT.
    const Complex inverseTwoI(0.0, -0.5);
    const Complex imaginaryUnit(0.0, 1.0);
    for (int row = 0; row < fftColumnsCount; ++row) {
        int mirrorRow = (fftColumnsCount - row) % fftColumnsCount;
        for (int column = 0; column < fftRowsCount; ++column) {
            int mirrorColumn = (fftRowsCount - column) % fftRowsCount;
            size_t index = static_cast<size_t>(row) * fftRowsCount + column;
            size_t mirror =
                static_cast<size_t>(mirrorRow) * fftRowsCount + mirrorColumn;

            Complex motifColors =
                (spare[index] + conj(spare[mirror])) * 0.5;
            Complex motifMask =
                (spare[index] - conj(spare[mirror])) * inverseTwoI;
            Complex mosaicColors =
                (first[index] + conj(first[mirror])) * 0.5;
            Complex mosaicSquares =
                (first[index] - conj(first[mirror])) * inverseTwoI;

            Complex colorProducts = motifColors * mosaicColors;
            Complex selectedSquares = motifMask * mosaicSquares;
            second[index] =
                colorProducts + imaginaryUnit * selectedSquares;
        }
    }

    fftRows(second, fftColumnsCount, fftRowsCount, true);
    transpose(second, first, fftColumnsCount, fftRowsCount);
    fftRows(first, fftRowsCount, fftColumnsCount, true);

    vector<pair<int, int>> matches;
    for (int row = 0; row + motifRows <= mosaicRows; ++row) {
        for (int column = 0; column + motifColumns <= mosaicColumns;
             ++column) {
            size_t convolutionIndex =
                static_cast<size_t>(row + motifRows - 1) * fftColumnsCount +
                column + motifColumns - 1;
            long long colorProducts =
                llround(first[convolutionIndex].real());
            long long selectedSquares =
                llround(first[convolutionIndex].imag());

            // This is the integer sum of (motifColor-mosaicColor)^2 over
            // all non-empty motif cells, and is zero exactly for a match.
            long long mismatchScore =
                motifSquareSum - 2 * colorProducts + selectedSquares;
            if (mismatchScore == 0) {
                matches.emplace_back(row + 1, column + 1);
            }
        }
    }

    cout << matches.size() << '\n';
    for (auto [row, column] : matches) {
        cout << row << ' ' << column << '\n';
    }
    return 0;
}
