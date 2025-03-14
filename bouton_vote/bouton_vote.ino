// Define sensor pins and corresponding letters
const int sensorPins[4] = {A0, A1, A2, A3};
const char sensorLetters[4] = {'A', 'B', 'C', 'D'};

// ADC settings: Adjust ADC_MAX if your board uses a different range (e.g., 2047 for 11-bit ADC)
const int ADC_MAX = 1023;          // Typical 10-bit ADC maximum
const int threshold = ADC_MAX / 10;  // 10% threshold

// Debounce/release time (100 ms)
const unsigned long debounceTime = 100; // in milliseconds

// Array to keep track of when each sensor first goes above the threshold
unsigned long sensorDebounce[4] = {0, 0, 0, 0};

// Flag to indicate that a vote (or error) has been registered
bool voteRegistered = false;

// Timer for ensuring all sensors are released before a new vote can occur
unsigned long releaseTimer = 0;

void setup() {
  Serial.begin(9600);
}

void loop() {
  unsigned long currentTime = millis();

  // Vote session: if no vote has been registered, check for sensor presses
  if (!voteRegistered) {
    for (int i = 0; i < 4; i++) {
      int sensorValue = analogRead(sensorPins[i]);
      
      if (sensorValue > threshold) {
        // Start the debounce timer if not already started for this sensor
        if (sensorDebounce[i] == 0) {
          sensorDebounce[i] = currentTime;
        }
        // Check if the sensor has stayed above threshold for debounceTime
        if (currentTime - sensorDebounce[i] >= debounceTime) {
          Serial.print("Vote for ");
          Serial.println(sensorLetters[i]);
          voteRegistered = true;
          break;  // Exit the loop once a vote is registered
        }
      } else {
        // Sensor is below threshold; reset its debounce timer
        sensorDebounce[i] = 0;
      }
    }
  }
  // Vote has been registered; wait for all sensors to be released before the next vote
  else {
    bool allReleased = true;
    for (int i = 0; i < 4; i++) {
      int sensorValue = analogRead(sensorPins[i]);
      if (sensorValue > threshold) {
        allReleased = false;
        break;
      }
    }
    if (allReleased) {
      if (releaseTimer == 0) {
        releaseTimer = currentTime;
      } else if (currentTime - releaseTimer >= debounceTime) {
        voteRegistered = false;
        releaseTimer = 0;
      }
    } else {
      // If any sensor is still pressed, reset the release timer
      releaseTimer = 0;
    }
  }
}
