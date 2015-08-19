#include <iostream>
#include "TCanvas.h"
#include "TH1F.h"
#include <unistd.h>
#include "TBrowser.h"
#include <TApplication.h>

void OpenTBrowser()
{
  TBrowser *tb = new TBrowser();
}

int main(int argc, char **argv)
{
  TApplication theApp("App", &argc, argv);
  OpenTBrowser();
  theApp.Run();
  return 0;
}
