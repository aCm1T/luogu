def solve():
    s=input()
    stack = []
    for i in s:
        if i == '(' or i =='[' or i =='{':
            stack += i
        elif ( i ==')' or i ==']' or  i =='}' ) and len(stack)==0:
            print("NO")
            return            
        elif i ==')' and stack[-1]=='(':
            stack.pop()
        elif i ==']' and stack[-1]=='[':
            stack.pop()
        elif i =='}' and stack[-1]=='{':
            stack.pop()
        else :
            print("NO")
            return
    if len(stack) == 0:
        print("YES")
    elif len(stack) > 0:
        print("NO")

def main():
    t = int(input())
    for _ in range(t):
        solve()

if __name__ == "__main__":
    main()
