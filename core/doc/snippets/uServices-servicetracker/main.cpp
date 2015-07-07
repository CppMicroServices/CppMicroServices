#include <usServiceTracker.h>
#include <usGetModuleContext.h>

US_USE_NAMESPACE

struct IFooService {};

///! [tt]
struct MyTrackedClass { /* ... */ };
//! [tt]

//! [ttt]
struct MyTrackedClassTraits : public TrackedTypeTraitsBase<MyTrackedClass, MyTrackedClassTraits>
{
  static bool IsValid(const TrackedType&)
  {
    // Dummy implementation
    return true;
  }

  static void Dispose(TrackedType&)
  {}

  static TrackedType DefaultValue()
  {
    return TrackedType();
  }
};
//! [ttt]

//! [customizer]
struct MyTrackingCustomizer : public ServiceTrackerCustomizer<IFooService, MyTrackedClass>
{
  virtual MyTrackedClass AddingService(const ServiceReferenceType&)
  {
    return MyTrackedClass();
  }

  virtual void ModifiedService(const ServiceReferenceType&, MyTrackedClass)
  {
  }

  virtual void RemovedService(const ServiceReferenceType&, MyTrackedClass)
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

  virtual void ModifiedService(const ServiceReferenceType&, MyTrackedClass)
  {
  }

  virtual void RemovedService(const ServiceReferenceType&, MyTrackedClass)
  {
  }
};

int main(int /*argc*/, char* /*argv*/[])
{
  {
//! [tracker]
MyTrackingCustomizer myCustomizer;
ServiceTracker<IFooService, MyTrackedClassTraits> tracker(GetModuleContext(), &myCustomizer);
//! [tracker]
  }

  {
//! [tracker2]
MyTrackingPointerCustomizer myCustomizer;
ServiceTracker<IFooService, TrackedTypeTraits<IFooService,MyTrackedClass*> > tracker(GetModuleContext(), &myCustomizer);
//! [tracker2]
  }

  // For compilation test purposes only
  MyTrackingCustomizerVoid myCustomizer2;
  try
  {
    ServiceTracker<void, MyTrackedClassTraits> tracker2(GetModuleContext(), &myCustomizer2);
    ServiceTracker<void, TrackedTypeTraits<void,MyTrackedClass*> > tracker3(GetModuleContext());
  }
  catch (const us::ServiceException&)
  {}

  return 0;
}

#include <usModuleInitialization.h>

US_INITIALIZE_MODULE
