
//! [Activator]
#include "DictionaryService.h"

#include <usModuleActivator.h>
#include <usModuleContext.h>
#include <usGetModuleContext.h>
#include <usServiceProperties.h>

// Replace that include with your own base class declaration
#include US_BASECLASS_HEADER

#include <set>
#include <algorithm>
#include <memory>

US_USE_NAMESPACE

/**
 * This class implements a module activator that uses the module
 * context to register an English language dictionary service
 * with the C++ Micro Services registry during static initialization
 * of the module. The dictionary service interface is
 * defined in a separate file and is implemented by a nested class.
 */
class US_ABI_LOCAL MyActivator : public ModuleActivator
{

private:

  /**
   * A private inner class that implements a dictionary service;
   * see DictionaryService for details of the service.
   */
  class DictionaryImpl : public US_BASECLASS_NAME, public DictionaryService
  {
    // The set of words contained in the dictionary.
    std::set<std::string> m_dictionary;

  public:

    DictionaryImpl()
    {
      m_dictionary.insert("welcome");
      m_dictionary.insert("to");
      m_dictionary.insert("the");
      m_dictionary.insert("micro");
      m_dictionary.insert("services");
      m_dictionary.insert("tutorial");
    }

    /**
     * Implements DictionaryService.checkWord(). Determines
     * if the passed in word is contained in the dictionary.
     * @param word the word to be checked.
     * @return true if the word is in the dictionary,
     *         false otherwise.
     **/
    bool checkWord(const std::string& word)
    {
      std::string lword(word);
      std::transform(lword.begin(), lword.end(), lword.begin(), ::tolower);

      return m_dictionary.find(lword) != m_dictionary.end();
    }
  };

  std::auto_ptr<DictionaryImpl> m_dictionaryService;


public:

  /**
   * Implements ModuleActivator::Load(). Registers an
   * instance of a dictionary service using the module context;
   * attaches properties to the service that can be queried
   * when performing a service look-up.
   * @param context the context for the module.
   */
  void Load(ModuleContext* context)
  {
    m_dictionaryService.reset(new DictionaryImpl);
    ServiceProperties props;
    props["Language"] = std::string("English");
    context->RegisterService<DictionaryService>(m_dictionaryService.get(), props);
  }

  /**
   * Implements ModuleActivator::Unload(). Does nothing since
   * the C++ Micro Services library will automatically unregister any registered services.
   * @param context the context for the module.
   */
  void Unload(ModuleContext* /*context*/)
  {
    // NOTE: The service is automatically unregistered
  }

};

US_EXPORT_MODULE_ACTIVATOR(DictionaryServiceModule, MyActivator)
//![Activator]

#include <usModuleInitialization.h>

US_INITIALIZE_MODULE("DictionaryServiceModule", "", "", "1.0.0")

int main(int /*argc*/, char* /*argv*/[])
{
  //![GetDictionaryService]
  ServiceReference dictionaryServiceRef = GetModuleContext()->GetServiceReference<DictionaryService>();
  if (dictionaryServiceRef)
  {
    DictionaryService* dictionaryService = GetModuleContext()->GetService<DictionaryService>(dictionaryServiceRef);
    if (dictionaryService)
    {
      std::cout << "Dictionary contains 'Tutorial': " << dictionaryService->checkWord("Tutorial") << std::endl;
    }
  }
  //![GetDictionaryService]
  return 0;
}
