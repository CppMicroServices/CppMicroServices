#include <usModuleActivator.h>
#include <usModuleContext.h>

#include "SingletonOne.h"
#include "SingletonTwo.h"

US_USE_NAMESPACE

class MyActivator : public ModuleActivator
{

public:

  MyActivator()
    : m_SingletonOne(NULL)
    , m_SingletonTwo(NULL)
  {}

  //![0]
  void Load(ModuleContext* context)
  {
    // The Load() method of the module activator is called during static
    // initialization time of the shared library.

    // First create and register a SingletonTwoService instance.
    m_SingletonTwo = new SingletonTwoService;
    m_SingletonTwoReg = context->RegisterService<SingletonTwoService>(m_SingletonTwo);

    // Now the SingletonOneService constructor will get a valid
    // SingletonTwoService instance.
    m_SingletonOne = new SingletonOneService;
    m_SingletonOneReg = context->RegisterService<SingletonOneService>(m_SingletonOne);
  }
  //![0]

  //![1]
  void Unload(ModuleContext* /*context*/)
  {
    // Services are automatically unregistered during unloading of
    // the shared library after the call to Unload(ModuleContext*)
    // has returned.

    // Since SingletonOneService needs a non-null SingletonTwoService
    // instance in its destructor, we explicitly unregister and delete the
    // SingletonOneService instance here. This way, the SingletonOneService
    // destructor will still get a valid SingletonTwoService instance.
    m_SingletonOneReg.Unregister();
    delete m_SingletonOne;

    // For singletonTwoService, we could rely on the automatic unregistering
    // by the service registry and on automatic deletion if you used
    // smart pointer reference counting. You must not delete service instances
    // in this method without unregistering them first.
    m_SingletonTwoReg.Unregister();
    delete m_SingletonTwo;
  }
  //![1]

private:

  SingletonOneService* m_SingletonOne;
  SingletonTwoService* m_SingletonTwo;

  ServiceRegistration<SingletonOneService> m_SingletonOneReg;
  ServiceRegistration<SingletonTwoService> m_SingletonTwoReg;

};

US_EXPORT_MODULE_ACTIVATOR(MyActivator)

int main()
{
}
