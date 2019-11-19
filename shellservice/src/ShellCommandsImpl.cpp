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

#include <memory>

#include "cppmicroservices/Bundle.h"
#include "cppmicroservices/BundleActivator.h"
#include "cppmicroservices/BundleContext.h"

#include "cppmicroservices/shellservice/ShellCommandsImpl.h"

#include <iostream>
#include <iomanip> // std::setw

namespace cppmicroservices {
    
    // us-ls
    void ShellCommandLs::doCommand(BundleContext bc, std::vector<std::string>) {
        std::cout << std::left
                  << std::setw(4)  << "Id"
                  << std::setw(26) << "Symbolic Name"
                  << std::setw(10) << "Version"
                  << std::setw(12) << "State"
                  << std::endl;
        std::cout << std::string(52, '-') << std::endl;
        
        std::vector<cppmicroservices::Bundle> bundles = bc.GetBundles();
        
        std::sort(bundles.begin(), bundles.end(), [](const cppmicroservices::Bundle& a, const cppmicroservices::Bundle& b)->bool {
            return (a.GetBundleId() < b.GetBundleId());
        });
        
        for(auto bundle : bundles)
        {
            std::cout << std::left
                      << std::setw(4)  << bundle.GetBundleId()
                      << std::setw(26) << bundle.GetSymbolicName()
                      << std::setw(10) << bundle.GetVersion()
                      << std::setw(12) << bundle.GetState()
                      << std::endl;
        }
    }
    
    void ShellCommandLs::help() {
        std::cout << std::left
                  << std::setw(15) << "us-ls"
                  << std::setw(100) << "list bundle information"
                  << std::endl << std::endl;
    }
    
    // us-install
    void ShellCommandInstall::doCommand(BundleContext bc, const std::vector<std::string> arguments) {
        for (auto path : arguments) {
            bc.InstallBundles(path);
        }
    }
    
    void ShellCommandInstall::help() {
        std::cout << std::left
        << std::setw(15) << "us-install"
        << std::setw(100) << "installs bundle(s) located at given path(s)"
        << std::endl
        << std::setw(15) << "" << "us-install <path1> <path2> ..."
        << std::endl << std::endl;
    }
    
    // us-uninstall
    void ShellCommandUninstall::doCommand(BundleContext bc, const std::vector<std::string> arguments) {
        for (auto bundleId : arguments) {
            auto bundle = bc.GetBundle(std::stoi(bundleId));
            bundle.Uninstall();
        }
    }
    void ShellCommandUninstall::help() {
        std::cout << std::left
        << std::setw(15) << "us-uninstall"
        << std::setw(100) << "uninstalls bundle(s) by bundle ID(s)"
        << std::endl
        << std::setw(15) << "" << "us-uninstall <id1> <id2> ..."
        << std::endl << std::endl;
    }
    
    // us-start
    void ShellCommandStart::doCommand(BundleContext bc, const std::vector<std::string> arguments) {
        for (auto bundleId : arguments) {
            auto bundle = bc.GetBundle(std::stoi(bundleId));
            bundle.Start();
        }
    }
    void ShellCommandStart::help() {
        std::cout << std::left
        << std::setw(15) << "us-start"
        << std::setw(100) << "starts bundle(s) by bundle ID(s)"
        << std::endl
        << std::setw(15) << "" << "us-start <id1> <id2> ..."
        << std::endl << std::endl;    }
    
    // us-stop
    void ShellCommandStop::doCommand(BundleContext bc, const std::vector<std::string> arguments) {
        for (auto bundleId : arguments) {
            auto bundle = bc.GetBundle(std::stoi(bundleId));
            bundle.Stop();
        }
    }
    
    void ShellCommandStop::help() {
        std::cout << std::left
        << std::setw(15) << "us-stop"
        << std::setw(100) << "stops bundle(s) by bundle ID(s)"
        << std::endl
        << std::setw(15) << "" << "us-start <id1> <id2> ..."
        << std::endl << std::endl;
    }
    
}
