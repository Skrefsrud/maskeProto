#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <esp32-hal-ledc.h>


// BLE UUIDs (using Nordic UART Service UUIDs)
#define SERVICE_UUID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"

const int ledPin = 2; // GPIO pin for the built-in LED
int pwmChannel = 0;   // PWM channel used for LED control
int brightness = 255; // Default brightness (0-255)

bool deviceConnected = false;
unsigned long blinkDelay = 1000; // Default delay (in milliseconds)
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

            // Split the received string into frequency and brightness
            int commaIndex = rxValue.indexOf(',');
            if (commaIndex > 0) {
                String freqStr = rxValue.substring(0, commaIndex);
                String brightnessStr = rxValue.substring(commaIndex + 1);

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
            } else {
                Serial.println("Invalid format. Expected 'frequency,brightness'");
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

    if (currentMillis - previousMillis >= blinkDelay) {
        previousMillis = currentMillis;

        // Toggle LED state
        ledState = !ledState;

        if (ledState) {
            // Turn LED on with specified brightness
            ledcWrite(ledPin, brightness);
        } else {
            // Turn LED off
            ledcWrite(ledPin, 0);
        }
    }
}




