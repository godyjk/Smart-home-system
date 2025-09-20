/*
Pins:
WinOpn:35;
WinCls:36;
DoorOpn:37;
*/


#include <Wire.h>
#include <Adafruit_Fingerprint.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#define FINGERPRINT_SENSOR_RX_PIN 16
#define FINGERPRINT_SENSOR_TX_PIN 17
int ifFin = 0;

// Define a service UUID for communication
#define SERVICE_UUID        "867a936f-a51f-4a96-a135-e3794c8364b9"
// Define a characteristic UUID to send/receive data
#define CHARACTERISTIC_UUID_READ_WRITE "a7c65324-a198-41d6-83c8-17440cbceac9"

#define CHARACTERISTIC_UUID_NOTIFY     "5ce7a22c-8d34-41c5-b0df-20dd25091748"

Adafruit_Fingerprint finger = Adafruit_Fingerprint(&Serial1);

BLEServer *pServer = NULL;
BLECharacteristic *pReadWriteCharacteristic = NULL;
BLECharacteristic *pNotifyCharacteristic = NULL;
bool deviceConnected = false;
bool oldDeviceConnected = false;
uint32_t value = 0;  // Data to send via notification


// Callback for server connection/disconnection events
class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
    }
};

// Callback for handling read/write to the characteristic
class MyCallbacks: public BLECharacteristicCallbacks {
    // When phone writes data to ESP32 (write to characteristic)
    void onWrite(BLECharacteristic *pCharacteristic) {
      String receivedData = pCharacteristic->getValue();
      if (receivedData.length() > 0) {
        Serial.println("Received from phone: " + receivedData);
        String ack = "ACK\n";        
        delay(300);
        pNotifyCharacteristic->setValue(ack.c_str());
        pNotifyCharacteristic->notify();
        receivedData.trim();

        if(receivedData.equals("winOpn")){
          digitalWrite(35,HIGH);
          delay(1000);
          String winO = "===Window opened.===\n";
          pNotifyCharacteristic->setValue(winO.c_str());
          pNotifyCharacteristic->notify();
        }
        
        if(receivedData.equals("winCls")){
          digitalWrite(35,HIGH);
          delay(1000);
          String winC = "===Window closed.===\n";
          pNotifyCharacteristic->setValue(winC.c_str());
          pNotifyCharacteristic->notify();
        }

        if(receivedData.equals("doorOpn")){
          digitalWrite(37,HIGH);
          delay(1000);
          String doorOp = "===Door opened.===\n";
          pNotifyCharacteristic->setValue(doorOp.c_str());
          pNotifyCharacteristic->notify();
        }


      }
    }

    // When phone reads data from ESP32 (optional)
    void onRead(BLECharacteristic *pCharacteristic) {
      Serial.println("Phone is reading from ESP32");
    }
};



void setup() {
  Serial.begin(115200);
  Serial1.begin(57600, SERIAL_8N1, FINGERPRINT_SENSOR_RX_PIN, FINGERPRINT_SENSOR_TX_PIN);
  finger.begin(57600);
  pinMode(35,OUTPUT);
  pinMode(36,OUTPUT);
  pinMode(37,OUTPUT);
        digitalWrite(37,LOW);
  int door_stat = 0;
  int win_stat = 0;
  Serial.println("AS608 Serving");
  Serial.println("Found fingerprint sensor :)");
  //pinMode(37,INPUT);

  BLEDevice::init("Click_Install_OpenGFW");
    // Create BLE server and set callbacks
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());
  // Create BLE service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Create Read/Write characteristic
  pReadWriteCharacteristic = pService->createCharacteristic(
                               CHARACTERISTIC_UUID_READ_WRITE,
                               BLECharacteristic::PROPERTY_READ |
                               BLECharacteristic::PROPERTY_WRITE |
                               BLECharacteristic::PROPERTY_NOTIFY
                             );
  pReadWriteCharacteristic->setCallbacks(new MyCallbacks());

  // Create Notify characteristic for sending messages to the phone
  pNotifyCharacteristic = pService->createCharacteristic(
                             CHARACTERISTIC_UUID_NOTIFY,
                             BLECharacteristic::PROPERTY_NOTIFY
                           );
  pNotifyCharacteristic->addDescriptor(new BLE2902());
  // Add CCCD to Read/Write characteristic for proper handling
  pReadWriteCharacteristic->addDescriptor(new BLE2902());

  // Start the service
  pService->start();

  // Start advertising
  pServer->getAdvertising()->start();
  Serial.println("Waiting for a client to connect...");
}

void loop() {
  uint8_t id = 0;
  //Serial.println("Place your finger on the sensor...");

  while (finger.getImage() != FINGERPRINT_OK);

  if (finger.image2Tz(1) != FINGERPRINT_OK) {
    Serial.println("Failed to convert image to template :(");
    return;
  }

  if (finger.fingerFastSearch() == FINGERPRINT_OK) {
    id = finger.fingerID;
    Serial.print("Fingerprint matched with ID #");
    Serial.println(id);
    Serial.println("还真是");
    doorOpn2();
    
    String message2 = "===Fingerprint match!===\n";
    pNotifyCharacteristic->setValue(message2.c_str());  // Set message as value
    pNotifyCharacteristic->notify();  // Notify phone with the message
    
    String doorO = "===Door opening...===\n";
    pNotifyCharacteristic->setValue(doorO.c_str());  // Set message as value
    pNotifyCharacteristic->notify();  // Notify phone with the message
    //checkPer();
    //delay(3000);
    //digitalWrite(18,LOW);
    //Serial.println("关门！");
    //checkPer();
    //Addings
    //delay(200);


  } else {
    String message3 = "===Fingerprint *NO MATCH*===\n";
    pNotifyCharacteristic->setValue(message3.c_str());  // Set message as value
    pNotifyCharacteristic->notify();  // Notify phone with the message
  }




  // If phone is connected, send notifications
  if (deviceConnected) {
    value++;  // Increment the value to send
    String message = "State #" + String(value) + " BLE in connection.\n";
    //String message = "0";
    pNotifyCharacteristic->setValue(message.c_str());  // Set message as value
    pNotifyCharacteristic->notify();  // Notify phone with the message

    Serial.println("Sent to phone: " + message);
    delay(2000);  // Send message every 2 seconds
  }

  // Handle reconnection logic if needed
  if (!deviceConnected && oldDeviceConnected) {
    delay(500);  // Give time for the disconnect to fully process
    pServer->startAdvertising();  // Restart advertising
    oldDeviceConnected = deviceConnected;
  }

  if (deviceConnected && !oldDeviceConnected) {
    oldDeviceConnected = deviceConnected;
  }
  delay(1000);
  digitalWrite(35,LOW);
    digitalWrite(36,LOW);
      digitalWrite(37,LOW);
}
/*
void winOpn(){
  digitalWrite(35,HIGH);
  delay(3000);
    digitalWrite(35,LOW);
    delay(500);
}

void winCls(){
  digitalWrite(36,HIGH);
  delay(3000);
    digitalWrite(36,LOW);
        delay(500);
}
*/
void doorOpn(){
  digitalWrite(37,HIGH);
  delay(1000);
    digitalWrite(37,LOW);
        delay(500);
}
void doorOpn2(){
  digitalWrite(36,HIGH);
  delay(1000);
    digitalWrite(36,LOW);
        delay(500);
}