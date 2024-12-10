#include <Servo.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Initialize LCD (address 0x27, 16 columns, 2 rows)
LiquidCrystal_I2C lcd(0x27, 16, 2);

Servo Servo1;

int servoPin = 9;
int sensorPinIn = 4;
int sensorPinOut = 5;
int sensorPin[6] = {6, 7, 8, 10, 11, 12}; // Initialize sensor pins
int outputPin = 13; // LED output

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

void setup() {
    // Initialize the servo and the pins
    Servo1.attach(servoPin);
    pinMode(sensorPinIn, INPUT);
    pinMode(sensorPinOut, INPUT);

    // Initialize sensor pins as input
    for (int i = 0; i < 6; i++) {
        pinMode(sensorPin[i], INPUT);
    }

    // LCD setup
    lcd.begin(16, 2);
    lcd.backlight();
    lcd.setCursor(4, 0);
    lcd.print("Welcome!");
    delay(2000); // Wait for 2 seconds
}

void checkSlot() {
    // Create the custom characters once
    lcd.createChar(0, customCharF); // Occupied slot
    lcd.createChar(1, customCharE); // Free slot

    // Loop through all slots and update their status on the LCD
    for (int i = 0; i < 6; i++) {
        int sensorValue = digitalRead(sensorPin[i]);
        lcd.setCursor(i + 4 , 1); // Set cursor to the corresponding column for each slot

        if (sensorValue == LOW) {
            lcd.write(0); // Display occupied character
        } else {
            lcd.write(1); // Display free character
        }
    }
}

void loop() {
    // Read sensor values for entry and exit
    int sensorValueIn = digitalRead(sensorPinIn);
    int sensorValueOut = digitalRead(sensorPinOut);

    // Check if object is detected at entry or exit
    if (sensorValueIn == LOW || sensorValueOut == LOW) {
        // Move the servo to 90 degrees (open the gate)
        Servo1.write(90);
        Serial.println("Object detected! Gate opened.");
    } else {
        // Reset the servo to the initial position (close the gate)
        Servo1.write(0);
        Serial.println("No object detected. Gate closed.");
    }

    // Continuously check parking slots and update the LCD
    checkSlot();

    delay(100); // Small delay to make the LCD update readable
}
