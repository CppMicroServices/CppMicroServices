#include "ServiceImpl.hpp"

namespace sample
{

    bool
    ServiceComponent18::isDependencyInjected()
    {
        return constructorHit;
    }

} // namespace sample
