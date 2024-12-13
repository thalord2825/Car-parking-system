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
  // Initialize LCD (address 0x27, 16 columns, 2 rows)
  LiquidCrystal_I2C lcd(0x27, 16, 2);
  Servo Servo1;
  SoftwareSerial Bluetooth(10,11);  

  uint8_t id;
  Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

  int servoPin = 9;
  int sensorPinIn = 4;
  int sensorPinOut = 5;
  int sensorPin[1] = {7}; // Initialize slots sensor pins
  int totalSlots = 1;
  int occupiedSlots = 0; 
  String slotRemain = "";


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


  uint8_t nextId = 1; // Initialize the ID counter


  void sendBluetooth(String status, int label){
    if(label == 1) {
      status = "1 " + status;
    }else
      status = "2 " + status;
    Bluetooth.println(status);
    Serial.println("Sent: " + status);
  }

  int receiveBluetooth() {
    Serial.print("Waiting for command: ");
    unsigned long startTime = millis(); // Record the start time
    while (!Bluetooth.available()) {
      if (millis() - startTime > 5000) { // 5-second timeout
        Serial.println("Timeout waiting for Bluetooth command.");
        return -1; // Indicate timeout
      }
    }
    
    String command = Bluetooth.readStringUntil('\n'); // Read until newline (or timeout internally)
    command.trim(); // Remove any extra whitespace
    Serial.print("Received command: ");
    Serial.println(command);

    // Validate and process the command
    if (command == "1") {
      return 1;
    } else if (command == "2") {
      return 2;
    } else {
      Serial.println("Invalid command received.");
      return 0; // Indicate an invalid command
    }
  }

  // function to enroll new user
  uint8_t enrollFingerprint() {
    lcd.clear();
    lcd.print("Processing...");
    int p = -1;

    // Automatically generate the next available ID
    id = nextId;

    Serial.println("Place your finger on the sensor...");
    sendBluetooth("Place your finger on the sensor...", 1);
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
    sendBluetooth("Remove your finger", 1);
    delay(2000);

    Serial.println("Place the same finger again...");
    sendBluetooth("Place the same finger again...", 1);

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
      nextId++; // Increment ID for the next enrollment
      return id;
    } else {
      Serial.println("Error storing fingerprint.");
  return -7;
    }
  }

  // Function to check if a fingerprint is stored in the database
  int checkFingerprintStored() {
    Serial.println("Place your finger on the sensor...");
    sendBluetooth("Place your finger on the sensor...", 1);
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


  // Check for authenticated user
  bool authentication() {
    lcd.clear();
    lcd.print("Processing..."); //  update lcd status
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

  // check remaining slots in parking lot
  void checkSlot() {
      // Create the custom characters once
      lcd.createChar(0, customCharF); // Occupied slot
      lcd.createChar(1, customCharE); // Free slot

      String tmp = "";
      int freeSlotList[1]; //Create list of slots is free
      occupiedSlots = 0;
      // Loop through all slots and update their status on the LCD
      for (int i = 0; i < totalSlots; i++) {
          int sensorValue = digitalRead(sensorPin[i]);
          lcd.setCursor(i, 1); // Set cursor to the corresponding column for each slot

          if (sensorValue == LOW) {
            lcd.write(0); // Display non-free character
            freeSlotList[i] = 0;
          } else {
              lcd.write(1); // Display free character
              freeSlotList[i] = i + 1;
          }
          Serial.print(freeSlotList[i]);
          tmp += freeSlotList[i];
      }
        Serial.println(); 

        // calculate occupied slots
        for(int i = 0; i < totalSlots; i++ ) {
          if(freeSlotList[i] == 0)
            occupiedSlots++;
        }
        int freeSlots = totalSlots - occupiedSlots; // Calculate free slots
        Serial.print("Free Slots: "); 
        Serial.println(freeSlots); // Show total free slots  
        Serial.print("No Free Slots: "); 
        Serial.println(occupiedSlots); // Show total free slots  

        // if there is any changes in parking lot
        if(tmp != slotRemain) {
          sendBluetooth("Free-slot:" + String(freeSlots), 2 );
          slotRemain = tmp;
        }
  }


  void setup() {
    // Initialize sensors for coming car and comeout car
      Servo1.attach(servoPin);
      pinMode(sensorPinIn, INPUT);
      pinMode(sensorPinOut, INPUT);

      Serial.begin(9600);

      // Initialize lcd 
      lcd.begin(16, 2);
      lcd.backlight();
      lcd.clear();
      lcd.print("Welcome");

      // Initialize slot pin
      for(int i = 0; i < totalSlots; i++) {
        pinMode(sensorPin[i], INPUT);
      }

      // Update initialized lcd
      checkSlot();

      Bluetooth.begin(9600);  
      sendBluetooth("Welcome", 1); // send welcome text via bluetooth
  }

  // Loop for every frame
  void loop() {
        int sensorValueIn = digitalRead(sensorPinIn);
        int sensorValueOut= digitalRead(sensorPinOut);


    if (sensorValueIn == LOW || sensorValueOut == LOW) {
      Serial.println("detect object, rotate 90");
      if(sensorValueIn == LOW) {
        // car goes in the parking lot
        if(occupiedSlots == totalSlots) {
          lcd.clear();
          lcd.print("None empty slot");
          return;
        }
        // if there is at least one empty slot in parking lot
        // do fingerprint authentication
        if(authentication()) {
          Servo1.write(90);
          lcd.clear();
          lcd.print("Come in");
          sendBluetooth("Welcome", 1);
        }else {
          Serial.println("Authentication fail");
          lcd.clear();
          lcd.print("Invalid");
          sendBluetooth("Invalid", 1);
          delay(500); // wait for bluetooth transfer
          int command = receiveBluetooth(); // receive choice from user
          delay(100);
            switch(command){
              case 1:
                uint8_t enrollResult = enrollFingerprint();
                if (enrollResult > 0) {
                  Serial.print("Enrollment successful with ID: ");
                  Serial.println(enrollResult);
                  sendBluetooth("Enrollment successful", 1);
                } else {
                  sendBluetooth("Enroll fail",1);
                  Serial.println("Enrollment failed. Try again.");
                }
              break;
              case 2: 
                Serial.print("Canceled");
                lcd.clear();
                lcd.print("Canceled");
                break;
              default:
                Serial.print("Error");
                lcd.clear();
                lcd.print("Error");
            }
        }
      }else if(sensorValueOut == LOW) {
        // car goes out the parking lot
        Servo1.write(90);
        lcd.clear();
        lcd.print("Goodbye");
        sendBluetooth("Goodbye", 1);
      }
      delay(2000); //  delay for coming car
    }else {
      Servo1.write(0);
      Serial.println("No object detected, rotate 0");
      lcd.clear();
      lcd.print("Welcome");
      delay(2000); // delay for come out car
    }

    // Check for remaining empty slots
    checkSlot();
  }