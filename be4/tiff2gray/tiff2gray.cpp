#include "tiffio.h"
#include <cstdlib>
#include <iostream>
#include <string.h>

using namespace std;

void usage() {
  cerr << "usage";
}

void error(string message) {
  cerr << message << endl;
  exit(1);
}

/*
 * convert black and white to gray
 */
struct bw2gray {
  private:
  static uint8 T[256][8];
  public:
  static inline size_t process(uint8* to, uint8* from, size_t size) {
    for(size_t i = size; i--;) memcpy(to + i*8, T[from[i]], 8);
    return 8*size;
  }
};


struct bw_mw2gray {
  private:
  static uint8 T[256][8];
  public:
  static inline size_t process(uint8* to, uint8* from, size_t size) {
    for(size_t i = size; i--;) memcpy(to + i*8, T[from[i]], 8);
    return 8*size;
  }
};


uint8 bw2gray::T[256][8] = {
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,255,  0,  0,  0,  0,  0,  0,255,  0,  0,  0,  0,  0,  0,  0,255,255,
  0,  0,  0,  0,  0,255,  0,  0,  0,  0,  0,  0,  0,255,  0,255,  0,  0,  0,  0,  0,255,255,  0,  0,  0,  0,  0,  0,255,255,255,
  0,  0,  0,  0,255,  0,  0,  0,  0,  0,  0,  0,255,  0,  0,255,  0,  0,  0,  0,255,  0,255,  0,  0,  0,  0,  0,255,  0,255,255,
  0,  0,  0,  0,255,255,  0,  0,  0,  0,  0,  0,255,255,  0,255,  0,  0,  0,  0,255,255,255,  0,  0,  0,  0,  0,255,255,255,255,
  0,  0,  0,255,  0,  0,  0,  0,  0,  0,  0,255,  0,  0,  0,255,  0,  0,  0,255,  0,  0,255,  0,  0,  0,  0,255,  0,  0,255,255,
  0,  0,  0,255,  0,255,  0,  0,  0,  0,  0,255,  0,255,  0,255,  0,  0,  0,255,  0,255,255,  0,  0,  0,  0,255,  0,255,255,255,
  0,  0,  0,255,255,  0,  0,  0,  0,  0,  0,255,255,  0,  0,255,  0,  0,  0,255,255,  0,255,  0,  0,  0,  0,255,255,  0,255,255,
  0,  0,  0,255,255,255,  0,  0,  0,  0,  0,255,255,255,  0,255,  0,  0,  0,255,255,255,255,  0,  0,  0,  0,255,255,255,255,255,
  0,  0,255,  0,  0,  0,  0,  0,  0,  0,255,  0,  0,  0,  0,255,  0,  0,255,  0,  0,  0,255,  0,  0,  0,255,  0,  0,  0,255,255,
  0,  0,255,  0,  0,255,  0,  0,  0,  0,255,  0,  0,255,  0,255,  0,  0,255,  0,  0,255,255,  0,  0,  0,255,  0,  0,255,255,255,
  0,  0,255,  0,255,  0,  0,  0,  0,  0,255,  0,255,  0,  0,255,  0,  0,255,  0,255,  0,255,  0,  0,  0,255,  0,255,  0,255,255,
  0,  0,255,  0,255,255,  0,  0,  0,  0,255,  0,255,255,  0,255,  0,  0,255,  0,255,255,255,  0,  0,  0,255,  0,255,255,255,255,
  0,  0,255,255,  0,  0,  0,  0,  0,  0,255,255,  0,  0,  0,255,  0,  0,255,255,  0,  0,255,  0,  0,  0,255,255,  0,  0,255,255,
  0,  0,255,255,  0,255,  0,  0,  0,  0,255,255,  0,255,  0,255,  0,  0,255,255,  0,255,255,  0,  0,  0,255,255,  0,255,255,255,
  0,  0,255,255,255,  0,  0,  0,  0,  0,255,255,255,  0,  0,255,  0,  0,255,255,255,  0,255,  0,  0,  0,255,255,255,  0,255,255,
  0,  0,255,255,255,255,  0,  0,  0,  0,255,255,255,255,  0,255,  0,  0,255,255,255,255,255,  0,  0,  0,255,255,255,255,255,255,
  0,255,  0,  0,  0,  0,  0,  0,  0,255,  0,  0,  0,  0,  0,255,  0,255,  0,  0,  0,  0,255,  0,  0,255,  0,  0,  0,  0,255,255,
  0,255,  0,  0,  0,255,  0,  0,  0,255,  0,  0,  0,255,  0,255,  0,255,  0,  0,  0,255,255,  0,  0,255,  0,  0,  0,255,255,255,
  0,255,  0,  0,255,  0,  0,  0,  0,255,  0,  0,255,  0,  0,255,  0,255,  0,  0,255,  0,255,  0,  0,255,  0,  0,255,  0,255,255,
  0,255,  0,  0,255,255,  0,  0,  0,255,  0,  0,255,255,  0,255,  0,255,  0,  0,255,255,255,  0,  0,255,  0,  0,255,255,255,255,
  0,255,  0,255,  0,  0,  0,  0,  0,255,  0,255,  0,  0,  0,255,  0,255,  0,255,  0,  0,255,  0,  0,255,  0,255,  0,  0,255,255,
  0,255,  0,255,  0,255,  0,  0,  0,255,  0,255,  0,255,  0,255,  0,255,  0,255,  0,255,255,  0,  0,255,  0,255,  0,255,255,255,
  0,255,  0,255,255,  0,  0,  0,  0,255,  0,255,255,  0,  0,255,  0,255,  0,255,255,  0,255,  0,  0,255,  0,255,255,  0,255,255,
  0,255,  0,255,255,255,  0,  0,  0,255,  0,255,255,255,  0,255,  0,255,  0,255,255,255,255,  0,  0,255,  0,255,255,255,255,255,
  0,255,255,  0,  0,  0,  0,  0,  0,255,255,  0,  0,  0,  0,255,  0,255,255,  0,  0,  0,255,  0,  0,255,255,  0,  0,  0,255,255,
  0,255,255,  0,  0,255,  0,  0,  0,255,255,  0,  0,255,  0,255,  0,255,255,  0,  0,255,255,  0,  0,255,255,  0,  0,255,255,255,
  0,255,255,  0,255,  0,  0,  0,  0,255,255,  0,255,  0,  0,255,  0,255,255,  0,255,  0,255,  0,  0,255,255,  0,255,  0,255,255,
  0,255,255,  0,255,255,  0,  0,  0,255,255,  0,255,255,  0,255,  0,255,255,  0,255,255,255,  0,  0,255,255,  0,255,255,255,255,
  0,255,255,255,  0,  0,  0,  0,  0,255,255,255,  0,  0,  0,255,  0,255,255,255,  0,  0,255,  0,  0,255,255,255,  0,  0,255,255,
  0,255,255,255,  0,255,  0,  0,  0,255,255,255,  0,255,  0,255,  0,255,255,255,  0,255,255,  0,  0,255,255,255,  0,255,255,255,
  0,255,255,255,255,  0,  0,  0,  0,255,255,255,255,  0,  0,255,  0,255,255,255,255,  0,255,  0,  0,255,255,255,255,  0,255,255,
  0,255,255,255,255,255,  0,  0,  0,255,255,255,255,255,  0,255,  0,255,255,255,255,255,255,  0,  0,255,255,255,255,255,255,255,
255,  0,  0,  0,  0,  0,  0,  0,255,  0,  0,  0,  0,  0,  0,255,255,  0,  0,  0,  0,  0,255,  0,255,  0,  0,  0,  0,  0,255,255,
255,  0,  0,  0,  0,255,  0,  0,255,  0,  0,  0,  0,255,  0,255,255,  0,  0,  0,  0,255,255,  0,255,  0,  0,  0,  0,255,255,255,
255,  0,  0,  0,255,  0,  0,  0,255,  0,  0,  0,255,  0,  0,255,255,  0,  0,  0,255,  0,255,  0,255,  0,  0,  0,255,  0,255,255,
255,  0,  0,  0,255,255,  0,  0,255,  0,  0,  0,255,255,  0,255,255,  0,  0,  0,255,255,255,  0,255,  0,  0,  0,255,255,255,255,
255,  0,  0,255,  0,  0,  0,  0,255,  0,  0,255,  0,  0,  0,255,255,  0,  0,255,  0,  0,255,  0,255,  0,  0,255,  0,  0,255,255,
255,  0,  0,255,  0,255,  0,  0,255,  0,  0,255,  0,255,  0,255,255,  0,  0,255,  0,255,255,  0,255,  0,  0,255,  0,255,255,255,
255,  0,  0,255,255,  0,  0,  0,255,  0,  0,255,255,  0,  0,255,255,  0,  0,255,255,  0,255,  0,255,  0,  0,255,255,  0,255,255,
255,  0,  0,255,255,255,  0,  0,255,  0,  0,255,255,255,  0,255,255,  0,  0,255,255,255,255,  0,255,  0,  0,255,255,255,255,255,
255,  0,255,  0,  0,  0,  0,  0,255,  0,255,  0,  0,  0,  0,255,255,  0,255,  0,  0,  0,255,  0,255,  0,255,  0,  0,  0,255,255,
255,  0,255,  0,  0,255,  0,  0,255,  0,255,  0,  0,255,  0,255,255,  0,255,  0,  0,255,255,  0,255,  0,255,  0,  0,255,255,255,
255,  0,255,  0,255,  0,  0,  0,255,  0,255,  0,255,  0,  0,255,255,  0,255,  0,255,  0,255,  0,255,  0,255,  0,255,  0,255,255,
255,  0,255,  0,255,255,  0,  0,255,  0,255,  0,255,255,  0,255,255,  0,255,  0,255,255,255,  0,255,  0,255,  0,255,255,255,255,
255,  0,255,255,  0,  0,  0,  0,255,  0,255,255,  0,  0,  0,255,255,  0,255,255,  0,  0,255,  0,255,  0,255,255,  0,  0,255,255,
255,  0,255,255,  0,255,  0,  0,255,  0,255,255,  0,255,  0,255,255,  0,255,255,  0,255,255,  0,255,  0,255,255,  0,255,255,255,
255,  0,255,255,255,  0,  0,  0,255,  0,255,255,255,  0,  0,255,255,  0,255,255,255,  0,255,  0,255,  0,255,255,255,  0,255,255,
255,  0,255,255,255,255,  0,  0,255,  0,255,255,255,255,  0,255,255,  0,255,255,255,255,255,  0,255,  0,255,255,255,255,255,255,
255,255,  0,  0,  0,  0,  0,  0,255,255,  0,  0,  0,  0,  0,255,255,255,  0,  0,  0,  0,255,  0,255,255,  0,  0,  0,  0,255,255,
255,255,  0,  0,  0,255,  0,  0,255,255,  0,  0,  0,255,  0,255,255,255,  0,  0,  0,255,255,  0,255,255,  0,  0,  0,255,255,255,
255,255,  0,  0,255,  0,  0,  0,255,255,  0,  0,255,  0,  0,255,255,255,  0,  0,255,  0,255,  0,255,255,  0,  0,255,  0,255,255,
255,255,  0,  0,255,255,  0,  0,255,255,  0,  0,255,255,  0,255,255,255,  0,  0,255,255,255,  0,255,255,  0,  0,255,255,255,255,
255,255,  0,255,  0,  0,  0,  0,255,255,  0,255,  0,  0,  0,255,255,255,  0,255,  0,  0,255,  0,255,255,  0,255,  0,  0,255,255,
255,255,  0,255,  0,255,  0,  0,255,255,  0,255,  0,255,  0,255,255,255,  0,255,  0,255,255,  0,255,255,  0,255,  0,255,255,255,
255,255,  0,255,255,  0,  0,  0,255,255,  0,255,255,  0,  0,255,255,255,  0,255,255,  0,255,  0,255,255,  0,255,255,  0,255,255,
255,255,  0,255,255,255,  0,  0,255,255,  0,255,255,255,  0,255,255,255,  0,255,255,255,255,  0,255,255,  0,255,255,255,255,255,
255,255,255,  0,  0,  0,  0,  0,255,255,255,  0,  0,  0,  0,255,255,255,255,  0,  0,  0,255,  0,255,255,255,  0,  0,  0,255,255,
255,255,255,  0,  0,255,  0,  0,255,255,255,  0,  0,255,  0,255,255,255,255,  0,  0,255,255,  0,255,255,255,  0,  0,255,255,255,
255,255,255,  0,255,  0,  0,  0,255,255,255,  0,255,  0,  0,255,255,255,255,  0,255,  0,255,  0,255,255,255,  0,255,  0,255,255,
255,255,255,  0,255,255,  0,  0,255,255,255,  0,255,255,  0,255,255,255,255,  0,255,255,255,  0,255,255,255,  0,255,255,255,255,
255,255,255,255,  0,  0,  0,  0,255,255,255,255,  0,  0,  0,255,255,255,255,255,  0,  0,255,  0,255,255,255,255,  0,  0,255,255,
255,255,255,255,  0,255,  0,  0,255,255,255,255,  0,255,  0,255,255,255,255,255,  0,255,255,  0,255,255,255,255,  0,255,255,255,
255,255,255,255,255,  0,  0,  0,255,255,255,255,255,  0,  0,255,255,255,255,255,255,  0,255,  0,255,255,255,255,255,  0,255,255,
255,255,255,255,255,255,  0,  0,255,255,255,255,255,255,  0,255,255,255,255,255,255,255,255,  0,255,255,255,255,255,255,255,255};
  


uint8 bw_mw2gray::T[256][8] = {
255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,  0,255,255,255,255,255,255,  0,255,255,255,255,255,255,255,  0,  0,
255,255,255,255,255,  0,255,255,255,255,255,255,255,  0,255,  0,255,255,255,255,255,  0,  0,255,255,255,255,255,255,  0,  0,  0,
255,255,255,255,  0,255,255,255,255,255,255,255,  0,255,255,  0,255,255,255,255,  0,255,  0,255,255,255,255,255,  0,255,  0,  0,
255,255,255,255,  0,  0,255,255,255,255,255,255,  0,  0,255,  0,255,255,255,255,  0,  0,  0,255,255,255,255,255,  0,  0,  0,  0,
255,255,255,  0,255,255,255,255,255,255,255,  0,255,255,255,  0,255,255,255,  0,255,255,  0,255,255,255,255,  0,255,255,  0,  0,
255,255,255,  0,255,  0,255,255,255,255,255,  0,255,  0,255,  0,255,255,255,  0,255,  0,  0,255,255,255,255,  0,255,  0,  0,  0,
255,255,255,  0,  0,255,255,255,255,255,255,  0,  0,255,255,  0,255,255,255,  0,  0,255,  0,255,255,255,255,  0,  0,255,  0,  0,
255,255,255,  0,  0,  0,255,255,255,255,255,  0,  0,  0,255,  0,255,255,255,  0,  0,  0,  0,255,255,255,255,  0,  0,  0,  0,  0,
255,255,  0,255,255,255,255,255,255,255,  0,255,255,255,255,  0,255,255,  0,255,255,255,  0,255,255,255,  0,255,255,255,  0,  0,
255,255,  0,255,255,  0,255,255,255,255,  0,255,255,  0,255,  0,255,255,  0,255,255,  0,  0,255,255,255,  0,255,255,  0,  0,  0,
255,255,  0,255,  0,255,255,255,255,255,  0,255,  0,255,255,  0,255,255,  0,255,  0,255,  0,255,255,255,  0,255,  0,255,  0,  0,
255,255,  0,255,  0,  0,255,255,255,255,  0,255,  0,  0,255,  0,255,255,  0,255,  0,  0,  0,255,255,255,  0,255,  0,  0,  0,  0,
255,255,  0,  0,255,255,255,255,255,255,  0,  0,255,255,255,  0,255,255,  0,  0,255,255,  0,255,255,255,  0,  0,255,255,  0,  0,
255,255,  0,  0,255,  0,255,255,255,255,  0,  0,255,  0,255,  0,255,255,  0,  0,255,  0,  0,255,255,255,  0,  0,255,  0,  0,  0,
255,255,  0,  0,  0,255,255,255,255,255,  0,  0,  0,255,255,  0,255,255,  0,  0,  0,255,  0,255,255,255,  0,  0,  0,255,  0,  0,
255,255,  0,  0,  0,  0,255,255,255,255,  0,  0,  0,  0,255,  0,255,255,  0,  0,  0,  0,  0,255,255,255,  0,  0,  0,  0,  0,  0,
255,  0,255,255,255,255,255,255,255,  0,255,255,255,255,255,  0,255,  0,255,255,255,255,  0,255,255,  0,255,255,255,255,  0,  0,
255,  0,255,255,255,  0,255,255,255,  0,255,255,255,  0,255,  0,255,  0,255,255,255,  0,  0,255,255,  0,255,255,255,  0,  0,  0,
255,  0,255,255,  0,255,255,255,255,  0,255,255,  0,255,255,  0,255,  0,255,255,  0,255,  0,255,255,  0,255,255,  0,255,  0,  0,
255,  0,255,255,  0,  0,255,255,255,  0,255,255,  0,  0,255,  0,255,  0,255,255,  0,  0,  0,255,255,  0,255,255,  0,  0,  0,  0,
255,  0,255,  0,255,255,255,255,255,  0,255,  0,255,255,255,  0,255,  0,255,  0,255,255,  0,255,255,  0,255,  0,255,255,  0,  0,
255,  0,255,  0,255,  0,255,255,255,  0,255,  0,255,  0,255,  0,255,  0,255,  0,255,  0,  0,255,255,  0,255,  0,255,  0,  0,  0,
255,  0,255,  0,  0,255,255,255,255,  0,255,  0,  0,255,255,  0,255,  0,255,  0,  0,255,  0,255,255,  0,255,  0,  0,255,  0,  0,
255,  0,255,  0,  0,  0,255,255,255,  0,255,  0,  0,  0,255,  0,255,  0,255,  0,  0,  0,  0,255,255,  0,255,  0,  0,  0,  0,  0,
255,  0,  0,255,255,255,255,255,255,  0,  0,255,255,255,255,  0,255,  0,  0,255,255,255,  0,255,255,  0,  0,255,255,255,  0,  0,
255,  0,  0,255,255,  0,255,255,255,  0,  0,255,255,  0,255,  0,255,  0,  0,255,255,  0,  0,255,255,  0,  0,255,255,  0,  0,  0,
255,  0,  0,255,  0,255,255,255,255,  0,  0,255,  0,255,255,  0,255,  0,  0,255,  0,255,  0,255,255,  0,  0,255,  0,255,  0,  0,
255,  0,  0,255,  0,  0,255,255,255,  0,  0,255,  0,  0,255,  0,255,  0,  0,255,  0,  0,  0,255,255,  0,  0,255,  0,  0,  0,  0,
255,  0,  0,  0,255,255,255,255,255,  0,  0,  0,255,255,255,  0,255,  0,  0,  0,255,255,  0,255,255,  0,  0,  0,255,255,  0,  0,
255,  0,  0,  0,255,  0,255,255,255,  0,  0,  0,255,  0,255,  0,255,  0,  0,  0,255,  0,  0,255,255,  0,  0,  0,255,  0,  0,  0,
255,  0,  0,  0,  0,255,255,255,255,  0,  0,  0,  0,255,255,  0,255,  0,  0,  0,  0,255,  0,255,255,  0,  0,  0,  0,255,  0,  0,
255,  0,  0,  0,  0,  0,255,255,255,  0,  0,  0,  0,  0,255,  0,255,  0,  0,  0,  0,  0,  0,255,255,  0,  0,  0,  0,  0,  0,  0,
  0,255,255,255,255,255,255,255,  0,255,255,255,255,255,255,  0,  0,255,255,255,255,255,  0,255,  0,255,255,255,255,255,  0,  0,
  0,255,255,255,255,  0,255,255,  0,255,255,255,255,  0,255,  0,  0,255,255,255,255,  0,  0,255,  0,255,255,255,255,  0,  0,  0,
  0,255,255,255,  0,255,255,255,  0,255,255,255,  0,255,255,  0,  0,255,255,255,  0,255,  0,255,  0,255,255,255,  0,255,  0,  0,
  0,255,255,255,  0,  0,255,255,  0,255,255,255,  0,  0,255,  0,  0,255,255,255,  0,  0,  0,255,  0,255,255,255,  0,  0,  0,  0,
  0,255,255,  0,255,255,255,255,  0,255,255,  0,255,255,255,  0,  0,255,255,  0,255,255,  0,255,  0,255,255,  0,255,255,  0,  0,
  0,255,255,  0,255,  0,255,255,  0,255,255,  0,255,  0,255,  0,  0,255,255,  0,255,  0,  0,255,  0,255,255,  0,255,  0,  0,  0,
  0,255,255,  0,  0,255,255,255,  0,255,255,  0,  0,255,255,  0,  0,255,255,  0,  0,255,  0,255,  0,255,255,  0,  0,255,  0,  0,
  0,255,255,  0,  0,  0,255,255,  0,255,255,  0,  0,  0,255,  0,  0,255,255,  0,  0,  0,  0,255,  0,255,255,  0,  0,  0,  0,  0,
  0,255,  0,255,255,255,255,255,  0,255,  0,255,255,255,255,  0,  0,255,  0,255,255,255,  0,255,  0,255,  0,255,255,255,  0,  0,
  0,255,  0,255,255,  0,255,255,  0,255,  0,255,255,  0,255,  0,  0,255,  0,255,255,  0,  0,255,  0,255,  0,255,255,  0,  0,  0,
  0,255,  0,255,  0,255,255,255,  0,255,  0,255,  0,255,255,  0,  0,255,  0,255,  0,255,  0,255,  0,255,  0,255,  0,255,  0,  0,
  0,255,  0,255,  0,  0,255,255,  0,255,  0,255,  0,  0,255,  0,  0,255,  0,255,  0,  0,  0,255,  0,255,  0,255,  0,  0,  0,  0,
  0,255,  0,  0,255,255,255,255,  0,255,  0,  0,255,255,255,  0,  0,255,  0,  0,255,255,  0,255,  0,255,  0,  0,255,255,  0,  0,
  0,255,  0,  0,255,  0,255,255,  0,255,  0,  0,255,  0,255,  0,  0,255,  0,  0,255,  0,  0,255,  0,255,  0,  0,255,  0,  0,  0,
  0,255,  0,  0,  0,255,255,255,  0,255,  0,  0,  0,255,255,  0,  0,255,  0,  0,  0,255,  0,255,  0,255,  0,  0,  0,255,  0,  0,
  0,255,  0,  0,  0,  0,255,255,  0,255,  0,  0,  0,  0,255,  0,  0,255,  0,  0,  0,  0,  0,255,  0,255,  0,  0,  0,  0,  0,  0,
  0,  0,255,255,255,255,255,255,  0,  0,255,255,255,255,255,  0,  0,  0,255,255,255,255,  0,255,  0,  0,255,255,255,255,  0,  0,
  0,  0,255,255,255,  0,255,255,  0,  0,255,255,255,  0,255,  0,  0,  0,255,255,255,  0,  0,255,  0,  0,255,255,255,  0,  0,  0,
  0,  0,255,255,  0,255,255,255,  0,  0,255,255,  0,255,255,  0,  0,  0,255,255,  0,255,  0,255,  0,  0,255,255,  0,255,  0,  0,
  0,  0,255,255,  0,  0,255,255,  0,  0,255,255,  0,  0,255,  0,  0,  0,255,255,  0,  0,  0,255,  0,  0,255,255,  0,  0,  0,  0,
  0,  0,255,  0,255,255,255,255,  0,  0,255,  0,255,255,255,  0,  0,  0,255,  0,255,255,  0,255,  0,  0,255,  0,255,255,  0,  0,
  0,  0,255,  0,255,  0,255,255,  0,  0,255,  0,255,  0,255,  0,  0,  0,255,  0,255,  0,  0,255,  0,  0,255,  0,255,  0,  0,  0,
  0,  0,255,  0,  0,255,255,255,  0,  0,255,  0,  0,255,255,  0,  0,  0,255,  0,  0,255,  0,255,  0,  0,255,  0,  0,255,  0,  0,
  0,  0,255,  0,  0,  0,255,255,  0,  0,255,  0,  0,  0,255,  0,  0,  0,255,  0,  0,  0,  0,255,  0,  0,255,  0,  0,  0,  0,  0,
  0,  0,  0,255,255,255,255,255,  0,  0,  0,255,255,255,255,  0,  0,  0,  0,255,255,255,  0,255,  0,  0,  0,255,255,255,  0,  0,
  0,  0,  0,255,255,  0,255,255,  0,  0,  0,255,255,  0,255,  0,  0,  0,  0,255,255,  0,  0,255,  0,  0,  0,255,255,  0,  0,  0,
  0,  0,  0,255,  0,255,255,255,  0,  0,  0,255,  0,255,255,  0,  0,  0,  0,255,  0,255,  0,255,  0,  0,  0,255,  0,255,  0,  0,
  0,  0,  0,255,  0,  0,255,255,  0,  0,  0,255,  0,  0,255,  0,  0,  0,  0,255,  0,  0,  0,255,  0,  0,  0,255,  0,  0,  0,  0,
  0,  0,  0,  0,255,255,255,255,  0,  0,  0,  0,255,255,255,  0,  0,  0,  0,  0,255,255,  0,255,  0,  0,  0,  0,255,255,  0,  0,
  0,  0,  0,  0,255,  0,255,255,  0,  0,  0,  0,255,  0,255,  0,  0,  0,  0,  0,255,  0,  0,255,  0,  0,  0,  0,255,  0,  0,  0,
  0,  0,  0,  0,  0,255,255,255,  0,  0,  0,  0,  0,255,255,  0,  0,  0,  0,  0,  0,255,  0,255,  0,  0,  0,  0,  0,255,  0,  0,
  0,  0,  0,  0,  0,  0,255,255,  0,  0,  0,  0,  0,  0,255,  0,  0,  0,  0,  0,  0,  0,  0,255,  0,  0,  0,  0,  0,  0,  0,  0};












template<class T>
void tiff_copy(TIFF *input, TIFF *output) {
  uint8 buffer_in[TIFFStripSize(input)];
  uint8 buffer_out[TIFFStripSize(output)];
  int nb_strip = TIFFNumberOfStrips(input);  
  for(int n = 0; n < nb_strip; n++) {
    int size = TIFFReadEncodedStrip(input, n, buffer_in, -1);
    if(size == -1) error("Error reading data");
    size = T::process(buffer_out, buffer_in, size);
    if(TIFFWriteEncodedStrip(output, n, buffer_out, size) == -1) error("Error writing data");
  }
}


#define pack(a,b,c,d)	((long)(((a)<<24)|((b)<<16)|((c)<<8)|(d)))

int main(int argc, char* argv[]) {
  TIFF *INPUT, *OUTPUT;
  char *input_file = 0, *output_file = 0;
  uint32 width, height, rowsperstrip;
  uint16 bitspersample, samplesperpixel, photometric, compression = -1, planarconfig;

  for(int i = 1; i < argc; i++) {
    if(argv[i][0] == '-') {
      switch(argv[i][1]) {
        case 'c': // compression
          if(++i == argc) error("Error in -c option");
          if(strncmp(argv[i], "none",4) == 0) compression = COMPRESSION_NONE;
          else if(strncmp(argv[i], "zip",3) == 0) compression = COMPRESSION_ADOBE_DEFLATE;
          else if(strncmp(argv[i], "packbits",8) == 0) compression = COMPRESSION_PACKBITS;
          else if(strncmp(argv[i], "jpeg",4) == 0) compression = COMPRESSION_JPEG;
          else if(strncmp(argv[i], "lzw",3) == 0) compression = COMPRESSION_LZW;
          else compression = COMPRESSION_NONE;          
          break;          
        default : 
          usage();
          exit(0);
      }
    }
    else {
      if(!input_file) input_file = argv[i];
      else if(!output_file) output_file = argv[i];
      else error("Error : argument must specify exactly one input file and one output file");
    }
  }
  if(!output_file) error("Error : argument must specify exactly one input file and one output file");
 
  INPUT = TIFFOpen(input_file, "r");
  if(INPUT == NULL) error("Unable to open input file: " + string(input_file));
  
  if(compression == (uint16)(-1)) TIFFGetField(INPUT, TIFFTAG_COMPRESSION, &compression);

  if(! TIFFGetField(INPUT, TIFFTAG_IMAGEWIDTH, &width)                   ||
     ! TIFFGetField(INPUT, TIFFTAG_IMAGELENGTH, &height)                 ||
     ! TIFFGetField(INPUT, TIFFTAG_BITSPERSAMPLE, &bitspersample)        ||
     ! TIFFGetFieldDefaulted(INPUT, TIFFTAG_PLANARCONFIG, &planarconfig) ||
     ! TIFFGetField(INPUT, TIFFTAG_PHOTOMETRIC, &photometric)            ||
     ! TIFFGetField(INPUT, TIFFTAG_ROWSPERSTRIP, &rowsperstrip)          ||
     ! TIFFGetFieldDefaulted(INPUT, TIFFTAG_SAMPLESPERPIXEL, &samplesperpixel))
    error("Error reading input file: " + string(input_file));
        
  if(planarconfig != 1) error("Sorry : only planarconfig = 1 is supported");

  OUTPUT = TIFFOpen(output_file, "w");
  if(OUTPUT == NULL) error("Unable to open output file: " + string(output_file));
  if(! TIFFSetField(OUTPUT, TIFFTAG_IMAGEWIDTH, width)                   ||
     ! TIFFSetField(OUTPUT, TIFFTAG_IMAGELENGTH, height)                 ||
     ! TIFFSetField(OUTPUT, TIFFTAG_BITSPERSAMPLE, 8)                    ||
     ! TIFFSetField(OUTPUT, TIFFTAG_SAMPLESPERPIXEL, 1)                  ||
     ! TIFFSetField(OUTPUT, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK) ||
     ! TIFFSetField(OUTPUT, TIFFTAG_ROWSPERSTRIP, rowsperstrip)          ||
     ! TIFFSetField(OUTPUT, TIFFTAG_COMPRESSION, compression))
    error("Error writing output file: " + string(output_file));

  switch(pack(planarconfig,photometric,samplesperpixel,bitspersample)) {
    case pack(PLANARCONFIG_CONTIG,PHOTOMETRIC_MINISWHITE,1,1) : tiff_copy<bw_mw2gray>(INPUT, OUTPUT); break;
    case pack(PLANARCONFIG_CONTIG,PHOTOMETRIC_MINISBLACK,1,1) : tiff_copy<bw2gray>(INPUT, OUTPUT); break;
    default : error("Unsupported input format");
  }  
  
  TIFFClose(INPUT);
  TIFFClose(OUTPUT);
  return 0;
}

