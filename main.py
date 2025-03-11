import re
import serial
import schedule
import time

def reading_fingerprint_data():
    arduino = serial.Serial('com8',9600)
    print("Connection to arduino established")
    start_capturing_data = False
    hexFingerprintBuffer = ""
    while True:
        arduino_data = arduino.readline()

        decoded_values = str(arduino_data[0:len(arduino_data)].decode("utf-8"))
        print(decoded_values)
        list_values = decoded_values.split('x')

        if "Done" in decoded_values:
            print("Debug: Done")
            start_capturing_data = False
            arduino.close()
            break

        if start_capturing_data:
            hexFingerprintBuffer += decoded_values

        if "Fingerprint Template in HEX:" in decoded_values:
            print("Debug: Start")
            start_capturing_data = True

        time.sleep(0.3)
    
    return hexFingerprintBuffer
    



def clean_hex_string(hex_string):
    """Remove spaces and newlines from hex data."""
    return re.sub(r'[^0-9A-Fa-f]', '', hex_string)

def hex_to_binary(hex_string):
    """Convert hex string to binary string."""
    hex_string = clean_hex_string(hex_string)
    return bin(int(hex_string, 16))[2:].zfill(len(hex_string) * 4)

def hamming_distance(bin1, bin2):
    """Compute Hamming distance between two binary strings."""
    return sum(b1 != b2 for b1, b2 in zip(bin1, bin2))

def similarity_percentage(bin1, bin2):
    """Compute similarity percentage based on Hamming distance."""
    total_bits = max(len(bin1), len(bin2))
    distance = hamming_distance(bin1.ljust(total_bits, '0'), bin2.ljust(total_bits, '0'))
    return ((total_bits - distance) / total_bits) * 100




#reading data from arduino

print("HI we're comparing two fingerprint to calculate the similarity between them: ")

print("Fingerprint number 1: ")
hex1 = reading_fingerprint_data()

print("Fingerprint number 2: ")
hex2 = reading_fingerprint_data()


# Convert HEX to Binary
binary1 = hex_to_binary(hex1)
binary2 = hex_to_binary(hex2)

# Compute Similarity
similarity = similarity_percentage(binary1, binary2)

print(f"Fingerprint Similarity: {similarity:.2f}%")
