Example 2b - Alternative Dictionary Service Module    {#MicroServices_Example2b}
==================================================

This example creates an alternative implementation of the dictionary service
defined in \ref MicroServices_Example2 "Example 2". The source code for the
module is identical except that instead of using English words, French words
are used. The only other difference is that in this module we do not need to
define the dictionary service interface again, since we can just link the
definition from the module in Example 2. The main point of this example is
to illustrate that multiple implementations of the same service may exist;
this example will also be of use to us in \ref MicroServices_Example5 "Example 5".

In the following source code, the module uses its module context
to register the dictionary service. We implement the dictionary service as an
inner class of the module activator class, but we could have also put it in a
separate file. The source code for our module is as follows in a file called
`dictionaryclient/Activator.cpp`:

\snippet frenchdictionary/Activator.cpp Activator

For an introduction how to compile our source code, see \ref MicroServices_Example1.
Because we use the `IDictionaryService` definition from Example 2, we also
need to make sure that the proper include paths and linker dependencies are set:

\include frenchdictionary/CMakeLists.txt

After running the `usCoreExamplesDriver` program we should make sure that the
module from Example 1 is active. We can use the `s` shell command to get
a list of all modules, their state, and their module identifier number.
If the Example 1 module is not active, we should load the module using the
load command and the module's identifier number or name that is displayed
by the `s` command. Now we can load our dictionary service module by typing
the `l frenchdictionary` command:

\verbatim
CppMicroServices-build> bin/usCoreExamplesDriver
> s
Id | Name                 | Status
-----------------------------------
 - | dictionaryservice    | -
 - | eventlistener        | -
 - | frenchdictionary     | -
 1 | CppMicroServices     | LOADED
> l eventlistener
Starting to listen for service events.
> l frenchdictionary
Ex1: Service of type IDictionaryService/1.0 registered.
Ex1: Service of type IDictionaryService/1.0 registered.
> s
Id | Name                 | Status
-----------------------------------
 1 | CppMicroServices     | LOADED
 2 | Event Listener       | LOADED
 3 | Dictionary Service   | LOADED
 4 | French Dictionary    | LOADED
>
\endverbatim

To unload the module, use the `u <id>` command. If the module from
\ref MicroServices_Example1 "Example 1" is still active,
then we should see it print out the details of the service event it receives
when our new module registers its dictionary service. Using the `usCoreExamplesDriver`
commands `u` and `l` we can unload and load it at will, respectively. Each
time we load and unload our dictionary service module, we should see the details
of the associated service event printed from the module from Example 1. In
\ref MicroServices_Example3 "Example 3", we will create a client for our
dictionary service. To exit `usCoreExamplesDriver`, we use the `q` command.

\note Because our french dictionary module has a link dependency on the
dictionary service module from Example 2, this module is automatically loaded
by the operating system loader. Unloading it will only succeed if there are
no other dependent modules like our french dictionary module currently loaded.

Next: \ref MicroServices_Example3

Previous: \ref MicroServices_Example2
