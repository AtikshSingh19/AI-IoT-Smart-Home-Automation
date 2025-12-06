CODE:
// ==========================================
// COMBINED CODE: Gesture + MQ135 + ACS72 + LCD
// (CORRECTED FOR BLYNK AI + LCD + ALL FIXES)
// ==========================================
// --- BLYNK CONFIGURATION ---
#define BLYNK_TEMPLATE_ID "TMPL3PpUyWLwI"
#define BLYNK_TEMPLATE_NAME "Home Automation Project"
#define BLYNK_AUTH_TOKEN "-OEwcoIVmfUPMEJ1O05Mbsvd28ONR4aj"
#define BLYNK_PRINT Serial
// --- LIBRARIES ---
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <LiquidCrystal_I2C.h> // LCD Library
// --- WIFI CREDENTIALS ---
char wifi_ssid[] = "ARJUN";
char wifi_pass[] = "arjun123";
// --- HARDWARE PINS ---
const int LED_PIN = 23; // Gesture LED
const int MQ135_PIN = 34; // Gas Sensor (Analog)
const int ACS712_PIN = 35; // Current Sensor (Analog)
const int BUZZER_PIN = 19; // Buzzer
// --- OBJECTS & VARIABLES ---
BlynkTimer timer;
LiquidCrystal_I2C lcd(0x27, 16, 2); // LCD Object (Address 0x27)
// Sensor Settings
int gasThreshold = 2000;
float sensitivity = 0.185;
float vRef = 3.3; // ESP32 is a 3.3V device
// AI Energy Calculation
float homeVoltage = 230.0; // Voltage for AI power calculation
float totalEnergy = 0;
unsigned long lastTime = 0;
float myZeroPoint = 2.3260; // 3.3V / 2. If using 5V ACS712, change this to 2.5
// ==========================
// SETUP
// ==========================
void setup() {
 Serial.begin(115200);
 // -- Initialize Pins --
 pinMode(LED_PIN, OUTPUT);
 pinMode(BUZZER_PIN, OUTPUT);
 pinMode(MQ135_PIN, INPUT);
 pinMode(ACS712_PIN, INPUT);

 digitalWrite(LED_PIN, LOW);
 digitalWrite(BUZZER_PIN, LOW);
 // -- Initialize LCD --
 lcd.init();
 lcd.backlight();
 lcd.setCursor(0, 0);
 lcd.print("System Booting..");
 Serial.println("System Booting...");
 // -- Connect to Blynk --
 Blynk.begin(BLYNK_AUTH_TOKEN, wifi_ssid, wifi_pass);

 lcd.clear();
 lcd.setCursor(0, 0);
 lcd.print("Blynk Connected!");
 Serial.println("Blynk Connected!");
 delay(1000);
 // -- Start Timer (Run sensors every 1000ms) --
 timer.setInterval(1000L, runSensors);
 lastTime = millis();
}
// ==========================
// MAIN LOOP
// ==========================
void loop() {
 Blynk.run();
 timer.run();
}
// ==========================
// TASK 1: BLYNK GESTURE
// ==========================
BLYNK_WRITE(V9) {
 String command = param.asString();
 if (command == "LED_ON") {
 digitalWrite(LED_PIN, HIGH);
 } else if (command == "LED_OFF") {
 digitalWrite(LED_PIN, LOW);
 }
}
// ==========================
// TASK 2: SENSORS & DISPLAY
// ==========================
void runSensors() {
 // --- A. READ SENSORS ---
 int gasValue = analogRead(MQ135_PIN);
 int adcValue = analogRead(ACS712_PIN);
 // --- B. CALCULATE CURRENT (Amps) ---
 float voltage = (adcValue / 4095.0) * vRef;

 // *** CHANGED THIS ***: Use myZeroPoint. Calibrate this value as needed.
 float current = (voltage - myZeroPoint) / sensitivity;

 // Noise Filter: Force to 0 if value is tiny
 if (abs(current) < 0.25) {
 current = 0.00;
 }
 // --- C. CALCULATE POWER (W) & ENERGY (Wh) ---
 // *** CHANGED THIS ***: Using homeVoltage for AI
 float power = homeVoltage * abs(current);

 // *** ADDED THIS BACK ***: Energy calculation for AI
 unsigned long currentTime = millis();
 float timeStepInHours = (currentTime - lastTime) / 3600000.0;
 totalEnergy += power * timeStepInHours;
 lastTime = currentTime;
 // --- D. UPDATE BUZZER & BLYNK ALERTS ---
 if (gasValue > gasThreshold) {
 digitalWrite(BUZZER_PIN, HIGH);
 Blynk.virtualWrite(V1, "ALERT");
 } else {
 digitalWrite(BUZZER_PIN, LOW);
 Blynk.virtualWrite(V1, "Normal");
 }

 // --- E. SEND ALL DATA TO BLYNK (FOR AI) ---
 // *** THIS BLOCK IS NOW INSIDE THE FUNCTION ***
 Blynk.virtualWrite(V5, gasValue); // Send raw gas to AI
 Blynk.virtualWrite(V6, power); // Send power to AI
 Blynk.virtualWrite(V7, totalEnergy); // Send energy to AI
 Blynk.virtualWrite(V10, abs(current)); // Send current for dashboard
 // --- F. UPDATE LCD ---
 // *** THIS BLOCK IS NOW INSIDE THE FUNCTION ***
 lcd.clear();
 // Row 0: Gas Status
 lcd.setCursor(0, 0);
 lcd.print("Gas: ");
 lcd.print(gasValue);
 if(gasValue > gasThreshold) lcd.print(" !"); // Exclamation if danger
 // Row 1: Power Info
 lcd.setCursor(0, 1);
 lcd.print("P:");
 lcd.print(power, 1); // Show 1 decimal place
 lcd.print("W");

 lcd.setCursor(9, 1); // Move to right side
 lcd.print("I:");
 lcd.print(abs(current), 2); // Show 2 decimal places
 lcd.print("A");

 // --- G. PRINT TO SERIAL MONITOR ---
 // *** THIS LINE IS NOW INSIDE THE FUNCTION ***
 Serial.printf("Gas: %d | Power: %.1f W | Current: %.2f A | Energy: %.4f Wh\n",
 gasValue, power, abs(current), totalEnergy);

} // <-- *** THIS IS THE CORRECT, FINAL BRACE for runSensors ***