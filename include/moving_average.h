#ifndef MOVING_AVERAGE_H
#define MOVING_AVERAGE_H
#include <Arduino.h>
struct MovingAverage {
    float* buffer;
    int size;
    int index;
    int count;
    float sum;
  };
  
  inline void initMovingAverage(MovingAverage &ma, int bufferSize) {
    ma.buffer = (float*)calloc(bufferSize, sizeof(float));
    ma.size = bufferSize;
    ma.index = 0;
    ma.count = 0;
    ma.sum = 0;
  }
  
  // === Update ===
  inline float updateMovingAverage(MovingAverage &ma, float newValue) {
    if (ma.count == ma.size) {
      ma.sum -= ma.buffer[ma.index];
    } else {
      ma.count++;
    }
  
    ma.buffer[ma.index] = newValue;
    ma.sum += newValue;
    ma.index = (ma.index + 1) % ma.size;
  
    return ma.sum / ma.count;
  }
  
  // === Free memory ===
  inline void freeMovingAverage(MovingAverage &ma) {
    if (ma.buffer != nullptr) {
      free(ma.buffer);
      ma.buffer = nullptr;
    }
  }
#endif