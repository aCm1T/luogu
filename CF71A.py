def solve():
    s=str(input())
    if len(s)<=10:
        print(s)
    else:
        print(s[0],end='')
        print(len(s)-2,end='')
        print(s[-1])

def main():
    t = int(input())
    for _ in range(t):
        solve()

if __name__ == "__main__":
    main()
