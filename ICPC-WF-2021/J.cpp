#include <algorithm>
#include <functional>
#include <iostream>
#include <vector>

using namespace std;

struct Producer {
    char type = 0;
    int first = 0;
    int second = 0;
    int port = 0;
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    long long inputLength;
    int nodeCount, queryCount;
    cin >> inputLength >> nodeCount >> queryCount;

    const int maxIdentifier = max(100005, 2 * nodeCount + 5);
    vector<Producer> producer(maxIdentifier);

    for (int i = 0; i < nodeCount; ++i) {
        char type;
        int x, y, z;
        cin >> type >> x >> y >> z;
        if (type == 'S') {
            producer[y] = {'S', x, 0, 0};
            producer[z] = {'S', x, 0, 1};
        } else {
            producer[z] = {'M', x, y, 0};
        }
    }

    vector<long long> length(maxIdentifier, -1);
    length[1] = inputLength;

    function<long long(int)> getLength = [&](int wire) -> long long {
        if (length[wire] != -1) {
            return length[wire];
        }

        const Producer& p = producer[wire];
        if (p.type == 'S') {
            const long long input = getLength(p.first);
            length[wire] = (p.port == 0 ? (input + 1) / 2 : input / 2);
        } else {
            length[wire] = getLength(p.first) + getLength(p.second);
        }
        return length[wire];
    };

    while (queryCount--) {
        int wire;
        long long index;
        cin >> wire >> index;

        if (index > getLength(wire)) {
            cout << "none\n";
            continue;
        }

        while (wire != 1) {
            const Producer& p = producer[wire];
            if (p.type == 'S') {
                index = 2 * index - (p.port == 0 ? 1 : 0);
                wire = p.first;
                continue;
            }

            const long long firstLength = getLength(p.first);
            const long long secondLength = getLength(p.second);
            const long long paired = min(firstLength, secondLength);
            if (index <= 2 * paired) {
                if (index % 2 == 1) {
                    index = (index + 1) / 2;
                    wire = p.first;
                } else {
                    index /= 2;
                    wire = p.second;
                }
            } else if (firstLength > secondLength) {
                index -= secondLength;
                wire = p.first;
            } else {
                index -= firstLength;
                wire = p.second;
            }
        }

        cout << index << '\n';
    }

    return 0;
}
