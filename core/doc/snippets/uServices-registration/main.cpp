#include <usBundleActivator.h>
#include <usBundleContext.h>
#include <usServiceFactory.h>
#include <usServiceInterface.h>

US_USE_NAMESPACE


struct InterfaceA { virtual ~InterfaceA() {} };
struct InterfaceB { virtual ~InterfaceB() {} };
struct InterfaceC { virtual ~InterfaceC() {} };

//! [1-1]
class MyService : public InterfaceA
{};
//! [1-1]

//! [2-1]
class MyService2 : public InterfaceA, public InterfaceB
{};
//! [2-1]

class MyActivator : public BundleActivator
{

public:

  void Start(BundleContext* context)
  {
    Register1(context);
    Register2(context);
    RegisterFactory1(context);
    RegisterFactory2(context);
  }

  void Register1(BundleContext* context)
  {
//! [1-2]
MyService* myService = new MyService;
context->RegisterService<InterfaceA>(myService);
//! [1-2]
  }

  void Register2(BundleContext* context)
  {
//! [2-2]
MyService2* myService = new MyService2;
context->RegisterService<InterfaceA, InterfaceB>(myService);
//! [2-2]
  }

  void RegisterFactory1(BundleContext* context)
  {
//! [f1]
class MyServiceFactory : public ServiceFactory
{
  virtual InterfaceMap GetService(Bundle* /*bundle*/, const ServiceRegistrationBase& /*registration*/)
  {
    MyService* myService = new MyService;
    return MakeInterfaceMap<InterfaceA>(myService);
  }

  virtual void UngetService(Bundle* /*bundle*/, const ServiceRegistrationBase& /*registration*/,
                            const InterfaceMap& service)
  {
    delete ExtractInterface<InterfaceA>(service);
  }
};

MyServiceFactory* myServiceFactory = new MyServiceFactory;
context->RegisterService<InterfaceA>(myServiceFactory);
//! [f1]
  }

  void RegisterFactory2(BundleContext* context)
  {
//! [f2]
class MyServiceFactory : public ServiceFactory
{
  virtual InterfaceMap GetService(Bundle* /*bundle*/, const ServiceRegistrationBase& /*registration*/)
  {
    MyService2* myService = new MyService2;
    return MakeInterfaceMap<InterfaceA,InterfaceB>(myService);
  }

  virtual void UngetService(Bundle* /*bundle*/, const ServiceRegistrationBase& /*registration*/,
                            const InterfaceMap& service)
  {
    delete ExtractInterface<InterfaceA>(service);
  }
};

MyServiceFactory* myServiceFactory = new MyServiceFactory;
context->RegisterService<InterfaceA,InterfaceB>(static_cast<ServiceFactory*>(myServiceFactory));
//! [f2]
// In the RegisterService call above, we could remove the static_cast because local types
// are not considered in template argument type deduction and hence the compiler choose
// the correct RegisterService<I1,I2>(ServiceFactory*) overload. However, local types are
// usually the exception and using a non-local type for the service factory would make the
// compiler choose RegisterService<I1,I2,Impl>(Impl*) instead, unless we use the static_cast.
  }


  void Stop(BundleContext* /*context*/)
  { /* cleanup */ }

};

US_EXPORT_BUNDLE_ACTIVATOR(MyActivator)

int main(int /*argc*/, char* /*argv*/[])
{
  MyActivator ma;
  return 0;
}
