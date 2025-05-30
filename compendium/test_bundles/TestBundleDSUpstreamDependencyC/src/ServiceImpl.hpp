#ifndef _SERVICE_IMPL_HPP_
#define _SERVICE_IMPL_HPP_

#include "TestInterfaces/Interfaces.hpp"
#include "cppmicroservices/servicecomponent/ComponentContext.hpp"
#include <iostream>
namespace dependent
{

    using ComponentContext = cppmicroservices::service::component::ComponentContext;

    class TestBundleDSUpstreamDependencyIsActivatedImpl : public test::TestBundleDSUpstreamDependencyIsActivated
    {
      public:
        TestBundleDSUpstreamDependencyIsActivatedImpl(){
          getCount(true);
        }
        ~TestBundleDSUpstreamDependencyIsActivatedImpl() override;

        void
        Activate(std::shared_ptr<ComponentContext> const&)
        {
            activated = true;
        }

        bool
        isActivated() override
        {
            return activated;
        }

        size_t numberCreated() override {
          return getCount(false);
        }
      private: 
        bool activated = false;
        size_t getCount(bool increment){
          static int count = []() {
              return 0;
          }();

          if (increment){
            ++count;
          }
          return count;
        }
    };
} // namespace dependent

#endif // _SERVICE_IMPL_HPP_
