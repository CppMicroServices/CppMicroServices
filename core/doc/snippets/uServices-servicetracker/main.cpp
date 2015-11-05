#include <usServiceTracker.h>
#include <usGetBundleContext.h>

using namespace us;

struct IFooService {};

///! [tt]
struct MyTrackedClass {
  explicit operator bool() const { return true; }
  /* ... */
};
//! [tt]

//! [customizer]
struct MyTrackingCustomizer : public ServiceTrackerCustomizer<IFooService, MyTrackedClass>
{
  virtual MyTrackedClass AddingService(const ServiceReferenceType&)
  {
    return MyTrackedClass();
  }

  virtual void ModifiedService(const ServiceReferenceType&, MyTrackedClass&)
  {
  }

  virtual void RemovedService(const ServiceReferenceType&, MyTrackedClass&)
  {
  }
};
//! [customizer]

struct MyTrackingPointerCustomizer : public ServiceTrackerCustomizer<IFooService, MyTrackedClass*>
{
  virtual MyTrackedClass* AddingService(const ServiceReferenceType&)
  {
    return new MyTrackedClass();
  }

  virtual void ModifiedService(const ServiceReferenceType&, MyTrackedClass*)
  {
  }

  virtual void RemovedService(const ServiceReferenceType&, MyTrackedClass*)
  {
  }
};

// For compilation test purposes only
struct MyTrackingCustomizerVoid : public ServiceTrackerCustomizer<void, MyTrackedClass>
{
  virtual MyTrackedClass AddingService(const ServiceReferenceType&)
  {
    return MyTrackedClass();
  }

  virtual void ModifiedService(const ServiceReferenceType&, MyTrackedClass&)
  {
  }

  virtual void RemovedService(const ServiceReferenceType&, MyTrackedClass&)
  {
  }
};

int main(int /*argc*/, char* /*argv*/[])
{
  {
//! [tracker]
MyTrackingCustomizer myCustomizer;
ServiceTracker<IFooService, MyTrackedClass> tracker(GetBundleContext(), &myCustomizer);
//! [tracker]
  }

  {
//! [tracker2]
MyTrackingPointerCustomizer myCustomizer;
ServiceTracker<IFooService, MyTrackedClass*> tracker(GetBundleContext(), &myCustomizer);
//! [tracker2]
  }

  // For compilation test purposes only
  MyTrackingCustomizerVoid myCustomizer2;
  try
  {
    ServiceTracker<void, MyTrackedClass> tracker2(GetBundleContext(), &myCustomizer2);
    ServiceTracker<void, MyTrackedClass*> tracker3(GetBundleContext());
  }
  catch (const us::ServiceException&)
  {}

  return 0;
}

