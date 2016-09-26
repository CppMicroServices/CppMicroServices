/*=============================================================================

 Library: CppMicroServices

 Copyright (c) The CppMicroServices developers. See the COPYRIGHT
 file at the top-level directory of this distribution and at
 https://github.com/CppMicroServices/CppMicroServices/COPYRIGHT .

 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at

 http://www.apache.org/licenses/LICENSE-2.0

 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.

 =============================================================================*/

#include "cppmicroservices/Bundle.h"
#include "cppmicroservices/Framework.h"
#include "cppmicroservices/FrameworkEvent.h"
#include "cppmicroservices/FrameworkFactory.h"
#include "cppmicroservices/LDAPFilter.h"

#include "TestingMacros.h"
#include "TestUtils.h"

using namespace cppmicroservices;

void TestLDAPFilterMatchBundle(const Bundle& bundle)
{
  LDAPFilter ldapMatchCase( "(bundle.testproperty=YES)" );
  LDAPFilter ldapKeyMismatchCase( "(bundle.TestProperty=YES)" );
  LDAPFilter ldapValueMismatchCase( "(bundle.testproperty=Yes)" );

  // Exact string match of both key and value
  US_TEST_CONDITION(ldapMatchCase.Match(bundle), " Evaluating LDAP expr: " + ldapMatchCase.ToString());

  // Testing case-insensitive key (should still pass)
  US_TEST_CONDITION(ldapKeyMismatchCase.Match(bundle), " Evaluating LDAP expr: " + ldapKeyMismatchCase.ToString());

  // Testing case-insensitive value (should fail)
  US_TEST_CONDITION(!ldapValueMismatchCase.Match(bundle), " Evaluating LDAP expr: " + ldapValueMismatchCase.ToString());
}

void TestLDAPFilterMatchServiceReferenceBase(Bundle bundle)
{
  LDAPFilter ldapMatchCase( "(service.testproperty=YES)" );
  LDAPFilter ldapKeyMismatchCase( "(service.TestProperty=YES)" );
  LDAPFilter ldapValueMismatchCase( "(service.testproperty=Yes)" );

  bundle.Start();

  auto thisBundleCtx = bundle.GetBundleContext();
  ServiceReferenceU sr = thisBundleCtx.GetServiceReference("cppmicroservices::TestBundleLQService");

  // Make sure the obtained ServiceReferenceBase object is not null
  US_TEST_CONDITION(sr, " Checking non-empty ServiceReferenceBase object");

  // Exact string match of both key and value
  US_TEST_CONDITION(ldapMatchCase.Match(sr), " Evaluating LDAP expr: " + ldapMatchCase.ToString());

  // Testing case-insensitive key (should still pass)
  US_TEST_CONDITION(ldapKeyMismatchCase.Match(sr), " Evaluating LDAP expr: " + ldapKeyMismatchCase.ToString());

  // Testing case-insensitive value (should fail)
  US_TEST_CONDITION(!ldapValueMismatchCase.Match(sr), " Evaluating LDAP expr: " + ldapValueMismatchCase.ToString());

  bundle.Stop();

  // Testing the behavior after the bundle has stopped (service properties
  // should still be available for queries according to OSGi spec 5.2.1).
  US_TEST_CONDITION(ldapMatchCase.Match(sr), " Evaluating LDAP expr: " + ldapMatchCase.ToString());
}

int LDAPQueryTest(int /*argc*/, char* /*argv*/[])
{
  US_TEST_BEGIN("LDAPQueryTest");

  FrameworkFactory factory;
  auto framework = factory.NewFramework();
  framework.Start();

  auto bundle = testing::InstallLib(framework.GetBundleContext(), "TestBundleLQ");

  US_TEST_OUTPUT(<< "Testing LDAP query of bundle properties:")
  TestLDAPFilterMatchBundle(bundle);

  US_TEST_OUTPUT(<< "Testing LDAP query of service properties:")
  TestLDAPFilterMatchServiceReferenceBase(bundle);

  framework.Stop();
  framework.WaitForStop(std::chrono::milliseconds::zero());

  US_TEST_END()
}
