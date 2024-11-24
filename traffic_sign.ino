#include <SPI.h>
#include <SX127XLT.h>  // Include the SX127XLT library
#include <Adafruit_NeoPixel.h>

// Define LoRa pins
#define NSS   16   
#define NRESET 17   
#define DIO0  21
#define LORA_DEVICE DEVICE_SX1278
#define NEOPIXEL_PIN 4
#define NUM_PIXELS 1

SX127XLT lora;  // Create an instance of the SX127XLT library
Adafruit_NeoPixel strip(NUM_PIXELS, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

// Device-specific addresses
const uint8_t MY_ADDRESS = 0x02;      // Unique address for this device
const uint8_t TARGET_ADDRESS = 0x03; // Address of the target device

uint32_t timeout = 5000;   // Set a suitable timeout (5 seconds)
int8_t txPower = 20;       // Set transmission power (e.g., 14 dBm)
uint8_t wait = 1;          // Wait for transmission to complete
int16_t speedLimit;

void setup() {
  Serial.begin(9600);    // Start the Serial Monitor
  while (!Serial);

  SPI.begin();  // Initialize SPI
  if (!lora.begin(NSS, NRESET, DIO0, LORA_DEVICE)) {  // Initialize the LoRa device
    Serial.println("LoRa Device Initialization Failed!");
    while (1);  // Halt if initialization fails
  }
  Serial.println("LoRa Device Initialized");

  // Setup LoRa transmission settings
  lora.setupLoRa(434000000, 0, LORA_SF7, LORA_BW_125, LORA_CR_4_5, LDRO_AUTO); // Set frequency, spreading factor, etc.
  lora.setSyncWord(0x12);

  // Initialize NeoPixel
  strip.begin();
  strip.show();
  strip.setBrightness(255);

  // Initialize speed limit
  speedLimit = 0;
}

void loop() {
  // Check for incoming packets
  uint8_t buffer[255];
  if (lora.receive(buffer, sizeof(buffer), timeout, wait)) {
    
    uint8_t senderAddress = buffer[0];
    uint8_t recipientAddress = buffer[1];

    if (recipientAddress == MY_ADDRESS) {                              
      Serial.print(F("Message recieved from device"));
      Serial.println(senderAddress);

      speedLimit = (buffer[2] << 8) | buffer[3];
      Serial.print(F("Speed limit receieved: "));
      Serial.println(speedLimit);
      sendSpeedLimit(TARGET_ADDRESS, speedLimit, 1000);
    } else {
      Serial.println(F("Message not for this device. Ignored."));
    }

  } else {
    Serial.println("No packet received.");
  }

  setSpeedLimitColor(speedLimit);
  delay(50);  // 0.5s delay
  Serial.println();
}

void sendSpeedLimit(uint8_t recipientAddress, int16_t speedLimit, uint32_t duration) {
  uint32_t startTime = millis(); // Record the start time
  uint8_t packet[255];
  packet[0] = MY_ADDRESS;         // Sender address
  packet[1] = recipientAddress;   // Recipient address
  packet[2] = (speedLimit >> 8) & 0xFF;  // High byte of speed limit
  packet[3] = speedLimit & 0xFF;         // Low byte of speed limit

  // Send packets continuously for the specified duration
  while (millis() - startTime < duration) {
    if (lora.transmit(packet, 4, timeout, txPower, wait)) {  // 4 bytes: 2 for addresses, 2 for speed limit
      Serial.println(F("Speed limit sent successfully."));
    } else {
      Serial.println(F("Failed to send speed limit."));
    }

    delay(10); // Optional delay between packets (e.g., 100ms), adjust as necessary
  }
}

void setSpeedLimitColor(int16_t speedLimit) {
  if (speedLimit < 5) strip.setPixelColor(0, 255, 0, 0);            // RED
  else if (speedLimit < 10) strip.setPixelColor(0, 255, 165, 0);    // ORANGE
  else if (speedLimit < 15) strip.setPixelColor(0, 255, 255, 0);    // YELLOW
  else if (speedLimit < 20) strip.setPixelColor(0, 0, 255, 0);      // GREEN
  else if (speedLimit < 25) strip.setPixelColor(0, 0, 0, 255);      // BLUE
  else if (speedLimit < 30) strip.setPixelColor(0, 75, 0, 130);     // INDIGO
  else if (speedLimit < 35) strip.setPixelColor(0, 238, 130, 238);  // VIOLET
  else strip.setPixelColor(0, 255, 105, 180);                       // PINK

  strip.show();
}
