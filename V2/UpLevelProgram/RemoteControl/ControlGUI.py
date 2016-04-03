import serial
import Tkinter
import time
import pickle
import pdb


class CarAdmin():

    def __init__(self, name):
        self.name = name
        self.State = 's'
        self.RcvBuffer = []
        self.LastSentOrder = 's'
        self.LastAckTime = 0
        print("serial port available:\n")
        print(self.serial_ports())
        sPortChoic = raw_input("Input the port to open\n")
        self.port = serial.Serial(sPortChoic, 9600)
        self.SentOrderRecord = 0
        self.last_direct_order = 0
        self.LeftPWM = 0
        self.RightPWM = 0
        self.LeftRotate = 0
        self.RightRotate = 0
        self.calibra_state = 0
        self.last_order_time = 0
        self.pwm_degree = 1
        self.MAX_PWM_DEGREE = 4
        self.pwm_preset = []
        self.debug_left_pwm_buf = 0
        self.debug_right_rotate_buf = 0
        with open('.\pwm.ini', 'r') as f:
            self.pwm_preset = pickle.load(f)
            f.close()
        self.pwm_degree = 1
        self.LeftPWM = self.pwm_preset[self.pwm_degree-1][0]
        self.RightPWM = self.pwm_preset[self.pwm_degree-1][1]
        self.debug_left_rotate_buf = 0
        self.debug_right_rotate_buf = 0
        self.debug_left_pwm_buf = 0
        self.debug_right_pwm_buf = 0

    def TurnLeft(self):
        self.State = 'l'
        self.SendOrder('l')

    def TurnRight(self):
        self.state = 'r'
        self.SendOrder('r')

    def Forward(self):
        self.state = 'f'
        self.SendOrder('f')

    def Backward(self):
        self.state = 'b'
        self.SendOrder('b')

    def Stop(self):
        self.state = 's'
        self.SendOrder('s')

    def SendOrder(self, order):
        if self.SentOrderRecord != order:
            self.SentOrderRecord = order
            print "SendOrder:%s\n" % order
        self.port.write([0x53])
        self.port.write([order])
        self.port.write([0x01])
        self.port.write([ord(order) + 1])
        self.LastSentOrder = order
        return order

    def Send_Direct_Order(self, frag_index, PWM_left, PWM_right, left_rotate,
                          right_rotate):
        check_sum = 0x54 + frag_index + PWM_left + PWM_right + \
            left_rotate + right_rotate
        check_sum = check_sum & 0xff
        self.port.write([0x54])
        self.port.write([frag_index])
        self.port.write([PWM_left])
        self.port.write([PWM_right])
        self.port.write([left_rotate])
        self.port.write([right_rotate])
        self.port.write([check_sum])
        if self.debug_left_rotate_buf != left_rotate or self.debug_right_rotate_buf != right_rotate\
                or self.debug_left_pwm_buf != PWM_left or self.debug_right_pwm_buf != PWM_right:
            self.debug_left_rotate_buf = left_rotate
            self.debug_right_rotate_buf = right_rotate
            self.debug_left_pwm_buf = PWM_left
            self.debug_right_pwm_buf = PWM_right
            print "New msg:", frag_index, PWM_left, PWM_right, left_rotate,right_rotate ,'\n'

        return 0

    def serial_ports(self):
        """Lists serial ports

        :raises EnvironmentError:
            On unsupported or unknown platforms
        :returns:
            A list of available serial ports
        """
        ports = ['COM' + str(i + 1) for i in range(256)]
        result = []
        for port in ports:
            try:
                s = serial.Serial(port)
                s.close()
                result.append(port)
            except (OSError, serial.SerialException):
                pass
        return result

    def minus1_left_pwm(self):
        self.LeftPWM -= 1
        self.Send_Direct_Order(1, self.LeftPWM, self.RightPWM,
                               self.LeftRotate, self.RightRotate,)
        # self.left_pwm_label_var.set(str(self.LeftPWM))

    def minus1_right_pwm(self):
        self.RightPWM -= 1
        self.Send_Direct_Order(1, self.LeftPWM, self.RightPWM,
                               self.LeftRotate, self.RightRotate,)
        # self.right_pwm_label_var.set(str(self.RightPWM))

    def plus1_left_pwm(self):
        self.LeftPWM += 1
        self.Send_Direct_Order(1, self.LeftPWM, self.RightPWM,
                               self.LeftRotate, self.RightRotate,)
        # self.left_pwm_label_var.set(str(self.LeftPWM))

    def plus1_right_pwm(self):
        self.RightPWM += 1
        self.Send_Direct_Order(1, self.LeftPWM, self.RightPWM,
                               self.LeftRotate, self.RightRotate,)
        # self.right_pwm_label_var.set(str(self.RightPWM))

    def plus5_left_pwm(self):
        self.LeftPWM += 5
        self.Send_Direct_Order(1, self.LeftPWM, self.RightPWM,
                               self.LeftRotate, self.RightRotate,)
        self.left_pwm_label_var.set(str(self.LeftPWM))

    def plus5_right_pwm(self):
        self.RightPWM += 5
        self.Send_Direct_Order(1, self.LeftPWM, self.RightPWM,
                               self.LeftRotate, self.RightRotate,)
        self.right_pwm_label_var.set(str(self.RightPWM))

    def pause(self):
        self.LeftRotate = 0
        self.RightRotate = 0
        self.Send_Direct_Order(1, self.LeftPWM, self.RightPWM,
                               self.LeftRotate, self.RightRotate,)

    def Continue_calibrate(self):
        self.LeftRotate = 1
        self.RightRotate = 1
        self.Send_Direct_Order(1, self.LeftPWM, self.RightPWM,
                               self.LeftRotate, self.RightRotate,)

    def save_pwm(self):
        with open('.\pwm.ini', 'w') as f:
            pickle.dump(self.pwm_preset, f)
            f.close()

    def next_pwm_degree(self):
        self.pwm_preset[self.pwm_degree-1][0] = self.LeftPWM
        self.pwm_preset[self.pwm_degree-1][1] = self.RightPWM
        if self.pwm_degree > self.MAX_PWM_DEGREE:
            print "All degree has been calibrated"
            self.calibra_state = 0
        else:
            self.pwm_degree += 1
            print "Current degree is :", str(self.pwm_degree)
            self.LeftPWM = self.pwm_preset[self.pwm_degree-1][0]
            self.RightPWM = self.pwm_preset[self.pwm_degree-1][1]
            self.Send_Direct_Order(1, self.LeftPWM, self.RightPWM,
                                   self.LeftRotate, self.RightRotate,)
            self.left_pwm_label_var.set(self.LeftPWM)
            self.right_pwm_label_var.set(self.RightPWM)
        return 0

    def start_calibra(self):
        self.pwm_degree = 1
        self.LeftRotate = 1
        self.RightRotate = 1
        print "***Start Calibration***\n", "Current degree is :", str(self.pwm_degree)
        self.LeftPWM = self.pwm_preset[self.pwm_degree-1][0]
        self.RightPWM = self.pwm_preset[self.pwm_degree-1][1]
        self.Send_Direct_Order(1, self.LeftPWM, self.RightPWM,
                               self.LeftRotate, self.RightRotate,)
        self.left_pwm_label_var.set(self.LeftPWM)
        self.right_pwm_label_var.set(self.RightPWM)
        return 0

    def Run(self):
        self.calibra_panel = Tkinter.Tk()
        self.botton_left_minus1 = Tkinter.Button(
            self.calibra_panel, text="left minus 1",
            command=self.minus1_left_pwm)
        self.botton_left_minus1.pack()
        self.botton_right_minus1 = Tkinter.Button(
            self.calibra_panel, text="right minus 1",
            command=self.minus1_right_pwm)
        self.botton_right_minus1.pack()
        self.botton_left_plus1 = Tkinter.Button(
            self.calibra_panel, text="Left plus 1",
            command=self.plus1_left_pwm)
        self.botton_left_plus1.pack()
        self.botton_right_plus1 = Tkinter.Button(
            self.calibra_panel, text="right plus 1",
            command=self.plus1_right_pwm)
        self.botton_right_plus1.pack()
        self.botton_left_plus5 = Tkinter.Button(
            self.calibra_panel, text="Left plus 5",
            command=self.plus5_left_pwm)
        self.botton_left_plus5.pack()
        self.botton_right_plus5 = Tkinter.Button(
            self.calibra_panel, text="right plus 5",
            command=self.plus5_right_pwm)
        self.botton_right_plus5.pack()
        self.botton_pause = Tkinter.Button(
            self.calibra_panel, text="pause", command=self.pause)
        self.botton_pause.pack()
        self.botton_continue = Tkinter.Button(
            self.calibra_panel, text="continue", command=self.Continue_calibrate)
        self.botton_continue.pack()
        self.botton_save_pwm = Tkinter.Button(
            self.calibra_panel, text="Save pwm", command=self.save_pwm)
        self.botton_save_pwm.pack()
        self.botton_next_degree = Tkinter.Button(
            self.calibra_panel, text="next pwm degree", command=self.next_pwm_degree)
        self.botton_next_degree.pack()
        self.botton_start_calibra = Tkinter.Button(
            self.calibra_panel, text="Start calibra",
            command=self.start_calibra)
        self.botton_start_calibra.pack()
        self.left_pwm_label_var = Tkinter.StringVar()
        self.label_left_pwm = Tkinter.Label(self.calibra_panel, text="0")
        self.label_left_pwm['textvariable'] = self.left_pwm_label_var
        self.label_left_pwm.pack()
        self.right_pwm_label_var = Tkinter.StringVar()
        self.label_right_pwm = Tkinter.Label(self.calibra_panel, text="0")
        self.label_right_pwm['textvariable'] = self.right_pwm_label_var
        self.label_right_pwm.pack()
        while True:
            self.calibra_panel.update()
            # self.Send_Direct_Order(1, 128, 128, 1, 1)
            if time.time() > self.last_order_time + 0.5:
                self.last_order_time = time.time()
                self.Send_Direct_Order(1, self.LeftPWM, self.RightPWM,
                                       self.LeftRotate, self.RightRotate,)


if __name__ == '__main__':

    Admin = CarAdmin('Car')
    Admin.Run()
