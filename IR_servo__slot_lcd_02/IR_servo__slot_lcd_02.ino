#include <Servo.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>

// Initialize LCD (address 0x27, 16 columns, 2 rows)
LiquidCrystal_I2C lcd(0x27, 16, 2);
Servo Servo1;
SoftwareSerial Bluetooth(10,11);  


int servoPin = 9;
int sensorPinIn = 2;
int sensorPinOut = 3;
int sensorPin[5] = {4,5,6,7,8}; // Initialize slots sensor pins
int totalSlots = 5;
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


void sendBluetooth(String status, int label){
  if(label == 1) {
    status = "1 " + status;
  }else
    status = "2 " + status;
  Bluetooth.println(status);
  Serial.println("Sent: " + status);
}


// check remaining slots in parking lot
void checkSlot() {
    // Create the custom characters for the first time
    lcd.createChar(0, customCharF); // Occupied slot
    lcd.createChar(1, customCharE); // Free slot

    String tmp = "";
    int freeSlotList[5]; // Array to track which slots are free
    occupiedSlots = 0;

    // Iterate over all parking slots
    for (int i = 0; i < totalSlots; i++) {
        int sensorValue = digitalRead(sensorPin[i]); // Read the sensor for this slot

        // Set cursor position on LCD for this slot
        lcd.setCursor(i, 1); // Draw each slot icon in the second row

        if (sensorValue == LOW) {
            // Slot is occupied
            lcd.write(0); // Display occupied character
            freeSlotList[i] = 0; // Mark slot as occupied
        } else {
            // Slot is free
            lcd.write(1); // Display free character
            freeSlotList[i] = i + 1; // Mark slot as free
        }

        // Debugging output to Serial Monitor
        Serial.print(freeSlotList[i]);
        tmp += freeSlotList[i];
    }

    Serial.println(); 

    // Calculate the number of occupied and free slots
    for (int i = 0; i < totalSlots; i++) {
        if (freeSlotList[i] == 0) {
            occupiedSlots++;
        }
    }

    int freeSlots = totalSlots - occupiedSlots; // Calculate free slots
    Serial.print("Free Slots: ");
    Serial.println(freeSlots);
    Serial.print("Occupied Slots: ");
    Serial.println(occupiedSlots);

    // If there is any change in parking lot status, update Bluetooth
    if (tmp != slotRemain) {
        sendBluetooth("Free-slot:" + String(freeSlots), 2);
        slotRemain = tmp; // Update the slot status cache
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

    Bluetooth.begin(9600);  
    sendBluetooth("Welcome", 1); // send welcome text via bluetooth

    
    // Update initialized lcd
    checkSlot();
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
        lcd.print("Full slot");
        sendBluetooth("Full slot", 1);
        return;
      }
      // if there is at least one empty slot in parking lot
      // do fingerprint authentication
      Servo1.write(90);
      lcd.clear();
      lcd.print("Come in");
      sendBluetooth("Welcome", 1);
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
    delay(1000); // delay for come out car
  }

  // Check for remaining empty slots
  checkSlot();
}