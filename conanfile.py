from conan import ConanFile
from conan.tools.cmake import CMake, CMakeToolchain, CMakeDeps, cmake_layout
from conan.tools.files import get, copy, rmdir, replace_in_file
from conan.tools.scm import Version
from conan.tools.microsoft import is_msvc_static_runtime
from conan.tools.build import check_min_cppstd
from conan.errors import ConanInvalidConfiguration
import os
import platform

required_conan_version = ">=1.52.0"

def IsWindows():
    return 'Windows' == platform.system()

def IsMacOS():
    return 'Darwin' == platform.system()

def IsLinux():
    return 'Linux' == platform.system()

upstreamPackageLibInfoLookup = {
    'Windows': {
        'shared': {
            'ext': 'dll',
            'src_loc': 'bin'
        },
        'static': {
            'ext': 'lib',
            'src_loc': 'lib'
        }
    },
    'Darwin': {
        'shared': {
            'ext': 'dylib',
            'src_loc': 'lib'
        },
        'static': {
            'ext': 'a',
            'src_loc': 'lib'
        }
    },
    'Linux': {
        'shared': {
            'ext': 'so',
            'src_loc': 'lib'
        },
        'static': {
            'ext': 'a',
            'src_log': 'lib'
        }
    }
}

def GetGeneralUpstreamDependencyInfo(prop, isShared):
    return upstreamPackageLibInfoLookup[platform.system()]['shared' if isShared else 'static'][prop]
                 
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

        if not IsWindows() and not IsMacOS() and not IsLinux():
            raise ConanInvalidConfiguration("Building CppMicroServices is not supported on your platform.")

    def source(self):
        get(self, **self.conan_data["sources"][self.version], strip_root=True)

    def layout(self):
        cmake_layout(self, build_folder="build")

        # Linux and macOS sets self.folders.build differently than it does on Windows 
        if IsLinux() or IsMacOS():
            self.folders.build = 'build'
        
    def configure(self):
        self.options["gtest"].shared = False
        self.options["benchmark"].shared = False
        
    def imports(self):
        lib_ext = GetGeneralUpstreamDependencyInfo('ext', self.options.shared)
        src_loc = GetGeneralUpstreamDependencyInfo('src_loc', self.options.shared)

        if IsWindows():
            dst_folder = self.folders.build + f'/bin/{str(self.settings.build_type)}'
        elif IsLinux():
            dst_folder = self.folders.build + '/bin'
        elif IsMacOS():
            dst_folder = self.folders.build + '/bin'

        self.copy(f'*.{lib_ext}*', dst=dst_folder, src=src_loc)
            
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
