#include <Arduino.h>
#include <SignalProcessing.h>
#define BUFFER_SIZE 10

movingAverage ma(BUFFER_SIZE); //movingAverage nama(bufferSize);

void setup() {
    Serial.begin(9600);
    // Inisialisasi moving average
    ma.init();
}

void loop() {
    // Tambahkan nilai yang mau di masukkan
    int a = 15;   
    // Update
    float nilaiMA = ma.update(a);
    // Ambil semua data
    float* semuaData = ma.getBuffer();
    // Tampilkan di Serial Monitor
    Serial.println("Nilai yang sudah dirata rata : " + String(nilaiMA));
    Serial.println("Semua Data :");
    for(int i = 0; i < ma.getCount(); i++)
    {
        Serial.println(semuaData[i]);
    }
}