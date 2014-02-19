Example 1 - Service Event Listener    {#MicroServices_Example1}
==================================

This example creates a simple module that listens for service events.
This example does not do much at first, because it only prints out the details
of registering and unregistering services. In the next example we will create
a module that implements a service, which will cause this module to actually
do something. For now, we will just use this example to help us understand the
basics of creating a module and its activator.

A module gains access to the C++ Micro Services API using a unique instance
of ModuleContext. This unique module context can be used during static
initialization of the module or at any later point during the life-time of the
module. To execute code during static initialization (and de-initialization)
time, the module must provide an implementation of the ModuleActivator interface;
this interface has two methods, Load() and Unload(), that both receive the
module's context and are called when the module is loaded (statically initialized)
and unloaded, respectively.

\note You do not need to remember the ModuleContext instance within the
ModuleActivator::Load() method and provide custom access methods for later
retrieval. Use the GetModuleContext() function to easily retrieve the current
module's context.

In the following source code, our module implements
the ModuleActivator interface and uses the context to add itself as a listener
for service events (in the `eventlistener/Activator.cpp` file):

\snippet eventlistener/Activator.cpp Activator

After implementing the C++ source code for the module activator, we must *export*
the activator such that the C++ Micro Services library can create an instance
of it and call the `Load()` and `Unload()` methods:

\dontinclude eventlistener/Activator.cpp
\skipline US_EXPORT

Now we need to compile the source code. This example uses CMake as the build
system and the top-level CMakeLists.txt file could look like this:

\dontinclude examples/CMakeLists.txt
\skip project
\until eventlistener

and the CMakeLists.txt file in the eventlistener subdirectory is:

\include eventlistener/CMakeLists.txt

The call to `#usFunctionGenerateModuleInit` is necessary to integrate the shared
library as a module within the C++ Micro Service library. If you are not using
CMake, you have to place a macro call to `#US_INITIALIZE_MODULE` yourself into the
module's source code, e.g. in `Activator.cpp`. Have a look at the
\ref MicroServices_GettingStarted documentation for more details about using CMake
or other build systems (e.g. Makefiles) when writing modules.

To run the examples contained in the C++ Micro Services library, we use a small
driver program called `usCoreExamplesDriver`:

\verbatim
CppMicroServices-build> bin/usCoreExamplesDriver
> h
h               This help text
l <id | name>   Load the module with id <id> or name <name>
u <id>          Unload the module with id <id>
s               Print status information
q               Quit
>
\endverbatim

Typing `s` at the command prompt lists the available, loaded, and unloaded modules.
To load the eventlistener module, type `l eventlistener` at the command prompt:

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
 1 | CppMicroServices     | LOADED
> l eventlistener
Starting to listen for service events.
>
\endverbatim

The above command loaded the eventlistener module (by loading its shared library).
Keep in mind, that this module will not do much at this point since it only
listens for service events and we are not registering any services. In the next
example we will register a service that will generate an event for this module to
receive. To exit the `usCoreExamplesDriver`, use the `q` command.

Next: \ref MicroServices_Example2
