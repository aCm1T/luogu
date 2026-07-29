#include <algorithm>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
using namespace std;

const long long INF = (1LL << 62);

int B, S, nvars, bank0Mask;
vector<char> ttype;
vector<long long> tval;
vector<int> tend;
long long trans[13][13], totalRefs;
vector<long long> groupScore;
vector<vector<long long> > memo;

void analyze(int a, int b, long long mul, int& firstv, int& lastv) {
  if (a == b) return;
  firstv = lastv = -1;
  for (int i = a; i < b; i++) {
    if (ttype[i] == 'V') {
      totalRefs += mul;
      if (!(bank0Mask & (1 << tval[i]))) {
        if (firstv == -1) firstv = tval[i];
        if (lastv != -1) trans[lastv][tval[i]] += mul;
        lastv = tval[i];
      }
    } else if (ttype[i] == 'R') {
      if (tval[i] == 0) {
        i = tend[i];
        continue;
      }
      int fv, lv;
      analyze(i+1, tend[i], mul*tval[i], fv, lv);
      if (fv != -1) {
        trans[lv][fv] += mul*(tval[i]-1);
        if (firstv == -1) firstv = fv;
        if (lastv != -1) trans[lastv][fv] += mul;
        lastv = lv;
      }
      i = tend[i];
    }
  }
}

int bitcount(int mask) {
  return __builtin_popcount((unsigned)mask);
}

long long partitionSavings(int mask, int banksLeft) {
  if (mask == 0) return 0;
  if (banksLeft == 0 || bitcount(mask) > banksLeft * S) return -INF;
  if (bitcount(mask) <= S) return groupScore[mask];

  long long& ret = memo[mask][banksLeft];
  if (ret != -1) return ret;
  ret = -INF;

  int firstBit = mask & -mask;
  int rest = mask ^ firstBit;
  int varsLeft = bitcount(mask);
  int minGroup = max(1, varsLeft - (banksLeft - 1) * S);
  int maxGroup = min(S, varsLeft);

  for (int add = rest;; add = (add - 1) & rest) {
    int groupSize = bitcount(add) + 1;
    if (minGroup <= groupSize && groupSize <= maxGroup) {
      int group = firstBit | add;
      long long sub = partitionSavings(mask ^ group, banksLeft - 1);
      if (sub > -INF/2)
        ret = max(ret, groupScore[group] + sub);
    }
    if (add == 0) break;
  }
  return ret;
}

long long solveForBank0(int mask) {
  bank0Mask = mask;
  fill(&trans[0][0], &trans[0][0] + 13 * 13, 0);
  totalRefs = 0;

  int firstv, lastv;
  analyze(0, ttype.size(), 1, firstv, lastv);

  long long base = totalRefs + (firstv != -1);
  int remaining = ((1 << nvars) - 1) ^ bank0Mask;
  vector<vector<long long> > save(nvars, vector<long long>(nvars, 0));
  for (int i = 0; i < nvars; i++) {
    for (int j = 0; j < nvars; j++) {
      if (i == j) continue;
      base += trans[i][j];
      save[i][j] = trans[i][j] + trans[j][i];
    }
  }

  int totalMasks = 1 << nvars;
  groupScore.assign(totalMasks, 0);
  for (int m = 1; m < totalMasks; m++) {
    int bit = m & -m;
    int v = __builtin_ctz((unsigned)bit);
    int prev = m ^ bit;
    groupScore[m] = groupScore[prev];
    for (int u = 0; u < nvars; u++)
      if (prev & (1 << u))
        groupScore[m] += save[v][u];
  }

  memo.assign(totalMasks, vector<long long>(B, -1));
  long long saved = partitionSavings(remaining, B - 1);
  if (saved <= -INF/2) return INF;
  return base - saved;
}

int main() {
  for(;;) {
    string line;
    if (!getline(cin, line)) break;
    istringstream line1(line);
    if (!(line1 >> B >> S)) break;
    if (!getline(cin, line)) break;
    istringstream line2(line);
    vector<int> rstack;
    int remap[14];
    fill(remap, remap + 14, -1);
    nvars = 0;
    ttype.clear(); tval.clear(); tend.clear();
    string tok;
    while (line2 >> tok) {
      char ch = tok[0];
      ttype.push_back(ch);
      tval.push_back(0);
      tend.push_back(0);
      if (ch == 'E') {
        tend[rstack.back()] = tend.size()-1;
        rstack.pop_back();
      } else {
        long long val = stoll(tok.substr(1));
        if (ch == 'R') {
          tval.back() = val;
          rstack.push_back(tend.size()-1);
        } else if (ch == 'V') {
          int original = val - 1;
          if (remap[original] == -1)
            remap[original] = nvars++;
          tval.back() = remap[original];
        }
      }
    }

    long long best = INF;
    int all = (1 << nvars) - 1;
    for (int mask = 0; mask <= all; mask++) {
      int inBank0 = bitcount(mask);
      int remaining = nvars - inBank0;
      if (inBank0 <= S && remaining <= (B - 1) * S)
        best = min(best, solveForBank0(mask));
    }

    cout << best << endl;
  }
}
