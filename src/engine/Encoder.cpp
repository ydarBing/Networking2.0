/*
 *  FILE          Encoder.cpp
 *  AUTHOR        Brady Reuter
 *  DESCRIPTION
 *    Classes that encode/decode integral types into/from bit strings
 *    
 *    Copyright (C) 2013 DigiPen Institute of Technology. Reproduction or disclosure 
 *      of this file or its contents without the prior written consent of 
 *      DigiPen Institute of Technology is prohibited.
 */


#include "NetworkingPrecompiled.h"
#include "Encoder.h"
#include "NetworkingSystem.h"

//  when reading data
Encoder::Encoder(bool read, u08* data, s32 dataSize, bool packed /*= false*/) 
  :  m_dataSize(dataSize), m_unpackedWrittenBytes(0),
  m_allocated(false), m_pack(read), m_isPacking(packed), m_isReading(read)
{
  if(m_isPacking == false)
  {
    m_unpackedData = data;
    m_unpackedWritePointer = data;
    if(m_isReading)
      m_unpackedReadPointer = m_unpackedData;
  }
  else
  {
    m_packedData = data;
    for(s32 i = 0; i < dataSize; ++i)
      m_pack.m_data[i] = data[i];
  }
}

/*Encoder::Encoder(bool read)
 : m_unpackedDataSize(Networking::NetworkingSystem::GetMaxPacketLen()), m_unpackedWrittenBytes(0), m_allocated(true), 
   m_pack(read), m_isReading(read), m_isPacking(false)
{
  if(m_isPacking == false) // when packing this is set up when doneWriting is called
  {
    m_unpackedData = new u08[Networking::NetworkingSystem::GetMaxPacketLen()];
    m_unpackedWritePointer = m_unpackedData;
  }
}*/

Encoder::Encoder(bool read, s32 allocateSize, bool pack /*= false*/)
 : m_dataSize(allocateSize), m_unpackedWrittenBytes(0), m_allocated(false), 
   m_pack(read), m_isPacking(pack), m_isReading(read), m_packedData(nullptr)
{
  if(m_isPacking == false)
  {
    m_unpackedData = new u08[m_dataSize];
    m_allocated = true;
    m_unpackedWritePointer = m_unpackedData;
  }
}

Encoder::~Encoder()
{
  if(m_allocated && m_isPacking == false)
    delete[] m_unpackedData;
  else if(m_allocated && m_isPacking)
    delete[] m_packedData;
}

s32 Encoder::GetDataSize()
{
  if(m_isPacking)
    return m_pack.GetNumberOfBytesPushed();
  else
    return m_unpackedWrittenBytes;
}

u08* Encoder::GetData()
{
  if(m_isPacking)
    return m_packedData;
  else
    return m_unpackedData;
}

void Encoder::StartWriting(void)
{
  if(m_isPacking)
  {
    m_pack.ResetForWrite();
    if(m_packedData)
      delete[] m_packedData;
  }
}

void Encoder::StartReading(void)
{
  m_unpackedReadPointer = m_unpackedData;
  if(m_isPacking)
    m_pack.ResetForRead();
}

void Encoder::DoneWriting(void)
{
  assertion(m_isReading == false);
  // allocates correct amount of data to send
  if(m_isPacking && !m_allocated)
  {
    m_packedData = new u08[m_pack.GetNumberOfBytesPushed()];
    for(s32 i = 0; i < m_pack.GetNumberOfBytesPushed(); ++i)
      m_packedData[i] = m_pack.m_data[i];
    m_dataSize = m_pack.GetNumberOfBytesPushed();
    m_allocated  = true;
  }
}

/////
//write to data
/////
void Encoder::Write(f32 data, f32 fmin, f32 fmax, s32 numBits)
{
  if(m_isPacking == false)
  {
    assertion((m_unpackedWritePointer + sizeof(f32)) <= (m_unpackedData + m_dataSize)); //trying to write past allocated memory
    memcpy(m_unpackedWritePointer, &data, sizeof(f32));
    m_unpackedWritePointer += sizeof(f32);
    m_unpackedWrittenBytes += sizeof(f32);
  }
  else
    m_pack.Serialize(data, fmin, fmax, numBits);
}

void Encoder::Write(u08 data, u08 imin, u08 imax)
{
  if(m_isPacking == false)
  {  
    assertion((m_unpackedWritePointer + sizeof(u08)) <= (m_unpackedData + m_dataSize)); //trying to write past allocated memory
    memcpy(m_unpackedWritePointer, &data, sizeof(u08));
    m_unpackedWritePointer += sizeof(u08);
    m_unpackedWrittenBytes += sizeof(u08);
  }
  else
    m_pack.Serialize(data, imin, imax); 
}

void Encoder::Write(u16 data, u16 imin, u16 imax)
{
  if(m_isPacking == false)
  {  
    assertion((m_unpackedWritePointer + sizeof(u16)) <= (m_unpackedData + m_dataSize)); //trying to write past allocated memory
    memcpy(m_unpackedWritePointer, &data, sizeof(u16));
    m_unpackedWritePointer += sizeof(u16);
    m_unpackedWrittenBytes += sizeof(u16);
  }
  else
    m_pack.Serialize(data, imin, imax); 
}

void Encoder::Write(s32 data, s32 imin, s32 imax)
{
  if(m_isPacking == false)
  {
    assertion((m_unpackedWritePointer + sizeof(s32)) <= (m_unpackedData + m_dataSize)); //trying to write past allocated memory
    memcpy(m_unpackedWritePointer, &data, sizeof(s32));
    m_unpackedWritePointer += sizeof(s32);
    m_unpackedWrittenBytes += sizeof(s32);
  }
  else
    m_pack.Serialize(data, imin, imax);
}

void Encoder::Write(u32 data, u32 imin, u32 imax)
{
  if(m_isPacking == false)
  {
    assertion((m_unpackedWritePointer + sizeof(u32)) <= (m_unpackedData + m_dataSize)); //trying to write past allocated memory
    memcpy(m_unpackedWritePointer, &data, sizeof(u32));
    m_unpackedWritePointer += sizeof(u32);
    m_unpackedWrittenBytes += sizeof(u32);
  }
  else
    m_pack.Serialize(data, imin, imax); 
}

void Encoder::Write(u64 data, u64 imin, u64 imax)
{
  if(m_isPacking == false)
  {
    assertion((m_unpackedWritePointer + sizeof(u64)) <= (m_unpackedData + m_dataSize)); //trying to write past allocated memory
    memcpy(m_unpackedWritePointer, &data, sizeof(u64));
    m_unpackedWritePointer += sizeof(u64);
    m_unpackedWrittenBytes += sizeof(u64);
  }
  else
    m_pack.Serialize(data, imin, imax);
}

void Encoder::Write(bool data)
{
  if(m_isPacking == false)
  {  
    assertion((m_unpackedWritePointer + sizeof(bool)) <= (m_unpackedData + m_dataSize)); //trying to write past allocated memory
    memcpy(m_unpackedWritePointer, &data, sizeof(bool));
    m_unpackedWritePointer += sizeof(bool);
    m_unpackedWrittenBytes += sizeof(bool);
  }
  else
    m_pack.Serialize(data); 
}

//void Encoder::Write(c08 data)
//{
//  Write(*RECAST(u08*, data));
//}
//
//void Encoder::Read(c08& data)
//{
//  Read(*RECAST(u08*, data));
//}

void Encoder::WriteData(const c08* data, s32 size)
{
  if(m_isPacking == false)
  {
    assertion((m_unpackedWritePointer + size) <= (m_unpackedData + m_dataSize)); //trying to write past allocated memory
    memcpy(m_unpackedWritePointer, data, size);
    m_unpackedWritePointer += size;
    m_unpackedWrittenBytes += size;
  }
  else
    m_pack.AppendData(data, size);
}

void Encoder::WriteData(const u08* data, s32 size)
{
  if(m_isPacking == false)
  {
    assertion((m_unpackedWritePointer + size) <= (m_unpackedData + m_dataSize)); //trying to write past allocated memory
    memcpy(m_unpackedWritePointer, data, size);
    m_unpackedWritePointer += size;
    m_unpackedWrittenBytes += size;
  }
  else
    m_pack.AppendData(data, size);
}

void Encoder::WriteData(const s08* data, s32 size)
{
  if(m_isPacking == false)
  {
    assertion((m_unpackedWritePointer + size) <= (m_unpackedData + m_dataSize)); //trying to write past allocated memory
    memcpy(m_unpackedWritePointer, data, size);
    m_unpackedWritePointer += size;
    m_unpackedWrittenBytes += size;
  }
  else
    m_pack.AppendData(data, size);
}

// writing a string will place size of string and then string
//void Encoder::WriteString(istring& data)
//{
//  u32 size = dat.size();
//  WriteU32(size);
//
//  assertion((m_unpackedWritePointer + size) <= (m_unpackedData + m_unpackedDataSize)); //trying to write past allocated memory
//  memcpy(m_unpackedWritePointer, data.c_str(), size);
//  m_unpackedWritePointer += size;
//  m_unpackedWrittenBytes += size;
//}

// READING
void Encoder::Read(s32& data, s32 imin, s32 imax)
{
  if(m_isPacking == false)
  {
    memcpy(&data, m_unpackedReadPointer, sizeof(s32));
    m_unpackedReadPointer += sizeof(s32);
  }
  else
    m_pack.Serialize(data, imin, imax);
}

void Encoder::Read(u08& data, u08 imin, u08 imax)
{
  if(m_isPacking == false)
  {
    memcpy(&data, m_unpackedReadPointer, sizeof(u08));
    m_unpackedReadPointer += sizeof(u08);
  }
  else
    m_pack.Serialize(data, imin, imax);
}

void Encoder::Read(u16& data, u16 imin, u16 imax)
{
  if(m_isPacking == false)
  {
    memcpy(&data, m_unpackedReadPointer, sizeof(u16));
    m_unpackedReadPointer += sizeof(u16);
  }
  else
    m_pack.Serialize(data, imin, imax);
}

void Encoder::Read(u32& data, u32 imin, u32 imax)
{
  if(m_isPacking == false)
  {
    memcpy(&data, m_unpackedReadPointer, sizeof(u32));
    m_unpackedReadPointer += sizeof(u32);
  }
  else
    m_pack.Serialize(data, imin, imax);
}

void Encoder::Read(u64& data, u64 imin, u64 imax)
{
  
  if(m_isPacking == false)
  {
    memcpy(&data, m_unpackedReadPointer, sizeof(u64));
    m_unpackedReadPointer += sizeof(u64);
  }
  else
    m_pack.Serialize(data, imin, imax);
}

void Encoder::Read(f32& data, f32 fmin, f32 fmax, s32 numBits)
{
  if(m_isPacking == false)
  {
    memcpy(&data, m_unpackedReadPointer, sizeof(f32));
    m_unpackedReadPointer += sizeof(f32);
  }
  else
    m_pack.Serialize(data, fmin, fmax, numBits);
}

void Encoder::Read(bool& b)
{
  if(m_isPacking == false)
  {
    memcpy(&b, m_unpackedReadPointer, sizeof(bool));
    m_unpackedReadPointer += sizeof(bool);
  }
  else
    m_pack.Serialize(b);
}

bool Encoder::AtEnd(void)
{
  if(m_isPacking == false)
    return m_unpackedReadPointer >= (m_unpackedData + m_dataSize);
  else
    return m_pack.m_bytePos >= m_dataSize;
}

c08* Encoder::ReadData(u32 size)
{
  c08* out = new c08[size];
  if(m_isPacking == false)
  {
    memcpy(out, m_unpackedReadPointer, size);
    m_unpackedReadPointer += size;
  }
  else
  {
    s32 actuallyNextByte = 0;
    if(m_pack.m_bitPos != 0)
      actuallyNextByte = 1;
    memcpy(out, m_packedData + m_pack.m_bytePos + actuallyNextByte, size);
    m_pack.m_bitPos = 7;
    m_pack.m_bytePos += size;
  }
  return out;
}


void Encoder::ReadData(c08* ret, u32 size)
{
  if(m_isPacking == false)
  {
    memcpy(ret, m_unpackedReadPointer, size);
    m_unpackedReadPointer += size;
  }
  else
  {
    s32 actuallyNextByte = 0;
    if(m_pack.m_bitPos != 0)
      actuallyNextByte = 1;
    memcpy(ret, m_packedData + m_pack.m_bytePos + actuallyNextByte, size);
    m_pack.m_bitPos = 7;
    m_pack.m_bytePos += size;
  }
}
