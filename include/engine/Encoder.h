/*
 *  FILE          Encoder.h
 *  AUTHOR        Brady Reuter
 *  DESCRIPTION
 *    Classes to encode and decode infromation to/from bit strings
 *    
 *    Copyright (C) 2013 DigiPen Institute of Technology. Reproduction or disclosure 
 *      of this file or its contents without the prior written consent of 
 *      DigiPen Institute of Technology is prohibited.
 */


#ifndef ENCODER_DECODER_H
#define ENCODER_DECODER_H
#include "BitPack.h"

enum BitsWanted
{
  BW_1  = (1 << 1) - 1,
  BW_2  = (1 << 2) - 1,
  BW_3  = (1 << 3) - 1,
  BW_4  = (1 << 4) - 1,
  BW_5  = (1 << 5) - 1,
  BW_6  = (1 << 6) - 1,
  BW_7  = (1 << 7) - 1,
  BW_8  = (1 << 8) - 1,
  BW_9  = (1 << 9) - 1,
  BW_10 = (1 << 10) - 1,
  BW_11 = (1 << 11) - 1,
  BW_12 = (1 << 12) - 1,
  BW_13 = (1 << 13) - 1,
  BW_14 = (1 << 14) - 1,
  BW_15 = (1 << 15) - 1,
  BW_16 = (1 << 16) - 1,
  BW_17 = (1 << 17) - 1,
  BW_18 = (1 << 18) - 1,
  BW_19 = (1 << 19) - 1,
  BW_20 = (1 << 20) - 1,
};

class Encoder
{
  public:
    Encoder(bool read, u08* dat, s32 size, bool pack = false);
    Encoder(bool read, s32 allocateSize, bool packed = false);
    //Encoder(bool read);
    ~Encoder();

    s32 GetDataSize();
    u08* GetData();
    

    void Write(bool data);
    //void Write(c08 data);
    void Write(s32 data, s32 imin, s32 imax);
    void Write(u08 data, u08 imin = 0, u08 imax = 0xFF);
    void Write(u16 data, u16 imin = 0, u16 imax = 0xFFFF);
    void Write(u32 data, u32 imin = 0, u32 imax = 0xFFFFFFFF);
    void Write(u64 data, u64 imin = 0, u64 imax = 0xFFFFFFFFFFFFFFFF);
    void Write(f32 data, f32 fmin, f32 fmax, s32 numBits = 32);

    //void WriteChar(char);
    //void WriteString(istring&);
    
    //writes any data to data
    void WriteData(const c08* data, s32 size);
    void WriteData(const u08* data, s32 size);
    void WriteData(const s08* data, s32 size);

    void StartReading(void);
    void StartWriting(void);
    void DoneWriting(void);


    void Read(bool& b);
    //void Read(c08& data);
    void Read(s32& data, s32 imin, s32 imax);
    void Read(u08& data, u08 imin = 0, u08 imax = 0xFF);
    void Read(u16& data, u16 imin = 0, u16 imax = 0xFFFF);
    void Read(u32& data, u32 imin = 0, u32 imax = 0xFFFFFFFF);
    void Read(u64& data, u64 imin = 0, u64 imax = 0xFFFFFFFFFFFFFFFF);
    void Read(f32& data, f32 fmin, f32 fmax, s32 numBits = 32);
    
    // this returned pointer needs to be deleted
    c08* ReadData(u32 size);
    void ReadData(c08* ret, u32 size);
    bool AtEnd(void);//returns true if read pointer is at end of data

  private:
    //u08* GetPackedData(void)const;
    bool m_isPacking;
    Networking::BitArray m_pack;
    u08* m_packedData;

    s32  m_dataSize;
    s32  m_unpackedWrittenBytes;
    u08* m_unpackedData;
    u08* m_unpackedWritePointer;
    u08* m_unpackedReadPointer;

    bool m_allocated;
    bool m_isReading;
};

#endif
