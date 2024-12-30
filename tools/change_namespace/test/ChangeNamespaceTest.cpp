#include <gtest/gtest.h>
#include <fstream>
#include <cstdlib>
#include <string>
#include <filesystem>
#include "CNTestingConfig.h"

namespace fs = std::filesystem;

class NamespaceReplacerTest : public ::testing::Test {
protected:
    std::string exePath = EXE_PATH; // Path to the executable being tested
    fs::path tempDir; // Temporary directory for test files

    // Creates a temporary directory for each test
    void SetUp() override {
        tempDir = fs::temp_directory_path() / "change_namespace_test";
        fs::create_directories(tempDir);
    }

    // Cleans up the temporary directory after each test
    void TearDown() override {
        fs::remove_all(tempDir);
    }

    // Runs the namespace replacer on the given input
    // Creates temporary input file, executes the tool, and returns the result
    std::string runReplacer(const std::string& input, const std::string& newNamespace, const std::string& fileType = "cpp") {
        // Create temporary input file
        fs::path inputFile;
        if (fileType == "json") {
            inputFile = tempDir / "manifest.json";
        } else if (fileType == "config") {
            inputFile = tempDir / "config.in.h";
        } else {
            inputFile = tempDir / "input.cpp";
        }
        std::ofstream ofs(inputFile);
        ofs << input;
        ofs.close();

        // Create output directory
        fs::path outputDir = tempDir / "output";
        fs::create_directories(outputDir);

        // Run the executable
        runExecutable(tempDir.string(), newNamespace, outputDir.string());

        // Read the output file
        fs::path outputFile = outputDir / inputFile.filename();
        std::ifstream ifs(outputFile);
        std::string output((std::istreambuf_iterator<char>(ifs)),
                        (std::istreambuf_iterator<char>()));
        ifs.close();

        return output;
    }

    // Executes the namespace replacer tool with the given parameters
    // Throws an exception if the execution fails
    void runExecutable(const std::string& inputFile, const std::string& newNamespace, const std::string& outputDir) {
        std::string command = exePath + " --cppms=" + inputFile + 
                              " --namespace=" + newNamespace + 
                              " --namespace-alias " + outputDir;
        std::cout << "Command run: " << command << std::endl;
        int result = std::system(command.c_str());
        if (result != 0) {
            throw std::runtime_error("Executable failed with error code: " + std::to_string(result));
        }
    }

};

// Tests replacement of namespace declarations
TEST_F(NamespaceReplacerTest, ReplacesNamespaceDeclaration) {
    std::string input = R"(
namespace cppmicroservices {
    void someFunction() {
        // Some code here
    }
}
)";
    std::string expected = R"(
namespace newnamespace {} namespace cppmicroservices   = newnamespace; namespace newnamespace {
    void someFunction() {
        // Some code here
    }
}
)";
    
    std::string result = runReplacer(input, "newnamespace");
    EXPECT_EQ(result, expected);
}

// Tests replacement of using namespace declarations
TEST_F(NamespaceReplacerTest, ReplacesUsingNamespaceDeclaration) {
    std::string input = R"(
using namespace cppmicroservices;

void someFunction() {
    // Some code here
}
)";
    std::string expected = R"(
using namespace newnamespace;

void someFunction() {
    // Some code here
}
)";
    
    std::string result = runReplacer(input, "newnamespace");
    EXPECT_EQ(result, expected);
}

// Tests replacement of namespaces in template parameters
TEST_F(NamespaceReplacerTest, ReplacesNamespaceInTemplateParameters) {
    std::string input = R"(
std::vector<cppmicroservices::ServiceReference> serviceRefs;
std::map<cppmicroservices::Bundle, cppmicroservices::framework::BundleContext> bundleContextMap;
)";
    std::string expected = R"(
std::vector<newnamespace::ServiceReference> serviceRefs;
std::map<newnamespace::Bundle, newnamespace::framework::BundleContext> bundleContextMap;
)";
    
    std::string result = runReplacer(input, "newnamespace");
    EXPECT_EQ(result, expected);
}

// Tests replacement of namespace aliases
TEST_F(NamespaceReplacerTest, ReplacesNamespaceAlias) {
    std::string input = "namespace alias = cppmicroservices;";
    std::string expected = "namespace alias = newnamespace;";
    std::string result = runReplacer(input, "newnamespace");
    EXPECT_EQ(result, expected);
}

// Tests replacement of namespaces in #define macros
TEST_F(NamespaceReplacerTest, ReplacesDefine) {
    std::string input = "#define MACRO cppmicroservices";
    std::string expected = "#define MACRO newnamespace";
    std::string result = runReplacer(input, "newnamespace");
    EXPECT_EQ(result, expected);
}

// Tests replacement of namespaces enclosed in parentheses
TEST_F(NamespaceReplacerTest, ReplacesParenthesizedNamespace) {
    std::string input = "void func(cppmicroservices) {}";
    std::string expected = "void func(newnamespace) {}";
    std::string result = runReplacer(input, "newnamespace");
    EXPECT_EQ(result, expected);
}

// Tests replacement of namespace prefixes
TEST_F(NamespaceReplacerTest, ReplacesNamespacePrefix) {
    std::string input = "cppmicroservices::Bundle bundle;";
    std::string expected = "newnamespace::Bundle bundle;";
    std::string result = runReplacer(input, "newnamespace");
    EXPECT_EQ(result, expected);
}

// Tests error handling for invalid inputs
TEST_F(NamespaceReplacerTest, ThrowsExceptionOnInvalidInput) {
    // Test missing --cppms flag
    EXPECT_THROW({
        runExecutable("", "newns", tempDir.string());
    }, std::runtime_error);

    // Test missing --namespace flag
    EXPECT_THROW({
        runExecutable(tempDir.string(), "", tempDir.string());
    }, std::runtime_error);

    // Test missing output directory
    EXPECT_THROW({
        runExecutable(tempDir.string(), "newns", "");
    }, std::runtime_error);

    // Test non-existent input directory
    EXPECT_THROW({
        runExecutable("/non/existent/path", "newns", tempDir.string());
    }, std::runtime_error);

    // Test non-existent output directory
    EXPECT_THROW({
        runExecutable(tempDir.string(), "newns", "/non/existent/path");
    }, std::runtime_error);
}

// Tests replacement of namespaces in manifest.json files
TEST_F(NamespaceReplacerTest, ReplacesNamespaceInManifestJson) {
    std::string input = R"(
{
  "bundle.symbolic_name" : "cppmicroservices::example",
  "bundle.activator" : true,
  "bundle.name" : "Example Plugin",
  "bundle.description" : "A plugin using cppmicroservices framework"
}
)";
    std::string expected = R"(
{
  "bundle.symbolic_name" : "newnamespace::example",
  "bundle.activator" : true,
  "bundle.name" : "Example Plugin",
  "bundle.description" : "A plugin using cppmicroservices framework"
}
)";
    
    std::string result = runReplacer(input, "newnamespace","json");
    EXPECT_EQ(result, expected);
}

// Tests replacement of namespaces in config.h.in files
TEST_F(NamespaceReplacerTest, ReplacesNamespaceInConfigHIn) {
    std::string input = R"(
namespace cppmicroservices {
  // Some configuration code here
}
)";
    std::string expected = R"(
namespace newnamespace {} namespace cppmicroservices   = newnamespace; namespace newnamespace {
  // Some configuration code here
}
)";
    
    std::string result = runReplacer(input, "newnamespace", "config");
    EXPECT_EQ(result, expected);
}