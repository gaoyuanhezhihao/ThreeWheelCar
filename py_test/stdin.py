import sys

num=0
c=0
while True:
    num+=1
    c=sys.stdin.read()
    if c:
        print "Input"       
    print num
    time.sleep(0.5)
