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


#include <Wire.h>
#include <LiquidCrystal.h>
#include <RTClib.h>


// Initialize the RTC
RTC_PCF8523 rtc;


// Initialize the LCD: LiquidCrystal(rs, e, d4, d5, d6, d7)
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);


// Buffer to store the two most recent access times
char accessTimes[2][9];  // "HH:MM:SS" + null terminator


// Flag to indicate an access event has been received
volatile bool accessEventFlag = false;


// Function prototypes
void receiveEvent(int howMany);
void logAccessEvent();


void setup() {
  // Initialize Serial for debugging
  Serial.begin(9600);

  // Initialize I2C communication as slave with address 8
  Wire.begin(8);
  Wire.onReceive(receiveEvent);

  // Initialize the LCD
  lcd.begin(16, 2);  // 16 columns and 2 rows
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Smart Locker");
  lcd.setCursor(0, 1);
  lcd.print("System Ready");

  // Initialize RTC
  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("RTC not found!");
    while (1)
      ;  // Halt if RTC not found
  }

  if (!rtc.isrunning()) {
    Serial.println("RTC is NOT running!");
    // Uncomment the following line only once to set the RTC manually
    // rtc.adjust(DateTime(2024, 12, 4, 15, 35, 0)); // Year, Month, Day, Hour, Minute, Second
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    Serial.println("RTC time set to compile time.");
  }

  // Initialize accessTimes with empty strings
  accessTimes[0][0] = '\0';
  accessTimes[1][0] = '\0';
}


void loop() {
  // Check if an access event has been flagged
  noInterrupts();
  bool flag = accessEventFlag;
  accessEventFlag = false;
  interrupts();

  if (flag) {
    logAccessEvent();
  }

  // Removed the periodic current time display to prevent interference
}


// Function to handle incoming I2C data
void receiveEvent(int howMany) {
  while (Wire.available()) {
    char c = Wire.read();
    if (c == 'A') {  // Access event received
      accessEventFlag = true;
      Serial.println("Received 'A' from Arduino 1");
    }
  }
}


// Function to log access events
void logAccessEvent() {
  DateTime now = rtc.now();

  // Format the time as HH:MM:SS
  char timeBuffer[9];  // "HH:MM:SS" + null terminator
  sprintf(timeBuffer, "%02d:%02d:%02d", now.hour(), now.minute(), now.second());

  // Shift the existing access time to accessTimes[0]
  strcpy(accessTimes[0], accessTimes[1]);

  // Store the new access time in accessTimes[1]
  strncpy(accessTimes[1], timeBuffer, sizeof(accessTimes[1]) - 1);
  accessTimes[1][sizeof(accessTimes[1]) - 1] = '\0';  // Ensure null termination

  // Update LCD display with the two most recent access times
  lcd.setCursor(0, 0);
  if (accessTimes[0][0] != '\0') {  // Check if accessTimes[0] has a valid string
    lcd.print("            ");      // Clear previous content on first line
    lcd.setCursor(0, 0);
    lcd.print(accessTimes[0]);
  }

  lcd.setCursor(0, 1);
  lcd.print("            ");  // Clear previous content on second line
  lcd.setCursor(0, 1);
  lcd.print(accessTimes[1]);

  // Debugging output
  Serial.print("Access event at: ");
  Serial.println(timeBuffer);
}
