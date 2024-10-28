#ifndef _SERVICE_IMPL_HPP_
#define _SERVICE_IMPL_HPP_

#include <mutex>

#include "TestInterfaces/Interfaces.hpp"
#include "cppmicroservices/servicecomponent/ComponentContext.hpp"
#include "cppmicroservices/servicecomponent/usrDefinedMethodAssertion.hpp"
using ComponentContext = cppmicroservices::service::component::ComponentContext;

namespace sample
{

    class ServiceComponentWrongBindSig final
        : public test::Interface2
        , cppmicroservices::service::component::usrDefinedMethodAssertion
    {
      public:
        ServiceComponentWrongBindSig() = default;
        ~ServiceComponentWrongBindSig() = default;
        std::string ExtendedDescription() override;

        void Bindfoo(std::shared_ptr<test::Interface1> const& theFoo);
        void Unbindfoo(std::shared_ptr<test::Interface1> const& theFoo);

        // void Bindfoo(std::shared_ptr<test::Interface1> const& theFoo, bool /*someBool*/);
        // void Unbindfoo(std::shared_ptr<test::Interface1> const& theFoo, bool /*someBool*/);
        void Modified(std::shared_ptr<ComponentContext> const& context,
                      std::shared_ptr<cppmicroservices::AnyMap> const& configuration);
        void Activate(std::shared_ptr<ComponentContext> const& context);
        void Deactivate(std::shared_ptr<ComponentContext> const& context);

      private:
        std::shared_ptr<test::Interface1> foo;
        std::mutex fooMutex;
    };

} // namespace sample

#endif // _SERVICE_IMPL_HPP_
