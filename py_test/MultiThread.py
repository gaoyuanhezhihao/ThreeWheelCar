from threading import Thread
import time

# Define a function for the thread
def IO_thread():
   global GlobalVar
   while True:
##      a=raw_input("your command:time:%s\n"%time.time())
##      print "IO_thread:GlobalVar:%d\n"%GlobalVar
      GlobalVar += 1
      if GlobalVar >10000:
          return
def CPU_thread():
   global GlobalVar 
   while True:
##      print "CPU_thread:time:%s\n"%time.time()
      GlobalVar+=1
      if GlobalVar >10000:
          return
# Create two threads as follows
GlobalVar=0
try:
    Thread_IO = Thread(target=IO_thread, args=())
    Thread_CPU = Thread(target=CPU_thread, args=())
    StartTime = time.time()
##    Thread_IO.start()
    Thread_CPU.start()
##    Thread_IO.join()
    Thread_CPU.join()
except:
    print "Error: unable to start thread"
    raise
print "time expense:",time.time() - StartTime
