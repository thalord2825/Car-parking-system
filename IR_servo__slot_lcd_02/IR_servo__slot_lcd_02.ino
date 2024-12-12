#include <Servo.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>
#include <Adafruit_Fingerprint.h>
#if (defined(__AVR__) || defined(ESP8266)) && !defined(__AVR_ATmega2560__)
// For UNO and others without hardware serial
#include <SoftwareSerial.h>
SoftwareSerial mySerial(2, 3); // RX, TX
#else
// On boards with hardware serial (e.g., Mega), use hardware serial
#define mySerial Serial1
#endif
// Connect RX, TX of Bluetooth module
SoftwareSerial bluetooth(0,1);  

// Initialize LCD (address 0x27, 16 columns, 2 rows)
LiquidCrystal_I2C lcd(0x27, 16, 2);

Servo Servo1;
uint8_t id;
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

int servoPin = 9;
int sensorPinIn = 4;
int sensorPinOut = 5;
int sensorPin[6] = {7, 8, 10, 11, 12, 13}; // Initialize sensor pins
int totalSlots = 6;
int occupiedSlots = 0; // Initialize number of slots is taken

byte customCharF[] = {
    B11111,
    B11111,
    B11111,
    B11111,
    B11111,
    B11111,
    B11111,
    B11111
};

byte customCharE[] = {
    B11111,
    B10001,
    B10001,
    B10001,
    B10001,
    B10001,
    B10001,
    B11111
};
// Function to enroll a new fingerprint
uint8_t enrollFingerprint() {
  int p = -1;

  Serial.println("Ready to enroll a new fingerprint!");
  Serial.print("Enter the ID # (1 to 127) for the fingerprint: ");
  id = readnumber();
  if (id == 0) { // ID #0 not allowed
    Serial.println("Invalid ID. Try again.");
    return -1;
  }
  Serial.print("Enrolling fingerprint with ID: ");
  Serial.println(id);

  Serial.println("Place your finger on the sensor...");
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    if (p == FINGERPRINT_NOFINGER) continue;
    if (p != FINGERPRINT_OK) {
      Serial.println("Error taking image. Try again.");
      return -2;
    }
  }

  p = finger.image2Tz(1);
  if (p != FINGERPRINT_OK) {
    Serial.println("Error converting image. Try again.");
    return -3;
  }

  Serial.println("Remove your finger.");
  delay(2000);

  Serial.println("Place the same finger again...");
  p = 0;
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    if (p == FINGERPRINT_NOFINGER) continue;
    if (p != FINGERPRINT_OK) {
      Serial.println("Error taking image. Try again.");
      return -4;
    }
  }

  p = finger.image2Tz(2);
  if (p != FINGERPRINT_OK) {
    Serial.println("Error converting second image. Try again.");
    return -5;
  }

  p = finger.createModel();
  if (p != FINGERPRINT_OK) {
    Serial.println("Fingerprints did not match.");
    return -6;
  }

  p = finger.storeModel(id);
  if (p == FINGERPRINT_OK) {
    Serial.println("Fingerprint stored successfully!");
    return id;
  } else {
    Serial.println("Error storing fingerprint.");
    return -7;
  }
}

// Function to check if a fingerprint is stored in the database
int checkFingerprintStored() {
  Serial.println("Place your finger on the sensor...");

  uint8_t p = finger.getImage();
  if (p == FINGERPRINT_NOFINGER) {
    Serial.println("No finger detected.");
    return -1;
  } else if (p != FINGERPRINT_OK) {
    Serial.println("Error taking image.");
    return -2;
  }

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK) {
    Serial.println("Error converting image.");
    return -3;
  }

  p = finger.fingerSearch();
  if (p == FINGERPRINT_OK) {
    Serial.print("Fingerprint found! ID: ");
    Serial.println(finger.fingerID);
    return finger.fingerID;
  } else if (p == FINGERPRINT_NOTFOUND) {
    Serial.println("Fingerprint not found in the database.");
    return 0;
  } else {
    Serial.println("Error searching for fingerprint.");
    return -4;
  }
}

// Helper function to read a number from Serial
uint8_t readnumber() {
  uint8_t num = 0;
  while (num == 0) {
    while (!Serial.available());
    num = Serial.parseInt();
  }
  return num;
}


void checkSlot() {
    // Create the custom characters once
    lcd.createChar(0, customCharF); // Occupied slot
    lcd.createChar(1, customCharE); // Free slot

    int freeSlotList[6]; //Create list of slots is free

    // Loop through all slots and update their status on the LCD
    for (int i = 0; i < totalSlots; i++) {
        int sensorValue = digitalRead(sensorPin[i]);
        lcd.setCursor(i + 4 , 1); // Set cursor to the corresponding column for each slot

        if (sensorValue == LOW) {
          lcd.write(0); 
          if(occupiedSlots < 6)
            occupiedSlots++;
          freeSlotList[i] = 0;
        } else {
            lcd.write(1); // Display free character
            freeSlotList[i] = i + 1;
            if(occupiedSlots > 0 )
              occupiedSlots--;
        }
        Serial.print(freeSlotList[i]);
    }
      Serial.println(); 
      int freeSlots = totalSlots - occupiedSlots; // Calculate free slots
      Serial.print("Free Slots: "); 
      Serial.print(freeSlots); // Show total free slots   
}


void setup() {
    // Initialize the servo and the pins
    Servo1.attach(servoPin);
    pinMode(sensorPinIn, INPUT);
    pinMode(sensorPinOut, INPUT);

    // Initialize sensor pins as input
    for (int i = 0; i < totalSlots; i++) {
        pinMode(sensorPin[i], INPUT);
    }

    // LCD setup
    lcd.begin(16, 2);
    lcd.backlight();
    lcd.setCursor(4, 0);
    lcd.print("Welcome!");
    delay(2000); // Wait for 2 seconds

    //Serial for Bluetooth
    Serial.begin(9600);
    while (!Serial); // Wait for Serial Monitor to open
    Serial.println("Initializing fingerprint sensor...");
    
    finger.begin(57600);
    if (finger.verifyPassword()) {
      Serial.println("Fingerprint sensor initialized.");
    } else {
      Serial.println("Fingerprint sensor not found. Check connections.");
      while(1);
    }
}



bool authentication() {
  int result = checkFingerprintStored();
  if (result > 0) {
    Serial.print("Fingerprint recognized with ID: ");
    Serial.println(result);
    return true;
  } else if (result == 0) {
    Serial.println("Fingerprint not recognized.");
  } else {
    Serial.println("Error during fingerprint checking. Try again.");
  }
  return false;
}


void loop() {
    // Read sensor values for entry and exit
    int sensorValueIn = digitalRead(sensorPinIn);
    int sensorValueOut = digitalRead(sensorPinOut);

    // Check if object is detected at entry or exit
    if (sensorValueIn == LOW) {
      if (occupiedSlots != 6) {
        if (authentication()) {
          // Move the servo to 90 degrees (open the gate)
          Servo1.write(90);
          Serial.println("Object detected! Gate opened.");

          // Continuously check parking slots and update the LCD
          checkSlot();
        } else {
          uint8_t enrollResult = enrollFingerprint();
          if (enrollResult > 0) {
            Serial.print("Enrollment successful with ID: ");
            Serial.println(enrollResult);
          } else {
            Serial.println("Enrollment failed. Try again.");
          }
        }
      } else {
        Serial.println("None slot is empty");
      }
    } else { 
      // Reset the servo to the initial position (close the gate)
      Servo1.write(0);
      Serial.println("No object detected. Gate closed.");
    }

    if (sensorValueOut == LOW) { 
      // Move the servo to 90 degrees (open the gate)
      Servo1.write(90);
      Serial.println("Object detected! Gate opened.");
      checkSlot();
    } else {
      // Reset the servo to the initial position (close the gate)
      Servo1.write(0);
      Serial.println("No object detected. Gate closed.");
    }

    delay(3000); // Small delay to make the LCD update readable
}


