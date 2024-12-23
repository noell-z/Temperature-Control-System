#include <LiquidCrystal_I2C.h>

// Pin Definitions
const int out1Pin = A2;    // Output 1 from comparator
const int out2Pin = A3;    // Output 2 from comparator
const int pwmPin = 8;      // PWM output pin connected to the fan
const int irSensorPin = 2; // Pin connected to IR sensor OUT

// Variables for RPM Calculation
volatile int pulseCount = 0; // Count of pulses detected
unsigned long lastTime = 0;
int rpm = 0;

// Initialize the LCD
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Track previous messages to avoid redundant updates
String previousMessage = "";

void setup() {
  // Initialize Serial Monitor for debugging
  Serial.begin(9600);

  // Initialize the LCD
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("System Ready");
  delay(2000); // Display for 2 seconds
  lcd.clear();

  // Configure pins
  pinMode(out1Pin, INPUT);
  pinMode(out2Pin, INPUT);
  pinMode(pwmPin, OUTPUT);
  pinMode(irSensorPin, INPUT_PULLUP);

  // Attach interrupt for pulse counting
  attachInterrupt(digitalPinToInterrupt(irSensorPin), countPulse, FALLING);
}

void loop() {
  // Read comparator outputs
  int out1Value = analogRead(out1Pin);
  int out2Value = analogRead(out2Pin);

  // Define thresholds for HIGH (4-5V) and LOW (0-1V)
  bool out1High = out1Value > 600; // Out1 is HIGH if value > ~4V
  bool out2High = out2Value > 600; // Out2 is HIGH if value > ~4V

  // Determine the system state
  String currentMessage;
  if (out1High && out2High) {
    currentMessage = "Within Window";
    digitalWrite(pwmPin, LOW); // Fan off
    rpm = 0; // No RPM in this state
  } else if (out1High && !out2High) {
    currentMessage = "Above Window - Cooling";
    analogWrite(pwmPin, 255); // Full fan speed
  } else if (!out1High && out2High) {
    currentMessage = "Below Window";
    digitalWrite(pwmPin, LOW); // Fan off
    rpm = 0; // No RPM in this state
  }

  // Display the current system state
  if (currentMessage != previousMessage) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(currentMessage);
    previousMessage = currentMessage;
  }

  // Calculate and display RPM if the fan is active
  unsigned long currentTime = millis();
  if (currentTime - lastTime >= 1000) {
    rpm = pulseCount * 60; // Convert pulses/sec to RPM
    pulseCount = 0;        // Reset pulse count
    lastTime = currentTime;

    // Update LCD with RPM
    lcd.setCursor(0, 1);
    lcd.print("Fan Speed: ");
    lcd.print(rpm);
    lcd.print(" RPM");

    // Log RPM to Serial Monitor
    Serial.print("RPM: ");
    Serial.println(rpm);
  }
}

void countPulse() {
  pulseCount++; // Increment pulse count on each pulse detection
}