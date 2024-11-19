#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

// BLE UUIDs (using Nordic UART Service UUIDs)
#define SERVICE_UUID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"

// LED configuration
const int ledPins[] = {4, 5, 18}; // GPIO pins for the LEDs
const int numLEDs = sizeof(ledPins) / sizeof(ledPins[0]);
const int pwmChannel = 0;
const int freq = 5000;      // 5 kHz frequency
const int resolution = 8;   // 8-bit resolution

int brightness = 255; // Default brightness (0-255)
int fadeSpeed = 50;   // Default fade speed percentage (0-100)
unsigned long blinkDelay = 5000; // Default delay (in milliseconds)

bool deviceConnected = false;

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
        deviceConnected = true;
        Serial.println("Client connected");
    }

    void onDisconnect(BLEServer* pServer) {
        deviceConnected = false;
        Serial.println("Client disconnected");
        // Restart advertising
        pServer->getAdvertising()->start();
        Serial.println("Advertising restarted");
    }
};

class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
        std::string rxValue = pCharacteristic->getValue();
        if (rxValue.length() > 0) {
            String receivedValue = String(rxValue.c_str());
            Serial.print("Received: ");
            Serial.println(receivedValue);

            // Split the received string into frequency, brightness, and fadeSpeed
            int firstCommaIndex = receivedValue.indexOf(',');
            int secondCommaIndex = receivedValue.indexOf(',', firstCommaIndex + 1);
            if (firstCommaIndex > 0 && secondCommaIndex > firstCommaIndex) {
                String freqStr = receivedValue.substring(0, firstCommaIndex);
                String brightnessStr = receivedValue.substring(firstCommaIndex + 1, secondCommaIndex);
                String fadeSpeedStr = receivedValue.substring(secondCommaIndex + 1);

                // Parse frequency (blink delay)
                unsigned long newDelay = freqStr.toInt();
                if (newDelay > 0) {
                    blinkDelay = newDelay;
                    Serial.print("Blink delay updated to: ");
                    Serial.println(blinkDelay);
                } else {
                    Serial.println("Invalid frequency value received.");
                }

                // Parse brightness
                int newBrightness = brightnessStr.toInt();
                if (newBrightness >= 0 && newBrightness <= 255) {
                    brightness = newBrightness;
                    Serial.print("Brightness updated to: ");
                    Serial.println(brightness);
                } else {
                    Serial.println("Invalid brightness value received.");
                }

                // Parse fade speed
                int newFadeSpeed = fadeSpeedStr.toInt();
                if (newFadeSpeed >= 0 && newFadeSpeed <= 100) {
                    fadeSpeed = newFadeSpeed;
                    Serial.print("Fade speed updated to: ");
                    Serial.println(fadeSpeed);
                } else {
                    Serial.println("Invalid fade speed value received.");
                }
            } else {
                Serial.println("Invalid format. Expected 'frequency,brightness,fadeSpeed'");
            }
        }
    }
};

void setup() {
    Serial.begin(115200);

    // Configure PWM channel
    ledcSetup(pwmChannel, freq, resolution);

    // Attach multiple pins to the same PWM channel
    for (int i = 0; i < numLEDs; i++) {
        ledcAttachPin(ledPins[i], pwmChannel);
    }

    // Initialize BLE
    BLEDevice::init("ESP32_BLE");

    // Create BLE Server
    BLEServer *pServer = BLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks());

    // Create BLE Service
    BLEService *pService = pServer->createService(SERVICE_UUID);

    // Create BLE Characteristic
    BLECharacteristic *pCharacteristicRX = pService->createCharacteristic(
                                              CHARACTERISTIC_UUID_RX,
                                              BLECharacteristic::PROPERTY_WRITE
                                          );

    // Set Callbacks
    pCharacteristicRX->setCallbacks(new MyCallbacks());

    // Start the service
    pService->start();

    // Start advertising
    BLEAdvertising *pAdvertising = pServer->getAdvertising();
    pAdvertising->start();
    Serial.println("Waiting for a client connection...");
}

void loop() {
    unsigned long currentMillis = millis();
    unsigned long elapsedTime = currentMillis % blinkDelay;
    unsigned long halfCycle = blinkDelay / 2;
    unsigned long fadeTime = (halfCycle * fadeSpeed) / 100;

    if (elapsedTime < halfCycle) {
        // First half of the cycle (ON phase)
        if (fadeSpeed > 0 && elapsedTime < fadeTime) {
            // Fade in
            int currentBrightness
::contentReference[oaicite:0]{index=0}
 
        }
    }
}