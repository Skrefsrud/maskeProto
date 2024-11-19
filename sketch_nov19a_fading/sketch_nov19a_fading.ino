#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <esp32-hal-ledc.h>

// BLE UUIDs (using Nordic UART Service UUIDs)
#define SERVICE_UUID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"

const int ledPin = 4; // GPIO pin for the built-in LED
int pwmChannel = 0;   // PWM channel used for LED control
int brightness = 255; // Default brightness (0-255)
int fadeSpeed = 50;   // Default fade speed percentage (0-100)

bool deviceConnected = false;
unsigned long blinkDelay = 5000; // Default delay (in milliseconds)
unsigned long previousMillis = 0;
bool ledState = LOW;

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
        String rxValue = pCharacteristic->getValue();
        if (rxValue.length() > 0) {
            Serial.print("Received: ");
            Serial.println(rxValue);

            // Split the received string into frequency, brightness, and fadeSpeed
            int firstCommaIndex = rxValue.indexOf(',');
            int secondCommaIndex = rxValue.indexOf(',', firstCommaIndex + 1);
            if (firstCommaIndex > 0 && secondCommaIndex > firstCommaIndex) {
                String freqStr = rxValue.substring(0, firstCommaIndex);
                String brightnessStr = rxValue.substring(firstCommaIndex + 1, secondCommaIndex);
                String fadeSpeedStr = rxValue.substring(secondCommaIndex + 1);

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

    // Set up the LED PWM functionality
    ledcAttach(ledPin, 5000, 8); // Attach pin with 5 kHz frequency and 8-bit resolution

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
            int currentBrightness = (brightness * elapsedTime) / fadeTime;
            ledcWrite(ledPin, currentBrightness);
        } else {
            // Max brightness
            ledcWrite(ledPin, brightness);
        }
    } else {
        // Second half of the cycle (OFF phase)
        unsigned long t = elapsedTime - halfCycle;
        if (fadeSpeed > 0 && t < fadeTime) {
            // Fade out
            int currentBrightness = (brightness * (fadeTime - t)) / fadeTime;
            ledcWrite(ledPin, currentBrightness);
        } else {
            // LED off
            ledcWrite(ledPin, 0);
        }
    }
}
