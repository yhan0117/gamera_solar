import msvcrt 
import serial
import numpy as np
import serial.tools.list_ports as ports
import os
 
def Port():
    portsList = []

    for onePort in ports.comports():
        portsList.append(str(onePort))
        print(str(onePort))
    val = input("Select Port: COM")

    for x in range(0, len(portsList)):
        if portsList[x].startswith("COM" + str(val)):
            portvar = "COM" + str(val)  # open serial port
            print("Port selected: " + portvar)         # check which port was really used

    return portvar

def log(info):
    '''
    - 0: program begin
    - 1: board ready
    - 2: calibration begin
    - 3: calibration min throttle
    - 4: motor decelerating
    - 5: arduino turned off
    - int: motor PWM
    - empty: no return, invalid character
    '''
    match info:
        case b'0\r\n':
            print("Program Starting ...")
        case b'2\r\n':
            print("Calibration begin\nPlug in battery then press any key after tone")
        case b'3\r\n':
            print("Sending min throttle\nPower cycle after tone then press any key")
        case b'4\r\n':
            while serialInst.readline() != b'5\r\n':
                print("Motor decelerating")
            print("MCU turned off, program terminating ...")
            exit()
        case b'':
            print(end="")
        case default:
            print((info.decode('ascii')).strip())            

    # for debugging, comment out
    # print(info)


    
if __name__ == "__main__":

    serialInst = serial.Serial() 
    serialInst.timeout = 1.0

    serialInst.port = Port()
    serialInst.baudrate = 115200
    serialInst.open()
    info = serialInst.read()
    while info != b'1':
        info = serialInst.read()
        print("Waiting for arduino to start, will try every second")

    print("Board initialized...")
    print("Press c and leave power unplugged to calibrate motor, otherwise press any key to skip")

    serialInst.timeout = 0.1

    while True:
        if msvcrt.kbhit():
            # read key
            key = msvcrt.getch()

            # clear screen (backspace)
            if key == b'\x08':
                os.system('cls')
            
            # anything else send to MCU
            else:
                # read second byte if arrow key 
                if key == b'\xe0':
                    key = msvcrt.getch()

                # used for debugging, comment out for normal operation
                # print("Key Pressed:", str(bytes.decode(key, encoding="utf-8", errors = "ignore")))

                # transmit data only if theres keyboard input
                key = key + b'\t'   # delimiter
                serialInst.write(key)
                # print(key)

                # data returned from Arduino
                serialInst.flush()
                info = serialInst.readline()  

                # clear screen and decode return
                log(info)
