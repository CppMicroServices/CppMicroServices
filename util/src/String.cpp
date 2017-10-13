#include <cppmicroservices/GlobalConfig.h>
#include "cppmicroservices/util/String.h"
#include "cppmicroservices/util/Error.h"
#include <exception>
#include <memory>


#ifdef US_PLATFORM_WINDOWS
#include <windows.h>

namespace cppmicroservices {
  namespace util {
    //-------------------------------------------------------------------
    // Unicode Utility functions
    //-------------------------------------------------------------------

    inline void ThrowInvalidArgument(const std::string& msgprefix)
    {
      throw std::invalid_argument(msgprefix + " Error: " + GetLastWin32ErrorStr());
    }

    // function returns empty string if inStr is empty or if the conversion failed
    std::wstring ToWString(const std::string& inStr)
    {
      if (inStr.empty())
      {
        return std::wstring();
      }
      int wchar_count = MultiByteToWideChar(CP_UTF8, 0, inStr.c_str(), -1, NULL, 0);
      std::unique_ptr<wchar_t[]> wBuf(new wchar_t[wchar_count]);
      wchar_count = MultiByteToWideChar(CP_UTF8, 0, inStr.c_str(), -1, wBuf.get(), wchar_count);
      if (wchar_count == 0)
      {
        ThrowInvalidArgument("Failed to convert " + inStr + " to UTF16.");
      }
      return wBuf.get();
    }

    // function return empty string if inWStr is empty or if the conversion failed
    std::string ToUTF8String(const std::wstring& inWStr)
    {
      if (inWStr.empty())
      {
        return std::string();
      }
      int char_count = WideCharToMultiByte(CP_UTF8, 0, inWStr.c_str(), -1, NULL, 0, NULL, NULL);
      std::unique_ptr<char[]> str(new char[char_count]);
      char_count = WideCharToMultiByte(CP_UTF8, 0, inWStr.c_str(), -1, str.get(), char_count, NULL, NULL);
      if (char_count == 0)
      {
        ThrowInvalidArgument("Failed to convert UTF16 string to UTF8.");
      }
      return str.get();
    }
  }
}
#endif