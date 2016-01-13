#include <usBundleActivator.h>
#include <usBundleContext.h>
#include <usServiceFactory.h>
#include <usServiceInterface.h>

using namespace us;


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
std::shared_ptr<MyService> myService = std::make_shared<MyService>();
context->RegisterService<InterfaceA>(myService);
//! [1-2]
  }

  void Register2(BundleContext* context)
  {
//! [2-2]
std::shared_ptr<MyService2> myService = std::make_shared<MyService2>();
context->RegisterService<InterfaceA, InterfaceB>(myService);
//! [2-2]
  }

  void RegisterFactory1(BundleContext* context)
  {
//! [f1]
class MyServiceFactory : public ServiceFactory
{
  virtual InterfaceMapConstPtr GetService(const std::shared_ptr<Bundle>& /*bundle*/, const ServiceRegistrationBase& /*registration*/)
  {
    return MakeInterfaceMap<InterfaceA>(std::make_shared<MyService>());;
  }

  virtual void UngetService(const std::shared_ptr<Bundle>& /*bundle*/, const ServiceRegistrationBase& /*registration*/,
                            const InterfaceMapConstPtr& /*service*/)
  {
    
  }
};

std::shared_ptr<MyServiceFactory> myServiceFactory = std::make_shared<MyServiceFactory>();
context->RegisterService<InterfaceA>(ToFactory(myServiceFactory));
//! [f1]
  }

  void RegisterFactory2(BundleContext* context)
  {
//! [f2]
class MyServiceFactory : public ServiceFactory
{
  virtual InterfaceMapConstPtr GetService(const std::shared_ptr<Bundle>& /*bundle*/, const ServiceRegistrationBase& /*registration*/)
  {
    return MakeInterfaceMap<InterfaceA,InterfaceB>(std::make_shared<MyService2>());
  }

  virtual void UngetService(const std::shared_ptr<Bundle>& /*bundle*/, const ServiceRegistrationBase& /*registration*/,
                            const InterfaceMapConstPtr& /*service*/)
  {
    
  }
};

std::shared_ptr<MyServiceFactory> myServiceFactory = std::make_shared<MyServiceFactory>();
context->RegisterService<InterfaceA,InterfaceB>(ToFactory(myServiceFactory));
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
