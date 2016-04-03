def call1(a,b):
    print a,b
def call2(a,b):
    print a
a=[1,2]
b=[3,4]
func = {1:call1, 2:call2}
func.get(2)(a,b)
