#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

struct Wind {
    int dx;
    int dy;
    vector<uint8_t> boundary;
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int width, height, k;
    cin >> width >> height >> k;
    const int cells = width * height;

    vector<Wind> winds(k);
    for (Wind& wind : winds) {
        int count;
        cin >> wind.dx >> wind.dy >> count;
        wind.boundary.assign(cells, false);
        while (count--) {
            int x, y;
            cin >> x >> y;
            --x;
            --y;
            wind.boundary[y * width + x] = true;
        }
    }

    vector<uint8_t> molecule(cells, false);
    vector<int> queue;
    queue.reserve(cells);

    // Every reported boundary is a molecule.
    for (const Wind& wind : winds) {
        for (int cell = 0; cell < cells; ++cell) {
            if (wind.boundary[cell] && !molecule[cell]) {
                molecule[cell] = true;
                queue.push_back(cell);
            }
        }
    }

    // At a non-boundary A, A being a molecule forces A-w to be one too.
    for (size_t head = 0; head < queue.size(); ++head) {
        const int cell = queue[head];
        const int x = cell % width;
        const int y = cell / width;
        for (const Wind& wind : winds) {
            if (wind.boundary[cell]) {
                continue;
            }
            const int px = x - wind.dx;
            const int py = y - wind.dy;
            if (0 <= px && px < width && 0 <= py && py < height) {
                const int predecessor = py * width + px;
                if (!molecule[predecessor]) {
                    molecule[predecessor] = true;
                    queue.push_back(predecessor);
                }
            }
        }
    }

    vector<uint8_t> empty(cells, false);
    queue.clear();
    auto forceEmpty = [&](int cell) {
        if (!empty[cell]) {
            empty[cell] = true;
            queue.push_back(cell);
        }
    };

    for (const Wind& wind : winds) {
        for (int cell = 0; cell < cells; ++cell) {
            const int x = cell % width;
            const int y = cell / width;
            const int px = x - wind.dx;
            const int py = y - wind.dy;
            const bool predecessorInside =
                0 <= px && px < width && 0 <= py && py < height;

            if (wind.boundary[cell]) {
                // A reported boundary has no molecule immediately upwind.
                if (predecessorInside) {
                    forceEmpty(py * width + px);
                }
            } else if (!predecessorInside) {
                // Otherwise this cell itself would be an unreported boundary.
                forceEmpty(cell);
            }
        }
    }

    // If B is empty, an unreported cell B+w must also be empty.
    for (size_t head = 0; head < queue.size(); ++head) {
        const int cell = queue[head];
        const int x = cell % width;
        const int y = cell / width;
        for (const Wind& wind : winds) {
            const int sx = x + wind.dx;
            const int sy = y + wind.dy;
            if (0 <= sx && sx < width && 0 <= sy && sy < height) {
                const int successor = sy * width + sx;
                if (!wind.boundary[successor]) {
                    forceEmpty(successor);
                }
            }
        }
    }

    // Unknown cells are empty in the minimum and occupied in the maximum.
    for (int y = 0; y < height; ++y) {
        string row(width, '.');
        for (int x = 0; x < width; ++x) {
            if (molecule[y * width + x]) {
                row[x] = '#';
            }
        }
        cout << row << '\n';
    }
    cout << '\n';
    for (int y = 0; y < height; ++y) {
        string row(width, '#');
        for (int x = 0; x < width; ++x) {
            if (empty[y * width + x]) {
                row[x] = '.';
            }
        }
        cout << row << '\n';
    }

    return 0;
}
