/*!************************************************************************************************
  @ingroup    test
  @file       FlyTestSec.c
  @copyright  2022 Drew Gislason
  @brief      Unit test for security functions
*///***********************************************************************************************
#include "FlyTest.h"
#include "FlySec.h"

unsigned PadLen(unsigned len)
{
  if(len % 16)
    len += (16 - (len % 16));
  return len;
}

void CmpBytes(const uint8_t *p1, const uint8_t *p2, unsigned len)
{
  unsigned  i;

  for(i = 0; i < len; ++i)
  {
    if(*p1 != *p2)
      printf("err: i %u, *p1 %02x, *p2 %02x\n", i, *p1, *p2);
    ++p1;
    ++p2;
  }
}

void SecPrint2Bufs(const char *szMsg, const void *pBuf1, unsigned len1, const void *pBuf2, unsigned len2)
{
  printf("%s: len %u, expected %u\n", szMsg, len1, len2);
  printf("got buf %p =\n", pBuf1);
  FlyTestDump(pBuf1, len1);
  printf("expected buf %p =\n", pBuf2);
  FlyTestDump(pBuf2, len2);
  CmpBytes(pBuf1, pBuf2, len2);
}

/*!------------------------------------------------------------------------------------------------
  Verify generation of random Nonce.
*///-----------------------------------------------------------------------------------------------
void FlyTestSecRandomNonce(void)
{
  unsigned      i;
  hSec_t        hSec = NULL;
  long          nonce = 0;

  FlyTestBegin();

  hSec = FlySecNew(100);
  if(!hSec)
    FlyTestFailed();
  if(FlyTestVerbose())
    FlyTestPrintf("\n");
  for(i = 0; i < 20; ++i)
  {
    if(FlyTestVerbose())
      FlyTestPrintf("  %ld\n", FlySecNonceGet(hSec));
    if(FlySecNonceGet(hSec) == nonce)
      FlyTestFailed();
    nonce = FlySecNonceGet(hSec);
    FlySecNonceNew(hSec);
  }

  FlyTestEnd();

  if(hSec)
    FlySecFree(hSec);
}

/*!------------------------------------------------------------------------------------------------
  Verify generation of random Nonce.
*///-----------------------------------------------------------------------------------------------
void FlyTestSecRandomPwd(void)
{
  unsigned      i;
  char          szPwd[12];
  char          szOld[12];

  FlyTestBegin();

  memset(szOld, 0, sizeof(szOld));
  if(FlyTestVerbose())
    FlyTestPrintf("\n");
  for(i = 0; i < 20; ++i)
  {
    FlySecPwdRandom(szPwd, sizeof(szPwd));
    if(FlyTestVerbose())
      FlyTestPrintf("  %s\n", szPwd);
    if(memcmp(szOld, szPwd, sizeof(szPwd)) == 0)
      FlyTestFailed();
    memcpy(szOld, szPwd, sizeof(szOld));
  }

  FlyTestEnd();
}

/*!------------------------------------------------------------------------------------------------
  Verify padding works
*///-----------------------------------------------------------------------------------------------
void FlyTestSecPad(void)
{
  static uint8_t  aPad1[]       = { 'A' };
  static uint8_t  aPad1Result[] = { 'A',0xf,0xf,0xf,0xf,0xf,0xf,0xf,0xf,0xf,0xf,0xf,0xf,0xf,0xf,0xf };
  static uint8_t  aPad2[]       = { '1','2','3','4','5','6','7','8','9','A','B','C','D','E','F' };
  static uint8_t  aPad2Result[] = { '1','2','3','4','5','6','7','8','9','A','B','C','D','E','F',0x1 };
  static uint8_t  aPad3[]       = { 'E','x','a','c','t','l','y','1','6','b','y','t','e','s','!',0x0 };
  static uint8_t  aPad3Result[] = { 'E','x','a','c','t','l','y','1','6','b','y','t','e','s','!',0x0 };
  unsigned        len;
  uint8_t         aBuf[32];

  FlyTestBegin();

  memcpy(aBuf, aPad1, sizeof(aPad1));
  len = FlySecPad(aBuf, sizeof(aPad1));
  if(len != sizeof(aPad1Result) || memcmp(aBuf, aPad1Result, len) != 0)
    FlyTestFailed();

  memcpy(aBuf, aPad2, sizeof(aPad2));
  len = FlySecPad(aBuf, sizeof(aPad2));
  if(len != sizeof(aPad2Result) || memcmp(aBuf, aPad2Result, len) != 0)
    FlyTestFailed();

  memcpy(aBuf, aPad3, sizeof(aPad3));
  len = FlySecPad(aBuf, sizeof(aPad3));
  if(len != sizeof(aPad3Result) || memcmp(aBuf, aPad3Result, len) != 0)
    FlyTestFailed();

  FlyTestEnd();
}

/*!------------------------------------------------------------------------------------------------
  Test encoding and decoding a couple of simple packets
*///-----------------------------------------------------------------------------------------------
void FlyTestSecBasic(void)
{
  hSec_t          hSec = NULL;
  uint8_t         aBuf[128];
  const uint8_t   aEncrypted[] = 
  // 00000000: fe 01 23 4e 00 1f 00 07  68 65 61 64 65 72 00 9d  |..#N....header..|
  // 00000010: e4 c7 4e 06 f5 cf ec 3b  e8 a9 a1 be 8f 60 66     |..N....;.....`f|
  { 0xfe,0x01,0x23,0x4e,0x00,0x1f,0x00,0x07,0x68,0x65,0x61,0x64,0x65,0x72,0x00,0x9d,
    0xe4,0xc7,0x4e,0x06,0xf5,0xcf,0xec,0x3b,0xe8,0xa9,0xa1,0xbe,0x8f,0x60,0x66
  };
  const char      szHdr[] = "header";
  const char      szPwd[] = "password";
  const uint8_t   aData[] = { '1','2','3','4','5','6','7','8','9','A' };
  unsigned        len;

  FlyTestBegin();

  hSec = FlySecNew(sizeof(aBuf));
  if(!hSec)
    FlyTestFailed();

  FlySecKeySet(hSec, szPwd, sizeof(szPwd));
  FlySecNonceSet(hSec, 99);

  len = FlySecEncode(hSec, aBuf, szHdr, sizeof(szHdr), aData, sizeof(aData));
  if((len != sizeof(aEncrypted)) || (memcmp(aEncrypted, aBuf, len) != 0))
  {
    SecPrint2Bufs("encode failed", aBuf, len, aEncrypted, sizeof(aEncrypted));
    FlyTestFailed();
  }

  if(!FlySecStreamFeed(hSec, aEncrypted, sizeof(aEncrypted)))
    FlyTestFailed();
  len = FlySecDecode(hSec, aBuf, NULL, NULL);
  if(len != sizeof(aData) || memcmp(aData, aBuf, len) != 0)
  {
    SecPrint2Bufs("decode failed", aBuf, len, aData, sizeof(aData));
    FlyTestFailed();
  }

  FlyTestEnd();

  if(hSec)
    FlySecFree(hSec);
}

/*!------------------------------------------------------------------------------------------------
  Test encoding and decoding a larger packet with size of stream and buffer exactly sized
*///-----------------------------------------------------------------------------------------------
void FlyTestSecLarger(void)
{
  hSec_t          hSec = NULL;
  const uint8_t   aEncrypted[] = 
  // 00000000: fe 01 56 0d 00 89 00 11  75 73 65 72 6e 61 6d 65  |..V.....username|
  // 00000010: 2c 31 32 33 34 35 36 2c  00 7a 58 60 2f 74 bb 22  |,123456,.zX`/t."|
  // 00000020: 36 78 f3 56 61 fc bd 99  02 ff 82 f2 db 2b 05 49  |6x.Va........+.I|
  // 00000030: 54 73 1a 89 c2 2f e3 a6  ae 39 fb 97 ad 64 29 df  |Ts.../...9...d).|
  // 00000040: 08 f9 1d 3f b5 12 1d 1e  d2 c2 75 08 76 37 ef 45  |...?......u.v7.E|
  // 00000050: ce 62 ec ae c9 df ae a2  7d 2d f4 09 2a 9d 66 7a  |.b......}-..*.fz|
  // 00000060: cd fd 75 38 f6 79 c8 1b  e2 78 31 dd be f8 1d d4  |..u8.y...x1.....|
  // 00000070: d2 b8 85 82 31 f7 e5 5b  13 88 d4 b7 22 d9 4c a5  |....1..[....".L.|
  // 00000080: 4f 5b 82 a1 d0 04 24 57  35                       |O[....$W5|
  { 0xfe,0x01,0x56,0x0d,0x00,0x89,0x00,0x11,0x75,0x73,0x65,0x72,0x6e,0x61,0x6d,0x65,
    0x2c,0x31,0x32,0x33,0x34,0x35,0x36,0x2c,0x00,0x7a,0x58,0x60,0x2f,0x74,0xbb,0x22,
    0x36,0x78,0xf3,0x56,0x61,0xfc,0xbd,0x99,0x02,0xff,0x82,0xf2,0xdb,0x2b,0x05,0x49,
    0x54,0x73,0x1a,0x89,0xc2,0x2f,0xe3,0xa6,0xae,0x39,0xfb,0x97,0xad,0x64,0x29,0xdf,
    0x08,0xf9,0x1d,0x3f,0xb5,0x12,0x1d,0x1e,0xd2,0xc2,0x75,0x08,0x76,0x37,0xef,0x45,
    0xce,0x62,0xec,0xae,0xc9,0xdf,0xae,0xa2,0x7d,0x2d,0xf4,0x09,0x2a,0x9d,0x66,0x7a,
    0xcd,0xfd,0x75,0x38,0xf6,0x79,0xc8,0x1b,0xe2,0x78,0x31,0xdd,0xbe,0xf8,0x1d,0xd4,
    0xd2,0xb8,0x85,0x82,0x31,0xf7,0xe5,0x5b,0x13,0x88,0xd4,0xb7,0x22,0xd9,0x4c,0xa5,
    0x4f,0x5b,0x82,0xa1,0xd0,0x04,0x24,0x57,0x35
  };
  const char      szHdr[] = "username,123456,";
  const char      szPwd[] = "password";
  const char      aData[] = "The quick brown fox jumped over the lazy dog. Every good boy does fine. More on this later. Or was it moron...";
  uint8_t         aBuf[FLySecSize(sizeof(szHdr),sizeof(aData))];
  unsigned        len;

  FlyTestBegin();

  hSec = FlySecNew(sizeof(aBuf));
  if(!hSec)
    FlyTestFailed();

  FlySecKeySet(hSec, szPwd, sizeof(szPwd));
  FlySecNonceSet(hSec, 123456L);

  len = FlySecEncode(hSec, aBuf, szHdr, sizeof(szHdr), aData, sizeof(aData));
  if((len != sizeof(aEncrypted)) || (memcmp(aEncrypted, aBuf, len) != 0))
  {
    SecPrint2Bufs("encode failed", aBuf, len, aEncrypted, sizeof(aEncrypted));
    FlyTestFailed();
  }

  if(!FlySecStreamFeed(hSec, aEncrypted, sizeof(aEncrypted)))
    FlyTestFailed();
  len = FlySecDecode(hSec, aBuf, NULL, NULL);
  if(len != sizeof(aData) || memcmp(aData, aBuf, len) != 0)
  {
    SecPrint2Bufs("decode failed", aBuf, len, aData,sizeof(aData));
    FlyTestFailed();
  }

  FlyTestEnd();

  if(hSec)
    FlySecFree(hSec);
}

/*!------------------------------------------------------------------------------------------------
  Test encoding and decoding with different nonces
*///-----------------------------------------------------------------------------------------------
void FlyTestSecNonce(void)
{
  hSec_t          hSec = NULL;
  const uint8_t   aEncrypted1[] = 
  // 00000000: fe 01 26 07 00 1b 00 03  00 00 07 8d e9 eb 44 1e  |..&...........D.|
  // 00000010: 7a 3c 28 03 4a 31 cd 3c  05 e5 91                 |z<(.J1.<...|
  { 0xfe,0x01,0x26,0x07,0x00,0x1b,0x00,0x03,0x00,0x00,0x07,0x8d,0xe9,0xeb,0x44,0x1e,
    0x7a,0x3c,0x28,0x03,0x4a,0x31,0xcd,0x3c,0x05,0xe5,0x91
  };
  const uint8_t   aEncrypted2[] = 
  // 00000000: fe 01 93 ce 00 1b 00 03  00 00 07 22 76 63 bc 8e  |..........."vc..|
  // 00000010: d1 39 6f 71 76 9c f8 4a  32 4f c7                 |.9oqv..J2O.|
  { 0xfe,0x01,0x93,0xce,0x00,0x1b,0x00,0x03,0x00,0x00,0x07,0x22,0x76,0x63,0xbc,0x8e,
    0xd1,0x39,0x6f,0x71,0x76,0x9c,0xf8,0x4a,0x32,0x4f,0xc7
  };
  const uint8_t   aHdr[] = { 0, 0, 7 };
  const char      szPwd[] = "This is a very long password, it should still work";
  const char      aData[] = "Hello World!";
  long            nonce1 = 1;
  long            nonce2 = LONG_MAX;
  uint8_t         aBuf[256];
  unsigned        len;

  FlyTestBegin();

  hSec = FlySecNew(sizeof(aBuf));
  if(!hSec)
    FlyTestFailed();

  // test encrypting with nonce 1
  FlySecKeySet(hSec, szPwd, sizeof(szPwd));
  FlySecNonceSet(hSec, nonce1);
  len = FlySecEncode(hSec, aBuf, aHdr, sizeof(aHdr), aData, sizeof(aData));
  if((len != sizeof(aEncrypted1)) || (memcmp(aEncrypted1, aBuf, len) != 0))
  {
    SecPrint2Bufs("encode1 failed", aBuf, len, aEncrypted1, sizeof(aEncrypted1));
    FlyTestFailed();
  }

  // make sure we can decrypt
  if(!FlySecStreamFeed(hSec, aEncrypted1, sizeof(aEncrypted1)))
    FlyTestFailed();
  len = FlySecDecode(hSec, aBuf, NULL, NULL);
  if(len != sizeof(aData) || memcmp(aData, aBuf, len) != 0)
  {
    SecPrint2Bufs("decode1 failed", aBuf, len, aData, sizeof(aData));
    FlyTestFailed();
  }

  // test encrypting with nonce2
  FlySecNonceSet(hSec, nonce2);
  len = FlySecEncode(hSec, aBuf, aHdr, sizeof(aHdr), aData, sizeof(aData));
  if((len != sizeof(aEncrypted2)) || (memcmp(aEncrypted2, aBuf, len) != 0))
  {
    SecPrint2Bufs("encode2 failed", aBuf, len, aEncrypted2, sizeof(aEncrypted2));
    FlyTestFailed();
  }

  // make sure we can decrypt
  if(!FlySecStreamFeed(hSec, aEncrypted2, sizeof(aEncrypted2)))
    FlyTestFailed();
  len = FlySecDecode(hSec, aBuf, NULL, NULL);
  if(len != sizeof(aData) || memcmp(aData, aBuf, len) != 0)
  {
    SecPrint2Bufs("decode2 failed", aBuf, len, aData, sizeof(aData));
    FlyTestFailed();
  }

  FlyTestEnd();

  if(hSec)
    FlySecFree(hSec);
}

/*!------------------------------------------------------------------------------------------------
  Set pwd, nonce from header: pwd1,9223372036854775807,
*///-----------------------------------------------------------------------------------------------
static bool_t TestProcessHdrKeyLookup(hSec_t hSec, const void *pHdr, unsigned hdrLen, void *pAuxData)
{
  const char *  szHdr = pHdr;
  const char   *psz;
  const char   *pszEnd;
  char          szPwd[32];
  char          szNonce[32];
  long          nonce = 0;
  unsigned      n;

  if(FlyTestVerbose())
    FlyTestPrintf("ProcessHdr: %s\n", szHdr);

  // verify aux data
  if(pAuxData == NULL || *(unsigned *)pAuxData != 99)
  {
    printf("Bad aux data\n");
    return FALSE;
  }

  // set password from header
  psz = szHdr;
  pszEnd = strchr(psz, ',');
  if(!pszEnd)
  {
    FlyTestPrintf("bad key\n");
    return FALSE;
  }
  n = (pszEnd - psz);
  strncpy(szPwd, psz, n);
  szPwd[n] = '\0'; 
  FlySecKeySet(hSec, szPwd, strlen(szPwd) + 1);

  // set nonce from header
  psz = pszEnd + 1;
  pszEnd = strchr(psz, ',');
  if(!pszEnd)
  {
    FlyTestPrintf("bad nonce2\n");
    return FALSE;
  }
  n = (pszEnd - psz);
  strncpy(szNonce, psz, n);
  szNonce[n] = '\0'; 
  sscanf(szNonce, "%ld", &nonce);
  if(!nonce)
  {
    FlyTestPrintf("bad nonce2\n");
    return FALSE;
  }
  FlySecNonceSet(hSec, nonce);

  if(FlyTestVerbose())
    FlyTestPrintf("ProcessHdr: pwd %s, nonce %ld\n", szPwd, nonce);

  return TRUE;
}

/*!------------------------------------------------------------------------------------------------
  Test encoding and decoding with key/nonce lookup as part of header
*///-----------------------------------------------------------------------------------------------
void FlyTestSecKeyLookup(void)
{
  hSec_t          hSec1 = NULL;
  hSec_t          hSec2 = NULL;
  const uint8_t   aEncrypted1[] = 
  // 00000000: fe 01 87 1f 00 52 00 0a  70 77 64 31 2c 31 31 31  |.....R..pwd1,111|
  // 00000010: 2c 00 ec ce e9 54 0b b0  78 95 41 dd ba 3f 12 b5  |,....T..x.A..?..|
  // 00000020: 52 29 ae 56 74 69 dd f2  f1 53 0f a3 db 41 d7 e1  |R).Vti...S...A..|
  // 00000030: 29 61 38 e5 c7 3e 05 1c  ab 6f 7e f0 d7 df ff 46  |)a8..>...o~....F|
  // 00000040: 76 ac 1f 2f 7d d7 f8 74  43 26 c3 29 58 c3 9a 67  |v../}..tC&.)X..g|
  // 00000050: 22 ad                                             |".|
  { 0xfe,0x01,0x87,0x1f,0x00,0x52,0x00,0x0a,0x70,0x77,0x64,0x31,0x2c,0x31,0x31,0x31,
    0x2c,0x00,0xec,0xce,0xe9,0x54,0x0b,0xb0,0x78,0x95,0x41,0xdd,0xba,0x3f,0x12,0xb5,
    0x52,0x29,0xae,0x56,0x74,0x69,0xdd,0xf2,0xf1,0x53,0x0f,0xa3,0xdb,0x41,0xd7,0xe1,
    0x29,0x61,0x38,0xe5,0xc7,0x3e,0x05,0x1c,0xab,0x6f,0x7e,0xf0,0xd7,0xdf,0xff,0x46,
    0x76,0xac,0x1f,0x2f,0x7d,0xd7,0xf8,0x74,0x43,0x26,0xc3,0x29,0x58,0xc3,0x9a,0x67,
    0x22,0xad
  };
  const uint8_t   aEncrypted2[] = 
  // 00000000: fe 01 b6 65 00 2f 00 17  64 69 66 66 65 72 65 6e  |...e./..differen|
  // 00000010: 74 20 70 77 64 32 2c 31  32 33 34 35 36 2c 00 7f  |t pwd2,123456,..|
  // 00000020: 4a 67 b0 7e 21 49 37 ec  1e 92 18 60 ef 53 8f     |Jg.~!I7....`.S.|
  { 0xfe,0x01,0xb6,0x65,0x00,0x2f,0x00,0x17,0x64,0x69,0x66,0x66,0x65,0x72,0x65,0x6e,
    0x74,0x20,0x70,0x77,0x64,0x32,0x2c,0x31,0x32,0x33,0x34,0x35,0x36,0x2c,0x00,0x7f,
    0x4a,0x67,0xb0,0x7e,0x21,0x49,0x37,0xec,0x1e,0x92,0x18,0x60,0xef,0x53,0x8f
  };
  const char      szPwd1[] = "pwd1";
  const char      szPwd2[] = "different pwd2";
  const char      aData1[] = "I'd gladly pay you on Tuesday for a hamburger today";
  const char      aData2[] = "spinich!";
  const char      szHdrPattern[] = "%s,%ld,";
  char            szHdr[32];   // pwd1,9223372036854775807,
  long            nonce1 = 111;
  long            nonce2 = 123456;
  unsigned        auxData = 99;
  uint8_t         aBuf[256];
  unsigned        len;
  unsigned        hdrLen;

  FlyTestBegin();

  hSec1 = FlySecNew(sizeof(aBuf));
  if(!hSec1)
    FlyTestFailed();

  hSec2 = FlySecNew(sizeof(aBuf));
  if(!hSec2)
    FlyTestFailed();

  // test encrypting with pwd/nonce 1
  FlySecKeySet(hSec1, szPwd1, sizeof(szPwd1));
  FlySecNonceSet(hSec1, nonce1);
  hdrLen = snprintf(szHdr, sizeof(szHdr), szHdrPattern, szPwd1, nonce1) + 1;
  len = FlySecEncode(hSec1, aBuf, szHdr, hdrLen, aData1, sizeof(aData1));
  if((len != sizeof(aEncrypted1)) || (memcmp(aEncrypted1, aBuf, len) != 0))
  {
    SecPrint2Bufs("encode1 failed", aBuf, len, aEncrypted1, sizeof(aEncrypted1));
    FlyTestFailed();
  }

  // make sure we can decrypt
  if(!FlySecStreamFeed(hSec2, aEncrypted1, sizeof(aEncrypted1)))
    FlyTestFailed();
  len = FlySecDecode(hSec2, aBuf, TestProcessHdrKeyLookup, &auxData);
  if(len != sizeof(aData1) || memcmp(aData1, aBuf, len) != 0)
  {
    SecPrint2Bufs("decode1 failed", aBuf, len, aData1, sizeof(aData1));
    FlyTestFailed();
  }

  // test encrypting with nonce2
  FlySecKeySet(hSec1, szPwd2, sizeof(szPwd2));
  FlySecNonceSet(hSec1, nonce2);
  hdrLen = snprintf(szHdr, sizeof(szHdr), szHdrPattern, szPwd2, nonce2) + 1;
  len = FlySecEncode(hSec1, aBuf, szHdr, hdrLen, aData2, sizeof(aData2));
  if((len != sizeof(aEncrypted2)) || (memcmp(aEncrypted2, aBuf, len) != 0))
  {
    SecPrint2Bufs("encode2 failed", aBuf, len, aEncrypted2, sizeof(aEncrypted2));
    FlyTestFailed();
  }

  // make sure we can decrypt
  if(!FlySecStreamFeed(hSec2, aEncrypted2, sizeof(aEncrypted2)))
    FlyTestFailed();
  len = FlySecDecode(hSec2, aBuf, TestProcessHdrKeyLookup, &auxData);
  if(len != sizeof(aData2) || memcmp(aData2, aBuf, len) != 0)
  {
    SecPrint2Bufs("decode2 failed", aBuf, len, aData2, sizeof(aData2));
    FlyTestFailed();
  }

  FlyTestEnd();

  if(hSec1)
    FlySecFree(hSec1);
  if(hSec2)
    FlySecFree(hSec2);
}

/*!------------------------------------------------------------------------------------------------
  Test filling the stream with various sized bytes that don't match up with packet size
*///-----------------------------------------------------------------------------------------------
void FlyTestSecStream(void)
{
  hSec_t          hSec = NULL;
  const uint8_t   aEncrypted1[] = 
  // 00000000: fe 01 87 1f 00 52 00 0a  70 77 64 31 2c 31 31 31  |.....R..pwd1,111|
  // 00000010: 2c 00 ec ce e9 54 0b b0  78 95 41 dd ba 3f 12 b5  |,....T..x.A..?..|
  // 00000020: 52 29 ae 56 74 69 dd f2  f1 53 0f a3 db 41 d7 e1  |R).Vti...S...A..|
  // 00000030: 29 61 38 e5 c7 3e 05 1c  ab 6f 7e f0 d7 df ff 46  |)a8..>...o~....F|
  // 00000040: 76 ac 1f 2f 7d d7 f8 74  43 26 c3 29 58 c3 9a 67  |v../}..tC&.)X..g|
  // 00000050: 22 ad                                             |".|
  { 0xfe,0x01,0x87,0x1f,0x00,0x52,0x00,0x0a,0x70,0x77,0x64,0x31,0x2c,0x31,0x31,0x31,
    0x2c,0x00,0xec,0xce,0xe9,0x54,0x0b,0xb0,0x78,0x95,0x41,0xdd,0xba,0x3f,0x12,0xb5,
    0x52,0x29,0xae,0x56,0x74,0x69,0xdd,0xf2,0xf1,0x53,0x0f,0xa3,0xdb,0x41,0xd7,0xe1,
    0x29,0x61,0x38,0xe5,0xc7,0x3e,0x05,0x1c,0xab,0x6f,0x7e,0xf0,0xd7,0xdf,0xff,0x46,
    0x76,0xac,0x1f,0x2f,0x7d,0xd7,0xf8,0x74,0x43,0x26,0xc3,0x29,0x58,0xc3,0x9a,0x67,
    0x22,0xad
  };
  const unsigned  aOffsets[] = { 0, 1, 2, FLY_SEC_PREAMBLE_SIZE, 0xf, 0x33, sizeof(aEncrypted1) };
  const uint8_t   aEncrypted2[] = 
  // 00000000: fe 01 b6 65 00 2f 00 17  64 69 66 66 65 72 65 6e  |...e./..differen|
  // 00000010: 74 20 70 77 64 32 2c 31  32 33 34 35 36 2c 00 7f  |t pwd2,123456,..|
  // 00000020: 4a 67 b0 7e 21 49 37 ec  1e 92 18 60 ef 53 8f     |Jg.~!I7....`.S.|
  { 0xfe,0x01,0xb6,0x65,0x00,0x2f,0x00,0x17,0x64,0x69,0x66,0x66,0x65,0x72,0x65,0x6e,
    0x74,0x20,0x70,0x77,0x64,0x32,0x2c,0x31,0x32,0x33,0x34,0x35,0x36,0x2c,0x00,0x7f,
    0x4a,0x67,0xb0,0x7e,0x21,0x49,0x37,0xec,0x1e,0x92,0x18,0x60,0xef,0x53,0x8f
  };
  const char      szData1[] = "I'd gladly pay you on Tuesday for a hamburger today";
  const char      szData2[] = "spinich!";
  unsigned        auxData = 99;
  uint8_t         szBuf[1024];
  unsigned        len;
  unsigned        i;

  FlyTestBegin();

  hSec = FlySecNew(sizeof(aEncrypted1) + sizeof(aEncrypted2));
  if(!hSec)
    FlyTestFailed();

  // try adding 1 byte at a time
  for(i = 0; i < sizeof(aEncrypted1); ++i)
  {
    if(!FlySecStreamFeed(hSec, &aEncrypted1[i], 1))
    {
      FlyTestPrintf("Failed to add byte %u\n", i);
      FlyTestFailed();
    }
  }
  // make sure we can decrypt
  len = FlySecDecode(hSec, szBuf, TestProcessHdrKeyLookup, &auxData);
  if(len != sizeof(szData1) || memcmp(szBuf, szData1, len) != 0)
  {
    SecPrint2Bufs("decode failed", szBuf, len, szData1, sizeof(szData1));
    FlyTestFailed();
  }
  // stream should be empty
  if(FlySecStreamLeft(hSec) != FlySecStreamSize(hSec))
  {
    FlyTestPrintf("stream not empty\n");
    FlyTestFailed();
  }

  // try adding both packets at once
  memcpy(szBuf, aEncrypted1, sizeof(aEncrypted1));
  memcpy(&szBuf[sizeof(aEncrypted1)], aEncrypted2, sizeof(aEncrypted2));
  if(!FlySecStreamFeed(hSec, szBuf, sizeof(aEncrypted1) + sizeof(aEncrypted2)))
  {
    FlyTestPrintf("Failed to add %u bytes\n", (unsigned)(sizeof(aEncrypted1) + sizeof(aEncrypted2)));
    FlyTestFailed();
  }
  // make sure we can decrypt
  len = FlySecDecode(hSec, szBuf, TestProcessHdrKeyLookup, &auxData);
  if(len != sizeof(szData1) || memcmp(szBuf, szData1, len) != 0)
  {
    SecPrint2Bufs("decode failed", szBuf, len, szData1, sizeof(szData1));
    FlyTestFailed();
  }
  len = FlySecDecode(hSec, szBuf, TestProcessHdrKeyLookup, &auxData);
  if(len != sizeof(szData2) || memcmp(szBuf, szData2, len) != 0)
  {
    SecPrint2Bufs("decode failed", szBuf, len, szData2, sizeof(szData2));
    FlyTestFailed();
  }
  // stream should be empty
  if(FlySecStreamLeft(hSec) != FlySecStreamSize(hSec))
  {
    FlyTestPrintf("stream not empty\n");
    FlyTestFailed();
  }

  // try adding packet with strange offsets
  for(i = 0; i < NumElements(aOffsets) - 1; ++i)
  {
    if(!FlySecStreamFeed(hSec, &aEncrypted1[aOffsets[i]], aOffsets[i + 1] - aOffsets[i]))
    {
      FlyTestPrintf("Failed to add offset %u, len %u\n", aOffsets[i], aOffsets[i + 1] - aOffsets[i]);
      FlyTestFailed();
    }
    if(FlyTestVerbose())
    {
      FlyTestPrintf("streamSize %u, streamLeft %u, offset %u, len %u\n", FlySecStreamSize(hSec),
        FlySecStreamLeft(hSec), aOffsets[i], aOffsets[i + 1] - aOffsets[i]);
    }
  }

  // make sure we can decrypt
  len = FlySecDecode(hSec, szBuf, TestProcessHdrKeyLookup, &auxData);
  if(len != sizeof(szData1) || memcmp(szBuf, szData1, len) != 0)
  {
    SecPrint2Bufs("decode failed", szBuf, len, szData1, sizeof(szData1));
    FlyTestFailed();
  }
  // stream should be empty
  if(FlySecStreamLeft(hSec) != FlySecStreamSize(hSec))
  {
    FlyTestPrintf("stream not empty\n");
    FlyTestFailed();
  }

 FlyTestEnd();

  if(hSec)
    FlySecFree(hSec);
}

void FlyTestSecFuzz(void)
{
  hSec_t          hSec = NULL;
  const uint8_t   aEncrypted1[] = 
  // 00000000: fe 01 87 1f 00 52 00 0a  70 77 64 31 2c 31 31 31  |.....R..pwd1,111|
  // 00000010: 2c 00 ec ce e9 54 0b b0  78 95 41 dd ba 3f 12 b5  |,....T..x.A..?..|
  // 00000020: 52 29 ae 56 74 69 dd f2  f1 53 0f a3 db 41 d7 e1  |R).Vti...S...A..|
  // 00000030: 29 61 38 e5 c7 3e 05 1c  ab 6f 7e f0 d7 df ff 46  |)a8..>...o~....F|
  // 00000040: 76 ac 1f 2f 7d d7 f8 74  43 26 c3 29 58 c3 9a 67  |v../}..tC&.)X..g|
  // 00000050: 22 ad                                             |".|
  { 0xfe,0x01,0x87,0x1f,0x00,0x52,0x00,0x0a,0x70,0x77,0x64,0x31,0x2c,0x31,0x31,0x31,
    0x2c,0x00,0xec,0xce,0xe9,0x54,0x0b,0xb0,0x78,0x95,0x41,0xdd,0xba,0x3f,0x12,0xb5,
    0x52,0x29,0xae,0x56,0x74,0x69,0xdd,0xf2,0xf1,0x53,0x0f,0xa3,0xdb,0x41,0xd7,0xe1,
    0x29,0x61,0x38,0xe5,0xc7,0x3e,0x05,0x1c,0xab,0x6f,0x7e,0xf0,0xd7,0xdf,0xff,0x46,
    0x76,0xac,0x1f,0x2f,0x7d,0xd7,0xf8,0x74,0x43,0x26,0xc3,0x29,0x58,0xc3,0x9a,0x67,
    0x22,0xad
  };
  const uint8_t   aEncrypted2[] = 
  // 00000000: fe 01 b6 65 00 2f 00 17  64 69 66 66 65 72 65 6e  |...e./..differen|
  // 00000010: 74 20 70 77 64 32 2c 31  32 33 34 35 36 2c 00 7f  |t pwd2,123456,..|
  // 00000020: 4a 67 b0 7e 21 49 37 ec  1e 92 18 60 ef 53 8f     |Jg.~!I7....`.S.|
  { 0xfe,0x01,0xb6,0x65,0x00,0x2f,0x00,0x17,0x64,0x69,0x66,0x66,0x65,0x72,0x65,0x6e,
    0x74,0x20,0x70,0x77,0x64,0x32,0x2c,0x31,0x32,0x33,0x34,0x35,0x36,0x2c,0x00,0x7f,
    0x4a,0x67,0xb0,0x7e,0x21,0x49,0x37,0xec,0x1e,0x92,0x18,0x60,0xef,0x53,0x8f
  };
  const char      szFuzz1[] = "fuzz";
  const uint8_t   aFuzz2[] = { 0xfe, 0x01 };
  const char      szData1[] = "I'd gladly pay you on Tuesday for a hamburger today";
  const char      szData2[] = "spinich!";
  uint8_t         aBuf[1024];
  unsigned        auxData = 99;
  unsigned        len;
  unsigned        totalLen;

  FlyTestBegin();

  hSec = FlySecNew(sizeof(aEncrypted1) + sizeof(aEncrypted2));
  if(!hSec)
    FlyTestFailed();

  // try adding fuzz, make sure it's removed
  if(!FlySecStreamFeed(hSec, szFuzz1, strlen(szFuzz1)))
  {
    FlyTestPrintf("Failed to add fuzz\n");
    FlyTestFailed();
  }
  if(FlySecStreamLen(hSec) != 0)
  {
    FlyTestPrintf("expected empty stream (fuzz removed)\n");
    FlyTestFailed();
  }

  // try adding a packet with fuzz in front
  memcpy(aBuf, szFuzz1, strlen(szFuzz1));
  memcpy(&aBuf[strlen(szFuzz1)], aEncrypted1, sizeof(aEncrypted1));
  if(!FlySecStreamFeed(hSec, aBuf, strlen(szFuzz1) + sizeof(aEncrypted1)))
  {
    FlyTestPrintf("Failed to add %u bytes\n", (unsigned)(strlen(szFuzz1) + sizeof(aEncrypted1)));
    FlyTestFailed();
  }
  if(FlySecStreamLen(hSec) == 0)
  {
    FlyTestPrintf("expected data in stream with fuzz removed\n");
    FlyTestFailed();
  }
  len = FlySecDecode(hSec, aBuf, TestProcessHdrKeyLookup, &auxData);
  if(len != sizeof(szData1) || memcmp(aBuf, szData1, len) != 0)
  {
    SecPrint2Bufs("decode failed", aBuf, len, szData1, sizeof(szData1));
    FlyTestFailed();
  }
  // stream should be empty
  if(FlySecStreamLen(hSec) != 0)
  {
    FlyTestPrintf("stream not empty\n");
    FlyTestFailed();
  }

  // try adding a packet with fuzz at end
  memcpy(&aBuf[0], aEncrypted1, sizeof(aEncrypted1));
  memcpy(&aBuf[sizeof(aEncrypted1)], szFuzz1, strlen(szFuzz1));
  if(!FlySecStreamFeed(hSec, aBuf, sizeof(aEncrypted1) + strlen(szFuzz1)))
  {
    FlyTestPrintf("Failed to add %u bytes\n", (unsigned)(sizeof(aEncrypted1) + strlen(szFuzz1)));
    FlyTestFailed();
  }
  if(FlySecStreamLen(hSec) == 0)
  {
    FlyTestPrintf("expected data in stream with fuzz removed\n");
    FlyTestFailed();
  }
  len = FlySecDecode(hSec, aBuf, TestProcessHdrKeyLookup, &auxData);
  if(len != sizeof(szData1) || memcmp(aBuf, szData1, len) != 0)
  {
    SecPrint2Bufs("decode failed", aBuf, len, szData1, sizeof(szData1));
    FlyTestFailed();
  }
  // stream not just have fuzz
  if(FlySecStreamLen(hSec) == 0)
  {
    FlyTestPrintf("expected fuzz in the stream\n");
    FlyTestFailed();
  }
  len = FlySecDecode(hSec,aBuf, TestProcessHdrKeyLookup, &auxData);
  if(len != 0)
  {
    FlyTestPrintf("expected nothing in decode\n");
    FlyTestFailed();
  }
  // stream should be empty (fuzz removed)
  if(FlySecStreamLen(hSec) != 0)
  {
    FlyTestPrintf("stream not empty\n");
    FlyTestFailed();
  }

  // try adding both packets at once, with fuzz in between
  memcpy(aBuf, aEncrypted1, sizeof(aEncrypted1));
  memcpy(&aBuf[sizeof(aEncrypted1)], aFuzz2, sizeof(aFuzz2));
  memcpy(&aBuf[sizeof(aEncrypted1) + sizeof(aFuzz2)], aEncrypted2, sizeof(aEncrypted2));
  totalLen = (unsigned)(sizeof(aEncrypted1) + sizeof(aFuzz2) + sizeof(aEncrypted2));
  FlyTestPrintf("-- adding totalLen %u --\n", totalLen);
  if(!FlySecStreamFeed(hSec, aBuf, totalLen))
  {
    FlyTestPrintf("Failed to add %u bytes\n", totalLen);
    FlyTestFailed();
  }
  // make sure we can decrypt
  len = FlySecDecode(hSec, aBuf, TestProcessHdrKeyLookup, &auxData);
  if(len != sizeof(szData1) || memcmp(aBuf, szData1, len) != 0)
  {
    SecPrint2Bufs("decode failed", aBuf, len, szData1, sizeof(szData1));
    FlyTestFailed();
  }
  len = FlySecDecode(hSec, aBuf, TestProcessHdrKeyLookup, &auxData);
  if(len != sizeof(szData2) || memcmp(aBuf, szData2, len) != 0)
  {
    SecPrint2Bufs("decode failed", aBuf, len, szData2, sizeof(szData2));
    FlyTestFailed();
  }
  // stream should be empty
  if(FlySecStreamLen(hSec) != 0)
  {
    FlyTestPrintf("stream not empty\n");
    FlyTestFailed();
  }

 FlyTestEnd();

  if(hSec)
    FlySecFree(hSec);
}

int main(int argc, const char *argv[])
{
  const char          szName[] = "test_sec";
  const sTestCase_t   aTestCases[] =
  { 
    { "FlyTestSecPad",          FlyTestSecPad },
    { "FlyTestSecRandomNonce",  FlyTestSecRandomNonce },
    { "FlyTestSecRandomPwd",    FlyTestSecRandomPwd },
    { "FlyTestSecBasic",        FlyTestSecBasic },
    { "FlyTestSecLarger",       FlyTestSecLarger },
    { "FlyTestSecNonce",        FlyTestSecNonce },
    { "FlyTestSecKeyLookup",    FlyTestSecKeyLookup },
    { "FlyTestSecStream",       FlyTestSecStream },
    { "FlyTestSecFuzz",         FlyTestSecFuzz }
  };
  hTestSuite_t        hSuite;
  int                 ret;

  FlyTestInit(szName, argc, argv);
  hSuite = FlyTestNew(szName, NumElements(aTestCases), aTestCases);
  FlyTestRun(hSuite);
  ret = FlyTestSummary(hSuite);
  FlyTestFree(hSuite);

  return ret;
}

