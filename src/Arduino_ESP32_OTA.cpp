/*
   This file is part of Arduino_ESP32_OTA.

   Copyright 2022 ARDUINO SA (http://www.arduino.cc/)

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

/******************************************************************************
   INCLUDE
 ******************************************************************************/

#include <Update.h>
#include "Arduino_ESP32_OTA.h"
#include "tls/amazon_root_ca.h"
#include "esp_ota_ops.h"

/******************************************************************************
   CTOR/DTOR
 ******************************************************************************/

Arduino_ESP32_OTA::Arduino_ESP32_OTA()
: _context(nullptr)
, _client(nullptr)
, _http_client(nullptr)
,_ca_cert{amazon_root_ca}
,_ca_cert_bundle{nullptr}
,_magic(0)
{

}

Arduino_ESP32_OTA::~Arduino_ESP32_OTA(){
  clean();
}

/******************************************************************************
   PUBLIC MEMBER FUNCTIONS
 ******************************************************************************/

Arduino_ESP32_OTA::Error Arduino_ESP32_OTA::begin(uint32_t magic)
{
  /* ... configure board Magic number */
  setMagic(magic);

  if(!isCapable()) {
    DEBUG_ERROR("%s: board is not capable to perform OTA", __FUNCTION__);
    return Error::NoOtaStorage;
  }

  if(Update.isRunning()) {
    Update.abort();
    DEBUG_DEBUG("%s: Aborting running update", __FUNCTION__);
  }

  if(!Update.begin(UPDATE_SIZE_UNKNOWN)) {
    DEBUG_ERROR("%s: failed to initialize flash update", __FUNCTION__);
    return Error::OtaStorageInit;
  }
  return Error::None;
}

void Arduino_ESP32_OTA::setCACert (const char *rootCA)
{
  if(rootCA != nullptr) {
    _ca_cert = rootCA;
  }
}

void Arduino_ESP32_OTA::setCACertBundle (const uint8_t * bundle)
{
  if(bundle != nullptr) {
    _ca_cert_bundle = bundle;
  }
}

void Arduino_ESP32_OTA::setMagic(uint32_t magic)
{
  _magic = magic;
}

void Arduino_ESP32_OTA::write_byte_to_flash(uint8_t data)
{
  Update.write(&data, 1);
}

int Arduino_ESP32_OTA::startDownload(const char * ota_url)
{
  assert(_context == nullptr);
  assert(_client == nullptr);
  assert(_http_client == nullptr);

  Error err = Error::None;
  int statusCode;
  int res;

  _context = new Context(ota_url, [this](uint8_t data){
    _context->writtenBytes++;
    write_byte_to_flash(data);
  });

  if(strcmp(_context->parsed_url.schema(), "http") == 0) {
    _client = new WiFiClient();
  } else if(strcmp(_context->parsed_url.schema(), "https") == 0) {
    _client = new WiFiClientSecure();
    if (_ca_cert != nullptr) {
      static_cast<WiFiClientSecure*>(_client)->setCACert(_ca_cert);
    } else if (_ca_cert_bundle != nullptr) {
      static_cast<WiFiClientSecure*>(_client)->setCACertBundle(_ca_cert_bundle);
    } else {
      DEBUG_VERBOSE("%s: CA not configured for download client");
    }
  } else {
    err = Error::UrlParseError;
    goto exit;
  }

  _http_client = new HttpClient(*_client, _context->parsed_url.host(), _context->parsed_url.port());

  res= _http_client->get(_context->parsed_url.path());

  if(res == HTTP_ERROR_CONNECTION_FAILED) {
    DEBUG_VERBOSE("OTA ERROR: http client error connecting to server \"%s:%d\"",
      _context->parsed_url.host(), _context->parsed_url.port());
    err = Error::ServerConnectError;
    goto exit;
  } else if(res == HTTP_ERROR_TIMED_OUT) {
    DEBUG_VERBOSE("OTA ERROR: http client timeout \"%s\"", _context->url);
    err = Error::OtaHeaderTimeout;
    goto exit;
  } else if(res != HTTP_SUCCESS) {
    DEBUG_VERBOSE("OTA ERROR: http client returned %d on  get \"%s\"", res, _context->url);
    err = Error::OtaDownload;
    goto exit;
  }

  statusCode = _http_client->responseStatusCode();

  if(statusCode != 200) {
    DEBUG_VERBOSE("OTA ERROR: get response on \"%s\" returned status %d", _context->url, statusCode);
    err = Error::HttpResponse;
    goto exit;
  }

  // The following call is required to save the header value , keep it
  if(_http_client->contentLength() == HttpClient::kNoContentLengthHeader) {
    DEBUG_VERBOSE("OTA ERROR: the response header doesn't contain \"ContentLength\" field");
    err = Error::HttpHeaderError;
    goto exit;
  }

exit:
  if(err != Error::None) {
    clean();
    return static_cast<int>(err);
  } else {
    return _http_client->contentLength();
  }
}

int Arduino_ESP32_OTA::downloadPoll()
{
  int http_res =  static_cast<int>(Error::None);;
  int res = 0;

  if(_http_client->available() == 0) {
    goto exit;
  }

  http_res = _http_client->read(_context->buffer, _context->buf_len);

  if(http_res < 0) {
    DEBUG_VERBOSE("OTA ERROR: Download read error %d", http_res);
    res = static_cast<int>(Error::OtaDownload);
    goto exit;
  }

  for(uint8_t* cursor=(uint8_t*)_context->buffer; cursor<_context->buffer+http_res; ) {
    switch(_context->downloadState) {
    case OtaDownloadHeader: {
      uint32_t copied = http_res < sizeof(_context->header.buf) ? http_res : sizeof(_context->header.buf);
      memcpy(_context->header.buf+_context->headerCopiedBytes, _context->buffer, copied);
      cursor += copied;
      _context->headerCopiedBytes += copied;

      // when finished go to next state
      if(sizeof(_context->header.buf) == _context->headerCopiedBytes) {
        _context->downloadState = OtaDownloadFile;

        _context->calculatedCrc32 = crc_update(
          _context->calculatedCrc32,
          &(_context->header.header.magic_number),
          sizeof(_context->header) - offsetof(OtaHeader, header.magic_number)
        );

        if(_context->header.header.magic_number != _magic) {
          _context->downloadState = OtaDownloadMagicNumberMismatch;
          res = static_cast<int>(Error::OtaHeaderMagicNumber);

          goto exit;
        }
      }

      break;
    }
    case OtaDownloadFile:
      _context->decoder.decompress(cursor, http_res - (cursor-_context->buffer)); // TODO verify return value

      _context->calculatedCrc32 = crc_update(
          _context->calculatedCrc32,
          cursor,
          http_res - (cursor-_context->buffer)
        );

      cursor += http_res - (cursor-_context->buffer);
      _context->downloadedSize += (cursor-_context->buffer);

      // TODO there should be no more bytes available when the download is completed
      if(_context->downloadedSize == _http_client->contentLength()) {
        _context->downloadState = OtaDownloadCompleted;
        res = 1;
      }

      if(_context->downloadedSize > _http_client->contentLength()) {
        _context->downloadState = OtaDownloadError;
        res = static_cast<int>(Error::OtaDownload);
      }
      // TODO fail if we exceed a timeout? and available is 0 (client is broken)
      break;
    case OtaDownloadCompleted:
      res = 1;
      goto exit;
    default:
      _context->downloadState = OtaDownloadError;
      res = static_cast<int>(Error::OtaDownload);
      goto exit;
    }
  }

exit:
  if(_context->downloadState == OtaDownloadError ||
      _context->downloadState == OtaDownloadMagicNumberMismatch) {
    clean(); // need to clean everything because the download failed
  } else if(_context->downloadState == OtaDownloadCompleted) {
    // only need to delete clients and not the context, since it will be needed
    if(_client != nullptr) {
      delete _client;
      _client = nullptr;
    }

    if(_http_client != nullptr) {
      delete _http_client;
      _http_client = nullptr;
    }
  }

  return res;
}

int Arduino_ESP32_OTA::downloadProgress()
{
  if(_context->error != Error::None) {
    return static_cast<int>(_context->error);
  } else {
    return _context->downloadedSize;
  }
}

size_t Arduino_ESP32_OTA::downloadSize()
{
  return _http_client!=nullptr ? _http_client->contentLength() : 0;
}

int Arduino_ESP32_OTA::download(const char * ota_url)
{
  int err = startDownload(ota_url);

  if(err < 0) {
    return err;
  }

  int res = 0;
  while((res = downloadPoll()) <= 0);

  return res == 1? _context->writtenBytes : res;
}

void Arduino_ESP32_OTA::clean()
{
  if(_client != nullptr) {
    delete _client;
    _client = nullptr;
  }

  if(_http_client != nullptr) {
    delete _http_client;
    _http_client = nullptr;
  }

  if(_context != nullptr) {
    delete _context;
    _context = nullptr;
  }
}

Arduino_ESP32_OTA::Error Arduino_ESP32_OTA::verify()
{
  assert(_context != nullptr);

  /* ... then finalize ... */
  _context->calculatedCrc32 ^= 0xFFFFFFFF;

  /* Verify the crc */
  if(_context->header.header.crc32 != _context->calculatedCrc32) {
    DEBUG_ERROR("%s: CRC32 mismatch", __FUNCTION__);
    return Error::OtaHeaderCrc;
  }

  clean();

  return Error::None;
}

Arduino_ESP32_OTA::Error Arduino_ESP32_OTA::update()
{
  Arduino_ESP32_OTA::Error res = Error::None;
  if(_context != nullptr && (res = verify()) != Error::None) {
    return res;
  }

  if (!Update.end(true)) {
    DEBUG_ERROR("%s: Failure to apply OTA update", __FUNCTION__);
    return Error::OtaStorageEnd;
  }

  return res;
}

void Arduino_ESP32_OTA::reset()
{
  ESP.restart();
}

bool Arduino_ESP32_OTA::isCapable()
{
  const esp_partition_t * ota_0  = esp_partition_find_first(ESP_PARTITION_TYPE_APP, ESP_PARTITION_SUBTYPE_APP_OTA_0, NULL);
  const esp_partition_t * ota_1  = esp_partition_find_first(ESP_PARTITION_TYPE_APP, ESP_PARTITION_SUBTYPE_APP_OTA_1, NULL);
  return ((ota_0 != nullptr) && (ota_1 != nullptr));
}

/******************************************************************************
   PROTECTED MEMBER FUNCTIONS
 ******************************************************************************/

Arduino_ESP32_OTA::Context::Context(
  const char* url, std::function<void(uint8_t)> putc)
    : url((char*)malloc(strlen(url)+1))
    , parsed_url(url)
    , downloadState(OtaDownloadHeader)
    , calculatedCrc32(0xFFFFFFFF)
    , headerCopiedBytes(0)
    , downloadedSize(0)
    , error(Error::None)
    , decoder(putc) {
      strcpy(this->url, url);
    }

Arduino_ESP32_OTA::Context::~Context(){
  free(url);
  url = nullptr;
}