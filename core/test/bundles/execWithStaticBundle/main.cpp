#include <cstdio>
#include <iostream>
#include <string>
#include <cassert>

#include "usFrameworkFactory.h"
#include "usFramework.h"
#include "usBundleContext.h"
#include "usBundleImport.h"

#include "usTestingMacros.h"
#include "usTestingConfig.h"

using namespace us;

/**
 * Test point to ensure the statically linked bundle is installed properly.
 */
int main(int ac, char **av)
{
  auto framework = FrameworkFactory().NewFramework();
  framework->Start();
  BundleContext* frameworkCtx = framework->GetBundleContext();
  auto bundle = frameworkCtx->InstallBundle(BIN_PATH + DIR_SEP + "usTestExecutableWithStaticBundle" + EXE_EXT + "/TestStaticBundle");
  try 
  {
	  if (bundle == nullptr)
		  throw std::runtime_error("Static bundle not found in the executable");
	  if (bundle->GetName() != "TestStaticBundle")
		  throw std::runtime_error("Check bundle name failed - " + bundle->GetName());
  }
  catch (const std::exception& ex)
  {
	  std::cerr << ex.what() << std::endl;
	  return -1;
  }
  framework->Stop();
  (void)ac;
  (void)av;
  return 0;
}

US_IMPORT_BUNDLE(TestStaticBundle)