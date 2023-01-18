#ifndef ESP32_OTA_UTILITY_H_
#define ESP32_OTA_UTILITY_H_

/**************************************************************************************
   INCLUDE
 **************************************************************************************/
#include <string>
#include <algorithm>
#include <cctype>
#include <functional>
#include <iostream>

struct URI {
  public:
    URI(const std::string& url_s) {
      this->parse(url_s);
    }
    std::string protocol_, host_, path_, query_;
  private:
    void parse(const std::string& url_s);
};

union HeaderVersion
{
  struct __attribute__((packed))
  {
    uint32_t header_version    :  6;
    uint32_t compression       :  1;
    uint32_t signature         :  1;
    uint32_t spare             :  4;
    uint32_t payload_target    :  4;
    uint32_t payload_major     :  8;
    uint32_t payload_minor     :  8;
    uint32_t payload_patch     :  8;
    uint32_t payload_build_num : 24;
  } field;
  uint8_t buf[sizeof(field)];
  static_assert(sizeof(buf) == 8, "Error: sizeof(HEADER.VERSION) != 8");
};

union OtaHeader
{
  struct __attribute__((packed))
  {
    uint32_t len;
    uint32_t crc32;
    uint32_t magic_number;
    HeaderVersion hdr_version;
  } header;
  uint8_t buf[sizeof(header)];
  static_assert(sizeof(buf) == 20, "Error: sizeof(HEADER) != 20");
};

uint32_t crc_update(uint32_t crc, const void * data, size_t data_len);

#endif /* ESP32_OTA_UTILITY_H_ */
