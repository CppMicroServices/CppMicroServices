Example 1 - Service Event Listener    {#MicroServices_Example1}
==================================

This example creates a simple bundle that listens for service events.
This example does not do much at first, because it only prints out the details
of registering and unregistering services. In the next example we will create
a bundle that implements a service, which will cause this bundle to actually
do something. For now, we will just use this example to help us understand the
basics of creating a bundle and its activator.

A bundle gains access to the C++ Micro Services API using a unique instance
of BundleContext. This unique bundle context can be used during static
initialization of the bundle or at any later point during the life-time of the
bundle. To execute code during static initialization (and de-initialization)
time, the bundle must provide an implementation of the BundleActivator interface;
this interface has two methods, Start() and Stop(), that both receive the
bundle's context and are called when the bundle is started and stopped, respectively.

\note You do not need to remember the BundleContext instance within the
BundleActivator::Start() method and provide custom access methods for later
retrieval. Use the GetBundleContext() function to easily retrieve the current
bundle's context.

In the following source code, our bundle implements
the BundleActivator interface and uses the context to add itself as a listener
for service events (in the `eventlistener/Activator.cpp` file):

\snippet eventlistener/Activator.cpp Activator

After implementing the C++ source code for the bundle activator, we must *export*
the activator such that the C++ Micro Services library can create an instance
of it and call the `Start()` and `Stop()` methods:

\dontinclude eventlistener/Activator.cpp
\skipline US_EXPORT

Now we need to compile the source code. This example uses CMake as the build
system and the top-level CMakeLists.txt file could look like this:

\dontinclude examples/CMakeLists.txt
\skip project
\until eventlistener

and the CMakeLists.txt file in the eventlistener subdirectory is:

\include eventlistener/CMakeLists.txt

The call to `#usFunctionGenerateBundleInit` is necessary to integrate the shared
library as a bundle within the C++ Micro Service library. If you are not using
CMake, you have to place a macro call to `#CPPMICROSERVICES_INITIALIZE_BUNDLE` yourself into the
bundle's source code, e.g. in `Activator.cpp`. Have a look at the
\ref MicroServices_GettingStarted documentation for more details about using CMake
or other build systems (e.g. Makefiles) when writing bundles.

To run the examples contained in the C++ Micro Services library, we use a small
driver program called `usFrameworkExamplesDriver`:

\verbatim
CppMicroServices-build> bin/usFrameworkExamplesDriver
> h
h               This help text
l <id | name>   Start the bundle with id <id> or name <name>
u <id>          Stop the bundle with id <id>
s               Print status information
q               Quit
>
\endverbatim

Typing `s` at the command prompt lists the available, started, and stopped bundles.
To start the eventlistener bundle, type `l eventlistener` at the command prompt:

\verbatim
> s
Id | Name                 | Status
-----------------------------------
 - | dictionaryclient     | -
 - | dictionaryclient2    | -
 - | dictionaryclient3    | -
 - | dictionaryservice    | -
 - | eventlistener        | -
 - | frenchdictionary     | -
 - | spellcheckclient     | -
 - | spellcheckservice    | -
 1 | CppMicroServices     | ACTIVE
> l eventlistener
Starting to listen for service events.
>
\endverbatim

The above command started the eventlistener bundle (by loading its shared library).
Keep in mind, that this bundle will not do much at this point since it only
listens for service events and we are not registering any services. In the next
example we will register a service that will generate an event for this bundle to
receive. To exit the `usFrameworkExamplesDriver`, use the `q` command.

Next: \ref MicroServices_Example2
