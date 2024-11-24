#include <SPI.h>
#include <SX127XLT.h>  // Include the SX127XLT library

// Define LoRa pins
#define NSS   16   // Chip Select pin for LoRa
#define NRESET 17   // Reset pin for LoRa
#define DIO0  21    // DIO0 pin (interrupt pin for LoRa)

SX127XLT lora;  // Create an instance of the SX127XLT library

// Device-specific addresses
const uint8_t MY_ADDRESS = 0x01;      // Unique address for this device
const uint8_t TARGET_ADDRESS = 0x02; // Address of the target device

uint32_t timeout = 5000;   // Set a suitable timeout (5 seconds)
int8_t txPower = 20;       // Set transmission power (e.g., 14 dBm)
uint8_t wait = 1;          // Wait for transmission to complete
int16_t speedLimit = 25;

void setup() {
  Serial.begin(9600);    // Start the Serial Monitor
  while (!Serial);

  SPI.begin();  // Initialize SPI
  if (!lora.begin(NSS, NRESET, DIO0, DEVICE_SX1278)) {  // Initialize the LoRa device
    Serial.println("LoRa Device Initialization Failed!");
    while (1);  // Halt if initialization fails
  }

  Serial.println("LoRa Device Initialized");

  // Setup LoRa transmission settings
  lora.setupLoRa(434000000, 0, LORA_SF7, LORA_BW_125, LORA_CR_4_5, LDRO_AUTO); // Set frequency, spreading factor, etc.
  lora.setSyncWord(0x12);
}

void loop() {
  if (Serial.available() > 0) {
    int newSpeedLimit = Serial.parseInt();  // Temporarily store the new speed limit

    // Validate the received speed limit
    if (newSpeedLimit > 0) {
      speedLimit = newSpeedLimit;  // Update the global speedLimit variable
      Serial.println("You entered: " + String(speedLimit));
    } else {
      Serial.println("Invalid input. Please enter a positive number.");
    }
  }

  sendSpeedLimit(TARGET_ADDRESS, speedLimit);
  delay(1000);  // 1s delay
}

void sendSpeedLimit(uint8_t recipientAddress, int16_t speedLimit) {
    // Create a packet with sender and recipient addresses, and speed limit
    uint8_t packet[255];
    packet[0] = MY_ADDRESS;         // Sender address
    packet[1] = recipientAddress;  // Recipient address
    packet[2] = (speedLimit >> 8) & 0xFF;  // High byte of speed limit
    packet[3] = speedLimit & 0xFF;         // Low byte of speed limit

    // Transmit the packet
    if (lora.transmit(packet, 4, timeout, txPower, wait)) {  // 4 bytes: 2 for addresses, 2 for speed limit
        Serial.println("Speed limit: " + String(speedLimit) + " sent successfully!");
    } else {
        Serial.println(F("Failed to send speed limit."));
    }
}
