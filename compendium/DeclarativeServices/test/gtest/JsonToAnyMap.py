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
import json
import types
from collections import OrderedDict

def lookahead(iterable):
    """Pass through all values from the given iterable, augmented by the
    information if there are more values to come after the current one
    (True), or if it is the last value (False).
    """
    it = iter(iterable)
    last = next(it)
    for val in it:
        yield last, True
        last = val
    yield last, False

def get_anymap_declaration(json_str, key=None, indent=0):
    """Given a json_str representing a json string, with optionally a name "key"
    at the top level and indentation specified by "indent",
    Return a C++ AnyMap representation of the json in string format.
    """
    return '\n'.join(generate_indent(get_json_string(json_str, key), indent))

def get_json_string(json_str, key=None):
    """Given a json_str representing a json string, with optionally a name "key"
    at the top level, Return a C++ AnyMap representation of the json in string format.
    """
    scrmap = json.loads(json_str, object_pairs_hook=OrderedDict)
    if key:
        scrmap = scrmap[key]
    return ''.join(generate_anymap(scrmap, 0))

def generate_indent(json_str, num=0):
    """Given a json_str representing a json string, with optionally
    the indentation specified in "num", generate the indented string.
    """
    for line in json_str.split('\n'):
        yield ' '*num + line

def generate_anymap(node, level):
    """Given a json node object specified by node and the level of the json tree
    hierarchy, parse the json and generate the C++ AnyMap representation in string
    format.
    """
    if type(node) == OrderedDict:
        yield 'std::map<std::string, Any> ({\n'
        for dictitem, has_more in lookahead(node.iteritems()):
            key, val = dictitem
            yield '  ' * (level + 1)
            yield '{{ "{0}", Any('.format(key)
            for line in generate_anymap(val, level + 1):
                yield line
            if has_more:
                yield ')},'
            else:
                yield ')}'
            yield '\n'
        yield '  ' * level
        yield '})'
    elif type(node) == types.ListType:
        yield 'std::vector<Any> {\n'
        for elem, has_more in lookahead(node):
            yield '  ' * (level + 1)
            for line in generate_anymap(elem, level + 1):
                yield line
            if has_more:
                yield ','
            yield '\n'
        yield '  ' * level
        yield '}'
    elif type(node) == types.IntType:
        yield str(node)
    elif type(node) == types.BooleanType:
        yield 'true' if node else 'false'
    elif type(node) == types.StringType or type(node) == types.UnicodeType:
        yield 'std::string("{0}")'.format(node)

if __name__ == "__main__":
    test_str = """
    {
    "scr" : { "version" : 1,
              "components": [{
                       "implementation-class": "DSSpellCheck::SpellCheckImpl",
                       "activate" : "Activate",
                       "deactivate" : "Deactivate",
                       "inject-references" : true,
                       "service": {
                       "scope": "SINGLETON",
                       "interfaces": ["SpellCheck::ISpellCheckService", "Foo::Bar"]
                       },
                       "references": [{
                         "name": "dictionary",
                         "interface": "DictionaryService::IDictionaryService",
                         "policy": "dynamic"
                       }]
                       }]
            }
    }
    """
    print get_anymap_declaration(test_str, 'scr', 2)
