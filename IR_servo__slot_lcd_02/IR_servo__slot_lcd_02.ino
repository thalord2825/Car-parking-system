#include <Servo.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Khai báo đối tượng LCD (địa chỉ 0x27 là địa chỉ phổ biến của LCD I2C)
LiquidCrystal_I2C lcd(0x27, 16, 2);

Servo Servo1;

int servoPin = 9;
int sensorPin = 2;
int sensorPin1 = 3;
int outputPin = 13;
int angle = 0;
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
  pinMode(outputPin, OUTPUT);
  pinMode(sensorPin, INPUT);
  Serial.begin(9600);
  //LCD setup
   lcd.begin(16, 2);
  lcd.backlight();
  lcd.print("Kinh chao quy khach");
  
}

void loop() {
  // Read the sensor value
  int sensorValue = digitalRead(sensorPin);
  int sensor1Value = digitalRead(sensorPin1);
  // Log the sensor value to the Serial Monitor
  Serial.print("SensorPin Value: ");
  Serial.println(sensorValue);
  
  // Check if object is detected (LOW means object detected)
  if (sensorValue == LOW) {
    // Turn on the LED (green)
    digitalWrite(outputPin, HIGH);
    
    // Move the servo to 90 degrees
    Servo1.write(90);
    
    // Log the angle to the Serial Monitor (optional)
    Serial.println("Object detected! Servo turned to 90 degrees.");
  } else {
    // Turn off the LED (turn off green LED)
    digitalWrite(outputPin, LOW);
    
    // Reset the servo to the initial position
    Servo1.write(0);
    
    // Log the angle to the Serial Monitor (optional)
    Serial.println("No object detected. Servo reset to 5 degrees.");
  }
  delay(1000);
  

  if(sensor1Value == LOW) {
  lcd.createChar(0, customCharF);
  lcd.setCursor(0, 1);
  lcd.write(0);
  }else {
  lcd.createChar(0, customCharE);
  lcd.setCursor(0, 1);
  lcd.write(0);
  }
}