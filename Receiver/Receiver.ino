#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// --- HARDWARE SETTINGS ---
#define LDR_PIN 14       // D5 on NodeMCU
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Timing must match the Transmitter exactly (50ms)
const int BIT_PERIOD = 50; 

// Timer variables for the 15-second reset
unsigned long lastMsgTime = 0;
bool isWaitingMode = true;

void setup() {
  pinMode(LDR_PIN, INPUT);
  Serial.begin(115200);

  // Initialize OLED with I2C Address 0x3C
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Loop forever if display fails
  }

  // Clear buffer and show the startup screen
  display.clearDisplay();
  showWaitingScreen();
}

void loop() {
  // 1. Check for Incoming Data (Active Low Start Bit means signal goes LOW)
  if (digitalRead(LDR_PIN) == LOW) {
    
    // If we were in "Waiting" mode, clear the screen first
    if (isWaitingMode) {
      display.clearDisplay();
      display.setCursor(0, 0);
      display.setTextSize(2); // Large text for the message
      display.setTextColor(WHITE);
      isWaitingMode = false;
    }

    // Read the character from the laser
    char receivedChar = readChar();
    
    // Print to Serial Monitor (for debugging)
    Serial.print(receivedChar);

    // Print to OLED Display
    display.write(receivedChar); // Use .write() for raw characters
    display.display();

    // Reset the 15-second timer because we just got a new character
    lastMsgTime = millis();
  }

  // 2. Check Timeouts
  // If 15 seconds have passed since the last character, show "Waiting..."
  if (!isWaitingMode && (millis() - lastMsgTime > 15000)) {
    showWaitingScreen();
  }
}

// --- FUNCTION TO DECODE LASER SIGNALS ---
char readChar() {
  // The Transmitter sends a Start Bit (50ms).
  // We wait 1.5x BIT_PERIOD (75ms) to skip the Start Bit 
  // and land exactly in the middle of the first Data Bit.
  delay(BIT_PERIOD + (BIT_PERIOD / 2));

  char receivedChar = 0;

  // Loop to read 8 bits (1 Byte)
  for (int i = 0; i < 8; i++) {
    int bitState = digitalRead(LDR_PIN);
    
    // Logic for Active Low:
    // If Pin is LOW (Light), it represents a '1'
    // If Pin is HIGH (Dark), it represents a '0'
    if (bitState == LOW) {
      receivedChar |= (1 << i); // Set the bit to 1
    }
    
    // Wait for the next bit
    delay(BIT_PERIOD);
  }
  
  // Wait for the Stop Bit to finish so we don't read it as a Start Bit
  delay(BIT_PERIOD); 
  
  return receivedChar;
}

// --- FUNCTION TO SHOW WAITING SCREEN ---
void showWaitingScreen() {
  display.clearDisplay();
  
  // Title Bar
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(20, 0);
  display.println("LIFI RECEIVER");
  
  // Horizontal Line
  display.drawLine(0, 10, 128, 10, WHITE);
  
  // Status Text
  display.setTextSize(1);
  display.setCursor(10, 30);
  display.println("Waiting for msg...");
  
  // Push to screen
  display.display();
  
  // Set flag so we know we are waiting
  isWaitingMode = true;
}