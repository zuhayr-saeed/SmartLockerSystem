//Personalized Smart Locker System
//Abstract:
//        The Personalized Smart Locker System is a secure storage solution
//        accessible only via a user-specific code. It employs two Arduinos:
//        one handles user input through a keypad and controls a servo
//        motor for locking mechanisms; the other logs access times using a
//        RTC and displays them on a 16x2 LCD screen. Communication between
//        the Arduinos is facilitated through the I2C protocol. An original
//        feature allows administrators to reset user codes using a master
//        key, enhancing security and ease of management. Additionally, the
//        system provides real-time feedback and maintains a reliable log of
//        access events, ensuring both user convenience and accountability.


#include <Keypad.h>
#include <Servo.h>
#include <Wire.h>


// Keypad configuration
const byte ROWS = 4; // Four rows
const byte COLS = 4; // Four columns


char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};


byte rowPins[ROWS] = {5, 4, 3, 2};  // Connect to the row pinouts of the keypad
byte colPins[COLS] = {9, 8, 7, 6};  // Connect to the column pinouts of the keypad


Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);


// Servo configuration
Servo lockServo;
const int servoPin = 10;


// Define locked and unlocked positions
const int LOCKED_POSITION = 90;   // Locked position at 90 degrees
const int UNLOCKED_POSITION = 0;  // Unlocked position at 0 degrees


// Variables for code entry and storage
String enteredCode = "";
String storedCode = "147C"; // Default access code


// Master key sequence for resetting code
const String masterKeySequence = "DDDD";
bool codeResetMode = false; // Flag to indicate code reset mode


// Buzzer configuration
const int buzzerPin = 11; // Pin connected to buzzer
unsigned long buzzerTime = 0; // Time tracking for buzzer sound duration


// Tracking locker state
bool isUnlocked = false; // Flag to track if the locker is currently unlocked


void setup() {
  Serial.begin(9600);       // Initialize serial communication for debugging
  lockServo.attach(servoPin); // Attach servo to the specified pin
  lockServo.write(LOCKED_POSITION); // Ensure servo is in locked position
  pinMode(buzzerPin, OUTPUT); // Set buzzer pin as output
  Wire.begin();             // Begin I2C communication as master
}


void loop() {
  char key = keypad.getKey();


  if (key) {
  Serial.print("Key pressed: ");
  Serial.println(key);


  if (isUnlocked) {
    // If locker is unlocked, listen for '*' to lock
    if (key == '*') {
      lockLocker();
      buzzerSound(true); // Success sound for locking
    } else {
      Serial.println("Locker is unlocked. Press '*' to lock.");
      buzzerSound(false); // Optional: Indicate that only '*' is valid here
    }
  } else if (codeResetMode) {
    // In code reset mode, collect new code
    if (key == '#') {
      if (enteredCode.length() > 0) {
        storedCode = enteredCode;
        Serial.println("New access code set.");
        buzzerSound(true); // Success sound for new code
      } else {
        Serial.println("No code entered. Access code remains unchanged.");
      }
      codeResetMode = false;
      enteredCode = "";
    } else {
      enteredCode += key;
      Serial.print("New code entry: ");
      Serial.println(enteredCode);
    }
  } else {
    // Normal operation mode
    if (key == '#') { // '#' is used to submit the code
      if (enteredCode == storedCode) {
        unlockLocker();
        sendAccessEvent();
        buzzerSound(true); // Correct unlock sound
      } else if (enteredCode == masterKeySequence) {
        codeResetMode = true;
        Serial.println("Master code accepted. Enter new access code followed by '#'.");
        buzzerSound(true); // Correct master key sound
        enteredCode = "";
      } else {
        buzzerSound(false); // Incorrect code sound
        Serial.println("Incorrect code. Access denied.");
      }
      enteredCode = ""; // Reset entered code after processing
    } else {
      enteredCode += key;
      Serial.print("Code entry: ");
      Serial.println(enteredCode);
    }
  }
  }


  // Non-blocking code for buzzer sound duration
  if (millis() - buzzerTime >= 1000) {
  digitalWrite(buzzerPin, LOW); // Turn off buzzer after 1 second
  }
}


void unlockLocker() {
  Serial.println("Locker Unlocked.");
  lockServo.write(UNLOCKED_POSITION); // Rotate servo to unlock position
  isUnlocked = true;   // Update locker state
}


void lockLocker() {
  Serial.println("Locker Locked.");
  lockServo.write(LOCKED_POSITION); // Rotate servo back to lock position
  isUnlocked = false; // Update locker state
}


void sendAccessEvent() {
  Wire.beginTransmission(8); // Address of Arduino 2
  Serial.println("Transmission started");
  Wire.write('A');        // Send a byte indicating access event
  Serial.println("Transmission sent");
  Wire.endTransmission();
  Serial.println("Access event sent to Arduino 2.");
}


void buzzerSound(bool success) {
  if (success) {
  // Correct code sound (long beep)
  tone(buzzerPin, 1000, 500); // 1000Hz for 500ms
  } else {
  // Incorrect code sound (short beep)
  tone(buzzerPin, 1000, 200); // 1000Hz for 200ms
  }
  buzzerTime = millis();
  digitalWrite(buzzerPin, HIGH);
}

