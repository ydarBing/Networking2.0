/*
 *  FILE          BitPack.cpp
 *  AUTHOR        Brady Reuter
 *  DESCRIPTION
 *    class to pack bits into an unsigned char array
 *    
 *    Copyright (C) 2013 DigiPen Institute of Technology. Reproduction or disclosure 
 *      of this file or its contents without the prior written consent of 
 *      DigiPen Institute of Technology is prohibited.
 */
#include "NetworkingPrecompiled.h"
#include "BitPack.h"

// testing
#include <vector>
#include "PRNG.h"
#include <ctime>

namespace Networking
{
  
BitArray::BitArray(bool read)
{
  if(read)
    ResetForRead();
  else
    ResetForWrite();
}

void BitArray::PrintPushedBits(void)
{
  // have to print out byte by byte because bitset must take const value
  std::cout << "Number of Bits Pushed: " << GetNumberOfBitsPushed() << std::endl;
  for(s32 i = 0; i < m_numBytes; ++i)
  {
    std::cout << "m_data[" << i << "] = ";
    std::cout << std::bitset<8>(m_data[i]) << std::endl;
  }
}

void BitArray::Serialize(bool& b)
{
  s32 temp = b ? 1 : 0;
  TPlateSerializeInt<s32>(&temp, 0, 1);
  b = temp ? 1 : 0;
}

// serialize unsigned char
void BitArray::Serialize(u08& val, s32 imin, s32 imax)
{
  s64 temp = val;
  SerializeInt(&temp, imin, imax);
  val = static_cast<u08>(temp);
}

// serialize unsigned short
void BitArray::Serialize(u16& val, u16 imin, u16 imax)
{
  u32 temp = val;
  SerializeUInt(&temp, imin, imax);
  val = static_cast<u16>(temp);
}

// serialize unsigned int
void BitArray::Serialize(u32& val, u32 imin, u32 imax)
{
  u32 temp = val;
  SerializeUInt(&temp, imin, imax);
  val = static_cast<u32>(temp);
}

// serialize unsigned int
void BitArray::Serialize(u64& val, u64 imin, u64 imax)
{
  u64 temp = val;
  SerializeULInt(&temp, imin, imax);
  val = static_cast<u64>(temp);
}

// serialize signed short
void BitArray::Serialize(s16& val, s16 imin, s16 imax)
{
  s64 temp = val;
  SerializeInt(&temp, imin, imax);
  val = static_cast<s16>(temp);
}

// serialize signed int
void BitArray::Serialize(s32& val, s32 imin, s32 imax)
{
  s64 temp = val;
  SerializeInt(&temp, imin, imax);
  val = static_cast<s32>(temp);
}

// serialize signed float
void BitArray::Serialize(f32& val, f32 fmin, f32 fmax, s32 totalNumBits)
{
  SerializeFloat(&val, fmin, fmax, totalNumBits);
}

void BitArray::AppendData(const s08* data, s32 numBytes)
{
  assertion(m_numBytes + numBytes < MAX_BYTES);
  s32 actuallyNextByte = 0;
  if(m_bitPos != 0)
    actuallyNextByte = 1;

  std::memcpy(m_data + m_bytePos + actuallyNextByte, data, numBytes);
  m_numBytes += numBytes;
  m_bytePos  += numBytes;
  m_bitPos = 0;
}


void BitArray::AppendData(const c08* data, s32 numBytes)
{
  assertion(m_numBytes + numBytes < MAX_BYTES);
  s32 actuallyNextByte = 0;
  if(m_bitPos != 0)
    actuallyNextByte = 1;

  std::memcpy(m_data + m_bytePos + actuallyNextByte, data, numBytes);
  m_numBytes += numBytes;
  m_bytePos  += numBytes;
  m_bitPos = 7; // when setting the next bit it will read the next byte
}

void BitArray::AppendData(const u08* data, s32 numBytes)
{
  assertion(m_numBytes + numBytes < MAX_BYTES);
  s32 actuallyNextByte = 0;
  if(m_bitPos != 0)
    actuallyNextByte = 1;

  std::memcpy(m_data + m_bytePos + actuallyNextByte, data, numBytes);
  m_numBytes += numBytes;
  m_bytePos  += numBytes;
  m_bitPos = 0;
}

void BitArray::SerializeInt(s64* val, s64 imin, s64 imax)
{
  TPlateSerializeInt<s64>(val, imin, imax);
}
void BitArray::SerializeUInt(u32* val, u32 imin, u32 imax)
{
  TPlateSerializeInt<u32>(val, imin, imax);
}

void BitArray::SerializeULInt(u64* val, u64 imin, u64 imax)
{
  TPlateSerializeInt<u64>(val, imin, imax);
}

void BitArray::SerializeFloat(f32* val, f32 fmin, f32 fmax, s32 totalNumBits)
{
  s64 range = (1 << totalNumBits) - 1;

  if(m_reading)
  {
    s64 n = 0;
    SerializeInt(&n, 0, range);
    // try and get float back as close to the original value
    *val = static_cast<f32>(n) * (fmax - fmin) / static_cast<f32>(range) + fmin;
  }
  else
  {
    f32 clamp = *val;
    if(clamp > fmax)
      clamp = fmax;
    else if(clamp < fmin)
      clamp = fmin;
    // convert into int so it can be easily serialized
    s64 n = static_cast<s32>((static_cast<f32>(range)) / (fmax - fmin) * (clamp - fmin));
    SerializeInt(&n, 0, range);
  }
}

bool BitArray::IsReading(void)const
{
  return m_reading;
}

const s32 BitArray::GetNumberOfBitsPushed(void)const
{
  // plus 1 because bit pos is 0-7 based
  return (m_bytePos * 8) + m_bitPos + 1;
}

const s32 BitArray::GetNumberOfBytesPushed(void)const
{
  return m_numBytes;
}

void BitArray::PushBit(s32 bit)
{
  if(m_bytePos >= MAX_BYTES)
    assertion(false); // trying to push to many bits
  // check if need to move to next byte
  if(++m_bitPos == 8)
  {
    m_bitPos = 0;
    ++m_bytePos;
    assertion(m_bytePos < MAX_BYTES);
    // initialize byte
    m_data[m_bytePos] = 0;
    ++m_numBytes;
  }
  assertion(m_bytePos < MAX_BYTES);
  m_data[m_bytePos] |= (bit & 1) << m_bitPos;
}

s32 BitArray::PopFrontBit(void)
{
  // check if ready to read next byte
  if(++m_bitPos == 8)
  {
		m_bitPos = 0;
    ++m_bytePos;
    m_readByte = m_data[m_bytePos];
  }
  u08 temp = m_readByte & 1;
  // move over next bit
  m_readByte >>= 1;
  return temp;
}

//u08* BitArray::GetData(void)const
//{
//  return m_data;
//}

void BitArray::ResetForRead(void)
{
  m_bytePos    = -1; // incremented right before it read
  m_bitPos     =  7; // incremented right before it read
  m_reading    = true;
}

void BitArray::ResetForWrite(void)
{
  m_bytePos    = -1; // incremented right before it writes
  m_bitPos     =  7; // incremented right before it writes
  m_numBytes   =  0;
  m_reading    = false;
}


void BitArray::ReadBits(u08* pOut, s32 numBits)
{
  u08 byte = 0;

  while(numBits >= 8)
  {
    byte = PopFrontBit();
    for(s32 i = 1; i < 8; ++i)
      byte |= PopFrontBit() << i;
    *pOut = byte;
    numBits -= 8;
  }

  if(numBits == 0)
    return;
  // read remaining bits in last byte
  byte = PopFrontBit();
  for(s32 i = 1; i < numBits; ++i)
    byte |= PopFrontBit() << i;

  *pOut = byte;
}

void BitArray::WriteBits(const u08* pIn, s32 numBits)
{
  u08 byte = 0;

  while(numBits >= 8)
  {
    byte = *pIn;
    // walk through byte pushing either 0 or 1
    for(s32 i = 0; i < 8; ++i)
      PushBit((byte >> i) & 1);
    numBits -= 8;
    pIn++;
  }

  byte = *pIn;
  // push the rest of the bits
  for(s32 i = 0; i < numBits; ++i)
    PushBit((byte >> i) & 1);
}
// given a range, find number of bits needed really fast
//u32 BitArray::BitsNeeded(u32 range)
//{
//	// See bit twiddling hacks (google/bing/yahoo it)
//	static const s32 MultiplyDeBruijnBitPosition[32] = 
//	{
//		0, 9, 1, 10, 13, 21, 2, 29, 11, 14, 16, 18, 22, 25, 3, 30,
//		8, 12, 20, 28, 15, 17, 24, 7, 19, 27, 23, 6, 26, 5, 4, 31
//	};
//
//	range |= range >> 1; // first round down to one less than a power of 2 
//	range |= range >> 2;
//	range |= range >> 4;
//	range |= range >> 8;
//	range |= range >> 16;
//
//	return 1 + MultiplyDeBruijnBitPosition[(u32)(range * 0x07C4ACDDU) >> 27];
//}

s32 BitArray::BitsNeeded(u64 range)
{
  static const s32 DeBruijnBitPos[64] = {
    63,  0, 58,  1, 59, 47, 53,  2,
    60, 39, 48, 27, 54, 33, 42,  3,
    61, 51, 37, 40, 49, 18, 28, 20,
    55, 30, 34, 11, 43, 14, 22,  4,
    62, 57, 46, 52, 38, 26, 32, 41,
    50, 36, 17, 19, 29, 10, 13, 21,
    56, 45, 25, 31, 35, 16,  9, 12,
    44, 24, 15,  8, 23,  7,  6,  5};

  range |= range >> 1; // first round down to one less than a power of 2 
	range |= range >> 2;
	range |= range >> 4;
	range |= range >> 8;
	range |= range >> 16;
	range |= range >> 32;
  return 1 + DeBruijnBitPos[((u64)((range - (range >> 1))*0x07EDD5E59A4E28C2)) >> 58];
}


s32 BitArray::UnitTest(void)
{
  BitArray bay(false);


  const float fmin = 0.0f, fmax = 3.0f;
  const float num = 2.987654f;
  f32 flen = num;
  bay.Serialize(flen, fmin, fmax, 10);
  flen = num;
  bay.Serialize(flen, fmin, fmax, 16);
  flen = num;
  bay.Serialize(flen, fmin, fmax, 20);
  flen = num;
  bay.Serialize(flen, fmin, fmax, 22);
  flen = num;
  bay.Serialize(flen, fmin, fmax, 25);
  flen = num;
  bay.Serialize(flen, fmin, fmax, 28);
  bay.PrintPushedBits();
  bay.ResetForRead();
  f32 fc1 = 0, fc2 = 0, fc3 = 0, fc4 = 0, fc5 = 0, fc6 = 0; 
  bay.Serialize(fc1, fmin, fmax, 10);
  bay.Serialize(fc2, fmin, fmax, 16);
  bay.Serialize(fc3, fmin, fmax, 20);
  bay.Serialize(fc4, fmin, fmax, 22);
  bay.Serialize(fc5, fmin, fmax, 25);
  bay.Serialize(fc6, fmin, fmax, 28);
  bay.ResetForWrite();
  // Signed MIN MAX CHECKS
  s16 ssMin  = INT16_MIN    , ssMinCheck  = 0;
  s16 ssMax  = INT16_MAX    , ssMaxCheck  = 0;
  s16 ssMin1 = INT16_MIN + 1, ssMin1Check = 0;
  s16 ssMax1 = INT16_MAX - 1, ssMax1Check = 0;
  s32 sMin   = INT32_MIN    , sMinCheck   = 0;
  s32 sMax   = INT32_MAX    , sMaxCheck   = 0;
  s32 sMin1  = INT32_MIN + 1, sMin1Check  = 0;
  s32 sMax1  = INT32_MAX - 1, sMax1Check  = 0;
  // Unsigned MIN MAX CHECKS
  u16 uuMin  = 0,             uuMinCheck  = 0;
  u16 uuMax  = UINT16_MAX,    uuMaxCheck  = 0;
  u32 uMin   = 0,             uMinCheck   = 0;
  u32 uMax   = UINT32_MAX,    uMaxCheck   = 0;
  u64 Max    = UINT64_MAX,    MaxCheck   = 0;
  // unsigned writing checks
  bay.Serialize(uuMin, 0, UINT16_MAX);
  bay.Serialize(uuMax, 0, UINT16_MAX);
  bay.Serialize(uMin,  0, UINT32_MAX);
  bay.Serialize(uMax,  0, UINT32_MAX);
  bay.Serialize(Max,   0, UINT64_MAX);
  // 16 writing checks
  bay.Serialize(ssMin , INT16_MIN, INT16_MAX);
  bay.Serialize(ssMax , INT16_MIN, INT16_MAX);
  bay.Serialize(ssMin1, INT16_MIN, INT16_MAX);
  bay.Serialize(ssMax1, INT16_MIN, INT16_MAX);
  // 32 writing checks
  bay.Serialize(sMin , INT32_MIN, INT32_MAX);
  bay.Serialize(sMax , INT32_MIN, INT32_MAX);
  bay.Serialize(sMin1, INT32_MIN, INT32_MAX);
  bay.Serialize(sMax1, INT32_MIN, INT32_MAX);
  bay.PrintPushedBits();
  bay.ResetForRead();
  // unsigned reading checks
  bay.Serialize(uuMinCheck, 0, UINT16_MAX);
  bay.Serialize(uuMaxCheck, 0, UINT16_MAX);
  bay.Serialize(uMinCheck,  0, UINT32_MAX);
  bay.Serialize(uMaxCheck,  0, UINT32_MAX);
  bay.Serialize(MaxCheck,   0, UINT64_MAX);
  // 16 reading checks
  bay.Serialize(ssMinCheck , INT16_MIN, INT16_MAX);
  bay.Serialize(ssMaxCheck , INT16_MIN, INT16_MAX);
  bay.Serialize(ssMin1Check, INT16_MIN, INT16_MAX);
  bay.Serialize(ssMax1Check, INT16_MIN, INT16_MAX);
  // 32 reading checks
  bay.Serialize(sMinCheck , INT32_MIN, INT32_MAX);
  bay.Serialize(sMaxCheck , INT32_MIN, INT32_MAX);
  bay.Serialize(sMin1Check, INT32_MIN, INT32_MAX);
  bay.Serialize(sMax1Check, INT32_MIN, INT32_MAX);

  if(ssMin  != ssMinCheck  || ssMax  != ssMaxCheck ||
     ssMin1 != ssMin1Check || ssMax1 != ssMax1Check)
   assertion(false);
  if(sMin  != sMinCheck  || sMax  != sMaxCheck ||
     sMin1 != sMin1Check || sMax1 != sMax1Check)
   assertion(false);
  if(uuMin != uuMinCheck || uuMax != uuMaxCheck ||
     uMin  != uMinCheck  || uMax  != uMaxCheck  ||
     Max   != MaxCheck)
   assertion(false);
  bay.PrintPushedBits();


  // stress test
  struct ForChecks
  {
    s32 min_;
    s32 max_;
    s32 val_;
    ForChecks(s32 val, s32 imax, s32 imin): min_(imin), max_(imax), val_(val) {};
  };
  Test::srand(2, 1);
  std::srand(static_cast<unsigned>(std::time(0)));
  // random number with random range reading and writing check
  s32 stress = std::rand() % 2048;
  bay.ResetForWrite();
  std::vector<ForChecks> nums, checks;
  nums.reserve(stress);
  for(int i = 0; i < stress; ++i)
  {
    s32 negative = (std::rand() % 2) == 0 ? 1 : -1;
    s32 imin = (std::rand() % INT32_MAX) * negative;
    s32 imax = std::rand() % INT32_MAX;
    if(imax < imin)
    {
      s32 temp = imin;
      imin = imax;
      imax = temp;
    }
    s32 val = Test::random(imin, imax);
    nums.push_back(ForChecks(val, imax, imin));
    bay.Serialize(val, imin, imax);
  }
  //bay.PrintPushedBits();
  bay.ResetForRead();
  for(int i = 0; i < stress; ++i)
  {
    s32 testVal = 0;
    bay.Serialize(testVal, nums[i].min_, nums[i].max_);
    assertion(testVal == nums[i].val_);
  }

  // random tests
  f32 f = 1.23456f;
  s32 val = 3;
  bool b  = true;
  u08  c  = 2;
  u32  i  = 11;
  u32  i2 = 100;
  u08  c2 = 3;
  bool bcheck;
  u08 ccheck, ccheck2;
  u32 ucheck, ucheck2;
  f32 fcheck;
  bay.ResetForWrite();
  bay.Serialize(f, 0, 100.0f, 10);
  bay.Serialize(b);
  bay.Serialize(c, 0, 2);
  bay.Serialize(i, 0, 11);
  bay.Serialize(i2, 0, 100);
  bay.Serialize(c2, 0, 3);
  bay.PrintPushedBits();
  bay.ResetForRead();
  bay.Serialize(fcheck, 0, 100.0f, 10);
  bay.Serialize(bcheck);
  bay.Serialize(ccheck, 0, 2);
  bay.Serialize(ucheck, 0, 11);
  bay.Serialize(ucheck2, 0, 100);
  bay.Serialize(ccheck2, 0, 3);
  s32 bp2 = bay.GetNumberOfBitsPushed();

  // Float Checks
  bay.ResetForWrite();
  bay.Serialize(f, 0, 2.0f, 10);
  bay.ResetForRead();
  bay.Serialize(fcheck, 0, 2.0f, 10);
  bay.ResetForWrite();
  bay.Serialize(f, 0, 1000.0f, 20);
  bay.Serialize(f, 0, 1000.0f, 21);
  bay.ResetForRead();
  bay.Serialize(fcheck, 0, 1000.0f, 20);
  bay.Serialize(fcheck, 0, 1000.0f, 21);

  return 0;
}



}//namepsace Networking

