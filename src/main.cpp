#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <MQ2.h>
#include <SignalProcessing.h>
#include "rs485_comm.h"
#include "eeprom_storage.h"
#include <MQ7.h>

// === PIN SETUP ===
#define MQ2_PIN 34       // Analog input for MQ-2
#define MQ7_PIN 35       // Analog input for MQ-7
#define ONE_WIRE_BUS 4   // DS18B20 data pin
#define RS485_DE_PIN 32  // RS485 DE pin
#define RS485_RE_PIN 33  // RS485 RE pin
#define RS485_TX_PIN 17  // RS485 TX pin
#define RS485_RX_PIN 16  // RS485 RX pin
#define BME280_SDA 21   // BME280 SDA pin
#define BME280_SCL 22   // BME280 SCL pin

// === Other Define ===
#define intervalDataRead 500
#define expectedSensorCount 4
#define DATA_BUFFER_SIZE 25
#define DATA_READ_PER_INTERVAL 2

// === MQ2 ===
int mq2Value = 0;
MQ2 mq2(MQ2_PIN);
movingAverage lpgValue(DATA_BUFFER_SIZE);
movingAverage coValue(DATA_BUFFER_SIZE);
movingAverage smokeValue(DATA_BUFFER_SIZE);

// === MQ7 ===
int mq7Value = 0;
MQ7 mq7(MQ7_PIN, 5.0);

// === BME280 ===
Adafruit_BME280 bme;
#define SEALEVELPRESSURE_HPA (1013.25)
movingAverage bmeHumidity(DATA_BUFFER_SIZE);
movingAverage bmePressure(DATA_BUFFER_SIZE);

// === DS18B20 ===
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature ds18b20(&oneWire);
DeviceAddress ds18b20Addresses[4];
int actualSensorCount = 0;
float** ds18b20Temp;

// === EEPROM ===
EEPROMStorage memory;
#define MAGIC_ADDR 0
#define MAGIC_NUMBER 0xDEADBEEF
#define ID_ADDR 4
String sensorID;

// === Global Variable ===
uint64_t lastDataRead = 0;
int dataIndex = 0;

// === MAX485 ===
#define RS485_BAUD 9600
RS485Comm rs485(Serial1, RS485_DE_PIN, RS485_RE_PIN, RS485_BAUD); // DE = GPIO32, RE = GPIO33

// === Funcs ===
void printAddress(DeviceAddress deviceAddress);
void sensorInit();
void readData();
void sendDataRS485();
void setNewID();
bool idCheck();
float avgArray(float** arr, int size);
int classifyCondition();


void setup() {
  Serial.begin(115200);
  memory.begin();
  delay(1000);

  // === Start DS18B20 ===
  ds18b20.begin();
  actualSensorCount = ds18b20.getDeviceCount();

  Serial.printf("🔍 Mendeteksi %d DS18B20 sensor...\n", actualSensorCount);

  for (int i = 0; i < actualSensorCount && i < expectedSensorCount; i++) {
    if (ds18b20.getAddress(ds18b20Addresses[i], i)) {
      Serial.print("Sensor ");
      Serial.print(i);
      Serial.print(" address: ");
      printAddress(ds18b20Addresses[i]);
    } else {
      Serial.printf("❌ Gagal membaca address sensor %d\n", i);
    }
  }

  // === Start BME280 ===
  if (!bme.begin(0x76)) { // 0x76 or 0x77 depending on your module
    Serial.println("❌ BME280 not found. Check wiring!");
    while (1);
  }

  // === Start MQ2 ===
  mq2.begin();

  // === Setup analog inputs ===
  analogReadResolution(10); // ESP32 uses 12-bit ADC

  rs485.begin();
  sensorInit();
  if(idCheck())
  {
    String id = memory.readString(4);
    Serial.printf("ID yang ada: %s\n", id);
  }
  else
  {
    setNewID();
  }
}

void loop() {
  readData();
  delay(500);
  sendDataRS485();
}


void readData()
{
  
  if(intervalDataRead < millis() - lastDataRead)
  {
    for(int i = 0; i < DATA_READ_PER_INTERVAL; i++)
    {
      // === Read MQ Sensors ===
      // lpgValue.update(mq2.readLPG());
      // coValue.update(mq2.readCO());
      // smokeValue.update(mq2.readSmoke());
      mq2Value = analogRead(MQ2_PIN);
      mq7Value = mq7.getPPM();

      // === Read DS18B20 ===
      ds18b20.requestTemperatures();
      delay(50);
      for (int j = 0; j < actualSensorCount; j++) {
        if(ds18b20Temp[j] != NULL)
        {
          ds18b20Temp[j][dataIndex] = ds18b20.getTempC(ds18b20Addresses[j]);
        }
      }

      // === Read BME280 ===
      bmeHumidity.update(bme.readHumidity());
      bmePressure.update(bme.readPressure() / 100.0F);

      // === Output to Serial ===
      Serial.println("==== Sensor Readings ====");
      Serial.printf("MQ2 LPG     : %.2f ppm\n", mq2.readLPG());
      Serial.printf("MQ2 CO      : %.2f ppm\n", mq2.readCO());
      Serial.printf("MQ2 Smoke   : %.2f ppm\n", mq2.readSmoke());
      Serial.printf("MQ7 CO      : %d\n", mq7Value);
      // Serial.println("==== DS18B20 Temperatures ====");
      // for (int j = 0; j < actualSensorCount; j++) {
      //   if(ds18b20Temp[j] != NULL)
      //   {
      //     Serial.printf("DS18B20 %d : %.2f °C\n", ds18b20Addresses[j], ds18b20Temp[j][dataIndex]);
      //   }
      // }
      // Serial.println("==== BME280 Readings ====");
      // Serial.printf("BME280 Humid : %.2f %%\n", bmeHumidity.getValue());
      // Serial.printf("BME280 Press : %.2f hPa\n", bmePressure.getValue());
      
      Serial.println("==== Kondisi ====");
      int condition = classifyCondition();
      if (condition == 3) {
        Serial.println("🔥 Kebakaran terdeteksi!");
      } else if (condition == 2) {
        Serial.println("🚨 Bahaya terdeteksi!");
      } else if (condition == 1) {
        Serial.println("⚠️ Waspada!");
      } else {
        Serial.println("✅ Ruangan Aman");
      }
      Serial.println("==========================\n");

      dataIndex = (dataIndex + 1) % DATA_BUFFER_SIZE;
      delay(50);
    }
    lastDataRead = millis();
  }
}

void printAddress(DeviceAddress deviceAddress) {
  for (uint8_t i = 0; i < 8; i++) {
    if (deviceAddress[i] < 16) Serial.print("0");
    Serial.print(deviceAddress[i], HEX);
  }
  Serial.println();
}

void sensorInit()
{
  // === Call 1D for MQ2 ===
  if(!lpgValue.init() || !coValue.init() || !smokeValue.init())
  {
    Serial.println("❌ Gagal mengalokasikan memori!");
    while (1);
  }

  // === Call 1D for BME280 ===
  if(!bmeHumidity.init() || !bmePressure.init())
  {
    Serial.println("❌ Gagal mengalokasikan memori!");
    while (1);
  }
  // === Call 2D for DS18B20 ===
  ds18b20Temp = (float**)calloc(actualSensorCount, sizeof(float*));
  if (!ds18b20Temp) {
    Serial.println("❌ Gagal mengalokasikan memori!");
    while (1);
  }
  for(int i = 0; i < actualSensorCount; i++)
  {
    ds18b20Temp[i] = (float*)calloc(DATA_BUFFER_SIZE, sizeof(float));
    if (!ds18b20Temp[i]) {
      Serial.printf("❌ Gagal alokasi untuk ds18b20Temp[%d]\n", i);
      while (1);
    }    
  }
}

void sendDataRS485()
{
  String data = "";

  // === ID sensor ===
  data += "SID:" + sensorID + ";";

  // // === Sensor MQ2 ===
  // data += "LPG:" + String(lpgValue.getValue(), 2) + ";";
  // data += "CO:" + String(coValue.getValue(), 2) + ";";
  // data += "SMK:" + String(smokeValue.getValue(), 2) + ";";
  data += "GAS:" + String(mq2Value) + ";";
  data += "CO:" + String(mq7Value) + ";";

  // === Sensor DS18B20 ===
  for (int i = 0; i < actualSensorCount; i++) {
    if(ds18b20Temp[i] != NULL)
    {
      data += "TEMP" + String(i+1) + ":" + String(ds18b20Temp[i][dataIndex], 2) + ";";
    }
  }

  // === BME280 ===
  data += "HUM:" + String(bmeHumidity.getValue(), 2) + ";";
  data += "PRS:" + String(bmePressure.getValue(), 2) + "\n"; 

  // === Kirim ke master via RS485 ===
  rs485.send(data);
  Serial.print("📤 Kirim RS485: ");
  Serial.print(data);
}

bool idCheck()
{
  uint32_t magic = memory.read<uint32_t>(MAGIC_ADDR);
  return magic == MAGIC_NUMBER;
}

void setNewID()
{
  uint32_t randPart = esp_random();
  uint32_t timePart = millis();
  sensorID = String(randPart, HEX) + String(timePart, HEX);
  memory.write<uint32_t>(MAGIC_ADDR, MAGIC_NUMBER);
  memory.writeString(4, sensorID);
  Serial.printf("ID baru: %s\n", sensorID);
}

int classifyCondition() {
  float avgTemp = avgArray(ds18b20Temp, actualSensorCount);
  int score = 0;
  if (avgTemp > 50) score++;
  if (bmeHumidity.getValue() < 30) score++;
  if (mq2Value > 400) score++;
  if (mq7Value > 20) score+= mq7Value/20;
  for(int i = 0; i < actualSensorCount; i++) 
  {
    if(abs(ds18b20Temp[i][dataIndex]-avgTemp) > 5)
    {
      score++;
      break;
    }
  }

  if (score >= 4) return 3; // Kebakaran
  else if (score == 3) return 2; // Bahaya
  else if (score == 2) return 1; // Waspada
  else return 0; // Normal
}

float avgArray(float** arr, int size)
{
  float sum = 0;
  for(int i = 0; i < size; i++)
  {
    sum += arr[i][dataIndex];
  }
  return sum / size;
}