#include "ManagedServiceImpl.hpp"

#include <cppmicroservices/BundleContext.h>
#include <cppmicroservices/Framework.h>
#include <cppmicroservices/FrameworkFactory.h>
#include <iostream>
#include <string>

namespace cppmicroservices {
namespace service {
namespace cm {
namespace test {

TestManagedServiceImpl2::TestManagedServiceImpl2()
  : m_counter{ 0 }
{}

TestManagedServiceImpl2::~TestManagedServiceImpl2() = default;

void TestManagedServiceImpl2::Updated(AnyMap const& props)
{
  {
    std::lock_guard<std::mutex> lk(m_counterMtx);
    m_counter++;
  } //unlock m_counterMtx

  if (!props.empty()) {
    std::lock_guard<std::mutex> lock(m_propertiesMutex);

    if (!props.AtCompoundKey("myMutex", Any()).Empty()) {
      m_cvMtx = cppmicroservices::any_cast<std::shared_ptr<std::mutex>>(
          props.at("myMutex"));
    }
   
    if (!props.AtCompoundKey("myConditionVariable", Any()).Empty()) {
      m_cv = cppmicroservices::any_cast<std::shared_ptr<std::condition_variable>>(
          props.at("myConditionVariable"));
    }
   } // unlock m_propertiesMutex

   // This is simulating a deadlock.
   if (m_cvMtx && m_cv) {
      std::unique_lock<std::mutex> lck(*m_cvMtx);
      m_cv->wait(lck);
   }
 
}

int TestManagedServiceImpl2::getCounter()
{
  std::lock_guard<std::mutex> lk(m_counterMtx);
  return m_counter;
}

} // namespace test
} // namespace cm
} // namespace service
} // namespace cppmicroservices
