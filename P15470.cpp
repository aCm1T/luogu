#include <bits/stdc++.h>
using namespace std;

struct Frame {
    int vertex;
    int forbidden;
    int last_jealous;
    int stage;
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    cin >> n;

    vector<int> jealous(n + 1), liked(n + 1);
    vector<vector<int>> jealous_children(n + 1);
    for (int i = 1; i <= n; ++i) {
        cin >> jealous[i] >> liked[i];
        jealous_children[jealous[i]].push_back(i);
    }

    /*
     * Search for a mixed cycle made of:
     *   - an edge x -> jealous[x], or
     *   - an edge x -> liked[x].
     *
     * After taking x -> liked[x], the following jealous chain is not
     * allowed to reach jealous[x].  Also, an accepted cycle must contain
     * at least one jealous edge; a cycle of liked edges alone cannot be
     * turned into a valid ordering.
     *
     * path_pos contains only the current DFS path.  Trying jealous edges
     * first is essential for the ordering construction below.
     */
    vector<int> path;
    path.reserve(n);
    vector<int> path_pos(n + 1, -1);
    int loop_start = -1;

    auto search = [&](int start) {
        vector<Frame> stack;
        stack.reserve(n);
        stack.push_back({start, jealous[start], 0, 0});

        while (!stack.empty()) {
            Frame &frame = stack.back();
            int v = frame.vertex;

            if (frame.stage == 0) {
                if (path_pos[v] != -1) {
                    if (path_pos[v] < frame.last_jealous) {
                        loop_start = path_pos[v];
                        return true;
                    }
                    stack.pop_back();
                    continue;
                }

                path_pos[v] = static_cast<int>(path.size());
                path.push_back(v);
                frame.stage = 1;
            }

            if (frame.stage == 1) {
                frame.stage = 2;
                if (jealous[v] != frame.forbidden) {
                    stack.push_back(
                        {jealous[v], frame.forbidden,
                         static_cast<int>(path.size()), 0});
                    continue;
                }
            }

            if (frame.stage == 2) {
                frame.stage = 3;
                stack.push_back(
                    {liked[v], jealous[v], frame.last_jealous, 0});
                continue;
            }

            path.pop_back();
            path_pos[v] = -1;
            stack.pop_back();
        }
        return false;
    };

    bool found = false;
    int start = 1;
    do {
        if (search(start)) {
            found = true;
            break;
        }
        start = jealous[start];
    } while (start != 1);

    if (!found) {
        cout << "impossible\n";
        return 0;
    }

    /*
     * path may have a prefix entering its cycle through jealous edges.
     * Keep that prefix aside, then rotate the cycle if necessary so that
     * its closing edge is jealous.  Every maximal jealous chain can then
     * be output backwards.  Across a liked edge x -> liked[x], the search
     * invariant guarantees:
     *
     *              x < liked[x] < jealous[x].
     */
    int prefix_end = loop_start;
    while (prefix_end > 0 &&
           jealous[path[prefix_end - 1]] == path[prefix_end]) {
        --prefix_end;
    }

    if (jealous[path.back()] != path[loop_start]) {
        for (int i = static_cast<int>(path.size()) - 2;; --i) {
            if (jealous[path[i]] == path[i + 1]) {
                rotate(path.begin() + loop_start, path.begin() + i + 1,
                       path.end());
                break;
            }
        }
    }

    vector<char> used(n + 1, false);
    vector<int> answer;
    vector<int> queue;
    answer.reserve(n);
    queue.reserve(n);

    auto visit = [&](int v) {
        if (used[v]) return;
        used[v] = true;
        answer.push_back(v);
        for (int child : jealous_children[v]) queue.push_back(child);
    };

    for (int i = 0; i < static_cast<int>(path.size());) {
        while (i >= prefix_end && i < loop_start) ++i;
        if (i == static_cast<int>(path.size())) break;

        int j = i;
        while (j + 1 < static_cast<int>(path.size()) &&
               jealous[path[j]] == path[j + 1]) {
            ++j;
        }
        for (int k = j; k >= i; --k) visit(path[k]);
        i = j + 1;
    }

    for (int i = loop_start - 1; i >= prefix_end; --i) visit(path[i]);

    for (size_t i = 0; i < queue.size(); ++i) visit(queue[i]);

    for (int i = 0; i < n; ++i) {
        if (i) cout << ' ';
        cout << answer[i];
    }
    cout << '\n';
    return 0;
}
