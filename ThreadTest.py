import thread
import ControlGUI
from  SuperClassTest import SuperClass
class ThreadTest(SuperClass):
    def t1(self):
        while True:
            self.num+=1
            print "t1:num:%d"%self.num
    def t2(self):
        while True:
            self.num+=1
            print "t2:num:%d"%self.num
    def run(self):
        self.num=0
        thread1=thread.start_new_thread(self.t1,())
        thread2=thread.start_new_thread(self.t2,())
        while True:
            self.num +=1
            print "main thread:num:%d"%self.num
            self.SuperPrint()
if __name__ =='__main__':
    t=ThreadTest()
    t.run()
