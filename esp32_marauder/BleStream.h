#ifndef BleStream_h
#define BleStream_h

#include <Arduino.h>
#include <BLECharacteristic.h>
#include <Print.h>

// Use a buffer size that is safe for BLE MTU. 512 is the max, but some clients might be smaller.
// Let's stick to a reasonable default. Most terminals send line by line anyway.
#define BLE_STREAM_BUFFER_SIZE 128

class BleStream : public Print {
public:
  // Constructor takes the characteristic to write to
  BleStream(BLECharacteristic* characteristic) : _characteristic(characteristic) {
    if (_characteristic == nullptr) {
        // Handle error case if needed
    }
    _buffer.reserve(BLE_STREAM_BUFFER_SIZE);
  }

  ~BleStream() {
    flush();
  }

  // The core `write` function that all `Print` methods (like .print, .println) will call
  virtual size_t write(uint8_t c) {
    if (_characteristic == nullptr) return 0;

    _buffer += (char)c;
    // Send data when a newline is encountered or the buffer is getting full
    if (c == '\n' || _buffer.length() >= BLE_STREAM_BUFFER_SIZE) {
      flush();
    }
    return 1;
  }

  // A more efficient `write` for blocks of data
  virtual size_t write(const uint8_t *buffer, size_t size) {
    if (_characteristic == nullptr) return 0;

    // Add data to buffer in chunks if necessary
    for (size_t i = 0; i < size; i++) {
      write(buffer[i]); // Use the single-byte write to handle flushing logic
    }
    return size;
  }

  // Method to send any remaining data in the buffer
  void flush() {
    if (_buffer.length() > 0 && _characteristic != nullptr) {
      _characteristic->setValue(_buffer.c_str());
      _characteristic->notify();
      _buffer = ""; // Clear the buffer
    }
  }

private:
  BLECharacteristic* _characteristic;
  String _buffer;
};

#endif