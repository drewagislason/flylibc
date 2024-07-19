#include "FlyTest.h"

void TcExampleSkip(void)
{
  FlyTestStubbed();
}

void TcExamplePass(void)
{
  FlyTestBegin();
  if(FlyTestVerbose() >= 1)
    FlyTestPrintf("\nThis Test Always Passes\n");
  FlyTestPassed();
  FlyTestEnd();
}

void TcExampleFail(void)
{
  FlyTestBegin();
  if(FlyTestGetYesNo("Do you want this test to fail?"))
  {
    FlyTestFailed();
  }
  FlyTestEnd();
}

int main(int argc, const char *argv[])
{
  const char          szName[] = "test_example";
  const sTestCase_t   aTestCases[] =
  {
    { "TcExamplePass",    TcExamplePass },
    { "TcExampleFail",    TcExampleFail, "M" },
    { "TcExampleSkip",    TcExampleSkip },
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
