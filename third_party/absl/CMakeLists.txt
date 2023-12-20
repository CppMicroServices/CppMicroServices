cmake_minimum_required(VERSION 3.2)

project(absl CXX)

set(_ABSL_SRCS
  "absl/base/log_severity.cc"
  "absl/base/internal/raw_logging.cc"
  "absl/base/internal/throw_delegate.cc"

  "absl/numeric/int128.cc"

  "absl/strings/ascii.cc"
  "absl/strings/charconv.cc"
  "absl/strings/match.cc"
  "absl/strings/numbers.cc"
  "absl/strings/str_cat.cc"
  "absl/strings/string_view.cc"
  "absl/strings/internal/charconv_bigint.cc"
  "absl/strings/internal/charconv_parse.cc"

  "absl/strings/internal/memutil.cc"

  "absl/types/bad_optional_access.cc"
)

set(_ABSL_HDRS
  "absl/base/attributes.h"
  "absl/base/casts.h"
  "absl/base/config.h"
  "absl/base/log_severity.h"
  "absl/base/macros.h"
  "absl/base/optimization.h"
  "absl/base/options.h"
  "absl/base/policy_checks.h"
  "absl/base/port.h"
  "absl/base/internal/atomic_hook.h"
  "absl/base/internal/endian.h"
  "absl/base/internal/errno_saver.h"
  "absl/base/internal/identity.h"
  "absl/base/internal/inline_variable.h"
  "absl/base/internal/invoke.h"
  "absl/base/internal/raw_logging.h"
  "absl/base/internal/throw_delegate.h"
  "absl/base/internal/unaligned_access.h"

  "absl/meta/type_traits.h"

  "absl/memory/memory.h"

  "absl/numeric/bits.h"
  "absl/numeric/int128.h"
  "absl/numeric/internal/bits.h"

  "absl/strings/ascii.h"
  "absl/strings/charconv.h"
  "absl/strings/escaping.h"
  "absl/strings/match.h"
  "absl/strings/numbers.h"
  "absl/strings/str_cat.h"
  "absl/strings/str_join.h"
  "absl/strings/string_view.h"
  "absl/strings/internal/charconv_bigint.h"
  "absl/strings/internal/charconv_parse.h"
  "absl/strings/internal/has_absl_stringify.h"
  "absl/strings/internal/memutil.h"
  "absl/strings/internal/ostringstream.h"
  "absl/strings/internal/resize_uninitialized.h"
  "absl/strings/internal/str_join_internal.h"
  "absl/strings/internal/stringify_sink.h"

  "absl/types/bad_optional_access.h"
  "absl/types/internal/optional.h"
  "absl/types/optional.h"

  "absl/utility/utility.h"
)

add_library(absl OBJECT ${_ABSL_SRCS} ${_ABSL_HDRS})

set_property(TARGET absl PROPERTY POSITION_INDEPENDENT_CODE 1)

target_include_directories(absl
  PUBLIC
    .
  PRIVATE
    .
)

if (MSVC)
  set_target_properties(absl PROPERTIES COMPILE_OPTIONS "/WX-")
endif()
