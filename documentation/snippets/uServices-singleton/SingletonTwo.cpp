#include "SingletonTwo.h"

#include "SingletonOne.h"
#include <iostream>

#include <usGetModuleContext.h>
#include <usModuleContext.h>

US_USE_NAMESPACE

SingletonTwo& SingletonTwo::GetInstance()
{
  static SingletonTwo instance;
  return instance;
}

SingletonTwo::SingletonTwo() : b(2)
{
  std::cout << "Constructing SingletonTwo" << std::endl;
}

SingletonTwo::~SingletonTwo()
{
  std::cout << "Deleting SingletonTwo" << std::endl;
  std::cout << "SingletonOne::a = " << SingletonOne::GetInstance().a << std::endl;
}

SingletonTwoService* SingletonTwoService::GetInstance()
{
  static ServiceReference serviceRef;
  static ModuleContext* context = GetModuleContext();

  if (!serviceRef)
  {
    // This is either the first time GetInstance() was called,
    // or a SingletonTwoService instance has not yet been registered.
    serviceRef = context->GetServiceReference<SingletonTwoService>();
  }

  if (serviceRef)
  {
    // We have a valid service reference. It always points to the service
    // with the lowest id (usually the one which was registered first).
    // This still might return a null pointer, if all SingletonTwoService
    // instances have been unregistered (during unloading of the library,
    // for example).
    return context->GetService<SingletonTwoService>(serviceRef);
  }
  else
  {
    // No SingletonTwoService instance was registered yet.
    return 0;
  }
}

SingletonTwoService::SingletonTwoService() : b(2)
{
  std::cout << "Constructing SingletonTwoService" << std::endl;
}

SingletonTwoService::~SingletonTwoService()
{
  std::cout << "Deleting SingletonTwoService" << std::endl;
}
