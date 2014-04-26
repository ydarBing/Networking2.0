/*
 *  FILE          BitPack.h
 *  AUTHOR        Brady Reuter
 *  DESCRIPTION
 *    class to pack bits into an unsigned char array
 *    
 *    Copyright (C) 2013 DigiPen Institute of Technology. Reproduction or disclosure 
 *      of this file or its contents without the prior written consent of 
 *      DigiPen Institute of Technology is prohibited.
 */

#include <bitset>

class Encoder;

namespace Networking
{
  static const s32 MAX_BYTES = 4096;//2048;
  // packs bits with no padding
  class BitArray
  {
  public:
    friend class Encoder;
    BitArray(bool read);
    bool IsReading(void)const;
    void ResetForRead(void);
    void ResetForWrite(void);
  
    const s32 GetNumberOfBytesPushed(void)const;
    const s32 GetNumberOfBitsPushed(void)const;
  
    void Serialize(bool& b);
    void Serialize(u08& val, s32 imin, s32 imax);
    void Serialize(u16 &val, u16 imin = 0, u16 imax = 0xFFFF);
    void Serialize(u32 &val, u32 imin = 0, u32 imax = 0xFFFFFFFF);
    void Serialize(u64 &val, u64 imin = 0, u64 imax = 0xFFFFFFFFFFFFFFFF);

    void Serialize(s16 &val, s16 imin, s16 imax);
    void Serialize(s32 &val, s32 imin, s32 imax);
    void Serialize(f32& val, f32 fmin, f32 fmax, s32 totalNumBits);
  
    // Anytime appendData is used, be sure previous data is byte aligned
    void AppendData(const s08* data, s32 numBytes);
    void AppendData(const u08* data, s32 numBytes);
    void AppendData(const c08* data, s32 numBytes);

    template <class T> void PrintBits(T val);
    void PrintPushedBits(void);
    //static u32 BitsNeeded(u32 range);
    static s32 BitsNeeded(u64 range);

    static s32 UnitTest(void);
  
  private:
  	template <class T> void TPlateSerializeInt(T* val, T imin, T imax);
    void SerializeInt(s64* v, s64 min, s64 max);
    void SerializeUInt(u32* v, u32 min, u32 max);
    void SerializeULInt(u64* v, u64 min, u64 max);
    void SerializeFloat(f32* val, f32 fmin, f32 fmax, s32 totalNumBits = 20);

    void PushBit(s32 bit);
    s32  PopFrontBit(void);
    void ReadBits(u08* pOut, s32 numBits); 
    void WriteBits(const u08* pIn, s32 numBits);

    //u08* GetData(void)const;

  private:
    s32 m_bitPos;     // current bit
    s32 m_bytePos;    // current byte
    s32 m_numBytes;   // total number of bytes packed
    bool m_reading;  
    u08 m_readByte;   // the current byte being read (used only when reading)
    u08 m_data[MAX_BYTES];
  };
  

}//namespace Networking

// templated functions here
#include "BitPack.hpp"
