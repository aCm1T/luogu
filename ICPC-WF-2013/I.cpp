#include <algorithm>
#include <iostream>
#include <vector>
using namespace std;

struct Entry {
  int start, index, depth;
};

int boxR, boxC, pondR, pondC;
vector<vector<int> > pond;

long long score(long long crossArea, long long depth) {
  long long curVol = depth * crossArea;
  long long extra = (curVol - 1) / ((long long)pondR * pondC - crossArea);
  return (depth + extra) * crossArea;
}

long long solve() {
  long long best = 0;
  for (int left = 0; left < pondC; left++) {
    vector<int> depths(pondR);
    for (int row = 0; row < pondR; row++)
      depths[row] = pond[row][left];

    for (int right = left; right < left + boxC && right < pondC; right++) {
      long long width = right - left + 1;
      int maxDim = boxR;
      if (width <= boxR) maxDim = boxC;

      for (int row = 0; row < pondR; row++)
        depths[row] = min(depths[row], pond[row][right]);

      vector<Entry> st;
      for (int row = 0; row < pondR; row++) {
        if (st.empty() || depths[row] > st.back().depth) {
          st.push_back({row, row, depths[row]});
        } else {
          while (!st.empty() && depths[row] <= st.back().depth) {
            Entry done = st.back();
            st.pop_back();
            long long height = min(row - done.start, maxDim);
            best = max(best, score(width * height, done.depth));
          }
          if (st.empty())
            st.push_back({0, row, depths[row]});
          else if (depths[row] > st.back().depth)
            st.push_back({st.back().index + 1, row, depths[row]});
        }
      }

      while (!st.empty()) {
        Entry done = st.back();
        st.pop_back();
        long long height = min(pondR - done.start, maxDim);
        best = max(best, score(width * height, done.depth));
      }
    }
  }
  return best;
}

int main() {
  int A, B;
  while (cin >> A >> B >> pondR >> pondC) {
    boxR = min(A, B);
    boxC = max(A, B);
    pond.assign(pondR, vector<int>(pondC));
    for (int row = 0; row < pondR; row++)
    for (int col = 0; col < pondC; col++)
      cin >> pond[row][col];

    cout << solve() << endl;
  }
}
