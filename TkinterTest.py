import Tkinter
import tkMessageBox

top = Tkinter.Tk()

def helloCallBack():
   tkMessageBox.showinfo( "Hello Python", "Hello World")

B = Tkinter.Button(top, text ="Hello,Python!", command = helloCallBack)

B.pack()
A = Tkinter.Button(top,text ="hi",command = helloCallBack)
A.pack()
top.mainloop()
