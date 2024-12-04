#include <filesystem>
#include <memory>

class FileHandler
{
public:

    typedef const char&                               reference;
   typedef reference                                 const_reference;
   typedef const char*                               iterator;
   typedef iterator                                  const_iterator;
   typedef std::size_t                               size_type;
   typedef std::ptrdiff_t                            difference_type;
   typedef char                                      value_type;
   typedef const char*                               pointer;
   typedef pointer                                   const_pointer;

   FileHandler();
   FileHandler(const std::filesystem::path& p);
   ~FileHandler();
   FileHandler(const FileHandler& that);
   FileHandler& operator=(const FileHandler& that);
   void close();
   void open(const std::filesystem::path& p);

   const_iterator         begin() const;
   const_iterator         end() const;

   size_type size() const;
   size_type max_size() const;
   bool      empty() const;

   const_reference operator[](size_type n) const;
   const_reference at(size_type n) const;
   const_reference front() const;
   const_reference back() const;
   void            swap(FileHandler& that);

private:
   void cow();
   struct implementation;
   std::shared_ptr<implementation> impl_ptr;
};