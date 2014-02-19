Example 3 - Dictionary Client Module   {#MicroServices_Example3}
====================================

This example creates a module that is a client of the dictionary service
implemented in \ref MicroServices_Example2 "Example 2". In the following
source code, our module uses its module context to query for a dictionary
service. Our client module uses the first dictionary service it finds and
if none are found it simply prints a message saying so and stops. Using
a service is the same as using any C++ class. The source code for our
module is as follows in a file called `dictionaryclient2/Activator.cpp`:

\snippet dictionaryclient/Activator.cpp Activator

Note that we do not need to unget or release the service in the Unload()
method, because the C++ Micro Services library will automatically do so
for us.

Since we are using the `IDictionaryService` interface defined in Example 1,
we must link our module to the `dictionaryservice` module:

\include dictionaryclient/CMakeLists.txt

After running the `usCoreExamplesDriver` executable, and loading the event
listener module, we can use the `l dictionaryclient` command to load
our dictionary client module:

\verbatim
CppMicroServices-debug> bin/usCoreExamplesDriver
> l eventlistener
Starting to listen for service events.
> l dictionaryclient
Ex1: Service of type IDictionaryService/1.0 registered.
Enter a blank line to exit.
Enter word:
\endverbatim

The above command loads the module and its dependencies (the `dictionaryservice`
module) in a single step. When we load the module, it will use the main thread to
prompt us for words. Enter one word at a time to check the words and enter a
blank line to stop checking words. To reload the module, we must use the `s`
command to get the module identifier number for the module and first use the
`u <id>` command to unload the module, then the `l <id>` command to re-load it.
To test the dictionary service, enter any of the words in the dictionary
(e.g., "welcome", "to", "the", "micro", "services", "tutorial") or any word not
in the dictionary.

This example client is simple enough and, in fact, is too simple. What would
happen if the dictionary service were to unregister suddenly? Our client would
abort with a segmentation fault due to a null pointer access when trying to use
the service object. This dynamic service availability issue is a central tenent
of the service model. As a result, we must make our client more robust in dealing
with such situations. In \ref MicroServices_Example4 "Example 4", we explore a
slightly more complicated dictionary client that dynamically monitors service
availability.

Next: \ref MicroServices_Example4

Previous: \ref MicroServices_Example2
