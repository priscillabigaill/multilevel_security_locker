#include <Adafruit_Fingerprint.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define rxPin 10
#define txPin 12
SoftwareSerial mySerial(rxPin, txPin);
Adafruit_Fingerprint finger(&mySerial);

// I2C LCD Initialization
LiquidCrystal_I2C lcd(0x27, 16, 2); // Adjust the address (0x27) as needed

void setup() {
  Serial.begin(9600);
  mySerial.begin(57600);
  lcd.begin(16, 2);
  lcd.backlight();

  // Initialize fingerprint sensor
  finger.begin(57600);
  if (finger.verifyPassword()) {
    lcd.print("Sensor Ready");
  } else {
    lcd.print("Sensor Error");
    while (true); // Halt system
  }
}

void loop() {
  lcd.setCursor(0, 0);
  lcd.print("Place finger...");

  int id = getFingerprintID();
  if (id >= 0) {
    lcd.clear();
    lcd.print("Success ID: ");
    lcd.setCursor(0, 1);
    lcd.print(id);
    delay(2000);
  } else {
    lcd.clear();
    lcd.print("Failed Reg.");
    delay(2000);
  }
}
int getFingerprintID() {
  int p = -1;

  lcd.clear();
  lcd.print("Wait for finger...");
  Serial.println("Waiting for finger...");

  // Wait until a finger is detected
  while (true) {
    p = finger.getImage();
    if (p == FINGERPRINT_OK) {
      Serial.println("Finger detected!");
      break; // Exit the loop when a finger is detected
    } else if (p == FINGERPRINT_NOFINGER) {
      // Keep waiting if no finger is detected
      delay(100);
    } else {
      // Handle other errors (e.g., sensor issues)
      Serial.println("Error detecting finger");
      return -1;
    }
  }

  // Capture the first fingerprint image
  lcd.print("Press again...");
  Serial.println("Capturing first image...");
  p = finger.image2Tz(1); // Save image to buffer 1
  if (p != FINGERPRINT_OK) {
    Serial.println("Failed to convert first image to template");
    return -1;
  }

  lcd.print("Lift finger");
  delay(2000);

  // Wait until the finger is lifted
  while (true) {
    p = finger.getImage();
    if (p == FINGERPRINT_NOFINGER) {
      Serial.println("Finger lifted!");
      break; // Exit the loop when the finger is lifted
    }
    delay(100);
  }

  // Wait until the finger is placed again
  lcd.print("Place again...");
  Serial.println("Waiting for finger again...");
  while (true) {
    p = finger.getImage();
    if (p == FINGERPRINT_OK) {
      Serial.println("Finger detected again!");
      break; // Exit the loop when a finger is detected
    } else if (p == FINGERPRINT_NOFINGER) {
      // Keep waiting if no finger is detected
      delay(100);
    } else {
      // Handle other errors (e.g., sensor issues)
      Serial.println("Error detecting finger");
      return -1;
    }
  }

  // Capture the second fingerprint image
  lcd.print("Processing...");
  Serial.println("Capturing second image...");
  p = finger.image2Tz(2); // Save image to buffer 2
  if (p != FINGERPRINT_OK) {
    Serial.println("Failed to convert second image to template");
    return -1;
  }

  // Create a fingerprint model
  Serial.println("Creating model...");
  p = finger.createModel();
  if (p != FINGERPRINT_OK) {
    Serial.println("Failed to create fingerprint model");
    return -1;
  }

  // Store the fingerprint model
  Serial.println("Storing model...");
  p = finger.storeModel(1); // Store template with ID 1
  if (p != FINGERPRINT_OK) {
    Serial.println("Failed to store fingerprint model");
    return -1;
  }

  Serial.println("Fingerprint registered successfully!");
  return 1; // Successfully registered
}