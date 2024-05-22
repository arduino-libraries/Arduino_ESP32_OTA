/*
   This file is part of Arduino_ESP32_OTA.

   Copyright 2020 ARDUINO SA (http://www.arduino.cc/)

   This software is released under the GNU General Public License version 3,
   which covers the main part of arduino-cli.
   The terms of this license can be found at:
   https://www.gnu.org/licenses/gpl-3.0.en.html

   You can be released from the requirements of the above licenses by purchasing
   a commercial license. Buying such a license is mandatory if you want to modify or
   otherwise use the software for commercial activities involving the Arduino
   software without disclosing the source code of your own applications. To purchase
   a commercial license, send an email to license@arduino.cc.
*/

#ifndef ARDUINO_ESP32_OTA_H_
#define ARDUINO_ESP32_OTA_H_

/******************************************************************************
 * INCLUDE
 ******************************************************************************/

#include <Arduino_DebugUtils.h>
#include <WiFiClientSecure.h>
#include "decompress/utility.h"
#include "decompress/lzss.h"
#include <ArduinoHttpClient.h>
#include <URLParser.h>
#include <stdint.h>

/******************************************************************************
   DEFINES
 ******************************************************************************/
#if defined (ARDUINO_NANO_ESP32)
  #define ARDUINO_ESP32_OTA_MAGIC 0x23410070
#else
  #define ARDUINO_ESP32_OTA_MAGIC 0x45535033
#endif
/******************************************************************************
   CONSTANTS
 ******************************************************************************/

static uint32_t const ARDUINO_ESP32_OTA_HTTP_HEADER_RECEIVE_TIMEOUT_ms = 10000;
static uint32_t const ARDUINO_ESP32_OTA_BINARY_HEADER_RECEIVE_TIMEOUT_ms = 10000;
static uint32_t const ARDUINO_ESP32_OTA_BINARY_BYTE_RECEIVE_TIMEOUT_ms = 2000;

/******************************************************************************
 * CLASS DECLARATION
 ******************************************************************************/

class Arduino_ESP32_OTA
{

public:

  enum class Error : int
  {
    None                 =  0,
    NoOtaStorage         = -2,
    OtaStorageInit       = -3,
    OtaStorageEnd        = -4,
    UrlParseError        = -5,
    ServerConnectError   = -6,
    HttpHeaderError      = -7,
    ParseHttpHeader      = -8,
    OtaHeaderLength      = -9,
    OtaHeaderCrc         = -10,
    OtaHeaderMagicNumber = -11,
    OtaDownload          = -12,
    OtaHeaderTimeout     = -13,
    HttpResponse         = -14
  };

  enum OTADownloadState: uint8_t {
    OtaDownloadHeader,
    OtaDownloadFile,
    OtaDownloadCompleted,
    OtaDownloadMagicNumberMismatch,
    OtaDownloadError
  };

           Arduino_ESP32_OTA();
  virtual ~Arduino_ESP32_OTA();

  Arduino_ESP32_OTA::Error begin(uint32_t magic = ARDUINO_ESP32_OTA_MAGIC);
  void setMagic(uint32_t magic);
  void setCACert(const char *rootCA);
  void setCACertBundle(const uint8_t * bundle);

  // blocking version for the download
  // returns the size of the downloaded binary
  int download(const char * ota_url);

  // start a download in a non blocking fashion
  // call downloadPoll, until it returns OtaDownloadCompleted
  // returns the value in content-length http header
  int startDownload(const char * ota_url);

  // This function is used to make the download progress.
  // it returns 0, if the download is in progress
  // it returns 1, if the download is completed
  // it returns <0 if an error occurred, following Error enum values
  virtual int downloadPoll();

  // this function is used to get the progress of the download
  // it returns a positive value when the download is progressing correctly
  // it returns a negative value on error following Error enum values
  int downloadProgress();

  // this function is used to get the size of the download
  // 0 if no download is in progress
  size_t downloadSize();

  virtual void write_byte_to_flash(uint8_t data);
  Arduino_ESP32_OTA::Error verify();
  Arduino_ESP32_OTA::Error update();
  void reset();
  static bool isCapable();

protected:
  struct Context {
    Context(
      const char* url,
      std::function<void(uint8_t)> putc);

    ~Context();

    char*             url;
    ParsedUrl         parsed_url;
    OtaHeader         header;
    OTADownloadState  downloadState;
    uint32_t          calculatedCrc32;
    uint32_t          headerCopiedBytes;
    uint32_t          downloadedSize;
    uint32_t          writtenBytes;

    // If an error occurred during download it is reported in this field
    Error             error;

    // LZSS decoder
    LZSSDecoder       decoder;

    const size_t buf_len = 64;
    uint8_t buffer[64];
  } *_context;

private:
  Client * _client;
  HttpClient* _http_client;
  const char * _ca_cert;
  const uint8_t * _ca_cert_bundle;
  uint32_t _magic;

  void clean();
};

#endif /* ARDUINO_ESP32_OTA_H_ */
