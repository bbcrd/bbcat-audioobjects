
#ifndef __CRC_32__
#define __CRC_32__

#include <aplibs-dsp/misc.h>

BBC_AUDIOTOOLBOX_START

uint32_t CRC32(const uint8_t *buffer, uint32_t buffer_length, uint32_t crc = 0);

BBC_AUDIOTOOLBOX_END

#endif
