#include <memory>
#include <string>

class CNApplication;
typedef std::shared_ptr<CNApplication> cn_app_ptr;

class CNApplication
{
public:
   virtual ~CNApplication();

   virtual void set_cppms_path(const char* p) = 0;
   virtual void set_destination(const char* p) = 0;
   virtual void set_namespace(const char* name) = 0;
   virtual void set_namespace_alias(bool) = 0;
   virtual std::string get_cppms_path() = 0;
   virtual std::string get_destination() = 0;
   virtual std::string get_namespace() = 0;

   virtual int run() = 0;

   static cn_app_ptr create();
};