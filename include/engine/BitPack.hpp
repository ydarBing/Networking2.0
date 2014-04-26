/*
 *  FILE          BitPack.hpp
 *  AUTHOR        Brady Reuter
 *  DESCRIPTION
 *    Templated functions of BitArray
 *    class to pack bits into an unsigned char array
 *    
 *    Copyright (C) 2013 DigiPen Institute of Technology. Reproduction or disclosure 
 *      of this file or its contents without the prior written consent of 
 *      DigiPen Institute of Technology is prohibited.
 */
namespace Networking
{

template <class T> 
void BitArray::PrintBits(T val)
{
  std::cout << std::bitset<sizeof(T) * 8>(val) << std::endl;
}

template <class T> 
void BitArray::TPlateSerializeInt(T* val, T imin, T imax)
{
  T range = imax - imin;
  s32 numBits = BitsNeeded(range);

  u08 c = 0;

  if(m_reading)
  {
    T mult = 1;
    *val = 0;

    while(numBits > 8)
    {
      ReadBits(&c, 8);
      *val |= mult * c; // no need to swap endian with this
      mult *= 256;
      numBits -= 8;
    }
    ReadBits(&c, numBits);
    *val |= mult * c;
    *val = *val + imin;// set it back into the range
  }
  else // writing data
  {
    T temp = *val - imin;
    // clamp data between 0 and range
    if(temp < 0)
      temp = 0;
    if(temp > range)
      temp = range;

    while(numBits > 8)
    {
      // have to just use 1 byte of information
      c = temp & 0xFF;
      WriteBits(&c, 8);// don't need to swap endian with this
      temp >>= 8;
      numBits -= 8;
    }
    c = temp & 0xFF;
    WriteBits(&c, numBits);
  }
}

}//namespace Networking
