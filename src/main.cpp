#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// === PIN SETUP ===
#define MQ2_PIN 36       // Analog input for MQ-2
#define MQ7_PIN 39       // Analog input for MQ-7
#define ONE_WIRE_BUS 4   // DS18B20 data pin

// === Other Define ===
#define intervalDataRead 500
#define expectedSensorCount 4
#define DATA_BUFFER_SIZE 100

// === BME280 ===
Adafruit_BME280 bme;
#define SEALEVELPRESSURE_HPA (1013.25)
float* bmeHumidity;
float* bmePressure;

// === DS18B20 ===
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature ds18b20(&oneWire);
DeviceAddress ds18b20Addresses[4];
int actualSensorCount = 0;
float** ds18b20Temp;

// === Global Variable ===
uint64_t lastDataRead = 0;

// === Funcs ===
void printAddress(DeviceAddress deviceAddress);
void init();
void readData();
void freeParse(char** buffer, int maxBuffer);
char** parse(char* input, const char* delim, int maxBuffer = 5);


void setup() {
  Serial.begin(115200);
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

  // === Setup analog inputs ===
  analogReadResolution(12); // ESP32 uses 12-bit ADC

  init();
}

void loop() {
  readData();
}


void readData()
{
  static int index = 0;
  if(intervalDataRead < millis() - lastDataRead)
  {
    for(int i = 0; i < 10; i++)
    {
      // === Read MQ Sensors ===
      int mq2Value = analogRead(MQ2_PIN);
      int mq7Value = analogRead(MQ7_PIN);

      // === Read DS18B20 ===
      ds18b20.requestTemperatures();
      Serial.println("==== DS18B20 Temperatures ====");
      for (int i = 0; i < actualSensorCount; i++) {
        if(ds18b20Temp[i] != NULL)
        {
          ds18b20Temp[i][index] = ds18b20.getTempC(ds18b20Addresses[i]);
        }
      }

      // === Read BME280 ===
      bmeHumidity[index] = bme.readHumidity();
      bmePressure[index] = bme.readPressure() / 100.0F;

      // === Output to Serial ===
      Serial.println("==== Sensor Readings ====");
      Serial.printf("MQ-2 (Analog): %d\n", mq2Value);
      Serial.printf("MQ-7 (Analog): %d\n", mq7Value);
      Serial.printf("BME280 Humid : %.2f %%\n", bmeHumidity[index]);
      Serial.printf("BME280 Press : %.2f hPa\n", bmePressure[index]);
      Serial.println("==========================\n");

      index = (index + 1) % DATA_BUFFER_SIZE;
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

void init()
{
  bmeHumidity = (float*)calloc(DATA_BUFFER_SIZE, sizeof(float));
  bmePressure = (float*)calloc(DATA_BUFFER_SIZE, sizeof(float));
  // === Call 2D for DS18B20 ===
  ds18b20Temp = (float**)calloc(actualSensorCount, sizeof(float*));
  if (!bmeHumidity || !bmePressure || !ds18b20Temp) {
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