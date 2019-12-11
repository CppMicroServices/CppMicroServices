"""
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
 """

import sys
from collections import OrderedDict
from JsonToAnyMap import get_anymap_declaration

def get_json_dict(f):
    """Parse each line of TestCodegenerator.cpp and return a dict
    with keys = name of the variable containing the string representation
    of the json in TestCodegenerator.cpp
    and values = the json in string format.
    """
    is_line_json = False
    jsondict = OrderedDict()
    for line in open(f):
        if line.startswith('  const std::string'):
            is_line_json = True
            cppvarname = line.strip().split()[2]
            jsondict[cppvarname] = []
            continue
        if line.startswith('  )manifest'):
            is_line_json = False
        if is_line_json:
            jsondict[cppvarname].append(line)
    for key, val in jsondict.iteritems():
        jsondict[key] = ''.join(val)
    return jsondict

if __name__ == "__main__":
    # Invoke like so:
    # python TestCodegenToAnyMap <path_to_codegen_cpp> <output_hpp_file>
    # First argument is path of TestCodegenerator.cpp
    # Second argument is the output file to write to.
    testcodegen_path = sys.argv[1]
    jsondict = get_json_dict(testcodegen_path)

    cpp_path = open(sys.argv[2], 'w')
    cpp_path.write('#include <map>\n')
    cpp_path.write('#include <string>\n')
    cpp_path.write('#include <vector>\n')
    cpp_path.write('#include "cppmicroservices/Any.h"\n\n')
    cpp_path.write('using cppmicroservices::Any;\n\n')
    for key, val in jsondict.iteritems():
        cpp_path.write('/*\n')
        cpp_path.write(val)
        cpp_path.write('*/\n')
        cpp_path.write('const std::map<std::string, Any> ' + key + ' =\n')
        cpp_path.write(get_anymap_declaration(val, "scr", 2))
        cpp_path.write(';\n\n')
