# SocketControl_MPU6050_Calibrate.py
'''
use MPU6050 to Calibrate the car when going straight.
'''
import time
import socket
import serial
from ControlGUI import CarAdmin
from threading import Thread
import Queue

queue_sock_center = Queue.Queue()
valid_move_order_global = ['l', 'r', 's']
all_valid_order_global = ['l', 'r', 's', 'f', 'b']
def recv_all(sock, length):
    data = ""
    while len(data) < length:
        more = sock.recv(length - len(data))
        if not more:
            raise EOFError('recv_all')
        data += more
    return data


class CarSocketAdmin(CarAdmin):

    def __init__(self, name, ServerIP, ServerPort, ID):
        CarAdmin.__init__(self, name)
        self.ServerIP = ServerIP
        self.ServerPort = ServerPort
        self.ID = ID
        self.RightAckFlag = 0
        Port_name_MPU6050 = raw_input("Choose the MPU6050 port\n")
        self.port_mpu6050 = serial.Serial(Port_name_MPU6050, 115200)
        self.angle = 0
        self.PWM_STEP_INC = 32
        # self.mpu6050_start_angle = 0
        # self.turning_angle = 0
        # self.Rcv_Buffer = 0
        # self.RcvByte = 0
        # self.Order_Sock_MPU6050 = 0
        # self.angle_over_360 = 0
        # self.stop_angle_over_360 = 0
        self.stop_angle_range = [[0, 0], [0, 0]]

    def TouchTheCar(self):
# if time.time() - self.LastAckTime > 1:
# print "Lost Connect\n"
        if time.time() - self.LastAckTime > 0.5:
            self.Send_Direct_Order(1, self.LeftPWM, self.RightPWM,
                                   self.LeftRotate, self.RightRotate)

    def SocketClient(self, q_sock_center):
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        s.bind((SERVERIP, SERVERPORT))
        s.listen(1)
        while True:
            print 'Listening at', s.getsockname()
            sc, sockname = s.accept()
            print 'We have accepted a connection from ', sockname
            print 'Socket connects', sc.getsockname(), 'and', sc.getpeername()
            while True:
                try:
                    message = sc.recv(1024)
                    sc.sendall("ok\n")
                    print "recv", repr(message), '\n'
                    command_tokens = message.split('\n')
                    if command_tokens[0] in ['g', 'f', 's', 'b']:
                        queue_sock_center.put(command_tokens[0])
                    elif command_tokens[0] in ['l', 'r'] and len(command_tokens) >= 2:
                        queue_sock_center.put(command_tokens[0])
                        queue_sock_center.put(int(command_tokens[1]))
                except Exception, e:
                    print "*** connection failed.", e, "\n Delete the couple ***"
                    sc.shutdown(socket.SHUT_RDWR)
                    sc.close()
                    break

    def ReadMPU6050(self):
        while True:
            # Read angle from MPU6050
            RcvByte = self.port_mpu6050.read()
            if ord(RcvByte) == 0x55:
                Rcv_Buffer = self.port_mpu6050.read(9)
                if ord(Rcv_Buffer[0]) == 0x53:
                    angle_tmp = (
                        ord(Rcv_Buffer[6]) << 8 | ord(Rcv_Buffer[5])) / 32768.0 * 180
                    if abs(angle_tmp - self.angle) > 0.1:
                        self.angle = angle_tmp

    def calculate_stop_range(self, turning_side, turning_angle, start_angle):
        if turning_side == 'l':
            stop_angle_point = start_angle + turning_angle
            if stop_angle_point > 360:
                stop_angle_point -= 360
            stop_range1_end = stop_angle_point + 20
            if stop_range1_end > 360:
                # there should be 2 stopping range. When the car's position angle
                # is in one of the two range(mostly a little lefter than stop point),
                # the car should stop
                stop_range2_end = stop_range1_end - 360
                self.stop_angle_range[0][0] = stop_angle_point
                self.stop_angle_range[0][1] = 360
                self.stop_angle_range[1][0] = 0
                self.stop_angle_range[1][1] = stop_range2_end
            else:
                # there is only one range.
                self.stop_angle_range[0][0] = stop_angle_point
                self.stop_angle_range[0][1] = stop_range1_end
                self.stop_angle_range[1] = [0, 0]
        elif turning_side == 'r':
            stop_angle_point = start_angle - turning_angle
            if stop_angle_point < 0:
                stop_angle_point += 360
            stop_range1_head = stop_angle_point - 20
            if stop_range1_head < 0:
                # there should be 2 stopping range. When the car's position angle
                # is in one of the two range(mostly a little righter than stop point),
                # the car should stop
                stop_range2_head = stop_range1_head + 360
                self.stop_angle_range[0][0] = 0
                self.stop_angle_range[0][1] = stop_angle_point
                self.stop_angle_range[1][0] = stop_range2_head
                self.stop_angle_range[1][1] = 360
            else:
                # there is only one range.
                self.stop_angle_range[0][0] = stop_range1_head
                self.stop_angle_range[0][1] = stop_angle_point
                self.stop_angle_range[1] = [0, 0]
        else:
            raise

    def if_order_changed(self, q):
        '''
        input: 1. old_order the order we get now
               2. q: queue from socket client thread to center thread
        output: 1. True: order changed, False: not changed
        '''
        if not q.empty():
            return True
        return False

    def check_if_stop(self):
        if self.stop_angle_range[0][0] < self.angle < self.stop_angle_range[0][1] or \
                self.stop_angle_range[1][0] < self.angle < self.stop_angle_range[1][1]:
            return True
        else:
            return False

    def turning(self, turn_side, turn_angle, queue_sock_center):
        self.calculate_stop_range(turn_side, turn_angle, self.angle)
        if turn_side == 'l':
            self.LeftRotate = 1
            self.RightRotate = 0
            self.Send_Direct_Order(1, self.LeftPWM, self.RightPWM,
                                   self.LeftRotate, self.RightRotate,)
        elif turn_side == 'r':
            self.LeftRotate = 0
            self.RightRotate = 1
            self.Send_Direct_Order(1, self.LeftPWM, self.RightPWM,
                                   self.LeftRotate, self.RightRotate,)
        print "car start turning\n"
        while True:
            # Continue turning?
            if self.check_if_stop():
                self.LeftRotate = 0
                self.RightRotate = 0
                self.Send_Direct_Order(1, self.LeftPWM, self.RightPWM,
                                       self.LeftRotate, self.RightRotate,)
                print "turning completed\n"
                return 1
            if self.if_order_changed(queue_sock_center):
                self.LeftRotate = 0
                self.RightRotate = 0
                self.Send_Direct_Order(1, self.LeftPWM, self.RightPWM,
                                       self.LeftRotate, self.RightRotate,)
                print "car stop turn by order changed\n"
                return 0

    def go_strait(self, direction, queue_sock_center):
        adjust_time = time.time()
        Adjust_GAP = 0.5
        self.LeftPWM = self.pwm_preset[self.pwm_degree-1][0]
        self.RightPWM = self.pwm_preset[self.pwm_degree-1][1]
        PWM_limit = {'l':{'down':(self.LeftPWM - self.PWM_STEP_INC/2), 'up':(self.LeftPWM + self.PWM_STEP_INC/2)},
                     'r':{'down':(self.RightPWM - self.PWM_STEP_INC/2), 'up':(self.RightPWM + self.PWM_STEP_INC/2)}}

        # set the direction
        if direction == 'b':
            self.LeftRotate = 2
            self.RightRotate = 2
        elif direction == 'f':
            self.LeftRotate = 1
            self.RightRotate = 1
        # Record the original angle
        angle_start = self.angle
        # calculate the invalid range
        left_invalid_rg, right_invalid_rg = self.calculate_invalid_rg(angle_start)
        # Send pwm order
        self.Send_Direct_Order(1, self.LeftPWM, self.RightPWM,
                               self.LeftRotate, self.RightRotate,)
        while True:
            self.TouchTheCar()
            # check the angle
            bias = self.check_angle_rg(self.angle, left_invalid_rg, right_invalid_rg)
            if bias == 'l' and time.time() - adjust_time > Adjust_GAP:
                if self.LeftPWM >= PWM_limit['l']['up']:
                    if self.RightPWM > PWM_limit['r']['down']:
                        self.minus1_right_pwm()
                    else:
                        print "control limit\n"
                else:
                    self.plus1_left_pwm()
                adjust_time = time.time()
            elif bias == 'r' and time.time() - adjust_time > Adjust_GAP:
                if self.RightPWM >= PWM_limit['r']['up']:
                    if self.LeftPWM >= PWM_limit['l']['down']:
                        self.minus1_left_pwm()
                    else:
                        print "control limit\n"
                else:
                    self.plus1_right_pwm()
                adjust_time = time.time()
            # New order Come?
            if not queue_sock_center.empty():
                return 1

    def check_angle_rg(self, angle_now, left_invalid_rg, right_invalid_rg):
        '''
        input: angle_now, left_invalid_rg, right_invalid_rg
        output: 'l': too left
                'r': too right
                0: don't need adjustment
        '''
        if left_invalid_rg[0]['little'] < angle_now <left_invalid_rg[0]['big']\
                or left_invalid_rg[1]['little'] < angle_now < left_invalid_rg[1]['big']:
            # print "too left"
            return 'l'
        elif right_invalid_rg[0]['little'] < angle_now < right_invalid_rg[0]['big']\
                or right_invalid_rg[1]['little'] < angle_now < right_invalid_rg[1]['big']:
            # print "too right"
            return 'r'
        else:
            return 0

    def calculate_invalid_rg(self, angle_start):
        '''
        input:void
        output:left_invalid_rg:
                [{'little':rg1_little_limit, 'big':rg1_big_limit},
                {'little':rg2_little_limit, 'big':rg2_big_limit}]
               right_invalid_rg:...
        ----------------------------------------------
        if there is only one invalid range in one side,
        rg2_little =0, rg2_big =0
        '''
        THRESHOLD = 5
        left_invalid_rg = [{'little': 0, 'big': 0}, {'little': 0, 'big': 0}]
        right_invalid_rg = [{'little': 0, 'big': 0}, {'little': 0, 'big': 0}]
        if angle_start < 180:  # 0<-->360 step point is in right side
            right_invalid_rg[1]['little'] = angle_start + 180
            if angle_start - THRESHOLD < 0:  # 4
                right_invalid_rg[1]['big'] = angle_start - THRESHOLD + 360
            else:  # 3. two range
                right_invalid_rg[1]['big'] = 360
                right_invalid_rg[0]['big'] = angle_start - THRESHOLD
                right_invalid_rg[0]['little'] = 0
            left_invalid_rg[1]['little'] = angle_start + THRESHOLD
            left_invalid_rg[1]['big'] = angle_start + 180
        else:  # 0<-->360 step point is in left side
            right_invalid_rg[1]['little'] = angle_start - 180
            right_invalid_rg[1]['big'] = angle_start - THRESHOLD
            left_invalid_rg[1]['big'] = angle_start - 180
            if angle_start + THRESHOLD > 360:  # step point in [x, x+5]
                left_invalid_rg[1]['little'] = angle_start + THRESHOLD - 360
            else:
                left_invalid_rg[1]['little'] = 0
                left_invalid_rg[0]['big'] = 0
                left_invalid_rg[0]['little'] = angle_start + THRESHOLD
        return left_invalid_rg, right_invalid_rg

    def Run(self):
        order = ''
        re = None
        ThreadSocket = Thread(
            target=self.SocketClient, args=(queue_sock_center,))
        # ThreadSerialRead = Thread(target=self.ReadTheSerial, args=())
        # ThreadSerialRead.start()
        ThreadMPU6050 = Thread(target=self.ReadMPU6050, args=())
        ThreadSocket.start()
        ThreadMPU6050.start()
        while True:
            self.TouchTheCar()
            # check the order
            if not queue_sock_center.empty():
                order = queue_sock_center.get()
                if order == 's':
                    self.LeftRotate = 0
                    self.RightRotate = 0
                    self.Send_Direct_Order(1, self.LeftPWM, self.RightPWM,
                                           self.LeftRotate, self.RightRotate,)
                elif order == 'b':
                    re = self.go_strait('b', queue_sock_center)
                elif order == 'f':
                    re = self.go_strait('f', queue_sock_center)
                elif order == 'l' and not queue_sock_center.empty():
                    re = self.turning('l', queue_sock_center.get(), queue_sock_center)
                elif order == 'r' and not queue_sock_center.empty():
                    re = self.turning('r', queue_sock_center.get(), queue_sock_center)
            # if re in valid_move_order_global:
            #     if re == 's':
            #         self.LeftRotate = 0
            #         self.RightRotate = 0
            #         self.Send_Direct_Order(1, self.LeftPWM, self.RightPWM,
            #                                self.LeftRotate, self.RightRotate,)
            #     elif re == 'l' and not queue_sock_center.empty():
            #         self.turning('l', queue_sock_center.get(), queue_sock_center)
            #     elif re == 'r' and not queue_sock_center.empty():
            #         self.turning('r', queue_sock_center.get(), queue_sock_center)
            #     re = None

if __name__ == '__main__':
    SERVERIP = '127.0.0.1'
    SERVERPORT = 8888
    Admin = CarSocketAdmin('CarCar', SERVERIP, SERVERPORT, 1)
    Admin.Run()
