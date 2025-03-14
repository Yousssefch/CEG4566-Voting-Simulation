#include <Adafruit_Fingerprint.h>
#include <SoftwareSerial.h>
#include <Keypad.h>

const byte ROWS = 4; 
const byte COLS = 4; 

char hexaKeys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

char choice = "A";

byte rowPins[ROWS] = {11, 10, 9, 8}; 
byte colPins[COLS] = {7, 6, 5, 4};
Keypad customKeypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);

// Define software serial pins (change if needed)
SoftwareSerial mySerial(2, 3); // (RX, TX)

Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

uint8_t enrollID = 1;  // Slot ID for fingerprint enrollment

void setup() {
    Serial.begin(9600);
    Serial.println("Fingerprint Enrollment - Slot 1");

    finger.begin(57600);

    if (finger.verifyPassword()) {
        Serial.println("Fingerprint sensor detected!");
    } else {
        Serial.println("Sensor not found. Check wiring!");
        // while (1); // Halt execution
    }
}

void loop() {

  //Get Data from Python: 1=> scan the fingerprint, 2=> Vote
  String x = "0";
  bool done = false;
 while(!done){

        Serial.println("Read value");
        x = Serial.readString(); // Wait for data to be fully received
        if(x == "1" || x == "2"){
            done = true;
        }
    delay(500); // Small delay for better serial read synchronization
  }


  if(x == "1"){
    //Scan Finger Print
    Serial.println("Press 1 to Enroll, 2 to Verify");

    done = false;
    while(!done){
      char customKey = customKeypad.getKey();
      delay(500);
      Serial.println(customKey);
      if (customKey == '1' || customKey == '2'){
        choice = customKey;
        done = true;
        break;
      }
    }

      Serial.println("\nPlace your finger to enroll in Slot 1...");
      
      if (captureFingerprint(1) != FINGERPRINT_OK) return;
      
      Serial.println("Remove finger...");
      delay(2000);

      Serial.println("Place the same finger again...");
      
      if (captureFingerprint(2) != FINGERPRINT_OK) return;

      Serial.println("Creating fingerprint template...");
      uint8_t result = finger.createModel();
      
      if (result == FINGERPRINT_OK) {
          Serial.println("Fingerprint template created!");
      } else {
          Serial.println("Failed to create template.");
          return;
      }

      Serial.println("Saving to slot 1...");
      result = finger.storeModel(enrollID);
      
      if (result == FINGERPRINT_OK) {
          Serial.println("Fingerprint enrolled successfully in Slot 1!");
          Serial.println("Retrieving fingerprint template in hex format...");
          getFingerprintTemplateHex(enrollID);  // Convert stored template to hex format
      } else {
          Serial.println("Failed to store fingerprint.");
      }

      delay(5000); // Wait before next enrollment
  }
  
  if(x=="2"){
    Serial.println("Voting Begun");
    delay(5000);
  }

    //Vote in the system
}

uint8_t captureFingerprint(uint8_t bufferID) {
    uint8_t result;
    
    while ((result = finger.getImage()) != FINGERPRINT_OK) {
        if (result == FINGERPRINT_NOFINGER) {
            Serial.print(".");
        } else {
            Serial.println("\nError capturing fingerprint.");
            return result;
        }
        delay(500);
    }

    Serial.println("\nFingerprint image captured.");
    
    result = finger.image2Tz(bufferID);
    if (result == FINGERPRINT_OK) {
        Serial.println("Fingerprint converted to template.");
    } else {
        Serial.println("Failed to convert fingerprint.");
        return result;
    }

    return FINGERPRINT_OK;
}

void getFingerprintTemplateHex(uint16_t id) {
    Serial.print("Loading fingerprint template from slot ");
    Serial.println(id);
    
    uint8_t result = finger.loadModel(id);
    if (result != FINGERPRINT_OK) {
        Serial.println("Failed to load fingerprint template.");
        return;
    }

    Serial.println("Transferring fingerprint template...");
    result = finger.getModel();
    if (result != FINGERPRINT_OK) {
        Serial.println("Failed to transfer fingerprint template.");
        return;
    }

    uint8_t buffer[534];  // 2 data packets
    memset(buffer, 0xFF, sizeof(buffer));

    uint32_t startTime = millis();
    int index = 0;
    
    while (index < 534 && (millis() - startTime) < 20000) {
        if (mySerial.available()) {
            buffer[index++] = mySerial.read();
        }
    }

    Serial.print(index);
    Serial.println(" bytes received. Converting to hex...");

    Serial.println("Fingerprint Template in HEX:");
    delay(400);
    
    for (int i = 0; i < 512; i++) {
        printHex(buffer[i]);
        if ((i + 1) % 16 == 0) Serial.println();
        else Serial.print(" ");
    }

    delay(400);

    Serial.println("\nDone.");
}

// Function to print data in hexadecimal format
void printHex(uint8_t num) {
    if (num < 0x10) Serial.print("0");  // Add leading zero for single-digit hex values
    Serial.print(num, HEX);
}
