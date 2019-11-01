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

#ifndef TEST_HTTP_REQUEST_H
#define TEST_HTTP_REQUEST_H

// URL encoding/decoding
#include "civetweb/civetweb.h"
#include <string>

/*!
 * This namespace contains various HTTP queries used in tests below
 */
namespace Queries {

// cta
static const std::string POST_FORM_DATA =
  "POST /test HTTP/1.1\r\n"
  "cache-control: no-cache\r\n"
  "Postman-Token: 96c8b96b-8fdf-4113-bfd7-b3a392af2e3a\r\n"
  "User-Agent: PostmanRuntime/7.1.5\r\n"
  "Accept: */*\r\n"
  "Host: localhost:8080\r\n"
  "accept-encoding: gzip, deflate\r\n"
  "content-type: multipart/form-data; "
  "boundary=--------------------------664942711418770126299479\r\n"
  "content-length: 276\r\n"
  "Connection: keep-alive\r\n"
  "\r\n"
  "----------------------------664942711418770126299479\r\n"
  "Content-Disposition: form-data; name=\"A\"\r\n"
  "\r\n"
  "the A value\r\n"
  "----------------------------664942711418770126299479\r\n"
  "Content-Disposition: form-data; name=\"B\"\r\n"
  "\r\n"
  "value of B\r\n"
  "----------------------------664942711418770126299479--\r\n";

/*!
 * Forged query which POST one file using "multipart/form-data"
 *
 * Method : POST
 * Target URI: http://127.0.0.1:8080/test
 */
static const std::string POST_MULTIPART_ONE_FILE_REQUEST =
  "POST /test HTTP/1.1\r\n"
  "Host: 127.0.0.1:8080\r\n"
  "User-Agent: test\r\n"
  "Accept: */*\r\n"
  "Content-Length: 214\r\n"
  "Content-Type: multipart/form-data; "
  "boundary=----WebKitFormBoundary7MA4YWxkTrZu0gW\r\n"
  "\r\n"
  "------WebKitFormBoundary7MA4YWxkTrZu0gW\r\n"
  "Content-Disposition: form-data; name=\"sampleFileFormName\"; "
  "filename=\"sampleFile.txt\"\r\n"
  "Content-Type: text/plain\r\n"
  "\r\n"
  "hello world\r\n"
  "\r\n"
  "------WebKitFormBoundary7MA4YWxkTrZu0gW--\r\n"
  "\r\n";

static const std::string POST_WITH_BODY =
  "POST /test HTTP/1.1\r\n"
  "Content-Type: text/plain\r\n"
  "cache-control: no-cache\r\n"
  "Postman-Token: 66deef3f-6c12-4e86-947d-4bc6565a5bc7\r\n"
  "User-Agent: test\r\n"
  "Accept: */*\r\n"
  "Host: 127.0.0.1:8080\r\n"
  "accept-encoding: gzip, deflate\r\n"
  "Content-length: 12\r\n"
  "Connection: keep-alive\r\n"
  "\r\n"
  "testpostbody\r\n\r\n";

/*!
 * Forged query which GET resource with two plain parameters
 *
 * Method : GET
 * Target URI: http://127.0.0.1:8080/test
 */
static const std::string GET_PARAMETERS_SIMPLE =
  "GET /test?a=value%20of%20a&b=value%20of%20b HTTP/1.1\r\n"
  "Host: 127.0.0.1:8080\r\n"
  "User-Agent: test\r\n"
  "Accept: */*\r\n"
  "\r\n";

/*!
 * Forged query which GET resource with two plain parameters
 *
 * Method : GET
 * Target URI: http://127.0.0.1:8080/test
 */
static const std::string GET_PARAMETERS_WITH_PLUS_AS_SPACE =
  "GET /test?a=value+of+a&b=value+of+b HTTP/1.1\r\n"
  "Host: 127.0.0.1:8080\r\n"
  "User-Agent: test\r\n"
  "Accept: */*\r\n"
  "\r\n";

static const std::string GET_MALFORMED_REQUEST =
  "GET /test?a=value%20of%20a&b=value%20of%20b HTTP/1.0"
  "Host: localhost:8080\r\n"
  "\r\n";

static const std::string GET_MISSING_HOST_HEADER_REQUEST =
  "GET /test?a=value%20of%20a&b=value%20of%20b HTTP/1.0\n\n" // buggy \n\n here
  "Host: localhost:8080\r\n"
  "\r\n";

/*!
* Forged query which PUT resource with two plain parameters
*
* Method : PUT
* Target URI: http://127.0.0.1:8080/test
*/
static const std::string PUT_PARAMETERS_SIMPLE =
  "PUT /test?a=value%20of%20a&b=value%20of%20b HTTP/1.1\r\n"
  "Host: 127.0.0.1:8080\r\n"
  "User-Agent: test\r\n"
  "Accept: */*\r\n"
  "\r\n";

/*!
* Forged query which PUT resource with two plain parameters
*
* Method : PUT
* Target URI: http://127.0.0.1:8080/test
*/
static const std::string PUT_PARAMETERS_WITH_PLUS_AS_SPACE =
  "PUT /test?a=value+of+a&b=value+of+b HTTP/1.1\r\n"
  "Host: 127.0.0.1:8080\r\n"
  "User-Agent: test\r\n"
  "Accept: */*\r\n"
  "\r\n";

static const std::string PUT_MALFORMED_REQUEST =
  "PUT /test?a=value%20of%20a&b=value%20of%20b HTTP/1.0"
  "Host: localhost:8080\r\n"
  "\r\n";

static const std::string PUT_MISSING_HOST_HEADER_REQUEST =
  "PUT /test?a=value%20of%20a&b=value%20of%20b HTTP/1.0\n\n"
  "Host: localhost:8080\r\n"
  "\r\n";

static const std::string PUT_WITH_BODY =
  "PUT /test HTTP/1.1\r\n"
  "Content-Type: text/plain\r\n"
  "cache-control: no-cache\r\n"
  "Postman-Token: 66deef3f-6c12-4e86-947d-4bc6565a5bc7\r\n"
  "User-Agent: test\r\n"
  "Accept: */*\r\n"
  "Host: 127.0.0.1:8080\r\n"
  "accept-encoding: gzip, deflate\r\n"
  "Content-length: 11\r\n"
  "Connection: keep-alive\r\n"
  "\r\n"
  "testputbody\r\n\r\n";

/*!
* Forged query which DELETE resource with two plain parameters
*
* Method : DELETE
* Target URI: http://127.0.0.1:8080/test
*/
static const std::string DELETE_PARAMETERS_SIMPLE =
  "DELETE /test?a=value%20of%20a&b=value%20of%20b HTTP/1.1\r\n"
  "Host: 127.0.0.1:8080\r\n"
  "User-Agent: test\r\n"
  "Accept: */*\r\n"
  "\r\n";

/*!
* Forged query which PUT resource with two plain parameters
*
* Method : PUT
* Target URI: http://127.0.0.1:8080/test
*/
static const std::string DELETE_PARAMETERS_WITH_PLUS_AS_SPACE =
  "DELETE /test?a=value+of+a&b=value+of+b HTTP/1.1\r\n"
  "Host: 127.0.0.1:8080\r\n"
  "User-Agent: test\r\n"
  "Accept: */*\r\n"
  "\r\n";

static const std::string DELETE_MALFORMED_REQUEST =
  "DELETE /test?a=value%20of%20a&b=value%20of%20b HTTP/1.0"
  "Host: localhost:8080\r\n"
  "\r\n";

static const std::string DELETE_MISSING_HOST_HEADER_RUQUEST =
  "DELETE /test?a=value%20of%20a&b=value%20of%20b HTTP/1.0\n\n"
  "Host: localhost:8080\r\n"
  "\r\n";
}

#endif
