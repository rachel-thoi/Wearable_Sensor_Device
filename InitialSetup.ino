#include "BLEDevice.h"
#include "BLEServer.h"
#include "BLEUtils.h"
#include "BLE2902.h"

// -------------------- BLE Config --------------------

// Bluetooth device name
const char *bleName = "ESP32_Bluetooth";

// UUIDs (replace these placeholders with real UUID strings)
#define SERVICE_UUID             "your_service_uuid_here"
#define CHARACTERISTIC_UUID_RX   "your_rx_characteristic_uuid_here"
#define CHARACTERISTIC_UUID_TX   "your_tx_characteristic_uuid_here"

// BLE characteristics
BLECharacteristic *pCharacteristic = nullptr;  // TX (Notify)

// -------------------- FSR Config --------------------

// Analog pins for FSRs (replace # with actual pin numbers)
const int fsrPins[4] = {#, #, #, #};

// Threshold to detect a "press" (replace # with a real threshold value)
const int fsrThreshold = #;

// Press states
bool fsrPressed[4] = {false, false, false, false};

// -------------------- Globals --------------------

// Received text + time of last message
String receivedText = "";
unsigned long lastMessageTime = 0;

// Forward declarations
void setupBLE();
void checkFSRs();

// -------------------- Arduino Setup/Loop --------------------

void setup() {
  Serial.begin(115200);  // Debug output

  // Set each FSR pin as input
  for (int i = 0; i < 4; i++) {
    pinMode(fsrPins[i], INPUT);
  }

  setupBLE();  // Initialize BLE
}

void loop() {
  checkFSRs();  // Read FSR values and build BLE message if any are pressed

  // If we have a message and it's been > 1s since last update, send it
  if (receivedText.length() > 0 && (millis() - lastMessageTime) > 1000) {
    Serial.print("Sending BLE message: ");
    Serial.println(receivedText);

    pCharacteristic->setValue(receivedText.c_str());
    pCharacteristic->notify();

    receivedText = "";
  }

  // Read from serial and send via BLE
  if (Serial.available() > 0) {
    String str = Serial.readStringUntil('\n');
    const char *newValue = str.c_str();

    pCharacteristic->setValue(newValue);
    pCharacteristic->notify();
  }
}

// -------------------- BLE Callbacks --------------------

class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer *pServer) override {
    Serial.println("Connected");
  }

  void onDisconnect(BLEServer *pServer) override {
    Serial.println("Disconnected");
  }
};

class MyCharacteristicCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) override {
    // Save received data and timestamp
    std::string value = std::string(pCharacteristic->getValue().c_str());
    receivedText = String(value.c_str());
    lastMessageTime = millis();

    Serial.print("Received: ");
    Serial.println(receivedText);
  }
};

// -------------------- BLE Setup --------------------

void setupBLE() {
  BLEDevice::init(bleName);  // Initialize BLE device

  BLEServer *pServer = BLEDevice::createServer();  // Create BLE server
  if (pServer == nullptr) {
    Serial.println("Error creating BLE server");
    return;
  }
  pServer->setCallbacks(new MyServerCallbacks());

  // Create BLE service
  BLEService *pService = pServer->createService(SERVICE_UUID);
  if (pService == nullptr) {
    Serial.println("Error creating BLE service");
    return;
  }

  // TX characteristic (Notify)
  pCharacteristic = pService->createCharacteristic(
      CHARACTERISTIC_UUID_TX,
      BLECharacteristic::PROPERTY_NOTIFY
  );
  pCharacteristic->addDescriptor(new BLE2902());

  // RX characteristic (Write)
  BLECharacteristic *pCharacteristicRX = pService->createCharacteristic(
      CHARACTERISTIC_UUID_RX,
      BLECharacteristic::PROPERTY_WRITE
  );
  pCharacteristicRX->setCallbacks(new MyCharacteristicCallbacks());

  // Start BLE
  pService->start();
  pServer->getAdvertising()->start();

  Serial.println("Waiting for a client connection...");
}

// -------------------- FSR Reading --------------------

void checkFSRs() {
  String message = "";
  int activeCount = 0;

  for (int i = 0; i < 4; i++) {
    int value = analogRead(fsrPins[i]);       // Read FSR analog value
    fsrPressed[i] = (value > fsrThreshold);   // Check threshold

    if (fsrPressed[i]) {
      if (message.length() > 0) {
        message += "+";
      }
      message += "FSR" + String(i + 1);       // e.g., "FSR1"
      activeCount++;
    }
  }

  if (activeCount > 0) {
    receivedText = message;       // Prepare message for BLE
    lastMessageTime = millis();   // Update timestamp
  }
}
