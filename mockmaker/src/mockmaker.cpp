/*****************************************************************************
 *                                                                           *
 * mockmaker.cpp:                                                            *
 *                                                                           *
 * Program entry point, handles command-line arguments and composes          *
 * individual pieces together.                                               *
 *                                                                           *
 *****************************************************************************/

#include <filesystem>
#include <iostream>
#include <fstream>
#include <cstring>
#include <cstdio>
#include <vector>
#include <set>
#include <map>
#include <clang-c/Index.h>

#include "Util.hpp"
#include "Mocks.hpp"
#include "Parse.hpp"

#define ENTRYPOINT "MockmakerEntrypoint.cpp"

using namespace std;
namespace fs = std::filesystem;

static vector<string> excluded_namespaces = {
  "std", "__gnu_cxx", "__cxx11"
};

/**
 * Pretty-print a function declaration, including its
 * return type and arguments.
 */
void print_function_decl(CXCursor cursor, string name_str, int code = 0) {
  auto cursor_type = clang_getCursorType(cursor);
  auto result_type = clang_getResultType(cursor_type);
  auto type_spelling = clang_getTypeSpelling(result_type);
  string type = cxstring_to_string(type_spelling);

  cout
    << "      \x1b[3"
    << (code == 0 ? "5mFunc" : (code == 1 ? "6mCtor" : "1mDtor"))
    << "\x1b[0m: \x1b[34m"
    << type
    << " \x1b[32m"
    << name_str
    << "\x1b[0m(\x1b[33m";
  int num_args = clang_Cursor_getNumArguments(cursor);
  for (int i = 0; i < num_args; ++i) {
      auto arg_cursor = clang_Cursor_getArgument(cursor, i);
      string arg_name = cxstring_to_string(clang_getCursorSpelling(arg_cursor));
      if (arg_name.empty()) arg_name = "(noname)";

      auto cursor_type = clang_getCursorType(cursor);
      auto arg_type = clang_getArgType(cursor_type, i);
      auto type_spelling = clang_getTypeSpelling(arg_type);
      string arg_data_type = cxstring_to_string(type_spelling);

      cout
        << arg_data_type
        << " "
        << arg_name
        << (i == num_args - 1 ? "" : "\x1b[0m, \x1b[33m");
  }
  cout << "\x1b[0m)" << endl;
}

/**
 * Visit an individual entity within the parse tree (e.g. a class declaration,
 * function declaration, etc.) and descend further into the AST if able.
 */
void visit_entity(CXCursor cursor, CXClientData raw_parse_state = nullptr) {
  ParseState *parse_state = static_cast<ParseState*>(raw_parse_state);
  ParseState next_parse_state =
    (parse_state == nullptr ? ParseState() : *parse_state);
  if (next_parse_state.continue_descending) {
    string name_str = cxstring_to_string(clang_getCursorSpelling(cursor));
    if (name_str.empty()) return;

    vector<string> template_params;
    CXCursorKind kind = clang_getCursorKind(cursor);
    switch (kind) {
      // Translation unit: Top-level entity for entry file
      case CXCursor_TranslationUnit: {
        if (SHOW_PRETTY_INFO) {
          cout << "\x1b[32mFile\x1b[0m: " << name_str << endl;
        }
        break;
      }
      // Namespace: Keep track to reconstruct later
      case CXCursor_Namespace: {
        // Do not mock STL or similar
        //   XXX: Nested namespaces are sometimes included (STL header location issue?)
        if (find(excluded_namespaces.begin(),
                 excluded_namespaces.end(),
                 name_str)
            != excluded_namespaces.end()
        ) {
          next_parse_state.continue_descending = false;
          return;
        }
        // If a list of namespaces was explicitly given, check for inclusion
        auto begin = next_parse_state.valid_namespaces->begin();
        auto end = next_parse_state.valid_namespaces->end();
        if (!next_parse_state.valid_namespaces->empty() &&
            !next_parse_state.has_namespace &&
            find(begin, end, name_str) == end) {
          next_parse_state.continue_descending = false;
          return;
        }
        if (SHOW_PRETTY_INFO) {
          cout << "  \x1b[33mNamespace\x1b[0m: " << name_str << endl;
        }

        next_parse_state.continue_descending = true;
        next_parse_state.has_namespace = true;
        next_parse_state.namespaces.push_back(name_str);
        break;
      }
      // Class declaration: Store in parse state to build mock class
      case CXCursor_ClassTemplate:
      case CXCursor_ClassTemplatePartialSpecialization:
      case CXCursor_StructDecl:
      case CXCursor_ClassDecl: {
        // TODO: Template parameters
        if (!next_parse_state.has_namespace) break;
        if (SHOW_PRETTY_INFO) {
          cout << "    \x1b[34mClass\x1b[0m: " << name_str << endl;
        }

        next_parse_state.class_name = string(name_str);

        // Ignore unnamed structs because they are unmockable
        if (name_str.find("unnamed") != string::npos) break;

        // Get current filename and add it to the include set
        CXSourceLocation loc = clang_getCursorLocation(cursor);
        CXFile file;
        unsigned int dummy;
        clang_getSpellingLocation(loc, &file, &dummy, &dummy, &dummy);
        string filename = cxstring_to_string(clang_getFileName(file));
        next_parse_state.out->includes.insert(filename);

        // Add a new mock class
        if (next_parse_state.out->mocks.count(name_str) == 0) {
          next_parse_state.out->mocks[name_str] = MockedClass(
            name_str,
            next_parse_state.namespaces,
            template_params
          );
        } else if (SHOW_PRETTY_INFO) {
          cout
            << INFO << "Class " << name_str << " has already been defined."
            << endl;
        }
        break;
      }
      // Method/function: Add mock method for each (or copy if constructor)
      case CXCursor_Constructor:
      case CXCursor_CXXMethod:
      case CXCursor_FunctionTemplate:
      case CXCursor_FunctionDecl: {
        if (kind == CXCursor_Constructor ||
            // Constructors with templates are not identified as constructors
            (kind == CXCursor_FunctionTemplate &&
             name_str == next_parse_state.class_name)) {
          if (!next_parse_state.has_namespace) break;
          if (SHOW_PRETTY_INFO) {
            print_function_decl(cursor, name_str, 1);
          }

          MockedClass *mocked = &next_parse_state.out->mocks[
            next_parse_state.class_name
          ];
          mocked->add_function(cursor, true);
          next_parse_state.out->n_methods++;
        } else {
          // TODO: Template parameters, non-class functions
          if (!next_parse_state.has_namespace ||
               next_parse_state.class_name.empty() ||
              !next_parse_state.out->mocks.count(next_parse_state.class_name)) {
            break;
          }
          if (SHOW_PRETTY_INFO) {
            print_function_decl(cursor, name_str);
          }

          MockedClass *mocked = &next_parse_state.out->mocks[
            next_parse_state.class_name
          ];
          mocked->add_function(cursor);
          next_parse_state.out->n_methods++;
        }
        break;
      }
      // Otherwise: Ignore
      default: break;
    }
  }

  // Continue generating AST
  clang_visitChildren(
    cursor,
    [](CXCursor c, CXCursor /*parent*/, CXClientData parse_state) {
      visit_entity(c, parse_state);

      // Always recurse into parse tree
      return CXChildVisit_Recurse;
    },
    &next_parse_state
  );
}

inline bool ends_with(std::string const &value, std::string const &ending) {
  if (ending.size() > value.size()) return false;
  return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
}

/**
 * Instantiate libclang with the given entry point function,
 * including necessary command line arguments to ensure clang
 * reads input as C++ with appropriate header paths.
 */
ParseResult make_mock(int argc, char **argv) {
  vector<const char*> args{
    // Force language to be C++ (even if file extension is .h)
    "-x",
    "c++",
    // Include search paths for headers
    "-I",
    argv[2]
  };

  string util;
  vector<string> namespaces;
  int mode = 0;

  for (int i = 4; i < argc; i++) {
    string arg(argv[i]);
    switch (mode) {
      case 0:
        if (arg == "--util") {
          mode = 1;
        } else if (arg == "--namespace") {
          mode = 2;
        } else if (arg == "--") {
          mode = 3;
        } else {
          cout << ERR << "Unrecognized or misplaced option \"" << arg << "\", ignoring." << endl;
        }
        break;
      case 1:
        util = arg;
        mode = 0;
        break;
      case 2:
        namespaces.push_back(arg);
        mode = 0;
        break;
      case 3:
        args.push_back(argv[i]);
        break;
      default: break;
    }
  }

  cout
    << INFO
    << "Spawning clang with args: \""
    << vec_join<const char*>(args, " ")
    << "\""
    << endl;

  fs::path path = fs::path(argv[2]);
  ofstream ofs(ENTRYPOINT, ios::out);

  // Utility header
  if (!util.empty()) {
    ofs << "#include \"" << util << "\"" << endl;
  }

  for (const auto &entry : fs::recursive_directory_iterator(path)) {
    if (fs::is_regular_file(entry.status())) {
      string filename = entry.path().string();
      if (ends_with(filename, ".h") || ends_with(filename, ".hpp")) {
        ofs << "#include \"" << filename << "\"" << endl;
      }
    }
  }
  ofs << "int main() { return 0; }" << endl;
  ofs.close();

  CXIndex index = clang_createIndex(0, 0);
  CXTranslationUnit unit;
  CXErrorCode err = clang_parseTranslationUnit2(
    index,
    ENTRYPOINT,
    args.data(), args.size(),
    nullptr, 0,
    CXTranslationUnit_SkipFunctionBodies,
    &unit
  );
  if (err != 0 || unit == nullptr) {
    cerr
      << ERR << "Failed to parse translation unit \"" << path
      << "\" (error code " << err << ")" << endl;
    error_code ec;
    fs::remove(ENTRYPOINT, ec);

    return ParseResult(true);
  }

  ParseResult out;
  ParseState initial_state(&out, &namespaces);
  visit_entity(clang_getTranslationUnitCursor(unit), &initial_state);

  clang_disposeTranslationUnit(unit);
  clang_disposeIndex(index);

  error_code ec;
  fs::remove(ENTRYPOINT, ec);

  return out;
}

/**
 * Program entry point: Check arguments, invoke parser, print output to stdout.
 */
int main(int argc, char **argv) {
  if (argc < 4 || strcmp(argv[3], "--") == 0) {
    cerr << "Usage: " << argv[0]
      << " <template file> <source directory> <output file> [--util Header.h] [--namespace ns1] [--namespace ns2] ... -- [clang flags]" << endl;
    cerr << "  Set "
      << INFO_ENV_VAR << "=1 for detailed parsing information." << endl;
    return 0;
  }

  ifstream templ(argv[1]);
  if (!templ.is_open()) {
    cerr
      << ERR << "Failed to open template file \"" << argv[1]
      << "\" for reading." << endl;
    return 1;
  }

  ParseResult pr = make_mock(argc, argv);
  if (pr.failed) return 2;

  cout
    << INFO
    << "Parsing completed successfully, generating "
    << argv[3]
    << "..."
    << endl;

  TeeDevice out = TeeDevice(argv[3]);

  bool hit_mocks = false;
  string line;
  while (getline(templ, line)) {
    if (line.find("{{includes}}") != string::npos) {
      for (const string &s : pr.includes) {
        string tmp = s;
        tmp.erase(0, strlen(argv[2]) + 3);
        out << "#include \"" << tmp << "\"" << endl;
      }
    } else if (line.find("{{mocks}}") != string::npos) {
      hit_mocks = true;
      for (auto const &[key, val] : pr.mocks) {
        out << val << endl;
      }
    } else {
      out << line << endl;
    }
  }
  templ.close();
  out.close();

  cout
    << INFO
    << "Successfully generated "
    << argv[3]
    << ": "
    << YELLOW
    << pr.mocks.size()
    << RESET
    << " classes, "
    << YELLOW
    << pr.n_methods
    << RESET
    << " methods."
    << endl;

  if (!hit_mocks) {
    cout
      << WARN
      << "Template file did not contain magic string \"{{mocks}}\", "
      << "so no mock classes were created."
      << endl;
  }

  return 0;
}
