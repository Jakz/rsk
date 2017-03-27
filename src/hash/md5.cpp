#include "base/common.h"
#include "base/exceptions.h"

#include <cstring>
#include <iostream>

#if defined(__BYTE_ORDER) && __BYTE_ORDER == __BIG_ENDIAN || \
\
  defined(__BIG_ENDIAN__) || \
  defined(__ARMEB__) || \
  defined(__THUMBEB__) || \
  defined(__AARCH64EB__) || \
  defined(_MIBSEB) || defined(__MIBSEB) || defined(__MIBSEB__)

#define IS_BIG_ENDIAN
constexpr bool IS_LITTLE_ENDIAN_ = false;

#elif defined(__BYTE_ORDER) && __BYTE_ORDER == __LITTLE_ENDIAN || \
\
  defined(__LITTLE_ENDIAN__) || \
  defined(__ARMEL__) || \
  defined(__THUMBEL__) || \
  defined(__AARCH64EL__) || \
  defined(_MIPSEL) || defined(__MIPSEL) || defined(__MIPSEL__)

#define IS_LITTLE_ENDIAN
constexpr bool IS_LITTLE_ENDIAN_ = true;

#else
  #error "Unknown endianness!"
#endif

union aliased_uint32
{
  u32 value;
  u8 bytes[4];
};

struct aliased_block
{
  aliased_uint32 values[16];
  u32 operator[](size_t index) const { return values[index].value; }
};

class MD5
{
public:
  using size_type = uint32_t;
  
  MD5();
  MD5(const std::string& text);
  void update(const byte *buf, size_t length);
  void update(const char *buf, size_t length);
  MD5& finalize();
  std::string hexdigest(bool uppercase) const;
  friend std::ostream& operator<<(std::ostream&, MD5 md5);
  
private:
  using block_t = std::conditional<IS_LITTLE_ENDIAN_, u8[64], aliased_block>::type;
  
  void init();
  static constexpr u32 blocksize = 64;
  
  void transform(const u8 block[blocksize]);
  static void decode(u32 output[], const u8 input[], size_t len);
  static void encode(u8 output[], const u32 input[], size_t len);
  
  bool finalized;
  u8 buffer[blocksize]; // bytes that didn't fit in last 64 byte chunk
  u32 count[2];   // 64bit counter for number of bits (lo, hi)
  u32 state[4];   // digest so far
  u8 digest[16]; // the result
  
  // low level logic operations
  static inline u32 F(u32 x, u32 y, u32 z) { return (x&y) | (~x&z); }
  static inline u32 G(u32 x, u32 y, u32 z) { return (x&z) | (y&~z); }
  static inline u32 H(u32 x, u32 y, u32 z) { return x^y^z; }
  static inline u32 I(u32 x, u32 y, u32 z) { return y ^ (x | ~z); }
  
  static inline u32 rotate_left(u32 x, int n) { return (x << n) | (x >> (32-n)); }
  
  using md5_functor = u32(*)(u32, u32, u32);
  
  template<md5_functor F>
  static inline void rf(u32& a, u32 b, u32 c, u32 d, u32 x, u32 s, u32 ac)
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

// decodes input (unsigned char) into output (u32). Assumes len is a multiple of 4.
void MD5::decode(u32* output, const u8* input, size_t len)
{
  for (unsigned int i = 0, j = 0; j < len; i++, j += 4)
    output[i] = ((u32)input[j]) | (((u32)input[j+1]) << 8) |
    (((u32)input[j+2]) << 16) | (((u32)input[j+3]) << 24);
}

//////////////////////////////

// encodes input (u32) into output (unsigned char). Assumes len is
// a multiple of 4.
void MD5::encode(u8* output, const u32* input, size_t len)
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
void MD5::transform(const u8 block[blocksize])
{
  static_assert(sizeof(aliased_uint32) == sizeof(u32), "");
  static_assert(sizeof(aliased_block) == sizeof(u32)*16, "");
  
  u32 a = state[0], b = state[1], c = state[2], d = state[3];
  
  u32 x[16];
  decode(x, block, blocksize);
  
  /* Round 1 */
  rf<F>(a, b, c, d, x[ 0], S11, 0xd76aa478); /* 1 */
  rf<F>(d, a, b, c, x[ 1], S12, 0xe8c7b756); /* 2 */
  rf<F>(c, d, a, b, x[ 2], S13, 0x242070db); /* 3 */
  rf<F>(b, c, d, a, x[ 3], S14, 0xc1bdceee); /* 4 */
  rf<F>(a, b, c, d, x[ 4], S11, 0xf57c0faf); /* 5 */
  rf<F>(d, a, b, c, x[ 5], S12, 0x4787c62a); /* 6 */
  rf<F>(c, d, a, b, x[ 6], S13, 0xa8304613); /* 7 */
  rf<F>(b, c, d, a, x[ 7], S14, 0xfd469501); /* 8 */
  rf<F>(a, b, c, d, x[ 8], S11, 0x698098d8); /* 9 */
  rf<F>(d, a, b, c, x[ 9], S12, 0x8b44f7af); /* 10 */
  rf<F>(c, d, a, b, x[10], S13, 0xffff5bb1); /* 11 */
  rf<F>(b, c, d, a, x[11], S14, 0x895cd7be); /* 12 */
  rf<F>(a, b, c, d, x[12], S11, 0x6b901122); /* 13 */
  rf<F>(d, a, b, c, x[13], S12, 0xfd987193); /* 14 */
  rf<F>(c, d, a, b, x[14], S13, 0xa679438e); /* 15 */
  rf<F>(b, c, d, a, x[15], S14, 0x49b40821); /* 16 */
  
  /* Round 2 */
  rf<G>(a, b, c, d, x[ 1], S21, 0xf61e2562); /* 17 */
  rf<G>(d, a, b, c, x[ 6], S22, 0xc040b340); /* 18 */
  rf<G>(c, d, a, b, x[11], S23, 0x265e5a51); /* 19 */
  rf<G>(b, c, d, a, x[ 0], S24, 0xe9b6c7aa); /* 20 */
  rf<G>(a, b, c, d, x[ 5], S21, 0xd62f105d); /* 21 */
  rf<G>(d, a, b, c, x[10], S22,  0x2441453); /* 22 */
  rf<G>(c, d, a, b, x[15], S23, 0xd8a1e681); /* 23 */
  rf<G>(b, c, d, a, x[ 4], S24, 0xe7d3fbc8); /* 24 */
  rf<G>(a, b, c, d, x[ 9], S21, 0x21e1cde6); /* 25 */
  rf<G>(d, a, b, c, x[14], S22, 0xc33707d6); /* 26 */
  rf<G>(c, d, a, b, x[ 3], S23, 0xf4d50d87); /* 27 */
  rf<G>(b, c, d, a, x[ 8], S24, 0x455a14ed); /* 28 */
  rf<G>(a, b, c, d, x[13], S21, 0xa9e3e905); /* 29 */
  rf<G>(d, a, b, c, x[ 2], S22, 0xfcefa3f8); /* 30 */
  rf<G>(c, d, a, b, x[ 7], S23, 0x676f02d9); /* 31 */
  rf<G>(b, c, d, a, x[12], S24, 0x8d2a4c8a); /* 32 */
  
  /* Round 3 */
  rf<H>(a, b, c, d, x[ 5], S31, 0xfffa3942); /* 33 */
  rf<H>(d, a, b, c, x[ 8], S32, 0x8771f681); /* 34 */
  rf<H>(c, d, a, b, x[11], S33, 0x6d9d6122); /* 35 */
  rf<H>(b, c, d, a, x[14], S34, 0xfde5380c); /* 36 */
  rf<H>(a, b, c, d, x[ 1], S31, 0xa4beea44); /* 37 */
  rf<H>(d, a, b, c, x[ 4], S32, 0x4bdecfa9); /* 38 */
  rf<H>(c, d, a, b, x[ 7], S33, 0xf6bb4b60); /* 39 */
  rf<H>(b, c, d, a, x[10], S34, 0xbebfbc70); /* 40 */
  rf<H>(a, b, c, d, x[13], S31, 0x289b7ec6); /* 41 */
  rf<H>(d, a, b, c, x[ 0], S32, 0xeaa127fa); /* 42 */
  rf<H>(c, d, a, b, x[ 3], S33, 0xd4ef3085); /* 43 */
  rf<H>(b, c, d, a, x[ 6], S34,  0x4881d05); /* 44 */
  rf<H>(a, b, c, d, x[ 9], S31, 0xd9d4d039); /* 45 */
  rf<H>(d, a, b, c, x[12], S32, 0xe6db99e5); /* 46 */
  rf<H>(c, d, a, b, x[15], S33, 0x1fa27cf8); /* 47 */
  rf<H>(b, c, d, a, x[ 2], S34, 0xc4ac5665); /* 48 */
  
  /* Round 4 */
  rf<I>(a, b, c, d, x[ 0], S41, 0xf4292244); /* 49 */
  rf<I>(d, a, b, c, x[ 7], S42, 0x432aff97); /* 50 */
  rf<I>(c, d, a, b, x[14], S43, 0xab9423a7); /* 51 */
  rf<I>(b, c, d, a, x[ 5], S44, 0xfc93a039); /* 52 */
  rf<I>(a, b, c, d, x[12], S41, 0x655b59c3); /* 53 */
  rf<I>(d, a, b, c, x[ 3], S42, 0x8f0ccc92); /* 54 */
  rf<I>(c, d, a, b, x[10], S43, 0xffeff47d); /* 55 */
  rf<I>(b, c, d, a, x[ 1], S44, 0x85845dd1); /* 56 */
  rf<I>(a, b, c, d, x[ 8], S41, 0x6fa87e4f); /* 57 */
  rf<I>(d, a, b, c, x[15], S42, 0xfe2ce6e0); /* 58 */
  rf<I>(c, d, a, b, x[ 6], S43, 0xa3014314); /* 59 */
  rf<I>(b, c, d, a, x[13], S44, 0x4e0811a1); /* 60 */
  rf<I>(a, b, c, d, x[ 4], S41, 0xf7537e82); /* 61 */
  rf<I>(d, a, b, c, x[11], S42, 0xbd3af235); /* 62 */
  rf<I>(c, d, a, b, x[ 2], S43, 0x2ad7d2bb); /* 63 */
  rf<I>(b, c, d, a, x[ 9], S44, 0xeb86d391); /* 64 */
  
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
void MD5::update(const unsigned char input[], size_t length)
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
void MD5::update(const char input[], size_t length)
{
  update((const unsigned char*)input, length);
}

//////////////////////////////

// MD5 finalization. Ends an MD5 message-digest operation, writing the
// the message digest and zeroizing the context.
MD5& MD5::finalize()
{
  static const uint8_t padding[64] = {
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
    memset(buffer, 0, sizeof(buffer));
    memset(count, 0, sizeof(count));
    
    finalized = true;
  }
  
  return *this;
}

//////////////////////////////

// return hex representation of digest as string
std::string MD5::hexdigest(bool uppercase = false) const
{
  if (!finalized)
    return "";
  
  char buf[33];
  for (int i=0; i<16; i++)
    sprintf(buf+i*2, uppercase ? "%02X" : "%02x", digest[i]);
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


#include "repository.h"

void run(const std::string& name, repository::arg_iterator begin, repository::arg_iterator end)
{
  args::ArgumentParser parser("");
  parser.Prog(name + " md5");
  
  args::Flag uppercase(parser, "uppercase", "Print result in uppercase format", {'u', "uppercase"}, false);
  args::Positional<std::string> directory(parser, "path", "File to calculate MD5 hash on");

  parser.ParseArgs(begin, end);
  
  path path = args::get(directory);
  
  if (!path.exists())
    throw exceptions::file_not_found(path);
  
  file_handle handle = file_handle(path, file_mode::READING);
  
  if (!handle)
    throw exceptions::error_opening_file(path);
  
  MD5 md5;

  size_t fileLength = handle.length();
  size_t current = 0;
  std::unique_ptr<byte[]> bufferPtr = std::unique_ptr<byte[]>(new byte[KB16]);
  byte* buffer = bufferPtr.get();
  
  while (current < fileLength)
  {
    size_t amountToProcess = std::min(KB16, fileLength - current);
    
    if (handle.read(buffer, 1, amountToProcess))
      md5.update(buffer, amountToProcess);
    else
      throw exceptions::error_reading_from_file(path);
    
    current += KB16;
  }
  
  md5.finalize();
  
  std::cout << md5.hexdigest(uppercase) << std::endl;
}

static const repository::CommandBuilder builder(repository::Command("md5", "computes MD5 hash of a file", run));
