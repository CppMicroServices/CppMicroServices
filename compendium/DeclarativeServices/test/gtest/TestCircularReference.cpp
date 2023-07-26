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
        CreateFakeReferenceMetadata(std::string intName, std::string cardinality, int min, int max)
        {
            metadata::ReferenceMetadata fakeMetadata {};
            fakeMetadata.name = "ref";
            fakeMetadata.interfaceName = intName;
            fakeMetadata.cardinality = cardinality;
            fakeMetadata.minCardinality = min;
            fakeMetadata.maxCardinality = max;
            return fakeMetadata;
        }

        TEST(TestCircularReference, circularReferenceOptionalTest)
        {

            auto mockRegistry = std::make_shared<MockComponentRegistry>();
            auto fakeLogger = std::make_shared<FakeLogger>();

            // service component A requires a reference to a service from service component B and B
            // requires a reference to a service from service component A.
            // Optional cardinality on B breaks the cycle and allows
            // the service components to be satisfied.
            auto componentAMetadata = std::make_shared<metadata::ComponentMetadata>();
            auto componentBMetadata = std::make_shared<metadata::ComponentMetadata>();

            auto nameA = us_service_interface_iid<dummy::Reference1>();
            auto nameB = us_service_interface_iid<dummy::Reference2>();

            componentAMetadata->implClassName = nameA;
            componentAMetadata->name = nameA;
            componentBMetadata->implClassName = nameB;
            componentBMetadata->name = nameB;

            metadata::ReferenceMetadata referenceA = CreateFakeReferenceMetadata(nameA, "1..1", 1, 1);
            metadata::ReferenceMetadata referenceB = CreateFakeReferenceMetadata(nameB, "0..1", 0, 1);

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

            ASSERT_EQ(cppmicroservices::service::component::runtime::dto::ComponentState::SATISFIED,
                      componentA->GetState()->GetValue());
            ASSERT_EQ(cppmicroservices::service::component::runtime::dto::ComponentState::SATISFIED,
                      componentB->GetState()->GetValue());

            framework.Stop();
            framework.WaitForStop(std::chrono::milliseconds::zero());
        }

        TEST(TestCircularReference, circularReferenceMandatoryTest)
        {
            auto mockRegistry = std::make_shared<MockComponentRegistry>();
            auto fakeLogger = std::make_shared<FakeLogger>();

            // service component A requires a reference to a service from service component B and B
            // requires a reference to a service from service component A.
            // In this case, neither A nor B will be satisfied and an error should be
            // logged about detecting a circular dependency.
            auto componentAMetadata = std::make_shared<metadata::ComponentMetadata>();
            auto componentBMetadata = std::make_shared<metadata::ComponentMetadata>();

            auto nameA = us_service_interface_iid<dummy::Reference1>();
            auto nameB = us_service_interface_iid<dummy::Reference2>();

            componentAMetadata->implClassName = nameA;
            componentAMetadata->name = nameA;
            componentBMetadata->implClassName = nameB;
            componentBMetadata->name = nameB;

            metadata::ReferenceMetadata referenceA = CreateFakeReferenceMetadata(nameA, "1..1", 1, 1);
            metadata::ReferenceMetadata referenceB = CreateFakeReferenceMetadata(nameB, "1..1", 1, 1);

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

            ASSERT_EQ(cppmicroservices::service::component::runtime::dto::ComponentState::UNSATISFIED_REFERENCE,
                      componentA->GetState()->GetValue());
            ASSERT_EQ(cppmicroservices::service::component::runtime::dto::ComponentState::UNSATISFIED_REFERENCE,
                      componentB->GetState()->GetValue());

            framework.Stop();
            framework.WaitForStop(std::chrono::milliseconds::zero());
        }
    } // namespace scrimpl
} // namespace cppmicroservices