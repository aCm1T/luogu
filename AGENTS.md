# luogu

## Cursor Cloud specific instructions

This repository is a single-file C++ competitive-programming solution (`solution.cpp`) for
Luogu problem P17025. There are no services, package managers, databases, tests, or lint
configuration — it is a standalone CLI that reads `stdin` and writes `stdout`.

### Build & run

The compiler (`g++` 13, C++20-capable) is preinstalled; nothing needs installing.

```bash
g++ -std=c++20 -O2 -o solution solution.cpp   # C++20 required (uses std::ranges, <bits/stdc++.h>)
./solution < input.txt                          # or type input then Ctrl-D
```

### Input / output format

- Line 1: `t` (number of test cases).
- Per test case: a line `n k q`, then `n-1` parent indices (parents of nodes `2..n`),
  then `q` query values (each toggles a node's "active" state).
- Output: the initial answer, then one line after each toggle (chromatic values mod 1e9+7).

Example (`printf '1\n4 3 1\n1 1 1\n2\n' | ./solution` → `6` then `6`).
