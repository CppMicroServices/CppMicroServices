#include "Factory.hpp"

namespace cppmicroservices
{
    namespace service
    {
        namespace cm
        {
            namespace test
            {
                TestManagedFactoryImpl::~TestManagedFactoryImpl() = default;

                cppmicroservices::ServiceProperties
                createProperties(std::string emId, cppmicroservices::AnyMap const& config)
                {
                    cppmicroservices::ServiceProperties props(config.cbegin(), config.cend());

                    props.emplace(std::make_pair("serID", emId));

                    return props;
                }

                void
                TestManagedFactoryImpl::Activate(
                    std::shared_ptr<cppmicroservices::service::component::ComponentContext> const& context)
                {
                    bc = context->GetBundleContext();
                }

                void
                TestManagedFactoryImpl::Deactivate(
                    std::shared_ptr<cppmicroservices::service::component::ComponentContext> const&)
                {
                    return;
                }

                void
                TestManagedFactoryImpl::Updated(std::string const& pid, AnyMap const& properties)
                {
                    if (regs.find(pid) != regs.end())
                    {
                        // MW_LOG_INFO(logger, << "Returning early due to existing " << pid);
                        return;
                    }

                    // namespace fme = foundation::msg_svc::eventmgr;
                    std::string ID((properties.AtCompoundKey("serID").ToString()));
                    std::shared_ptr<TestManagedFactoryServiceImpl> ser
                        = std::make_shared<TestManagedFactoryServiceImpl>(std::stoi(ID));
                    auto serviceProperties = createProperties(ID, properties);
                    if (!ser)
                    {
                        // FL_DIAG_ASSERT_ALWAYS("Inserting null eventmgr into cppmicroservices");
                        // MW_LOG_INFO(logger, << "Inserting null eventmgr into cppmicroservices");
                        return;
                    }
                    auto registration = bc.RegisterService<::test::TestManagedServiceFactoryServiceInterface>(std::move(ser),
                                                                                          std::move(serviceProperties));
                    regs.insert({ pid, std::move(registration) });
                }

                void
                TestManagedFactoryImpl::Removed(std::string const& pid)
                {
                    US_UNUSED(pid);
                }

            } // namespace test
        }     // namespace cm
    }         // namespace service
} // namespace cppmicroservices
