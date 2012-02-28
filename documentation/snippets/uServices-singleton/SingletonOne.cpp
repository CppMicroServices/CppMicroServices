#include "SingletonOne.h"

#include "SingletonTwo.h"

#include <iostream>
#include <cassert>

#include <usGetModuleContext.h>
#include <usModuleContext.h>

US_USE_NAMESPACE

//![s1]
SingletonOne& SingletonOne::GetInstance()
{
  static SingletonOne instance;
  return instance;
}
//![s1]

SingletonOne::SingletonOne() : a(1)
{
}

//![s1d]
SingletonOne::~SingletonOne()
{
  std::cout << "SingletonTwo::b = " << SingletonTwo::GetInstance().b << std::endl;
}
//![s1d]

//![ss1gi]
SingletonOneService* SingletonOneService::GetInstance()
{
  static ServiceReference serviceRef;
  static ModuleContext* context = GetModuleContext();

  if (!serviceRef)
  {
    // This is either the first time GetInstance() was called,
    // or a SingletonOneService instance has not yet been registered.
    serviceRef = context->GetServiceReference<SingletonOneService>();
  }

  if (serviceRef)
  {
    // We have a valid service reference. It always points to the service
    // with the lowest id (usually the one which was registered first).
    // This still might return a null pointer, if all SingletonOneService
    // instances have been unregistered (during unloading of the library,
    // for example).
    return context->GetService<SingletonOneService>(serviceRef);
  }
  else
  {
    // No SingletonOneService instance was registered yet.
    return 0;
  }
}
//![ss1gi]

SingletonOneService::SingletonOneService()
  : a(1)
{
  SingletonTwoService* singletonTwoService = SingletonTwoService::GetInstance();
  assert(singletonTwoService != 0);
  std::cout << "SingletonTwoService::b = " << singletonTwoService->b << std::endl;
}

//![ss1d]
SingletonOneService::~SingletonOneService()
{
  SingletonTwoService* singletonTwoService = SingletonTwoService::GetInstance();

  // The module activator must ensure that a SingletonTwoService instance is
  // available during destruction of a SingletonOneService instance.
  assert(singletonTwoService != 0);
  std::cout << "SingletonTwoService::b = " << singletonTwoService->b << std::endl;
}
//![ss1d]
