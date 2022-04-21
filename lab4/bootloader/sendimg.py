import argparse
import serial
import os

parser = argparse.ArgumentParser()
parser.add_argument('image', default='kernel8.img', type=str)
parser.add_argument('device',default='/dev/ttyUSB0', type=str)
args = parser.parse_args()

try:
    ser = serial.Serial(args.device,115200)
except:
    print("Serial init failed!")
    exit(1)

file_path = args.image
file_size = os.stat(file_path).st_size

ser.write(file_size.to_bytes(4, byteorder="big"))

print("Sending kernel...")

with open(file_path, 'rb', buffering = 0) as f:
    for i in range(file_size):
        ser.write(f.read(1))
        print(ser.readline())

print("done")