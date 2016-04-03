# Unblocking Server using Select
import socket
import traceback
import select


def accepth_new_connection(readable_fd, lonely_car, lonely_boss,
                           unknow_Connection_list, Married_car,
                           Married_boss, socket_map):
    client_sock, client_addr = in_socket.accept()
    print 'We have accepted a connection from ', client_addr
    unknow_Connection_list.append(client_sock)
    socket_map[client_sock.fileno()] = client_sock


def Verify_unknow_connection(readable_fd, lonely_car, lonely_boss,
                             unknow_Connection_list, Married_car,
                             Married_boss, socket_map):
    sc = socket_map[fd]
    login_msg = sc.recv(1024)
    print 'The incoming message says', repr(login_msg)
    try:
        Type, account, password, mate = login_msg.split("\n")
    except ValueError, e:
        print "wrong login message,", e
    if "Car" == Type:
        print "*car: ", account, "arrived"
        if account in lonely_car:
            lonely_car[account].close()
            lonely_car[account] = sc
        if mate in lonely_boss and account not in Married_car and\
           mate not in Married_boss:
            Married_boss[mate] = (lonely_boss.pop(mate), sc)
            print "**couple ready, let's go"
        else:
            lonely_car[account] = sc
    elif "Boss" == Type:
        print "*Boss: ", account, "arrived"
        if account in lonely_boss:
            lonely_boss[account].close()
            lonely_boss[account] = sc
        if mate in lonely_car and account not in Married_boss and\
           mate not in Married_car:
            Married_boss[mate] = (sc, lonely_car.pop())
            Married_boss[mate][0].sendall("Go")
            print "**couple ready, let's go"
        else:
            lonely_boss[account] = sc
    else:
        print "bad connection"
        unknow_Connection_list.remove(sc)
        sc.close()


def check_lonelyBoss(readable_fd, lonely_car, lonely_boss,
                     unknow_Connection_list, Married_car,
                     Married_boss, socket_map):
    pass


def check_lonelyCar(readable_fd, lonely_car, lonely_boss,
                    unknow_Connection_list, Married_car,
                    Married_boss, socket_map):
    pass


def check_Married_Boss(readable_fd, lonely_car, lonely_boss,
                       unknow_Connection_list, Married_car,
                       Married_boss, socket_map):


def check_Married_Car():


if __name__ == "__main__":
    socket_map = {}
    listen_socket = []
    readable_fds, writeable_fds, exceptional_fds = [], [], []
    unknow_Connection_list = []
    lonely_boss = {}
    lonely_car = {}
    Married_boss = {}
    Married_car = {}
    # initialization and listening
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    HOST = '114.214.198.136'
    PORT = 1060
    s.bind((HOST, PORT))
    s.listen(1)
    listening_fileno = s.fileno()
    socket_map[listening_fileno] = s
    listen_socket = [s]
    print 'Listening at', s.getsockname()
    while True:
        # check the readable socket
        read_sock = listen_socket + unknow_Connection_list + \
            lonely_boss.values() + lonely_car.values() + Married_boss.keys()
        readable_fds, writeable_fds, exceptional_fds = select.select(
            read_sock,  [], read_sock, 1)
        for fd in readable_fds:
            if fd in
            client_sock, client_addr = in_socket.accept()
            print 'We have accepted a connection from ', client_addr
            unknow_Connection_list.append(client_sock)
            socket_map[client_sock.fileno()] = client_sock
        # # check unknow socket
        # readable_fds, writeable_fds, exceptional_fds = select.select(
        #     unknow_Connection_list, [], unknow_Connection_list, 1)
        # Verify_unknow_connection(
        #     readable_fds, lonely_car, lonely_boss, unknow_Connection_list)
        # for fd in exceptional_fds:
        #     e_sock = socket_map[fd]
        #     print "error socket", fd, "close it"
        #     e_sock.close()
        #     del socket_map[fd]

        # check lonely Boss socket
        # check lonely Car socket
        # check Married Boss socket
        # check Married Car socket
