# Mockmaker

Mockmaker is a utility program for automatically generating mock classes for use with [Google Mock](https://google.github.io/googletest/gmock_cook_book.html), based on [libclang](https://clang.llvm.org/doxygen/group__CINDEX.html).  It accepts a path containing C++ source files and headers, finds all C++ classes, and generates mock classes that can then be used with [Google Test](https://google.github.io/googletest).

This project is intended as a replacement for the deprecated [`gmock_gen.py`](https://github.com/MrKepzie/google-mock/blob/master/scripts/generator/gmock_gen.py) Python script, but with a more robust and future-proof approach through the use of libclang.

## Compiling and Running

1. Set up libclang:
    - Linux/macOS/WSL:  Install LLVM via a package manager (apt, brew, etc.).  If available, install the development version of the package (`llvm-XX-dev` on Debian/Ubuntu, where XX is a release number).
        - Example:  `apt install llvm-18-dev`
    - Windows:  Download the latest build of Clang + LLVM for Windows from the LLVM Project’s [GitHub releases](https://github.com/llvm/llvm-project/releases/tag/llvmorg-18.1.8) (filename should be in the form `clang+llvm-18.1.8-x86_64-pc-windows-msvc.tar.xz`).  Extract the archive, and ensure its contents are accessible either by setting the `CMAKE_PROGRAM_PATH` environment variable to `C:\Users\...\llvm_extraction_location\bin` or adding this directory to the `PATH`.  If CMake complains about being unable to find `llvm-config`, something is still misconfigured.
    - **Note:  Mockmaker supports any major C++ compiler with support for C++17 (e.g. GCC, Clang, MSVC, etc.).  An LLVM installation is necessary simply to provide libclang’s parsing functionality, compiling the tool with clang itself is not necessary.**

2. Fetch the code:

```sh
$ git clone https://github.com/andrew-mathworks/mockmaker
```

3. Configure and compile:
    - Ensure that [CMake](https://cmake.org) is installed

```sh
$ cd mockmaker
$ mkdir build
$ cd build

# If this fails due to llvm-config missing, adjust the CMAKE_PROGRAM_PATH environment variable or system PATH
$ cmake ..
$ cmake --build .
```

4. Run:

```sh
$ ./mockmaker
Usage: ./mockmaker <template file> <source directory> <output file> [--util Header.h] [--namespace ns1] [--namespace ns2] ... -- [clang flags]
  Set MM_INFO=1 for detailed parsing information.
```

This help text means that:
- `template file`, `source directory`, and `output file` are all required arguments
- `--util` is an optional argument for specifying a utility header, or a file that contains anything needed for compilation (e.g. preprocessor macros generated at compile time)
- `--namespace` is an optional argument that can be specified zero or more times, restricting the namespace of mocked classes to only those specified (and their sub-namespaces)
- The literal string `--` is an optional argument.  If it is included, anything after it will be passed directly as arguments to clang.  This allows for setting the compiler search path, C++ standard to use, and other compilation options identically to how they would be done when compiling.

For example:

```sh
$ MM_INFO=1 ./mockmaker ../demo/Mocks.h.in ../demo/framework Mocks.h --util ../demo/util.h --namespace cppmicroservices
```

- This will generate the following debug output:

![demo.png](https://raw.githubusercontent.com/andrew-mathworks/mockmaker/main/demo/demo.png)

- It will also generate mocks like the following:

```cpp
[ License and includes omitted ]

namespace cppmicroservices
{

    class MockBadAnyCastException : public cppmicroservices::BadAnyCastException
    {
      public:
        MockBadAnyCastException(int msg) : BadAnyCastException(msg) {}
        MOCK_METHOD0(what, const char *());
    };

    class MockBundleAbstractTracked : public cppmicroservices::detail::BundleAbstractTracked
    {
      public:
        MockBundleAbstractTracked<S, T, R>(class BundleContext bc) : BundleAbstractTracked<S, T, R>(bc) {}
        MockBundleAbstractTracked<S, T, R>(class BundleContext bc) : BundleAbstractTracked<S, T, R>(bc) {}
        MOCK_METHOD1(SetInitial, void(const int &));
        MOCK_METHOD0(TrackInitial, void());
        MOCK_METHOD0(Close, void());
        MOCK_METHOD2(Track, void(S, R));
        MOCK_METHOD2(Untrack, void(S, R));

        MOCK_METHOD0(Size_unlocked, std::size_t());
        MOCK_METHOD0(IsEmpty_unlocked, bool());
        MOCK_METHOD1(GetCustomizedObject_unlocked, std::optional<T>(S));
        MOCK_METHOD1(GetTracked_unlocked, void(int &));
        MOCK_METHOD0(Modified, void());
        MOCK_METHOD0(GetTrackingCount, int());
        MOCK_METHOD1(CopyEntries_unlocked, void(int &));
        MOCK_METHOD2(CustomizerAdding, std::optional<T>(S, const R &));
        MOCK_METHOD3(CustomizerModified, void(S, const R &, const T &));
        MOCK_METHOD3(CustomizerRemoved, void(S, const R &, const T &));
        MOCK_METHOD2(TrackAdding, void(S, R));
        MOCK_METHOD2(CustomizerAddingFinal, bool(S, const std::optional<T> &));
        MOCK_METHOD1(SetInitial, void(const int &));
        MOCK_METHOD0(TrackInitial, void());
        MOCK_METHOD0(Close, void());
        MOCK_METHOD2(Track, void(S, R));
        MOCK_METHOD2(Untrack, void(S, R));

        MOCK_METHOD0(Size_unlocked, std::size_t());
        MOCK_METHOD0(IsEmpty_unlocked, bool());
        MOCK_METHOD1(GetCustomizedObject_unlocked, std::optional<T>(S));
        MOCK_METHOD1(GetTracked_unlocked, void(int &));
        MOCK_METHOD0(Modified, void());
        MOCK_METHOD0(GetTrackingCount, int());
        MOCK_METHOD1(CopyEntries_unlocked, void(int &));
        MOCK_METHOD2(CustomizerAdding, std::optional<T>(S, const R &));
        MOCK_METHOD3(CustomizerModified, void(S, const R &, const T &));
        MOCK_METHOD3(CustomizerRemoved, void(S, const R &, const T &));
        MOCK_METHOD2(TrackAdding, void(S, R));
        MOCK_METHOD2(CustomizerAddingFinal, bool(S, const std::optional<T> &));
    };
    
[ etc. ]
```

## Limitations

- Known issues:
    - Depending on the STL being used, not using the `--namespace` argument will lead to some `std` classes being mocked.  This is likely due to inline namespaces (i.e. most STL implementations using `namespace std { inline namespace _V1 { ...` or similar).
- Template parameters:
    - Currently, template parameters are incorrectly mocked due to libclang limitations and other engineering challenges.  Here is an example of the incorrect behavior (which must be rectified manually):

```cpp
// Parent class to be mocked
template<typename T, int N>
class Foo

{
  public:
    T* my_t;
    int my_n;
    Foo() : my_t(new T()), my_n(N) {}
};

// Mock class currently generated by Mockmaker (invalid C++, will not compile)
class MockFoo : public Foo
{
  public:
    MockFoo<T, N>() : Foo<T, N>() {}
};


// Desired output

template<typename T, int N>
class MockFoo : public Foo<T, N>
{
  public:
    MockFoo() : Foo<T, N>() {}
};
```

- Miscellaneous bugs and manual tweaking:
    - `const` methods:  Google Mock has a unique set of `MOCK_CONST_METHODn` macros for mocking const methods.  Because this was the minority of functions in CppMicroServices, this functionality was also not implemented.  Therefore any errors regarding loss of qualifiers must be rectified manually by inserting `CONST_`.
    - `virtual` parents:  In order for concrete classes to be mocked, their methods must be made `virtual` (and a virtual destructor must be added if one does not yet exist).  Because Mockmaker only generates code, it will not do this automatically.  Therefore any unexpected behavior where mock classes run their parents' methods are a result of this, and must be rectified manually by adding `virtual` to parent class' code.
