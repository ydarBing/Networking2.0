//---------------------------------------------------------------------------
#ifndef PRNG_H
#define PRNG_H
//---------------------------------------------------------------------------
namespace Test
{
  unsigned rand(void);              // returns a random 32-bit integer
  void srand(unsigned, unsigned);   // seed the generator
  int random(int low, int high);    // range
}
#endif

