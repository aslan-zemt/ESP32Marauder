#ifndef MultiStream_h
#define MultiStream_h

#include "Print.h"
#include <list>

class MultiStream : public Print {
  public:
    MultiStream() {}

    // Add a stream to the list of outputs
    void add(Print* stream) {
      if (stream) {
        _streams.push_back(stream);
      }
    }

    // The core `write` function that all `Print` methods call
    virtual size_t write(uint8_t c) {
      for (Print* stream : _streams) {
        stream->write(c);
      }
      return 1;
    }

    // A more efficient `write` for blocks of data
    virtual size_t write(const uint8_t *buffer, size_t size) {
      for (Print* stream : _streams) {
        stream->write(buffer, size);
      }
      return size;
    }

    // This is needed to properly override all virtual methods
    using Print::write;

  private:
    std::list<Print*> _streams;
};

#endif
