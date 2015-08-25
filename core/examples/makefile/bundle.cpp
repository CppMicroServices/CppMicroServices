#include <usBundleActivator.h>
#include <usBundleContext.h>
#include <usGetBundleContext.h>
#include <usServiceProperties.h>

#include "IDictionaryService.h"

#include <set>
#include <algorithm>
#include <memory>

US_USE_NAMESPACE

/**
 * This class implements a bundle activator that uses the bundle
 * context to register an English language dictionary service
 * with the C++ Micro Services registry during static initialization
 * of the bundle. The dictionary service interface is
 * defined in a separate file and is implemented by a nested class.
 */
class US_ABI_LOCAL MyActivator : public BundleActivator
{

private:

  /**
   * A private inner class that implements a dictionary service;
   * see DictionaryService for details of the service.
   */
  class DictionaryImpl : public IDictionaryService
  {
    // The set of words contained in the dictionary.
    std::set<std::string> m_Dictionary;

  public:

    DictionaryImpl()
    {
      m_Dictionary.insert("welcome");
      m_Dictionary.insert("to");
      m_Dictionary.insert("the");
      m_Dictionary.insert("micro");
      m_Dictionary.insert("services");
      m_Dictionary.insert("tutorial");
    }

    /**
     * Implements IDictionaryService::CheckWord(). Determines
     * if the passed in word is contained in the dictionary.
     *
     * @param word the word to be checked.
     * @return true if the word is in the dictionary,
     *         false otherwise.
     **/
    bool CheckWord(const std::string& word)
    {
      std::string lword(word);
      std::transform(lword.begin(), lword.end(), lword.begin(), ::tolower);

      return m_Dictionary.find(lword) != m_Dictionary.end();
    }
  };

  std::auto_ptr<DictionaryImpl> m_DictionaryService;


public:

  /**
   * Implements BundleActivator::Start(). Registers an
   * instance of a dictionary service using the bundle context;
   * attaches properties to the service that can be queried
   * when performing a service look-up.
   *
   * @param context the context for the bundle.
   */
  void Start(BundleContext* context)
  {
    m_DictionaryService.reset(new DictionaryImpl);
    ServiceProperties props;
    props["Language"] = std::string("English");
    context->RegisterService<IDictionaryService>(m_DictionaryService.get(), props);
  }

  /**
   * Implements BundleActivator::Stop(). Does nothing since
   * the C++ Micro Services library will automatically unregister any registered services.
   *
   * @param context the context for the bundle.
   */
  void Stop(BundleContext* /*context*/)
  {
    // NOTE: The service is automatically unregistered
  }

};

US_EXPORT_BUNDLE_ACTIVATOR(MyActivator)

#include <usBundleInitialization.h>

US_INITIALIZE_BUNDLE
