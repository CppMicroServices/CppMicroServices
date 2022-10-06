from conan import ConanFile
from conan.tools.cmake import CMake, CMakeToolchain, CMakeDeps, cmake_layout
from conan.tools.files import get, copy, rmdir, replace_in_file
from conan.tools.scm import Version
from conan.tools.microsoft import is_msvc_static_runtime
from conan.tools.build import check_min_cppstd
from conan.errors import ConanInvalidConfiguration
import os

required_conan_version = ">=1.52.0"

def IsMSVC(conanfile):
    return conanfile.info.settings.compiler == "Visual Studio" or conanfile.info.settings.compiler == "msvc"

class CppMicroServicesConan(ConanFile):
    name = "cppmicroservices"
    package_type = "library"
    version = "3.7.3"
    url = "https://github.com/conan-io/conan-center-index"
    homepage = "https://github.com/CppMicroServices/CppMicroServices"
    description = "An OSGi-like C++ dynamic module system and service registry"
    topics = ("microservices", "c++", "system-architecture")
    license = "Apache"
    
    requires = "gtest/cci.20210126", "benchmark/1.7.0", "spdlog/1.10.0"
    generators = "CMakeDeps", "CMakeToolchain"

    settings = "os", "arch", "compiler", "build_type"
    options = {
        "shared": [True, False],
        "testing": [True, False],
        "threading": [True, False]
    }
    default_options = {
        "shared": True,
        "testing": True,
        "threading": True
    }

    def validate(self):
        if self.info.settings.compiler.cppstd:
            check_min_cppstd(self, 17)

    def source(self):
        get(self, **self.conan_data["sources"][self.version], strip_root=True)

    def layout(self):
        cmake_layout(self, build_folder="build")
        
    def configure(self):
        self.options["gtest"].shared = self.options.shared
        self.options["benchmark"].shared = self.options.shared
        
    def imports(self):
        if IsMSVC(self):
            dst_folder = self.folders.build + f'/bin/{str(self.settings.build_type)}'
        else:
            dst_folder = self.folders.build + '/bin'

        print(dst_folder)
            
        self.copy("*.dll", dst=dst_folder, src="bin")
        self.copy("*.dylib", dst=dst_folder, src="lib")
        self.copy("*.so", dst=dst_folder, src="lib")

    def generate(self):
        tc = CMakeToolchain(self)
        tc.variables["BUILD_SHARED_LIBS"] = self.options.shared
        tc.variables["US_BUILD_TESTING"] = self.options.testing
        tc.variables["US_ENABLE_THREADING_SUPPORT"] = self.options.threading
        tc.generate()
        deps = CMakeDeps(self)
        deps.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install()
