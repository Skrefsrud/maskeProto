#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

// BLE UUIDs (using Nordic UART Service UUIDs)
#define SERVICE_UUID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"


const int ledPin = 2;

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

            // Try to parse the received value as an unsigned long (milliseconds)
            unsigned long newDelay = rxValue.toInt();
            if (newDelay > 0) {
                blinkDelay = newDelay;
                Serial.print("Blink delay updated to: ");
                Serial.println(blinkDelay);
            } else {
                Serial.println("Invalid value received.");
            }
        }
    }
};

void setup() {
  Serial.begin(115200);

  pinMode(ledPin, OUTPUT);

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
  // Blink the LED at the specified interval
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= blinkDelay) {
    previousMillis = currentMillis;

    // Toggle LED state
    ledState = !ledState;
    digitalWrite(ledPin, ledState);
  }

  // You can add other tasks here
}



