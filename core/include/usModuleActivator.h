/*=============================================================================

  Library: CppMicroServices

  Copyright (c) German Cancer Research Center,
    Division of Medical and Biological Informatics

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

#ifndef USMODULEACTIVATOR_H_
#define USMODULEACTIVATOR_H_

#ifndef US_MODULE_NAME
#error Missing US_MODULE_NAME preprocessor define
#endif

#include <usGlobalConfig.h>
#include <usModuleUtils_p.h>

US_BEGIN_NAMESPACE

class ModuleContext;

/**
 * \ingroup MicroServices
 *
 * Customizes the starting and stopping of a CppMicroServices module.
 * <p>
 * <code>%ModuleActivator</code> is an interface that can be implemented by
 * CppMicroServices modules. The CppMicroServices library can create instances of a
 * module's <code>%ModuleActivator</code> as required. If an instance's
 * <code>ModuleActivator::Load</code> method executes successfully, it is
 * guaranteed that the same instance's <code>%ModuleActivator::Unload</code>
 * method will be called when the module is to be unloaded. The CppMicroServices
 * library does not concurrently call a <code>%ModuleActivator</code> object.
 *
 * <p>
 * <code>%ModuleActivator</code> is an abstract class interface whose implementations
 * must be exported via a special macro. Implementations are usually declared
 * and defined directly in .cpp files.
 *
 * <p>
 * \snippet uServices-activator/main.cpp 0
 *
 * <p>
 * The class implementing the <code>%ModuleActivator</code> interface must have a public
 * default constructor so that a <code>%ModuleActivator</code>
 * object can be created by the CppMicroServices library.
 *
 */
struct ModuleActivator
{

  virtual ~ModuleActivator() {}

  /**
   * Called when this module is loaded. This method
   * can be used to register services or to allocate any resources that this
   * module may need globally (during the whole module lifetime).
   *
   * <p>
   * This method must complete and return to its caller in a timely manner.
   *
   * @param context The execution context of the module being loaded.
   * @throws std::exception If this method throws an exception, this
   *         module is marked as stopped and the framework will remove this
   *         module's listeners, unregister all services registered by this
   *         module, and release all services used by this module.
   */
  virtual void Load(ModuleContext* context) = 0;

  /**
   * Called when this module is unloaded. In general, this
   * method should undo the work that the <code>ModuleActivator::Load</code>
   * method started. There should be no active threads that were started by
   * this module when this method returns.
   *
   * <p>
   * This method must complete and return to its caller in a timely manner.
   *
   * @param context The execution context of the module being unloaded.
   * @throws std::exception If this method throws an exception, the
   *         module is still marked as unloaded, and the framework will remove
   *         the module's listeners, unregister all services registered by the
   *         module, and release all services used by the module.
   */
  virtual void Unload(ModuleContext* context) = 0;

};

US_END_NAMESPACE


/**
 * \ingroup MicroServices
 *
 * \brief Export a module activator class.
 *
 * \param _activator_type The fully-qualified type-name of the module activator class.
 *
 * Call this macro after the definition of your module activator to make it
 * accessible by the CppMicroServices library.
 *
 * Example:
 * \snippet uServices-activator/main.cpp 0
 */
#define US_EXPORT_MODULE_ACTIVATOR(_activator_type)                                       \
  extern "C" US_ABI_EXPORT US_PREPEND_NAMESPACE(ModuleActivator)* US_CONCAT(_us_module_activator_instance_, US_MODULE_NAME) () \
  {                                                                                       \
    struct ScopedPointer                                                                  \
    {                                                                                     \
      ScopedPointer(US_PREPEND_NAMESPACE(ModuleActivator)* activator = 0) : m_Activator(activator) {} \
      ~ScopedPointer() { delete m_Activator; }                                            \
      US_PREPEND_NAMESPACE(ModuleActivator)* m_Activator;                                 \
    };                                                                                    \
                                                                                          \
    static ScopedPointer activatorPtr;                                                    \
    if (activatorPtr.m_Activator == 0) activatorPtr.m_Activator = new _activator_type;    \
    return activatorPtr.m_Activator;                                                      \
  }

#endif /* USMODULEACTIVATOR_H_ */
