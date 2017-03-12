#include "base/common.h"

#include <cstring>
#include <iostream>

#define IS_BIG_ENDIAN (*(uint16_t *)"\0\xff" < 0x100)
#define IS_LITTLE_ENDIAN !(IS_BIG_ENDIAN)

class MD5
{
public:
  using size_type = uint32_t;
  
  MD5();
  MD5(const std::string& text);
  void update(const byte *buf, size_type length);
  void update(const char *buf, size_type length);
  MD5& finalize();
  std::string hexdigest() const;
  friend std::ostream& operator<<(std::ostream&, MD5 md5);
  
private:
  void init();
  using uint1 = u8;
  using uint4 = u32;
  static constexpr u32 blocksize = 64;
  
  void transform(const uint1 block[blocksize]);
  static void decode(uint4 output[], const uint1 input[], size_type len);
  static void encode(uint1 output[], const uint4 input[], size_type len);
  
  bool finalized;
  uint1 buffer[blocksize]; // bytes that didn't fit in last 64 byte chunk
  uint4 count[2];   // 64bit counter for number of bits (lo, hi)
  uint4 state[4];   // digest so far
  uint1 digest[16]; // the result
  
  // low level logic operations
  static inline uint4 F(uint4 x, uint4 y, uint4 z) { return (x&y) | (~x&z); }
  static inline uint4 G(uint4 x, uint4 y, uint4 z) { return (x&z) | (y&~z); }
  static inline uint4 H(uint4 x, uint4 y, uint4 z) { return x^y^z; }
  static inline uint4 I(uint4 x, uint4 y, uint4 z) { return y ^ (x | ~z); }
  
  static inline uint4 rotate_left(uint4 x, int n) { return (x << n) | (x >> (32-n)); }
  
  using md5_functor = uint4(*)(uint4, uint4, uint4);
  
  template<md5_functor F>
  static inline void rf(uint4& a, uint4 b, uint4 c, uint4 d, uint4 x, uint4 s, uint4 ac)
  {
    a = rotate_left(a + F(b, c, d) + x + ac, s) + b;
  }
};

#define S11 7
#define S12 12
#define S13 17
#define S14 22
#define S21 5
#define S22 9
#define S23 14
#define S24 20
#define S31 4
#define S32 11
#define S33 16
#define S34 23
#define S41 6
#define S42 10
#define S43 15
#define S44 21


//////////////////////////////////////////////

// default ctor, just initailize
MD5::MD5()
{
  init();
}

//////////////////////////////////////////////

// nifty shortcut ctor, compute MD5 for string and finalize it right away
MD5::MD5(const std::string &text)
{
  init();
  update(text.c_str(), text.length());
  finalize();
}

//////////////////////////////

void MD5::init()
{
  finalized = false;
  
  count[0] = 0;
  count[1] = 0;
  
  state[0] = 0x67452301;
  state[1] = 0xefcdab89;
  state[2] = 0x98badcfe;
  state[3] = 0x10325476;
}

//////////////////////////////

// decodes input (unsigned char) into output (uint4). Assumes len is a multiple of 4.
void MD5::decode(uint4* output, const uint1* input, size_type len)
{
  for (unsigned int i = 0, j = 0; j < len; i++, j += 4)
    output[i] = ((uint4)input[j]) | (((uint4)input[j+1]) << 8) |
    (((uint4)input[j+2]) << 16) | (((uint4)input[j+3]) << 24);
}

//////////////////////////////

// encodes input (uint4) into output (unsigned char). Assumes len is
// a multiple of 4.
void MD5::encode(uint1* output, const uint4* input, size_type len)
{
  for (size_type i = 0, j = 0; j < len; i++, j += 4) {
    output[j] = input[i] & 0xff;
    output[j+1] = (input[i] >> 8) & 0xff;
    output[j+2] = (input[i] >> 16) & 0xff;
    output[j+3] = (input[i] >> 24) & 0xff;
  }
}

//////////////////////////////

// apply MD5 algo on a block
void MD5::transform(const uint1 block[blocksize])
{
  uint4 a = state[0], b = state[1], c = state[2], d = state[3], x[16];
  decode (x, block, blocksize);
  
  /* Round 1 */
  rf<F>(a, b, c, d, x[ 0], S11, 0xd76aa478); /* 1 */
  rf<F> (d, a, b, c, x[ 1], S12, 0xe8c7b756); /* 2 */
  rf<F> (c, d, a, b, x[ 2], S13, 0x242070db); /* 3 */
  rf<F> (b, c, d, a, x[ 3], S14, 0xc1bdceee); /* 4 */
  rf<F> (a, b, c, d, x[ 4], S11, 0xf57c0faf); /* 5 */
  rf<F> (d, a, b, c, x[ 5], S12, 0x4787c62a); /* 6 */
  rf<F> (c, d, a, b, x[ 6], S13, 0xa8304613); /* 7 */
  rf<F> (b, c, d, a, x[ 7], S14, 0xfd469501); /* 8 */
  rf<F> (a, b, c, d, x[ 8], S11, 0x698098d8); /* 9 */
  rf<F> (d, a, b, c, x[ 9], S12, 0x8b44f7af); /* 10 */
  rf<F> (c, d, a, b, x[10], S13, 0xffff5bb1); /* 11 */
  rf<F> (b, c, d, a, x[11], S14, 0x895cd7be); /* 12 */
  rf<F> (a, b, c, d, x[12], S11, 0x6b901122); /* 13 */
  rf<F> (d, a, b, c, x[13], S12, 0xfd987193); /* 14 */
  rf<F> (c, d, a, b, x[14], S13, 0xa679438e); /* 15 */
  rf<F> (b, c, d, a, x[15], S14, 0x49b40821); /* 16 */
  
  /* Round 2 */
  rf<G> (a, b, c, d, x[ 1], S21, 0xf61e2562); /* 17 */
  rf<G> (d, a, b, c, x[ 6], S22, 0xc040b340); /* 18 */
  rf<G> (c, d, a, b, x[11], S23, 0x265e5a51); /* 19 */
  rf<G> (b, c, d, a, x[ 0], S24, 0xe9b6c7aa); /* 20 */
  rf<G> (a, b, c, d, x[ 5], S21, 0xd62f105d); /* 21 */
  rf<G> (d, a, b, c, x[10], S22,  0x2441453); /* 22 */
  rf<G> (c, d, a, b, x[15], S23, 0xd8a1e681); /* 23 */
  rf<G> (b, c, d, a, x[ 4], S24, 0xe7d3fbc8); /* 24 */
  rf<G> (a, b, c, d, x[ 9], S21, 0x21e1cde6); /* 25 */
  rf<G> (d, a, b, c, x[14], S22, 0xc33707d6); /* 26 */
  rf<G> (c, d, a, b, x[ 3], S23, 0xf4d50d87); /* 27 */
  rf<G> (b, c, d, a, x[ 8], S24, 0x455a14ed); /* 28 */
  rf<G> (a, b, c, d, x[13], S21, 0xa9e3e905); /* 29 */
  rf<G> (d, a, b, c, x[ 2], S22, 0xfcefa3f8); /* 30 */
  rf<G> (c, d, a, b, x[ 7], S23, 0x676f02d9); /* 31 */
  rf<G> (b, c, d, a, x[12], S24, 0x8d2a4c8a); /* 32 */
  
  /* Round 3 */
  rf<H> (a, b, c, d, x[ 5], S31, 0xfffa3942); /* 33 */
  rf<H> (d, a, b, c, x[ 8], S32, 0x8771f681); /* 34 */
  rf<H> (c, d, a, b, x[11], S33, 0x6d9d6122); /* 35 */
  rf<H> (b, c, d, a, x[14], S34, 0xfde5380c); /* 36 */
  rf<H> (a, b, c, d, x[ 1], S31, 0xa4beea44); /* 37 */
  rf<H> (d, a, b, c, x[ 4], S32, 0x4bdecfa9); /* 38 */
  rf<H> (c, d, a, b, x[ 7], S33, 0xf6bb4b60); /* 39 */
  rf<H> (b, c, d, a, x[10], S34, 0xbebfbc70); /* 40 */
  rf<H> (a, b, c, d, x[13], S31, 0x289b7ec6); /* 41 */
  rf<H> (d, a, b, c, x[ 0], S32, 0xeaa127fa); /* 42 */
  rf<H> (c, d, a, b, x[ 3], S33, 0xd4ef3085); /* 43 */
  rf<H> (b, c, d, a, x[ 6], S34,  0x4881d05); /* 44 */
  rf<H> (a, b, c, d, x[ 9], S31, 0xd9d4d039); /* 45 */
  rf<H> (d, a, b, c, x[12], S32, 0xe6db99e5); /* 46 */
  rf<H> (c, d, a, b, x[15], S33, 0x1fa27cf8); /* 47 */
  rf<H> (b, c, d, a, x[ 2], S34, 0xc4ac5665); /* 48 */
  
  /* Round 4 */
  rf<I> (a, b, c, d, x[ 0], S41, 0xf4292244); /* 49 */
  rf<I> (d, a, b, c, x[ 7], S42, 0x432aff97); /* 50 */
  rf<I> (c, d, a, b, x[14], S43, 0xab9423a7); /* 51 */
  rf<I> (b, c, d, a, x[ 5], S44, 0xfc93a039); /* 52 */
  rf<I> (a, b, c, d, x[12], S41, 0x655b59c3); /* 53 */
  rf<I> (d, a, b, c, x[ 3], S42, 0x8f0ccc92); /* 54 */
  rf<I> (c, d, a, b, x[10], S43, 0xffeff47d); /* 55 */
  rf<I> (b, c, d, a, x[ 1], S44, 0x85845dd1); /* 56 */
  rf<I> (a, b, c, d, x[ 8], S41, 0x6fa87e4f); /* 57 */
  rf<I> (d, a, b, c, x[15], S42, 0xfe2ce6e0); /* 58 */
  rf<I> (c, d, a, b, x[ 6], S43, 0xa3014314); /* 59 */
  rf<I> (b, c, d, a, x[13], S44, 0x4e0811a1); /* 60 */
  rf<I> (a, b, c, d, x[ 4], S41, 0xf7537e82); /* 61 */
  rf<I> (d, a, b, c, x[11], S42, 0xbd3af235); /* 62 */
  rf<I> (c, d, a, b, x[ 2], S43, 0x2ad7d2bb); /* 63 */
  rf<I> (b, c, d, a, x[ 9], S44, 0xeb86d391); /* 64 */
  
  state[0] += a;
  state[1] += b;
  state[2] += c;
  state[3] += d;
  
  // Zeroize sensitive information.
  memset(x, 0, sizeof x);
}

//////////////////////////////

// MD5 block update operation. Continues an MD5 message-digest
// operation, processing another message block
void MD5::update(const unsigned char input[], size_type length)
{
  // compute number of bytes mod 64
  size_type index = count[0] / 8 % blocksize;
  
  // Update number of bits
  if ((count[0] += (length << 3)) < (length << 3))
    count[1]++;
  count[1] += (length >> 29);
  
  // number of bytes we need to fill in buffer
  size_type firstpart = 64 - index;
  
  size_type i;
  
  // transform as many times as possible.
  if (length >= firstpart)
  {
    // fill buffer first, transform
    memcpy(&buffer[index], input, firstpart);
    transform(buffer);
    
    // transform chunks of blocksize (64 bytes)
    for (i = firstpart; i + blocksize <= length; i += blocksize)
      transform(&input[i]);
    
    index = 0;
  }
  else
    i = 0;
  
  // buffer remaining input
  memcpy(&buffer[index], &input[i], length-i);
}

//////////////////////////////

// for convenience provide a verson with signed char
void MD5::update(const char input[], size_type length)
{
  update((const unsigned char*)input, length);
}

//////////////////////////////

// MD5 finalization. Ends an MD5 message-digest operation, writing the
// the message digest and zeroizing the context.
MD5& MD5::finalize()
{
  static unsigned char padding[64] = {
    0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
  };
  
  if (!finalized) {
    // Save number of bits
    unsigned char bits[8];
    encode(bits, count, 8);
    
    // pad out to 56 mod 64.
    size_type index = count[0] / 8 % 64;
    size_type padLen = (index < 56) ? (56 - index) : (120 - index);
    update(padding, padLen);
    
    // Append length (before padding)
    update(bits, 8);
    
    // Store state in digest
    encode(digest, state, 16);
    
    // Zeroize sensitive information.
    memset(buffer, 0, sizeof buffer);
    memset(count, 0, sizeof count);
    
    finalized=true;
  }
  
  return *this;
}

//////////////////////////////

// return hex representation of digest as string
std::string MD5::hexdigest() const
{
  if (!finalized)
    return "";
  
  char buf[33];
  for (int i=0; i<16; i++)
    sprintf(buf+i*2, "%02x", digest[i]);
  buf[32]=0;
  
  return std::string(buf);
}

//////////////////////////////

std::ostream& operator<<(std::ostream& out, MD5 md5)
{
  return out << md5.hexdigest();
}

//////////////////////////////

std::string md5(const std::string str)
{
  MD5 md5 = MD5(str);
  
  return md5.hexdigest();
}

void test()
{
  MD5 md5 = MD5("helloworld");
  std::cout << md5 << std::endl;
  
  assert(md5.hexdigest() == "fc5e038d38a57032085441e7fe7010b0");
}