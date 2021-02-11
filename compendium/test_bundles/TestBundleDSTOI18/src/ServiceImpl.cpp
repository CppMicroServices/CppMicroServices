#include "ServiceImpl.hpp"

namespace sample {

void ServiceComponent18::Activate(const std::shared_ptr<ComponentContext>& /*ctxt*/)
{
}
  
void ServiceComponent18::Deactivate(const std::shared_ptr<ComponentContext>&)
{
}

bool ServiceComponent18::isDependencyInjected()
{
    return sample::ServiceComponent18::constructorHit;
}

}
