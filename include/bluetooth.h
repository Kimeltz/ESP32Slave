#ifndef BLUETOOTH_H
#define BLUETOOTH_H
#include <BluetoothSerial.h>
class BluetoothComm {
    private:
      BluetoothSerial btSerial;
      String deviceName;
    
    public:
        BluetoothComm(const String& name = "ESP32-BT") : deviceName(name) {}

        // Init bluetooth
        void begin() {
          if (!btSerial.begin(deviceName)) {
              Serial.println("❌ Bluetooth init gagal");
          } else {
              Serial.printf("📡 Bluetooth aktif sebagai \"%s\"\n", deviceName.c_str());
          }
        }

        // Kirim data
        void send(const String& data) {
            btSerial.println(data);
        }

        void send(int data) {
            btSerial.println(data);
        }

        // Read data
        bool available() {
            return btSerial.available();
        }

        String readLine() {
        String incoming = "";
        while (btSerial.available()) {
                char c = btSerial.read();
                if (c == '\n') break;
                incoming += c;
            }
            return incoming;
        }

        BluetoothSerial& getSerial() {
            return btSerial;
        }
};


#endif

/*
*** Example ***

#include "bluetooth.h"

BluetoothComm bt("FireMonitor-ESP");

void setup() {
  Serial.begin(115200);
  bt.begin(); // Aktifkan Bluetooth dengan nama "FireMonitor-ESP"
}

void loop() {
  if (bt.available()) {
    String msg = bt.readLine();
    Serial.print("📥 BT Received: ");
    Serial.println(msg);
  }

  bt.send("🔥 Status aman");
  delay(2000);
}

*/