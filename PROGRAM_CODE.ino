// working code
#include <Wire.h>
#include <Keypad.h>
#include <Password.h>
#include <Adafruit_Fingerprint.h>
#include <SoftwareSerial.h>
#include <LiquidCrystal_I2C.h>
#include <Servo.h>

Servo myservo;
// Function Prototypes
void registerFingerprint();
bool verifyFingerprint();
void sendFingerprintSuccess();
void resetPassword();
void doorlocked();
// void dooropen();
void processNumberKey(char key);
void checkFailedAttempts();
void activateAlarm();
bool keypass = false;
bool alarmDeactivated = true;

// Pin Definitions
#define buzzer 11
#define rxPin 10
#define txPin 12

// Initialize Components
SoftwareSerial mySerial(rxPin, txPin);
Adafruit_Fingerprint finger(&mySerial);
LiquidCrystal_I2C lcd(0x27, 16, 2); // Adjust the I2C address as needed

String newPasswordString; // Hold the new password
char newPassword[6]; // Character string of newPasswordString
byte a = 5;
bool value = false;
bool keypadPass = false;
Password password = Password("5972"); // Enter your password
byte maxPasswordLength = 6;
byte currentPasswordLength = 0;
const byte ROWS = 4; // Four rows
const byte COLS = 4; // Four columns
char keys[ROWS][COLS] = {
  {'D', 'C', 'B', 'A'},
  {'#', '9', '6', '3'},
  {'0', '8', '5', '2'},
  {'*', '7', '4', '1'},
};
byte rowPins[ROWS] = {2, 3, 4, 5};
byte colPins[COLS] = {6, 7, 8, 9};
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);
bool unlocked = false;
// Variables
bool fingerprintVerified = false;
int failedAttempts = 0;

// Define states as integer constants
const int STATE_WAITING_FOR_INPUT = 1;
const int STATE_FINGERPRINT_VERIFIED = 2;
const int STATE_UNLOCKED = 3;
const int STATE_FACE_RECOGNITION_PENDING = 4;

// Current system state
int currentState = 1;

void setup() {
  myservo.attach(13);
  myservo.write(180);
  Serial.begin(9600);     // Communication with Python
  delay(1000); 
  mySerial.begin(57600);  // Communication with the fingerprint sensor
  lcd.begin(16, 2);
  lcd.backlight();
  pinMode(buzzer, OUTPUT);

  lcd.print("Initializing...");
  delay(2000);
  lcd.clear();

  // Initialize Fingerprint Sensor
  finger.begin(57600);
  if (finger.verifyPassword()) {
    lcd.print("Sensor Ready");
  } else {
    lcd.print("Sensor Error");
    Serial.println("SENSOR_ERROR"); // Notify Python about sensor issue
    while (true); // Halt system
  }

  // Check for existing templates
  // int templateCount = finger.getTemplateCount();
  // if (templateCount == 0) {
  //   lcd.print("No templates.");
  //   lcd.setCursor(0, 1);
  //   lcd.print("Register now.");
  //   Serial.println("NO_FINGERPRINTS"); // Notify Python
  //   registerFingerprint(); // Register a fingerprint if none exist
  // }

  lcd.clear();
  lcd.print("Ready to scan.");
  Serial.println("READY_TO_SCAN"); // Notify Python that system is ready
}

void loop() {
    if (currentState == 1) {
      lcd.setCursor(0, 0);
      lcd.print("1 = Fingerprint");
      lcd.setCursor(0, 1);
      lcd.print("2 = Override");

      char key = keypad.getKey();
      if (key != NO_KEY) {
        if (key == '1') {
          lcd.clear();
          lcd.print("Place finger...");
          fingerprintVerified = verifyFingerprint();
          if (fingerprintVerified) {
            Serial.println("FINGERPRINT_OK");
            lcd.clear();
            lcd.print("Fingerprint OK");
            currentState = 2;
          } else {
            lcd.print("Try again");
            delay(2000);
            lcd.clear();
         }
        } else if (key == '2') {
          lcd.clear();
          useKeypad();
        }
      }
    }

    if (currentState == 2){
      lcd.setCursor(0, 0);
      lcd.print("Waiting for face");
      lcd.setCursor(0, 1);
      lcd.print("recognition...");
      
      if (Serial.available()) {
        String response = Serial.readStringUntil('\n');
        if (response == "FACE_RECOGNIZED") {
            // dooropen();
            myservo.write(90);
            digitalWrite(buzzer, HIGH);
            delay(300);
            digitalWrite(buzzer, LOW);
            delay(100);
            lcd.clear();
            lcd.print("Access Granted");
            delay(2000);
            lcd.clear();
            currentState = 3; // Proceed to unlocked state
        } else if (response == "FACE_NOT_RECOGNIZED") {
            lcd.clear();
            lcd.print("Face Not Found");
            delay(2000);
            lcd.clear();
            currentState = 1; // Reset to initial state
        }
      }
    }
    
    if (currentState == 3){
      lcd.setCursor(0, 0);
      lcd.print("1 = lock");
      char key3 = keypad.getKey();
      if (key3 != NO_KEY) {
        if (key3 == '1') {
          doorlocked();
          currentState = 1;
        }
      }
    }
  }

// void setState(int newState) {
//   if (currentState != newState) {
//     currentState = newState;
//     Serial.print("State changed to: ");
//     Serial.println(currentState); // Debugging log
//   }
// }

bool useKeypad() {
  lcd.clear();
  lcd.setCursor(1, 0);
  lcd.print("ENTER PASSWORD");

  while (true) { // Wait for key input
    char key2 = keypad.getKey(); // Get key press from the keypad
    if (key2 != NO_KEY) { // If a key is pressed
      if (key2 == 'D') { // Confirmation key
        if (value == true) {

          dooropen();
          // checkFailedAttempts();
          // activateAlarm();
          // doorlocked();

          value = false;

          return true;

        } else if (value == false) {

          dooropen();

          value = true;

          return true;

        }
      } else if (key2 == 'C') { // Clear the password if C is pressed
        resetPassword();
      } else { // Only process number keys
        processNumberKey(key2); // Add the number to the password
      }
    }
  }
}

void useKeypadAlarm() {
  digitalWrite(buzzer, HIGH);
  lcd.clear();
  lcd.print("ALARM! Too many");
  lcd.setCursor(0, 1);
  lcd.print("failed attempts");
  delay(1000);
  lcd.clear();
  resetPassword();
  // HEREE PROBLEM WHEN ENTERING PASS AFTER ALARM

  while (!alarmDeactivated) { // Wait for key input
    lcd.setCursor(1, 0);
    lcd.print("ENTER PASSWORD");
    char key4 = keypad.getKey(); // Get key press from the keypad
    if (key4 != NO_KEY) { // If a key is pressed
      if (key4 == 'D') { // Confirmation key
        alarmOff();
      } else if (key4 == 'C') { // Clear the password if C is pressed
        resetPassword();
      } else { // Only process number keys
        processNumberKey(key4); // Add the number to the password
      }
    }
  }
}

void alarmOff() {

  if (password.evaluate()) {

    digitalWrite(buzzer, LOW);
    delay(100);
    lcd.setCursor(0, 0);
    lcd.print("CORRECT PASSWORD");
    delay(2000);
    lcd.clear();
    a = 5;
    failedAttempts = 0;
    currentState = 1;
    alarmDeactivated = true;
    return;
  } else 
  resetPassword();
}

// Fingerprint Verification
bool verifyFingerprint() {
  int p = -1;
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
    case FINGERPRINT_OK:
      p = finger.image2Tz();
      if (p == FINGERPRINT_OK){
        p = finger.fingerFastSearch();
        if (p == FINGERPRINT_OK) {
          return true; // Match found
        } else {
          failedAttempts++;
          Serial.println("RETRY"); // Notify Python about retry
          lcd.clear();
          lcd.print("Retrying...");
          checkFailedAttempts();
          delay(1000);
          return false; // No match
        }      
      }
      break;
    case FINGERPRINT_NOFINGER:
      Serial.print(".");
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      break;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      break;
    default:
      Serial.println("Unknown error");
      break;
    }
  }


  // if (p != FINGERPRINT_OK) return false;

  // p = finger.image2Tz();
  // if (p != FINGERPRINT_OK) return false;

  // p = finger.fingerFastSearch();
  // if (p == FINGERPRINT_OK) {
  //   return true; // Match found
  // } else {
  //   return false; // No match
  // }
}

// Send Success Message to Python
void sendFingerprintSuccess() {
  Serial.println("FINGERPRINT_OK");
}

// Register a New Fingerprint
// void registerFingerprint() {
//   lcd.clear();
//   lcd.print("Place finger...");

//   int id = getFingerprintID();
//   if (id >= 0) {
//     lcd.clear();
//     lcd.print("Registered ID:");
//     lcd.setCursor(0, 1);
//     lcd.print(id);
//     Serial.println("FINGERPRINT_REGISTERED"); // Notify Python of success
//   } else {
//     lcd.clear();
//     lcd.print("Reg. failed.");
//     Serial.println("REGISTRATION_FAILED"); // Notify Python of failure
//   }
// }

int getFingerprintID() {
  int p = -1;

  while (p != FINGERPRINT_NOFINGER) {
    p = finger.getImage();
  }
  lcd.clear();
  lcd.print("Lift finger...");
  p = finger.image2Tz(1);
  if (p != FINGERPRINT_OK) return -1;

  lcd.print("Place again...");
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
  }

  lcd.print("Processing...");
  p = finger.image2Tz(2);
  if (p != FINGERPRINT_OK) return -1;

  p = finger.createModel();
  if (p != FINGERPRINT_OK) return -1;

  p = finger.storeModel(1); // Store template with ID 1
  if (p != FINGERPRINT_OK) return -1;

  return 1; // Successfully registered ID 1
}

// Handle Failed Attempts
void checkFailedAttempts() {
  if (failedAttempts >= 5) {
    activateAlarm();
    // failedAttempts = 0;
  }
}

// Activate Alarm
void activateAlarm() {
  lcd.clear();
  lcd.print("ALARM! Too many");
  lcd.setCursor(0, 1);
  lcd.print("failed attempts");
  digitalWrite(buzzer, HIGH);
  delay(500);
  digitalWrite(buzzer, LOW);
  delay(500);
  lcd.clear();
  alarmDeactivated = false;
  useKeypadAlarm();
}

// Log Data to Python
void logEvent(String message) {
  Serial.println(message);
}

void processNumberKey(char key) {
  lcd.setCursor(a, 1);
  lcd.print("*");
  a++;
  if (a == 11) {
    a = 5;
  }
  currentPasswordLength++;
  password.append(key);
  if (currentPasswordLength == maxPasswordLength) {
    password.evaluate();
  }
}



void dooropen() {

  if (password.evaluate()) {
    myservo.write(90);
    digitalWrite(buzzer, HIGH);
    delay(300);
    digitalWrite(buzzer, LOW);
    delay(100);
    lcd.setCursor(0, 0);
    lcd.print("CORRECT PASSWORD");
    lcd.setCursor(0, 1);
    lcd.print("DOOR OPENED");
    delay(2000);
    lcd.clear();
    a = 5;
    currentState = 3;
  } else {
    failedAttempts++;
    digitalWrite(buzzer, HIGH);
    delay(200);
    digitalWrite(buzzer, LOW);
    delay(200);
    digitalWrite(buzzer, HIGH);
    delay(200);
    digitalWrite(buzzer, LOW);
    delay(200);
    digitalWrite(buzzer, HIGH);
    delay(200);
    digitalWrite(buzzer, LOW);
    delay(200);
    lcd.setCursor(0, 0);
    lcd.print("WRONG PASSWORD!");
    lcd.setCursor(0, 1);
    lcd.print("PLEASE TRY AGAIN");
    delay(2000);
    checkFailedAttempts();
    lcd.clear();
    a = 5;
  }
  resetPassword();
}



void resetPassword() {
  password.reset();
  currentPasswordLength = 0;
  lcd.clear();
  a = 5;
}



void doorlocked() {
    myservo.write(180);
    digitalWrite(buzzer, HIGH);
    delay(300);
    digitalWrite(buzzer, LOW);
    delay(100);
    lcd.setCursor(0, 0);
    lcd.print("CORRECT PASSWORD");
    lcd.setCursor(2, 1);
    lcd.print("DOOR LOCKED");
    delay(2000);
    lcd.clear();
    a = 5;
    value = false;
    currentState = 1;

  resetPassword();

}

// void alarmOff() {

//   if (password.evaluate()) {

//     digitalWrite(buzzer, LOW);
//     delay(100);
//     lcd.setCursor(0, 0);
//     lcd.print("CORRECT PASSWORD");
//     delay(2000);
//     lcd.clear();
//     a = 5;
//     failedAttempts = 0;
//     currentState = 1;
//     return;
//   } else 
//   resetPassword();
// }

