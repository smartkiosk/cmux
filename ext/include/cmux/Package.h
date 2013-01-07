#ifndef __CMUX__PACKAGE__H__
#define __CMUX__PACKAGE__H__

#include <stdint.h>
#include <string.h>

#include <vector>
#include <deque>
#include <string>

namespace CMUX {

struct MessageHeader {
  uint16_t    length;
};

class Package {
public:
  Package();
  ~Package();

  inline bool hasError() {
    return m_hasError;
  }

  void clear();
  std::vector<unsigned char> marshal();
  void demarshal(const std::vector<unsigned char> &data);

  inline char readByte() { return readIntegral<char>(); }
  inline short readShort() { return readIntegral<short>(); }
  inline int readInt() { return readIntegral<int>(); }

  inline void writeByte(char value) { writeIntegral(value); }
  inline void writeShort(short value) { writeIntegral(value); }
  inline void writeInt(int value) { writeIntegral(value); }

  std::string readString();
  void writeString(const std::string &string);

  void readStream(void *data, size_t data_size);
  void writeStream(const void *data, size_t data_size);

private:
  template<typename T> inline T readIntegral() {
    T value;
    readStream(&value, sizeof(value));

    return value;
  }

  template<typename T> inline void writeIntegral(T value) {
    writeStream(&value, sizeof(value));
  }

  inline void raise() {
    m_hasError = true;
  }

  bool m_hasError;
  std::deque<unsigned char> m_storage;
};

}

#endif
