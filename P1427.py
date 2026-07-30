import sys
import math
import heapq
import bisect

from collections import deque, defaultdict, Counter
from itertools import accumulate

input = sys.stdin.readline

a=[]

def solve():
    s = input().split()
    s1 = s[::-1]
    for i in s1[1:]:
        print(i,end =' ')
    
def main():
    solve()

if __name__ == "__main__":
    main()
