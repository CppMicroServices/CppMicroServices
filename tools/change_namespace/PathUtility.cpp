#include "ChangeNamespaceImpl.hpp"
#include <cctype>

int compare_paths(const fs::path& a, const fs::path& b)
{
   const std::string& as = a.generic_string();
   const std::string& bs = b.generic_string();
   std::string::const_iterator i, j, k, l;
   i = as.begin();
   j = as.end();
   k = bs.begin();
   l = bs.end();
   while(i != j)
   {
      if(k == l)
         return -1;
      int r = std::tolower(*i) - std::tolower(*k);
      if(r) return r;
      ++i;
      ++k;
   }
   return (k == l) ? 0 : 1;
}