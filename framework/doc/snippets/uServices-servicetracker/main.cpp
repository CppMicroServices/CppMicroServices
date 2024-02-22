#include "cppmicroservices/BundleInitialization.h"
#include "cppmicroservices/GetBundleContext.h"
#include "cppmicroservices/ServiceTracker.h"

CPPMICROSERVICES_INITIALIZE_BUNDLE

using namespace cppmicroservices;

struct IFooService
{
};

///! [tt]
struct MyTrackedClass
{
    explicit operator bool() const { return true; }
    /* ... */
};
//! [tt]

//! [customizer]
struct MyTrackingCustomizer : public ServiceTrackerCustomizer<IFooService, MyTrackedClass>
{
    virtual std::shared_ptr<MyTrackedClass>
    AddingService(ServiceReference<IFooService> const&)
    {
        return std::shared_ptr<MyTrackedClass>();
    }

    virtual void
    ModifiedService(ServiceReference<IFooService> const&, std::shared_ptr<MyTrackedClass> const&)
    {
    }

    virtual void
    RemovedService(ServiceReference<IFooService> const&, std::shared_ptr<MyTrackedClass> const&)
    {
    }
};
//! [customizer]

// For compilation test purposes only
struct MyTrackingCustomizerVoid : public ServiceTrackerCustomizer<void, MyTrackedClass>
{
    virtual std::shared_ptr<MyTrackedClass>
    AddingService(ServiceReferenceU const&)
    {
        return std::shared_ptr<MyTrackedClass>();
    }

    virtual void
    ModifiedService(ServiceReferenceU const&, std::shared_ptr<MyTrackedClass> const&)
    {
    }

    virtual void
    RemovedService(ServiceReferenceU const&, std::shared_ptr<MyTrackedClass> const&)
    {
    }
};

int
main(int /*argc*/, char* /*argv*/[])
{
    std::cout << "This snippet is not meant to be executed.\n"
                 "It does not provide a complete working example.\n"
                 "See "
                 "http://docs.cppmicroservices.org/en/stable/doc/src/getting_started.html"
              << std::endl;
    return 0;

    {
        //! [tracker]
        MyTrackingCustomizer myCustomizer;
        ServiceTracker<IFooService, MyTrackedClass> tracker(GetBundleContext(), &myCustomizer);
        //! [tracker]
    }

    // For compilation test purposes only
    MyTrackingCustomizerVoid myCustomizer2;
    try
    {
        ServiceTracker<void, MyTrackedClass> tracker2(GetBundleContext(), &myCustomizer2);
    }
    catch (cppmicroservices::ServiceException const&)
    {
    }
}
