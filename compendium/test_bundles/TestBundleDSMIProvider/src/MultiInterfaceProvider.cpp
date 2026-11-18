#include "ServiceImpl.hpp"

namespace sample
{

    std::string
    MultiInterfaceProvider::WhoA()
    {
        return "A";
    }

    std::string
    MultiInterfaceProvider::WhoB()
    {
        return "B";
    }
} // namespace sample
