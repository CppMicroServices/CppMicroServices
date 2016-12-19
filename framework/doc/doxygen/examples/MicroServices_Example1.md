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
h                    This help text
start <id | name>    Start the bundle with id <id> or name <name>
stop <id | name>     Stop the bundle with id <id> or name <name>
status               Print status information
shutdown             Shut down the framework
\endverbatim

Typing `status` at the command prompt lists all installed bundles and their current
state. Note that the driver program pre-installs the example bundles, so they will
be listed initially with the `INSTALLED` state.
To start the eventlistener bundle, type `start eventlistener` at the command prompt:

\verbatim
> status
Id | Symbolic Name        | State
-----------------------------------
0 | system_bundle        | ACTIVE
1 | eventlistener        | INSTALLED
2 | dictionaryservice    | INSTALLED
3 | frenchdictionary     | INSTALLED
4 | dictionaryclient     | INSTALLED
5 | dictionaryclient2    | INSTALLED
6 | dictionaryclient3    | INSTALLED
7 | spellcheckservice    | INSTALLED
8 | spellcheckclient     | INSTALLED
> start eventlistener
Starting to listen for service events.
>
\endverbatim

The above command started the eventlistener bundle (implicitly loading its shared library).
Keep in mind, that this bundle will not do much at this point since it only
listens for service events and we are not registering any services. In the next
example we will register a service that will generate an event for this bundle to
receive. To exit the `usFrameworkExamplesDriver`, use the `q` command.

Next: \ref MicroServices_Example2
