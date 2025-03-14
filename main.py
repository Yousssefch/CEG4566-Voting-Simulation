import re
import serial
import time
import csv

DEBUG_FINGERPRINT_HEX = "AABBCCDDEEFF00112233445566778899AABBCCDDEEFF00112233445566778899"

def read_fingerprint_data():
    arduino = serial.Serial('com8', 9600)
    print("Connected to Arduino")
    start_capturing_fingerprint = False
    start_capturing_choice = False
    choice = ''
    hex_fingerprint = ""

    time.sleep(2)
    arduino.write(bytes("1",  'utf-8'))

    while True:
        arduino_data = arduino.readline().decode("utf-8").strip()
        print(arduino_data)
        if "Press" in arduino_data:
            start_capturing_choice = True

        if "Place your finger to enroll in Slot 1" in arduino_data:
            start_capturing_choice = False


        if "Done" in arduino_data:
            start_capturing_fingerprint = False
            arduino.close()
            break

        if start_capturing_fingerprint:
            hex_fingerprint += arduino_data
        
        if start_capturing_choice:
            choice = arduino_data

        if "Fingerprint Template in HEX:" in arduino_data:
            start_capturing_fingerprint = True
            
    
    return [hex_fingerprint, choice]

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


def check_fingerprint_match(hex_fingerprint):
    """Check scanned fingerprint against stored fingerprints in CSV."""
    scanned_binary = hex_to_binary(hex_fingerprint)
    
    with open('fingerprints.csv', newline='') as file:
        reader = csv.reader(file)
        for row in reader:
            if len(row) < 2:
                continue
            stored_id, stored_hex = row[0], row[1]
            stored_binary = hex_to_binary(stored_hex)
            similarity = similarity_percentage(scanned_binary, stored_binary)
            
            print(f"Comparing with ID {stored_id}: {similarity:.2f}% similarity")
            
            if similarity > 90:  # Threshold for match
                print(f"Match found! User ID: {stored_id}")
                return stored_id
    
    print("No match found.")
    return None

def voting_process():
    arduino = serial.Serial('com8', 9600)
    print("Connected to Arduino")

    time.sleep(2)
    arduino.write(bytes("2",  'utf-8'))

    while True:
        arduino_data = arduino.readline().decode("utf-8").strip()
        print(arduino_data)




def main():
    voting_process()
    

if __name__ == "__main__":
    main()
