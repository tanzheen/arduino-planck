import serial
import time

# Set up serial communication
ser = serial.Serial('/dev/cu.usbmodem1101', 11520)  # Replace 'COM3' with your Arduino's COM port
time.sleep(2)  # Wait for the serial connection to initialize

# Open a file to write data
with open('arduino_dual OPAMP new laser.txt', 'w') as f:
    while True:
        if ser.in_waiting > 0:
            line = ser.readline().decode('utf-8').strip()  # Read a line from the serial port
            print(line)  # Optional: Print the data to the console
            f.write(line + '\n')  # Write the data to the file

ser.close()  # Close the serial port when done