/*=============================================================================
 
 Library: CppMicroServices
 
 Copyright (c) The CppMicroServices developers. See the COPYRIGHT
 file at the top-level directory of this distribution and at
 https://github.com/saschazelzer/CppMicroServices/COPYRIGHT .
 
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

#include <usFrameworkFactory.h>
#include <usFramework.h>

#include <usBundle.h>
#include <usLDAPFilter.h>
#include <usLog.h>

#include "usTestUtils.h"
#include "usTestingMacros.h"
#include "usProperties_p.h"
#include "usServiceReferenceBasePrivate.h"


using namespace us;

void TestLDAPFilterMatchBundle(const std::shared_ptr<Bundle> bundle)
{
  LDAPFilter ldapMatchCase( "(bundle.testproperty=YES)" );
  LDAPFilter ldapKeyMismatchCase( "(bundle.TestProperty=YES)" );
  LDAPFilter ldapValueMismatchCase( "(bundle.testproperty=Yes)" );

  // Exact string match of both key and value
  US_TEST_CONDITION(ldapMatchCase.Match(*bundle), " Evaluating LDAP expr: " + ldapMatchCase.ToString());

  // Testing case-insensitive key (should still pass)
  US_TEST_CONDITION(ldapKeyMismatchCase.Match(*bundle), " Evaluating LDAP expr: " + ldapKeyMismatchCase.ToString());

  // Testing case-insensitive value (should fail)
  US_TEST_CONDITION(!ldapValueMismatchCase.Match(*bundle), " Evaluating LDAP expr: " + ldapValueMismatchCase.ToString());
}

void TestLDAPFilterMatchServiceReferenceBase(std::shared_ptr<Bundle> bundle)
{
  LDAPFilter ldapMatchCase( "(service.testproperty=YES)" );
  LDAPFilter ldapKeyMismatchCase( "(service.TestProperty=YES)" );
  LDAPFilter ldapValueMismatchCase( "(service.testproperty=Yes)" );

  bundle->Start();

  BundleContext* thisBundleCtx = bundle->GetBundleContext();
  ServiceReferenceU sr = thisBundleCtx->GetServiceReference("us::TestBundleLQService");

  // Make sure the obtained ServiceReferenceBase object is not null
  US_TEST_CONDITION(sr, " Checking non-empty ServiceReferenceBase object");

  // Exact string match of both key and value
  US_TEST_CONDITION(ldapMatchCase.Match(sr), " Evaluating LDAP expr: " + ldapMatchCase.ToString());

  // Testing case-insensitive key (should still pass)
  US_TEST_CONDITION(ldapKeyMismatchCase.Match(sr), " Evaluating LDAP expr: " + ldapKeyMismatchCase.ToString());

  // Testing case-insensitive value (should fail)
  US_TEST_CONDITION(!ldapValueMismatchCase.Match(sr), " Evaluating LDAP expr: " + ldapValueMismatchCase.ToString());

  bundle->Stop();
}

int usLDAPQueryTest(int /*argc*/, char* /*argv*/[])
{
  US_TEST_BEGIN("LDAPQueryTest");
    
  FrameworkFactory factory;
  std::shared_ptr<Framework> framework = factory.NewFramework();
  framework->Start();
    
  BundleContext* frameworkCtx = framework->GetBundleContext();
    
  InstallTestBundle(frameworkCtx, "TestBundleLQ");
  const auto& bundle = frameworkCtx->GetBundle("TestBundleLQ");

  US_TEST_OUTPUT(<< "Testing LDAP query of bundle properties:")
  TestLDAPFilterMatchBundle(bundle);

  US_TEST_OUTPUT(<< "Testing LDAP query of service properties:")
  TestLDAPFilterMatchServiceReferenceBase(bundle);
    
  framework->Stop();
    
  US_TEST_END()
}
