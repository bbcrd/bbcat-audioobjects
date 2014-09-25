
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define DEBUG_LEVEL 1
#include <aplibs-dsp/ByteSwap.h>

#include "RIFFChunks.h"
#include "RIFFChunk_Definitions.h"
#include "RIFFFile.h"

BBC_AUDIOTOOLBOX_START

/*--------------------------------------------------------------------------------*/
/** RIFF chunk - the first chunk of any WAVE file
 *
 * Don't read the data, no specific handling
 */
/*--------------------------------------------------------------------------------*/

// set chunk to RF64
void RIFFRIFFChunk::EnableRIFF64()
{
  RIFFChunk::EnableRIFF64();

  id   = RF64_ID;
  name = "RF64";
}

// just return true - no data to write
bool RIFFRIFFChunk::WriteChunkData(EnhancedFile *file)
{
  UNUSED_PARAMETER(file);
  return true;
}

void RIFFRIFFChunk::Register()
{
  RIFFChunk::RegisterProvider("RIFF", &Create);
  RIFFChunk::RegisterProvider("RF64", &Create);
}

/*----------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------*/
/** WAVE chunk - specifies that the file contains WAV data
 *
 * This isn't actually a proper chunk - there is no length, just the ID, hence
 * a specific ReadChunk() function
 */
/*--------------------------------------------------------------------------------*/
bool RIFFWAVEChunk::ReadChunk(EnhancedFile *file)
{
  UNUSED_PARAMETER(file);

  // there is no data after WAVE to read
  return true;
}

// special write chunk function (no length or data)
bool RIFFWAVEChunk::WriteChunk(EnhancedFile *file)
{
  uint32_t data[] = {id};
  bool success = false;

  // treat ID  as big-endian
  ByteSwap(data[0], SWAP_FOR_BE);

  if (file->fwrite(data, NUMBEROF(data), sizeof(data[0])) > 0)
  {
    datapos = file->ftell();
    success = true;
  }

  return success;
}

void RIFFWAVEChunk::Register()
{
  RIFFChunk::RegisterProvider("WAVE", &Create);
}

/*----------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------*/
/** ds64 chunk - specifies chunk sizes for RIFF64 files
 *
 * The chunk data is read, byte swapped and then processed
 *
 */
/*--------------------------------------------------------------------------------*/
void RIFFds64Chunk::Register()
{
  RIFFChunk::RegisterProvider("ds64", &Create);
}

void RIFFds64Chunk::ByteSwapData()
{
  ds64_CHUNK& chunk = *(ds64_CHUNK *)data;

  if (SwapLittleEndian())
  {
    BYTESWAP_VAR(chunk.RIFFSizeLow);
    BYTESWAP_VAR(chunk.RIFFSizeHigh);
    BYTESWAP_VAR(chunk.dataSizeLow);
    BYTESWAP_VAR(chunk.dataSizeHigh);
    BYTESWAP_VAR(chunk.SampleCountLow);
    BYTESWAP_VAR(chunk.SampleCountHigh);
    BYTESWAP_VAR(chunk.TableEntryCount);

    uint32_t i;
    for (i = 0; i < chunk.TableEntryCount; i++)
    {
      BYTESWAP_VAR(chunk.Table[i].ChunkSizeLow);
      BYTESWAP_VAR(chunk.Table[i].ChunkSizeHigh);
    }
  }
}

// create write data
bool RIFFds64Chunk::CreateWriteData()
{
  bool success = (length && data);

  if (!success)
  {
    length = sizeof(ds64_CHUNK);
    if ((data = new uint8_t[length]) != NULL)
    {
      memset(data, 0, length);

      success = true;
    }
  }

  return success;
}

uint64_t RIFFds64Chunk::GetRIFFSize() const
{
  const ds64_CHUNK *ds64;
  uint64_t size = 0;

  if ((ds64 = (const ds64_CHUNK *)GetData()) != NULL) size = Convert32bitSizes(ds64->RIFFSizeLow, ds64->RIFFSizeHigh);

  return size;
}

uint64_t RIFFds64Chunk::GetdataSize() const
{
  const ds64_CHUNK *ds64;
  uint64_t size = 0;

  if ((ds64 = (const ds64_CHUNK *)GetData()) != NULL) size = Convert32bitSizes(ds64->dataSizeLow, ds64->dataSizeHigh);

  return size;
}

uint64_t RIFFds64Chunk::GetSampleCount() const
{
  const ds64_CHUNK *ds64;
  uint64_t count = 0;

  if ((ds64 = (const ds64_CHUNK *)GetData()) != NULL) count = Convert32bitSizes(ds64->SampleCountLow, ds64->SampleCountHigh);

  return count;
}

uint_t RIFFds64Chunk::GetTableCount() const
{
  const ds64_CHUNK *ds64;
  uint_t count = 0;

  if ((ds64 = (const ds64_CHUNK *)GetData()) != NULL) count = ds64->TableEntryCount;

  return count;
}

uint64_t RIFFds64Chunk::GetTableEntrySize(uint_t entry, char *id) const
{
  const ds64_CHUNK *ds64;
  uint64_t size = 0;

  if (((ds64 = (const ds64_CHUNK *)GetData()) != NULL) && (entry < ds64->TableEntryCount))
  {
    size = Convert32bitSizes(ds64->Table[entry].ChunkSizeLow, ds64->Table[entry].ChunkSizeHigh);
    if (id) memcpy(id, ds64->Table[entry].ChunkId, sizeof(ds64->Table[entry].ChunkId));
  }

  return size;
}

void RIFFds64Chunk::SetRIFFSize(uint64_t size)
{
  ds64_CHUNK *ds64;

  if ((ds64 = (ds64_CHUNK *)GetData()) != NULL) {
    ds64->RIFFSizeLow  = (uint32_t)size;
    ds64->RIFFSizeHigh = (uint32_t)(size >> 32);
  }
}

void RIFFds64Chunk::SetdataSize(uint64_t size)
{
  ds64_CHUNK *ds64;

  if ((ds64 = (ds64_CHUNK *)GetData()) != NULL) {
    ds64->dataSizeLow  = (uint32_t)size;
    ds64->dataSizeHigh = (uint32_t)(size >> 32);
  }
}

void RIFFds64Chunk::SetSampleCount(uint64_t count)
{
  ds64_CHUNK *ds64;

  if ((ds64 = (ds64_CHUNK *)GetData()) != NULL) {
    ds64->SampleCountLow  = (uint32_t)count;
    ds64->SampleCountHigh = (uint32_t)(count >> 32);
  }
}

/*----------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------*/
/** fmt chunk - specifies the format of the WAVE data
 *
 * The chunk data is read, byte swapped and then processed
 *
 * Note this object is also derived from the SoundFormat class to allow generic
 * format handling facilities
 */
/*--------------------------------------------------------------------------------*/
void RIFFfmtChunk::Register()
{
  RIFFChunk::RegisterProvider("fmt ", &Create);
}

void RIFFfmtChunk::ByteSwapData()
{
  WAVEFORMAT_CHUNK& chunk = *(WAVEFORMAT_CHUNK *)data;

  if (SwapLittleEndian())
  {
    BYTESWAP_VAR(chunk.Format);
    BYTESWAP_VAR(chunk.Channels);
    BYTESWAP_VAR(chunk.SampleRate);
    BYTESWAP_VAR(chunk.BytesPerSecond);
    BYTESWAP_VAR(chunk.BlockAlign);
    BYTESWAP_VAR(chunk.BitsPerSample);
  }
}

bool RIFFfmtChunk::ProcessChunkData()
{
  const WAVEFORMAT_CHUNK& chunk = *(const WAVEFORMAT_CHUNK *)data;
  bool success = false;

  if (chunk.Format == WAVE_FORMAT_PCM)
  {
    // cannot handle anything other that PCM samples

    DEBUG2(("Reading format data"));

    // set parameters within SoundFormat according to data from this chunk
    SetSampleRate(chunk.SampleRate);
    SetChannels(chunk.Channels);

    // best guess at sample data format
    if (chunk.BitsPerSample <= 16)
    {
      SetSampleFormat(SampleFormat_16bit);
    }
    else if (chunk.BitsPerSample <= 24)
    {
      SetSampleFormat(SampleFormat_24bit);
    }
    else
    {
      SetSampleFormat(SampleFormat_32bit);
    }

    // WAVE is always little-endian
    SetSamplesBigEndian(false);

    success = true;
  }
  else ERROR("Format is %04x, not PCM", chunk.Format);

  return success;
}

// create write data
bool RIFFfmtChunk::CreateWriteData()
{
  bool success = (length && data);

  if (!success)
  {
    WAVEFORMAT_CHUNK chunk;

    memset(&chunk, 0, sizeof(chunk));

    chunk.Format         = WAVE_FORMAT_PCM;
    chunk.SampleRate     = samplerate;
    chunk.Channels       = channels;
    chunk.BytesPerSecond = samplerate * channels * bytespersample;
    switch (format)
    {
      case SampleFormat_16bit:
        // ONLY use 16 bit samples if sample format is explicitly 16 bit
        chunk.BitsPerSample = 16;
        break;

      default:
        // ALL other sample formats get stored as 24 bits 
        chunk.BitsPerSample = 24;
        break;
    }
    // block align MUST be even!
    chunk.BlockAlign = ((channels * bytespersample) & 1) ? 2 * bytespersample : bytespersample;

    length = sizeof(chunk);
    if ((data = new uint8_t[length]) != NULL)
    {
      memcpy(data, &chunk, length);

      success = true;
    }
  }

  return success;
}

/*----------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------*/
/** bext chunk - specifies the Broadcast Extension data
 *
 * The chunk data is read, byte swapped but not processed
 *
 */
/*--------------------------------------------------------------------------------*/
void RIFFbextChunk::Register()
{
  RIFFChunk::RegisterProvider("bext", &Create);
}

void RIFFbextChunk::ByteSwapData()
{
  BROADCAST_CHUNK& chunk = *(BROADCAST_CHUNK *)data;

  if (SwapLittleEndian())
  {
    BYTESWAP_VAR(chunk.TimeReferenceLow);
    BYTESWAP_VAR(chunk.TimeReferenceHigh);
    BYTESWAP_VAR(chunk.Version);
  }
}

// create write data
bool RIFFbextChunk::CreateWriteData()
{
  bool success = (length && data);

  if (!success)
  {
    BROADCAST_CHUNK chunk;

    memset(&chunk, 0, sizeof(chunk));

    length = sizeof(chunk);
    if ((data = new uint8_t[length]) != NULL)
    {
      memcpy(data, &chunk, length);

      success = true;
    }
  }
  
  return success;
}

/*----------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------*/
/** chna chunk - part of the ADM (EBU Tech 3364)
 *
 * The chunk data is read, byte swapped but not processed (it is handled by the parent)
 *
 */
/*--------------------------------------------------------------------------------*/
void RIFFchnaChunk::Register()
{
  RIFFChunk::RegisterProvider("chna", &Create);
}

void RIFFchnaChunk::ByteSwapData()
{
  CHNA_CHUNK& chunk = *(CHNA_CHUNK *)data;

  if (SwapLittleEndian())
  {
    BYTESWAP_VAR(chunk.TrackCount);
    BYTESWAP_VAR(chunk.UIDCount);

    uint16_t i;
    for (i = 0; i < chunk.UIDCount; i++)
    {
      BYTESWAP_VAR(chunk.UIDs[i].TrackNum);
    }
  }
}

/*----------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------*/
/** axml chunk - part of the ADM (EBU Tech 3364)
 *
 * The chunk data is read, not byte swapped (it is pure XML) and not processed (it is handled by the parent)
 *
 */
/*--------------------------------------------------------------------------------*/
void RIFFaxmlChunk::Register()
{
  RIFFChunk::RegisterProvider("axml", &Create);
}

/*----------------------------------------------------------------------------------------------------*/

RIFFdataChunk::~RIFFdataChunk()
{
}

/*--------------------------------------------------------------------------------*/
/** Initialise chunk for writing - called when empty chunk is first created
 */
/*--------------------------------------------------------------------------------*/
bool RIFFdataChunk::InitialiseForWriting()
{
  return CreateTempFile();
}

/*--------------------------------------------------------------------------------*/
/** data chunk - WAVE data
 *
 * The data is not read (it could be too big to fit in memory)
 *
 * Note that the object is also derived from SoundfileSamples which provides sample
 * reading and conversion facilities
 *
 */
/*--------------------------------------------------------------------------------*/
bool RIFFdataChunk::ReadChunk(EnhancedFile *file)
{
  bool success = false;

  if (RIFFChunk::ReadChunk(file))
  {
    // link file to SoundFileSamples object
    SetFile(file, datapos, length);

    success = true;
  }

  return success;
}

// set up data length before data is written
bool RIFFdataChunk::CreateWriteData()
{
  // set length
  length = totalbytes;
  return true;
}

// copy sample data from temporary file
bool RIFFdataChunk::WriteChunkData(EnhancedFile *file)
{
  std::vector<uint8_t> buffer;
  EnhancedFile *srcfile = this->file;
  bool success = false;

  // create temp buffer
  buffer.resize(0x10000);

  uint8_t  *buf    = &buffer[0];
  uint_t   buflen  = buffer.size();
  uint64_t length1 = length;
  size_t   bytes;

  DEBUG1(("Copying data from '%s' to '%s'", srcfile->getfilename().c_str(), file->getfilename().c_str()));

  // copy raw sample data from this->file to file
  srcfile->fflush();
  srcfile->rewind();
  while (length1 && ((bytes = srcfile->fread(buf, 1, buflen)) > 0))
  {
    if ((bytes = file->fwrite(buf, 1, MIN(length1, bytes))) > 0) length1 -= bytes;
    else {
      ERROR("Failed to write data to destination, error %s", strerror(errno));
      break;
    }
  }

  if (length1) ERROR("%lu bytes left to write to destination", (ulong_t)length1);
  else
  {
    DEBUG1(("Copied data from '%s' to '%s'", srcfile->getfilename().c_str(), file->getfilename().c_str()));
    success = true;
  }

  return success;
}

void RIFFdataChunk::Register()
{
  RIFFChunk::RegisterProvider("data", &Create);
}

/*----------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------*/
/** Register all providers from this file
 */
/*--------------------------------------------------------------------------------*/
void RegisterRIFFChunkProviders()
{
  RIFFRIFFChunk::Register();
  RIFFWAVEChunk::Register();
  RIFFds64Chunk::Register();
  RIFFfmtChunk::Register();
  RIFFbextChunk::Register();
  RIFFchnaChunk::Register();
  RIFFaxmlChunk::Register();
  RIFFdataChunk::Register();
}

BBC_AUDIOTOOLBOX_END
