#include <chrono>

#include <gtest/gtest.h>

#include "../../src/SCRExtensionRegistry.hpp"
#include "../../src/manager/ComponentConfigurationImpl.hpp"
#include "../../src/manager/ReferenceManager.hpp"
#include "../../src/manager/SingletonComponentConfiguration.hpp"

#include "cppmicroservices/Framework.h"
#include "cppmicroservices/FrameworkEvent.h"
#include "cppmicroservices/FrameworkFactory.h"

#include "../TestUtils.hpp"
#include "Mocks.hpp"

namespace scr = cppmicroservices::service::component::runtime;

namespace cppmicroservices
{
    namespace scrimpl
    {
        metadata::ReferenceMetadata
        CreateReferenceMetadata(std::string intName, std::string cardinality, int min, int max)
        {
            metadata::ReferenceMetadata fakeMetadata {};
            fakeMetadata.name = "ref";
            fakeMetadata.interfaceName = intName;
            fakeMetadata.cardinality = cardinality;
            fakeMetadata.minCardinality = min;
            fakeMetadata.maxCardinality = max;
            return fakeMetadata;
        }

        std::shared_ptr<metadata::ComponentMetadata>
        CreateComponentMetadata(std::string name)
        {
            auto ret = std::make_shared<metadata::ComponentMetadata>();
            ret->implClassName = name;
            ret->name = name;

            return ret;
        }

        TEST(TestCircularReference, circularReferenceOptionalTest)
        {
            auto mockRegistry = std::make_shared<ComponentRegistry>();
            auto fakeLogger = std::make_shared<FakeLogger>();

            // service component A requires a reference to a service from service component B and B
            // requires a reference to a service from service component A.
            // Optional cardinality on B breaks the cycle and allows
            // the service components to be satisfied.
            auto nameA = us_service_interface_iid<dummy::Reference1>();
            auto nameB = us_service_interface_iid<dummy::Reference2>();

            auto componentAMetadata = CreateComponentMetadata(nameA);
            auto componentBMetadata = CreateComponentMetadata(nameB);

            metadata::ReferenceMetadata referenceA = CreateReferenceMetadata(nameA, "1..1", 1, 1);
            metadata::ReferenceMetadata referenceB = CreateReferenceMetadata(nameB, "0..1", 0, 1);

            componentAMetadata->refsMetadata.emplace_back(referenceB);
            componentBMetadata->refsMetadata.emplace_back(referenceA);

            componentAMetadata->serviceMetadata.interfaces.emplace_back(referenceA.interfaceName);
            componentBMetadata->serviceMetadata.interfaces.emplace_back(referenceB.interfaceName);

            auto framework = cppmicroservices::FrameworkFactory().NewFramework();
            framework.Start();
            auto context = framework.GetBundleContext();

            auto asyncWorkSvc = std::make_shared<SCRAsyncWorkService>(context, fakeLogger);

            auto logger = std::make_shared<SCRLogger>(context);
            auto extRegistry = std::make_shared<SCRExtensionRegistry>(logger);

            auto configNotifier
                = std::make_shared<ConfigurationNotifier>(context, fakeLogger, asyncWorkSvc, extRegistry);

            auto componentA = std::make_shared<SingletonComponentConfigurationImpl>(componentAMetadata,
                                                                                    framework,
                                                                                    mockRegistry,
                                                                                    fakeLogger,
                                                                                    configNotifier);
            auto componentB = std::make_shared<SingletonComponentConfigurationImpl>(componentBMetadata,
                                                                                    framework,
                                                                                    mockRegistry,
                                                                                    fakeLogger,
                                                                                    configNotifier);
            componentB->Initialize();
            auto refB = context.GetServiceReference<dummy::Reference2>();
            ASSERT_EQ(refB.operator bool(), false);

            componentA->Initialize();
            auto refA = context.GetServiceReference<dummy::Reference1>();

            refB = context.GetServiceReference<dummy::Reference2>();
            // assert that references are invalid with unsatisfied configuration
            ASSERT_EQ(refA.operator bool(), true);
            ASSERT_EQ(refB.operator bool(), true);

            ASSERT_EQ(cppmicroservices::service::component::runtime::dto::ComponentState::SATISFIED,
                      componentA->GetState()->GetValue());
            ASSERT_EQ(cppmicroservices::service::component::runtime::dto::ComponentState::SATISFIED,
                      componentB->GetState()->GetValue());

            framework.Stop();
            framework.WaitForStop(std::chrono::milliseconds::zero());
        }

        TEST(TestCircularReference, circularReferenceMandatoryTest)
        {
            auto mockRegistry = std::make_shared<ComponentRegistry>();
            auto fakeLogger = std::make_shared<FakeLogger>();

            // service component A requires a reference to a service from service component B and B
            // requires a reference to a service from service component A.
            // In this case, neither A nor B will be satisfied and an error should be
            // logged about detecting a circular dependency.
            auto nameA = us_service_interface_iid<dummy::Reference1>();
            auto nameB = us_service_interface_iid<dummy::Reference2>();

            auto componentAMetadata = CreateComponentMetadata(nameA);
            auto componentBMetadata = CreateComponentMetadata(nameB);

            metadata::ReferenceMetadata referenceA = CreateReferenceMetadata(nameA, "1..1", 1, 1);
            metadata::ReferenceMetadata referenceB = CreateReferenceMetadata(nameB, "1..1", 1, 1);

            componentAMetadata->refsMetadata.emplace_back(referenceB);
            componentBMetadata->refsMetadata.emplace_back(referenceA);

            componentAMetadata->serviceMetadata.interfaces.emplace_back(referenceA.interfaceName);
            componentBMetadata->serviceMetadata.interfaces.emplace_back(referenceB.interfaceName);

            auto framework = cppmicroservices::FrameworkFactory().NewFramework();
            framework.Start();
            auto context = framework.GetBundleContext();

            auto asyncWorkSvc = std::make_shared<SCRAsyncWorkService>(context, fakeLogger);

            auto logger = std::make_shared<SCRLogger>(context);
            auto extRegistry = std::make_shared<SCRExtensionRegistry>(logger);

            auto configNotifier
                = std::make_shared<ConfigurationNotifier>(context, fakeLogger, asyncWorkSvc, extRegistry);

            auto componentA = std::make_shared<SingletonComponentConfigurationImpl>(componentAMetadata,
                                                                                    framework,
                                                                                    mockRegistry,
                                                                                    fakeLogger,
                                                                                    configNotifier);
            auto componentB = std::make_shared<SingletonComponentConfigurationImpl>(componentBMetadata,
                                                                                    framework,
                                                                                    mockRegistry,
                                                                                    fakeLogger,
                                                                                    configNotifier);

            componentA->Initialize();
            componentB->Initialize();

            auto refA = context.GetServiceReference<dummy::Reference1>();
            auto refB = context.GetServiceReference<dummy::Reference2>();

            // assert that references are invalid with unsatisfied configuration
            ASSERT_EQ(refA.operator bool(), false);
            ASSERT_EQ(refB.operator bool(), false);

            // component should not be satisfied
            ASSERT_EQ(cppmicroservices::service::component::runtime::dto::ComponentState::UNSATISFIED_REFERENCE,
                      componentA->GetState()->GetValue());
            ASSERT_EQ(cppmicroservices::service::component::runtime::dto::ComponentState::UNSATISFIED_REFERENCE,
                      componentB->GetState()->GetValue());

            framework.Stop();
            framework.WaitForStop(std::chrono::milliseconds::zero());
        }
    } // namespace scrimpl
} // namespace cppmicroservices