#include "ManagedServiceFactoryImpl.hpp"

#include <iostream>

namespace cppmicroservices {
namespace service {
namespace cm {
namespace test {

TestManagedServiceFactoryImpl::~TestManagedServiceFactoryImpl() = default;

void TestManagedServiceFactoryImpl::Updated(std::string const& pid, AnyMap const& properties) {
    std::lock_guard<std::mutex> lk(m_updatedMtx);
    if (properties.empty()) {
        std::cout << "    Updated called for pid \"" << pid << "\" with empty properties" << std::endl;
        m_updatedCallCount[pid] -= 1;
    } else {
        auto const incrementBy = cppmicroservices::any_cast<int>(properties.AtCompoundKey("anInt"));
        std::cout << "    Updated called for pid \"" << pid
                  << "\" with value: " << std::to_string(incrementBy) << std::endl;
        m_updatedCallCount[pid] += incrementBy;
    }
}

void TestManagedServiceFactoryImpl::Removed(std::string const& pid) {
    std::lock_guard<std::mutex> lk(m_removedMtx);
    std::cout << "    Removed called for pid \"" << pid << "\" with empty properties" << std::endl;
    ++m_removedCallCount[pid];
}

int TestManagedServiceFactoryImpl::getUpdatedCounter(std::string const& pid) {
    std::lock_guard<std::mutex> lk(m_updatedMtx);
    return m_updatedCallCount[pid];
}

int TestManagedServiceFactoryImpl::getRemovedCounter(std::string const& pid) {
    std::lock_guard<std::mutex> lk(m_removedMtx);
    return m_removedCallCount[pid];
}

std::shared_ptr<::test::TestManagedServiceFactoryServiceInterface> TestManagedServiceFactoryImpl::create(
    std::string const& config) {

    std::lock_guard<std::mutex> lk(m_updatedMtx);
    try {
        return std::make_shared<TestManagedServiceFactoryServiceImpl>(m_updatedCallCount.at(config));
    } catch (...) {
        return nullptr;
    }
}

} // namespace test
} // namespace cm
} // namespace service
} // namespace cppmicroservices
