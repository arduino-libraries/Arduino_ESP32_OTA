/* Original code: https://oku.edu.mie-u.ac.jp/~okumura/compression/lzss.c */

/**************************************************************************************
   INCLUDE
 **************************************************************************************/

#include "lzss.h"

/**************************************************************************************
   DEFINE
 **************************************************************************************/

#define EI 11             /* typically 10..13 */
#define EJ  4             /* typically 4..5 */
#define P   1             /* If match length <= P then output one character */
#define N (1 << EI)       /* buffer size */
#define L ((1 << EJ) + 1) /* lookahead buffer size */

#define LZSS_EOF (-1)

/**************************************************************************************
   GLOBAL VARIABLES
 **************************************************************************************/

/* Used to bind local module function to actual class instance */
static Arduino_ESP32_OTA * esp_ota_obj_ptr = 0;

static size_t LZSS_FILE_SIZE = 0;

int bit_buffer = 0, bit_mask = 128;
unsigned char buffer[N * 2];

static size_t bytes_written_fputc = 0;
static size_t bytes_read_fgetc = 0;

/**************************************************************************************
   PRIVATE FUNCTIONS
 **************************************************************************************/

void lzss_fputc(int const c)
{
  esp_ota_obj_ptr->write_byte_to_flash((uint8_t)c);
  
  /* write byte callback */
  bytes_written_fputc++;
}

int lzss_fgetc()
{
  /* lzss_file_size is set within SSUBoot:main 
   * and contains the size of the LZSS file. Once
   * all those bytes have been read its time to return
   * LZSS_EOF in order to signal that the end of
   * the file has been reached.
   */
  if (bytes_read_fgetc == LZSS_FILE_SIZE)
    return LZSS_EOF;

  /* read byte callback */
  uint8_t const c = esp_ota_obj_ptr->read_byte_from_network();
  bytes_read_fgetc++;

  return c;
}

/**************************************************************************************
   LZSS FUNCTIONS
 **************************************************************************************/

void putbit1(void)
{
  bit_buffer |= bit_mask;
  if ((bit_mask >>= 1) == 0) {
    lzss_fputc(bit_buffer);
    bit_buffer = 0;  bit_mask = 128;
  }
}

void putbit0(void)
{
  if ((bit_mask >>= 1) == 0) {
    lzss_fputc(bit_buffer);
    bit_buffer = 0;  bit_mask = 128;
  }
}

void output1(int c)
{
  int mask;
    
  putbit1();
  mask = 256;
  while (mask >>= 1) {
    if (c & mask) putbit1();
    else putbit0();
  }
}

void output2(int x, int y)
{
  int mask;
    
  putbit0();
  mask = N;
  while (mask >>= 1) {
    if (x & mask) putbit1();
    else putbit0();
  }
  mask = (1 << EJ);
  while (mask >>= 1) {
    if (y & mask) putbit1();
    else putbit0();
  }
}

int getbit(int n) /* get n bits */
{
  int i, x;
  static int buf, mask = 0;
    
  x = 0;
  for (i = 0; i < n; i++) {
    if (mask == 0) {
      if ((buf = lzss_fgetc()) == LZSS_EOF) return LZSS_EOF;
      mask = 128;
    }
    x <<= 1;
    if (buf & mask) x++;
    mask >>= 1;
  }
  return x;
}

void lzss_decode(void)
{
  int i, j, k, r, c;
    
  for (i = 0; i < N - L; i++) buffer[i] = ' ';
  r = N - L;
  while ((c = getbit(1)) != LZSS_EOF) {
    if (c) {
      if ((c = getbit(8)) == LZSS_EOF) break;
      lzss_fputc(c);
      buffer[r++] = c;  r &= (N - 1);
    } else {
      if ((i = getbit(EI)) == LZSS_EOF) break;
      if ((j = getbit(EJ)) == LZSS_EOF) break;
      for (k = 0; k <= j + 1; k++) {
        c = buffer[(i + k) & (N - 1)];
        lzss_fputc(c);
        buffer[r++] = c;  r &= (N - 1);
      }
    }
  }
}

/**************************************************************************************
   PUBLIC FUNCTIONS
 **************************************************************************************/

int lzss_download(Arduino_ESP32_OTA * instance, size_t const lzss_file_size)
{
  esp_ota_obj_ptr = instance;
  LZSS_FILE_SIZE = lzss_file_size;
  bytes_written_fputc = 0;
  bytes_read_fgetc = 0;
  lzss_decode();
  return bytes_written_fputc;
}
