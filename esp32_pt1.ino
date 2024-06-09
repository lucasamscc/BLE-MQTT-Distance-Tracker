// --- WIFI ---
#include <WiFi.h>
const char* ssid = "Claro_ Net 2.4Ghz";
const char* password = "3656306250";
WiFiClient esp32Client;

// --- MQTT --- 
#include <PubSubClient.h>
PubSubClient client(esp32Client);
const char* brokerUser = "teste";
const char* brokerPass = "1234";
const char* clientId = "esp32-01";
const char* broker = "broker.emqx.io";
const char* outTopic = "topico/scan";

// --- Bluetooth ---
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
int scanTime = 5; // segundos

// --- Setup ---
void setup() {
  Serial.begin(115200);
  conectaWifi();
  client.setServer(broker, 1883);
  Serial.println("Scanning");
  BLEDevice::init("");
}

const int rssi_ref = -74; // Reference RSSI at 1 meter
const float N = 2.0; // Path loss exponent
const int numReadings = 10; // Number of RSSI readings to average

int rssiReadings[numReadings]; // Array to store RSSI readings
int rssiIndex = 0; // Index to keep track of the current RSSI reading

class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    String address = advertisedDevice.getAddress().toString().c_str();
    int rssi = advertisedDevice.getRSSI();
    float distance = calculateDistance(getAverageRSSI(rssi));
    String deviceName = advertisedDevice.getName().c_str();
    String deviceIdentifier = "CustomID_" + address; 
    Serial.println("-------------------------");
    Serial.println("Identificador DETECTADO");
    Serial.println("Nome do dispositivo: ");
    Serial.println(deviceName);
    Serial.println("RSSI: ");
    Serial.println(rssi);
    Serial.println("Distance: ");
    Serial.println(distance);
    // Publish both identifier and distance
    String message = "Nome do dispositivo = " + String(deviceName)+ "\n" + "MAC ADDRESS = " + deviceIdentifier + "\n" + "Distancia = " + String(distance) + "\n";
    client.publish(outTopic, message.c_str(), true);
  }

  float calculateDistance(int rssi) {
    return pow(10, ((rssi_ref - rssi) / (10.0 * N)));
  }

  int getAverageRSSI(int rssi) {
    int total = 0;
    rssiReadings[rssiIndex] = rssi;
    rssiIndex = (rssiIndex + 1) % numReadings;

    for (int i = 0; i < numReadings; i++) {
      total += rssiReadings[i];
    }

    return total / numReadings;
  }
};

// --- Scan Bluetooth --- 
void scanBLE() {
  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true);
  BLEScanResults foundDevices = pBLEScan->start(scanTime);
}

// --- Conecta ao WiFi ---
void conectaWifi() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("Wifi connected");
  Serial.println("ip address: ");
  Serial.println(WiFi.localIP());
}

// --- Conecta ao MQTT ---
void conectaMQTT() {
  while(!client.connected()){
    client.connect(clientId, brokerUser, brokerPass);
  }
}

void loop() {
  if (!client.connected()) {
    conectaMQTT();
  }
  scanBLE();
  delay(2000);
}
