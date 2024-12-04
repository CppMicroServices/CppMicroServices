#include "FileHandler.hpp"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <istream>
#include <stdexcept>
#include <string>
#include <vector>

struct FileHandler::implementation
{
   std::vector<char> m_data;
};

FileHandler::FileHandler()
{
   impl_ptr.reset(new implementation());
}

FileHandler::FileHandler(const std::filesystem::path& p)
{
   impl_ptr.reset(new implementation());
   open(p);
}

FileHandler::~FileHandler()
{
}

FileHandler::FileHandler(const FileHandler& )
{
}

FileHandler& FileHandler::operator=(const FileHandler& that)
{
   impl_ptr = that.impl_ptr;
   return *this;
}

void FileHandler::close()
{
   cow();
   impl_ptr->m_data.clear();
}

void FileHandler::open(const std::filesystem::path& p)
{
   cow();
   std::ifstream is(p);
   if(!is)
   {
      std::string msg("Bad file name: ");
      msg += p.string();
      throw std::runtime_error(msg);
   }
   std::istreambuf_iterator<char> in(is);
   std::istreambuf_iterator<char> end;
   std::copy(in, end, std::back_inserter(impl_ptr->m_data));
}

FileHandler::const_iterator         FileHandler::begin() const
{
   return impl_ptr->m_data.size() ? &(impl_ptr->m_data[0]) : 0;
}

FileHandler::const_iterator         FileHandler::end() const
{
   return begin() + impl_ptr->m_data.size();
}

FileHandler::size_type FileHandler::size() const
{
   return impl_ptr->m_data.size();
}

FileHandler::size_type FileHandler::max_size() const
{
   return impl_ptr->m_data.max_size();
}

bool      FileHandler::empty() const
{
   return impl_ptr->m_data.empty();
}

FileHandler::const_reference FileHandler::operator[](FileHandler::size_type n) const
{
   return impl_ptr->m_data[n];
}

FileHandler::const_reference FileHandler::at(size_type n) const
{
   return impl_ptr->m_data.at(n);
}

FileHandler::const_reference FileHandler::front() const
{
   return impl_ptr->m_data.front();
}

FileHandler::const_reference FileHandler::back() const
{
   return impl_ptr->m_data.back();
}

void FileHandler::swap(FileHandler& that)
{
   impl_ptr.swap(that.impl_ptr);
}

void FileHandler::cow()
{
   if(impl_ptr.use_count() == 1)
      impl_ptr.reset(new implementation(*impl_ptr));
}