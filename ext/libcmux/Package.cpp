#include <cmux/Package.h>

namespace CMUX {

Package::Package() : m_hasError(false) {

}

Package::~Package() {

}

void Package::clear() {
  m_storage.clear();
  m_hasError = false;
}

std::vector<unsigned char> Package::marshal() {
  std::vector<unsigned char> data;
  data.insert(data.begin(), m_storage.begin(), m_storage.end());
  clear();

  return data;
}

void Package::demarshal(const std::vector<unsigned char> &data) {
  clear();
  m_storage.insert(m_storage.begin(), data.begin(), data.end());
}

void Package::readStream(void *data, size_t data_size) {
  if(hasError() || m_storage.size() < data_size) {
    raise();

    return;
  }

  unsigned char *cdata = (unsigned char *) data;

  std::copy(m_storage.begin(), m_storage.begin() + data_size, cdata);
  m_storage.erase(m_storage.begin(), m_storage.begin() + data_size);
}

void Package::writeStream(const void *data, size_t data_size) {
  const unsigned char *cdata = (const unsigned char *) data;

  m_storage.insert(m_storage.end(), cdata, cdata + data_size);
}


std::string Package::readString() {
  int length = readInt();
  std::string string;
  string.resize(length);
  readStream(&string[0], length);

  return string;
}

void Package::writeString(const std::string &string) {
  writeInt(string.size());
  writeStream(&string[0], string.size());
}

}
