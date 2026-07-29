#include <algorithm>
#include <iostream>
#include <vector>

using namespace std;

namespace {

constexpr int INF = 1'000'000'000;

class CostTree {
    struct Node {
        // Pending candidates plusConstant+i and minusConstant-i.
        int plusConstant;
        int minusConstant;
    };

    int size;
    vector<Node> tree;

    void applyPlus(int node, int value) {
        tree[node].plusConstant = min(tree[node].plusConstant, value);
    }

    void applyMinus(int node, int value) {
        tree[node].minusConstant = min(tree[node].minusConstant, value);
    }

    void push(int node) {
        applyPlus(node * 2, tree[node].plusConstant);
        applyPlus(node * 2 + 1, tree[node].plusConstant);
        applyMinus(node * 2, tree[node].minusConstant);
        applyMinus(node * 2 + 1, tree[node].minusConstant);
        tree[node] = {INF, INF};
    }

    void updatePlus(int node, int left, int right, int queryLeft,
                    int queryRight, int value) {
        if (queryLeft <= left && right <= queryRight) {
            applyPlus(node, value);
            return;
        }
        push(node);
        int middle = (left + right) / 2;
        if (queryLeft <= middle) {
            updatePlus(node * 2, left, middle, queryLeft, queryRight, value);
        }
        if (middle < queryRight) {
            updatePlus(node * 2 + 1, middle + 1, right, queryLeft,
                       queryRight, value);
        }
    }

    void updateMinus(int node, int left, int right, int queryLeft,
                     int queryRight, int value) {
        if (queryLeft <= left && right <= queryRight) {
            applyMinus(node, value);
            return;
        }
        push(node);
        int middle = (left + right) / 2;
        if (queryLeft <= middle) {
            updateMinus(node * 2, left, middle, queryLeft, queryRight, value);
        }
        if (middle < queryRight) {
            updateMinus(node * 2 + 1, middle + 1, right, queryLeft,
                        queryRight, value);
        }
    }

    void setPoint(int node, int left, int right, int position, int value) {
        if (left == right) {
            tree[node] = {value - position, value + position};
            return;
        }
        push(node);
        int middle = (left + right) / 2;
        if (position <= middle) {
            setPoint(node * 2, left, middle, position, value);
        } else {
            setPoint(node * 2 + 1, middle + 1, right, position, value);
        }
    }

    int getPoint(int node, int left, int right, int position) {
        if (left == right) {
            return min(tree[node].plusConstant + position,
                       tree[node].minusConstant - position);
        }
        push(node);
        int middle = (left + right) / 2;
        if (position <= middle) {
            return getPoint(node * 2, left, middle, position);
        }
        return getPoint(node * 2 + 1, middle + 1, right, position);
    }

public:
    explicit CostTree(int strandCount)
        : size(strandCount),
          tree(4 * strandCount + 4, Node{INF, INF}) {}

    void setPoint(int position, int value) {
        setPoint(1, 1, size, position, value);
    }

    int getPoint(int position) {
        return getPoint(1, 1, size, position);
    }

    // Apply cost[i] = min(cost[i], value + circularDistance(position, i)).
    void addCircularCone(int position, int value) {
        updatePlus(1, 1, size, position, size, value - position);
        updateMinus(1, 1, size, 1, position, value + position);

        // The same two directions after wrapping around between strands
        // size and 1. Applying them globally is harmless; outside the part
        // where they are shortest they are merely dominated candidates.
        applyPlus(1, value + size - position);
        applyMinus(1, value + size + position);
    }
};

struct Bridge {
    int distance;
    int firstStrand;
};

}  // namespace

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n, m, target;
    cin >> n >> m >> target;
    vector<Bridge> bridges(m);
    for (Bridge& bridge : bridges) {
        cin >> bridge.distance >> bridge.firstStrand;
    }
    sort(bridges.begin(), bridges.end(),
         [](const Bridge& lhs, const Bridge& rhs) {
             return lhs.distance > rhs.distance;
         });

    // Reverse time: start at the outside end of target and move inward.
    // Between original bridges, added bridges can move to a strand at cost
    // equal to circular distance.
    CostTree costs(n);
    costs.addCircularCone(target, 0);

    for (const Bridge& bridge : bridges) {
        int first = bridge.firstStrand;
        int second = first % n + 1;

        int oldFirst = costs.getPoint(first);
        int oldSecond = costs.getPoint(second);

        // Crossing this original bridge is mandatory, so its endpoint costs
        // swap. These assignments intentionally discard old candidates at
        // the two endpoints.
        int newFirst = oldSecond;
        int newSecond = oldFirst;
        costs.setPoint(first, newFirst);
        costs.setPoint(second, newSecond);

        // Before restoring the circular Lipschitz condition, only the outer
        // neighbor of each swapped endpoint can improve that endpoint.
        int beforeFirst = (first == 1 ? n : first - 1);
        int afterSecond = (second == n ? 1 : second + 1);
        int neighborCost = costs.getPoint(beforeFirst);
        if (neighborCost + 1 < newFirst) {
            costs.setPoint(first, neighborCost + 1);
        }
        neighborCost = costs.getPoint(afterSecond);
        if (neighborCost + 1 < newSecond) {
            costs.setPoint(second, neighborCost + 1);
        }

        // Propagate every improvement caused by the swap. If an endpoint was
        // lowered from its outer neighbor above, that neighbor's existing
        // cone already performs the needed propagation.
        costs.addCircularCone(first, newFirst);
        costs.addCircularCone(second, newSecond);
    }

    for (int strand = 1; strand <= n; ++strand) {
        cout << costs.getPoint(strand) << '\n';
    }
    return 0;
}
