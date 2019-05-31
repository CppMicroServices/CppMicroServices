
#include <cppmicroservices/ServiceInterface.h>

// A dummy interface and impl to use with service trackers
namespace {
    class Foo {
      public:
        virtual ~Foo() = default;
    };
    
    class FooImpl : public Foo {
    };
}

// Since the Foo interface is embedded in the test executable, its symbols are
// not exported. Using CPPMICROSERVICES_DECLARE_SERVICE_INTERFACE ensures that
// the symbols are exported correctly for use by CppMicroServices.
CPPMICROSERVICES_DECLARE_SERVICE_INTERFACE(Foo, "org.cppmicroservices.googlebenchmark.test.Foo");
