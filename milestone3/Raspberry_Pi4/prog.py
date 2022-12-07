import serial
ser1 = serial.Serial("/dev/ttyAMA1", 115200)
while (1):
    speed = raw_input("speed:")
    
    angle = raw_input("angle:")
    
    command = "speed: " + speed + " angle: " + angle
    ser1.write(bytes(command))
    
ser1.close()