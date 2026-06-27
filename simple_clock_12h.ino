/*
 * Simple 12-Hour Clock for ESP32/ESP8266
 * Uses NTP to get accurate time and displays on OLED
 * 
 * Hardware Requirements:
 * - ESP32 or ESP8266 board
 * - OLED Display (SSD1306, 128x64)
 * - WiFi connection
 * 
 * Connections (OLED to ESP):
 * VCC -> 3.3V
 * GND -> GND
 * SCL -> GPIO 22 (ESP32) / GPIO 5 (ESP8266)
 * SDA -> GPIO 21 (ESP32) / GPIO 4 (ESP8266)
 */

#include <WiFi.h>
#include <time.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ==================== CONFIGURATION ====================

// WiFi Credentials
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

// NTP Server Configuration
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 0;        // Your GMT offset in seconds (e.g., 0 for UTC)
const int daylightOffset_sec = 0;    // Daylight saving offset in seconds (e.g., 3600 for +1 hour)

// OLED Display Configuration
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C

// ==================== GLOBAL VARIABLES ====================

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// ==================== FUNCTION DECLARATIONS ====================

void connectToWiFi();
void initializeDisplay();
void displayTime();
String formatTime12Hour(struct tm* timeinfo);

// ==================== SETUP ====================

void setup() {
  Serial.begin(115200);
  
  // Initialize display
  initializeDisplay();
  
  // Connect to WiFi
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("Connecting to WiFi...");
  display.display();
  
  connectToWiFi();
  
  // Initialize and get time from NTP
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  
  display.println("Getting time...");
  display.display();
  
  // Wait for time to sync
  struct tm timeinfo;
  while (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    delay(1000);
  }
  
  display.clearDisplay();
  display.println("Clock Ready!");
  display.display();
  delay(1000);
}

// ==================== MAIN LOOP ====================

void loop() {
  displayTime();
  delay(1000); // Update every second
}

// ==================== FUNCTION DEFINITIONS ====================

void connectToWiFi() {
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 30) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi connected!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nWiFi connection failed!");
    display.clearDisplay();
    display.println("WiFi Failed!");
    display.display();
    while (1) delay(1000); // Halt if WiFi fails
  }
}

void initializeDisplay() {
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    while (1) delay(1000); // Don't proceed if display fails
  }
  
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setTextWrap(false);
  display.display();
}

void displayTime() {
  struct tm timeinfo;
  
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return;
  }
  
  // Clear display
  display.clearDisplay();
  
  // Format and display time in 12-hour format
  String timeStr = formatTime12Hour(&timeinfo);
  
  // Display time (centered)
  display.setTextSize(3);
  display.setCursor(10, 15);
  display.println(timeStr);
  
  // Display date below time
  display.setTextSize(1);
  display.setCursor(10, 50);
  
  char dateStr[30];
  strftime(dateStr, sizeof(dateStr), "%A, %B %d %Y", &timeinfo);
  display.println(dateStr);
  
  // Update display
  display.display();
}

String formatTime12Hour(struct tm* timeinfo) {
  int hour = timeinfo->tm_hour;
  int minute = timeinfo->tm_min;
  int second = timeinfo->tm_sec;
  String ampm = "AM";
  
  // Convert to 12-hour format
  if (hour >= 12) {
    ampm = "PM";
    if (hour > 12) {
      hour -= 12;
    }
  }
  if (hour == 0) {
    hour = 12; // Midnight is 12 AM
  }
  
  // Format with leading zeros
  String hourStr = (hour < 10) ? "0" + String(hour) : String(hour);
  String minStr = (minute < 10) ? "0" + String(minute) : String(minute);
  String secStr = (second < 10) ? "0" + String(second) : String(second);
  
  return hourStr + ":" + minStr + ":" + secStr + " " + ampm;
}
