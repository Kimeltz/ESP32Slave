#ifndef EEPROM_STORAGE_H
#define EEPROM_STORAGE_H

#include <EEPROM.h>

// === Atur ukuran EEPROM (ESP32 max 512 bytes default, bisa lebih besar jika diatur saat init) ===
#define EEPROM_SIZE 512

class EEPROMStorage {
  public:
    EEPROMStorage() {}

    // Inisialisasi EEPROM
    void begin() {
      if (!EEPROM.begin(EEPROM_SIZE)) {
        Serial.println("❌ Gagal inisialisasi EEPROM");
      } else {
        Serial.println("💾 EEPROM siap");
      }
    }

    // Tulis data bertipe primitif (int, float, bool, byte, dll)
    template <typename T>
    void write(int address, const T& value) {
      EEPROM.put(address, value);
      EEPROM.commit(); // Simpan ke flash (penting di ESP32)
    }

    // Baca data
    template <typename T>
    T read(int address) {
      T value;
      EEPROM.get(address, value);
      return value;
    }

    // Tulis string
    void writeString(int address, const String& str) {
      int len = str.length();
      EEPROM.write(address, len);
      for (int i = 0; i < len; i++) {
        EEPROM.write(address + 1 + i, str[i]);
      }
      EEPROM.commit();
    }

    // Baca string
    String readString(int address) {
      int len = EEPROM.read(address);
      char buf[len + 1];
      for (int i = 0; i < len; i++) {
        buf[i] = EEPROM.read(address + 1 + i);
      }
      buf[len] = '\0';
      return String(buf);
    }

    // Hapus data (tulis 0xFF)
    void clear(int startAddress, int length) {
      for (int i = startAddress; i < startAddress + length; i++) {
        EEPROM.write(i, 0xFF);
      }
      EEPROM.commit();
    }
};

#endif

/*
Example

#include "eeprom_storage.h"

EEPROMStorage memory;

void setup() {
  Serial.begin(115200);
  memory.begin();

  // Simpan nilai
  memory.write<int>(0, 1234);
  memory.write<float>(10, 36.5);
  memory.writeString(50, "ESP32 Rocks");

  // Baca kembali
  int i = memory.read<int>(0);
  float f = memory.read<float>(10);
  String s = memory.readString(50);

  Serial.printf("📦 Int: %d | Float: %.2f | String: %s\n", i, f, s.c_str());
}

void loop() {
  // ...
}
*/