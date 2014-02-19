Example 6 - Spell Checker Service Module   {#MicroServices_Example6}
========================================

In this example, we complicate things further by defining a new service that
uses an arbitrary number of dictionary services to perform its function. More
precisely, we define a spell checker service which will aggregate all dictionary
services and provide another service that allows us to spell check passages
using our underlying dictionary services to verify the spelling of words. Our
module will only provide the spell checker service if there are at least two
dictionary services available. First, we will start by defining the spell checker
service interface in a file called `spellcheckservice/ISpellCheckService.h`:

\snippet spellcheckservice/ISpellCheckService.h service

The service interface is quite simple, with only one method that needs to be
implemented. Because we provide an empty out-of-line destructor (defined in the
file `ISpellCheckService.cpp`) we must export the service interface by using the
module specific `SPELLCHECKSERVICE_EXPORT` macro.

In the following source code, the module needs to create a complete list of all
dictionary services; this is somewhat tricky and must be done carefully if done
manually via service event listners. Our module makes use of the `ServiceTracker`
and `ServiceTrackerCustomizer` classes to robustly react to service events related
to dictionary services. The module activator of our module now additionally implements
the `ServiceTrackerCustomizer` class to be automatically notified of arriving, departing,
or modified dictionary services. In case of a newly added dictionary service, our
`ServiceTrackerCustomizer::AddingService()` implementation checks if a spell checker
service was already registered and if not registers a new `ISpellCheckService` instance
if at lead two dictionary services are available.
If the number of dictionary services drops below two, our `ServiceTrackerCustomizer`
implementation un-registers the previously registered spell checker service instance.
These actions must be performed in a synchronized manner to avoid interference from
service events originating from different threads. The implementation of our module
activator is done in a file called `spellcheckservice/Activator.cpp`:

\snippet spellcheckservice/Activator.cpp Activator

Note that we do not need to unregister the service in stop() method, because the
C++ Micro Services library will automatically do so for us. The spell checker service
that we have implemented is very simple; it simply parses a given passage into words
and then loops through all available dictionary services for each word until it
determines that the word is correct. Any incorrect words are added to an error list
that will be returned to the caller. This solution is not optimal and is only intended
for educational purposes.

\note In this example, the service interface and implementation are both
contained in one module which exports the interface class. However, service
implementations almost never need to be exported and in many use cases
it is beneficial to provide the service interface and its implementation(s)
in separate modules. In such a scenario, clients of a service will only
have a link-time dependency on the shared library providing the service interface
(because of the out-of-line destructor) but not on any modules containing
service implementations. This often leads to modules which do not export
any symbols at all and hence need to be loaded into the running process
manually or by using the \ref MicroServices_AutoLoading "auto-loading mechanism".

\note Due to the link dependency of our module to the module containing the
dictionary service interface as well as a default implementation for it, there
might be at least one dictionary service registered when our module is
loaded, depending on your linker settings (e.g. on Windows, the linker usually
by default optimizes the link dependency away since our module does not actually
use any symbols from the dictionaryservice module. On Linux however, the link
dependency is kept by default.) To observe the dynamic registration and
un-registration of our spell checker service, we require the availability of
at least two dictionary services.

For an introduction how to compile our source code, see \ref MicroServices_Example1.

After running the `usCoreExamplesDriver` program we should make sure that the
module from Example 1 is active. We can use the `s` shell command to get
a list of all modules, their state, and their module identifier number.
If the Example 1 module is not active, we should load the module using the
load command and the module's identifier number or name that is displayed
by the `s` command. Now we can load the spell checker service module by
entering the `l spellcheckservice` command which will also trigger the loading
of the dictionaryservice module containing the english dictionary:

\verbatim
CppMicroServices-build> bin/usCoreExamplesDriver
> l eventlistener
Starting to listen for service events.
> l spellcheckservice
Ex1: Service of type IDictionaryService/1.0 registered.
> s
Id | Name                 | Status
-----------------------------------
 - | dictionaryclient     | -
 - | dictionaryclient2    | -
 - | dictionaryclient3    | -
 - | frenchdictionary     | -
 - | spellcheckclient     | -
 1 | CppMicroServices     | LOADED
 2 | Event Listener       | LOADED
 3 | Dictionary Service   | LOADED
 4 | Spell Check Service  | LOADED
>
\endverbatim

To trigger the registration of the spell checker service from our module, we
load the frenchdictionary using the `l frenchdictionary` command. If the module from
\ref MicroServices_Example1 "Example 1" is still active,
then we should see it print out the details of the service event it receives
when our new module registers its spell checker service:

\verbatim
CppMicroServices-build> bin/usCoreExamplesDriver
> l frenchdictionary
Ex1: Service of type IDictionaryService/1.0 registered.
Ex1: Service of type ISpellCheckService/1.0 registered.
>
\endverbatim

We can experiment with our spell checker service's dynamic availability by stopping
the french dictionary service; when the service is stopped,
the eventlistener module will print that our module is no longer offering its
spell checker service. Likewise, when the french dictionary service comes back, so will
our spell checker service. We create a client for our spell checker service in
\ref MicroServices_Example7 "Example 7". To exit the `usCoreExamplesDriver` program, we
use the `q` command.

Next: \ref MicroServices_Example7

Previous: \ref MicroServices_Example5
