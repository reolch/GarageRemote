/*
   GarageRemote
   Created by Reolch. 2022.10.19
*/

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

// https://www.uuidgenerator.net/
#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

// 電源ONで点灯するインジケータ
const int POWER_CONNECT_LED = 12;
// Bluetoothで端末と接続した際に点灯するインジケータ
const int CONNECT_LED = 13;
const int CH1 = 25;
const int OPEN = 26;
bool deviceConnected = false;

/*
   Server CallBack
   onConnect -> 接続されたとき
   onDisconnect -> 切断されたとき
*/
class ServerCallBacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      Serial.println("Connected");
      deviceConnected = true;
      digitalWrite(CONNECT_LED, HIGH);
    }

    void onDisconnect(BLEServer* pServer) {
      Serial.println("DisConnected");
      deviceConnected = false;
      digitalWrite(CONNECT_LED, LOW);
    }
};


/*
   Characteristic CallBack
   onWrite -> 受信するとき
*/
class CharacteristicCallbacks : public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic* pCharacteristic) {
      std::string value = pCharacteristic->getValue();

      if (value.length() > 0) {
        String str = value.c_str();
        Serial.println(str);

        digitalWrite(CH1, HIGH);
        delay(100);
        digitalWrite(CH1, LOW);
        digitalWrite(OPEN, HIGH);
        delay(100);
        digitalWrite(OPEN, LOW);
        Serial.println("Shutter Open！");
      }
    }
};

void setup() {
  Serial.begin(115200);
  pinMode(CONNECT_LED, OUTPUT);
  pinMode(POWER_CONNECT_LED, OUTPUT);
  pinMode(CH1, OUTPUT);
  pinMode(OPEN, OUTPUT);

  // 電源LED点灯
  digitalWrite(POWER_CONNECT_LED, HIGH);

  // この名前がスマホなどに表示される
  BLEDevice::init("ESP32_BLE_SERVER");
  BLEServer* pServer = BLEDevice::createServer();
  pServer -> setCallbacks(new ServerCallBacks());
  BLEService* pService = pServer->createService(SERVICE_UUID);
  // キャラクタリスティックの作成　→　「僕はこんなデータをやり取りするよできるよ」的な宣言
  BLECharacteristic* pCharacteristic = pService->createCharacteristic(
                                         CHARACTERISTIC_UUID,
                                         BLECharacteristic::PROPERTY_READ |
                                         BLECharacteristic::PROPERTY_WRITE
                                       );

  pCharacteristic->setCallbacks(new CharacteristicCallbacks());
  pService->start();
  BLEAdvertising* pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  // iPhone接続の問題に役立つ
  pAdvertising->setMinPreferred(0x06);
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();
  Serial.println("Characteristic defined! Now you can read it in your phone!");
}

void loop() {
  // 端末が未接続の場合、アドバタイジングを開始する
  if (deviceConnected == false) {
    BLEDevice::startAdvertising();
    Serial.println("start advertising");
  }
  delay(2000);
}
