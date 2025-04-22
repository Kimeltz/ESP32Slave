#include <Arduino.h>
#include "SignalProcessing.h"
#include <math.h>

movingAverage::movingAverage(int bufferSize)
    :_size(bufferSize), _index(0), _count(0), _sum(0.0f), _buffer(nullptr) {}

movingAverage::~movingAverage() {
    if (_buffer) free(_buffer);
}

bool movingAverage::init()
{
    if (_buffer) free(_buffer);
    _buffer = (float*) calloc(_size, sizeof(float));
    return (_buffer != nullptr);
}

float movingAverage::update(float newData)
{
    if (!_buffer) return 0.0f;
    if(_count == _size) _sum -= _buffer[_index];
    else _count++;

    _buffer[_index] = newData;
    _sum += newData;
    _index = (_index + 1) % _size; 
    return _sum / _count;
}

float* movingAverage::getBuffer()
{
    return _buffer; 
}

float movingAverage::getValue()
{
    if (_count == 0) return 0.0f;
    return _sum / _count;
}

int movingAverage::getSize()
{
    return _size;
}

int movingAverage::getCount()
{
    return _count;
}

float roundToNearest005(float val)
{
    return round(val * 20) / 20;
}
float* roundToNearest005(float* val, int size)
{
    for(int i = 0; i < size; i++)
    {
        val[i] = round(val[i] * 20) / 20;
    }
    return val;
}