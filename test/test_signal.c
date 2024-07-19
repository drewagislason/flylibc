/**************************************************************************************************
  test_signal.c
  Copyright 2024 Drew Gislason
  License: MIT <https://mit-license.org>
**************************************************************************************************/
#include "FlyTest.h"
#include "FlySignal.h"

static const char * m_szProgName;
static int          m_gotSig;
static hTestSuite_t m_hSuite;

// helper to TcSignalSegFault()
int TcSegFaultCallback(int sig)
{
  int ret = 1;
  m_gotSig = sig;
  if(sig == SIGSEGV)
    T_Passed();
  else
    T_Failed("Not SIGEGV", __FILE__, __LINE__);
  ret = FlyTestSummary(m_hSuite);
  _Exit(ret);
  return 0;
}

// helper to TcSignalSegFault()
char TcSegFaultFunc2(char *p)
{
  *p = 0;
  return *p;
}

// helper to TcSignalSegFault()
char TcSegFaultFunc1(char *p)
{
  char c;
  c = TcSegFaultFunc2(p);
  return c;
}

/*-------------------------------------------------------------------------------------------------
  test segment fault (NULL ptr)

  @ingroup    ned_test
  @returns    none
-------------------------------------------------------------------------------------------------*/
void TcSignalSegFault(void)
{
  char    *p = NULL;

  FlyTestBegin();

  // cause a segment fault (NULL ptr assignment)
  m_gotSig = 0;
  FlySigSetExit(m_szProgName, TcSegFaultCallback);
  TcSegFaultFunc1(p);
  if(m_gotSig != SIGSEGV)
    FlyTestFailed();

  FlyTestEnd();
}

// helper to TcSignalBadMem()
int TcBadMemCallback(int sig)
{
  int ret = 1;
  m_gotSig = sig;
  if(sig == SIGSEGV)
    T_Passed();
  else
    T_Failed("Not SIGEGV", __FILE__, __LINE__);
  ret = FlyTestSummary(m_hSuite);
  _Exit(ret);
  return 0;
}

// helper to TcSignalBadMem()
unsigned * TcBadMemFunc2(unsigned *pu)
{
  free(pu);
  return pu;
}

// helper to TcSignalBadMem()
void TcBadMemFunc1(unsigned *pu)
{
  pu = TcBadMemFunc2(pu);
  pu[1] = 99;
  FlyTestPrintf("%u, %u\n", *pu, pu[1]);
}

/*-------------------------------------------------------------------------------------------------
  test segment fault (NULL ptr)

  @ingroup    ned_test
  @returns    none
-------------------------------------------------------------------------------------------------*/
void TcSignalBadMem(void)
{
  unsigned  *pu;

  pu = malloc(sizeof(*pu));
  if(pu)
    *pu = 5;

  FlyTestBegin();

  // cause a segment fault (NULL ptr assignment)
  m_gotSig = 0;
  FlySigSetExit(m_szProgName, TcBadMemCallback);
  TcBadMemFunc1(pu);
  if(m_gotSig != SIGSEGV)
    FlyTestFailed();

  FlyTestEnd();
}

/*-------------------------------------------------------------------------------------------------
  Test each function
-------------------------------------------------------------------------------------------------*/
int main(int argc, const char *argv[])
{
  const char          szName[] = "test_signal";
  const sTestCase_t   aTestCases[] =
  {
    { "TcSignalSegFault", TcSignalSegFault },
    { "TcSignalBadMem",   TcSignalBadMem },
  };
  hTestSuite_t        hSuite;
  int                 ret;

  m_szProgName = argv[0];

  FlyTestInit(szName, argc, argv);
  hSuite = FlyTestNew(szName, NumElements(aTestCases), aTestCases);
  m_hSuite = hSuite;
  FlyTestRun(hSuite);
  ret = FlyTestSummary(hSuite);
  FlyTestFree(hSuite);

  return ret;
}
