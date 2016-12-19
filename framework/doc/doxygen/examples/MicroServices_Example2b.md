Example 2b - Alternative Dictionary Service Bundle    {#MicroServices_Example2b}
==================================================

This example creates an alternative implementation of the dictionary service
defined in \ref MicroServices_Example2 "Example 2". The source code for the
bundle is identical except that instead of using English words, French words
are used. The only other difference is that in this bundle we do not need to
define the dictionary service interface again, since we can just link the
definition from the bundle in Example 2. The main point of this example is
to illustrate that multiple implementations of the same service may exist;
this example will also be of use to us in \ref MicroServices_Example5 "Example 5".

In the following source code, the bundle uses its bundle context
to register the dictionary service. We implement the dictionary service as an
inner class of the bundle activator class, but we could have also put it in a
separate file. The source code for our bundle is as follows in a file called
`dictionaryclient/Activator.cpp`:

\snippet frenchdictionary/Activator.cpp Activator

For an introduction how to compile our source code, see \ref MicroServices_Example1.
Because we use the `IDictionaryService` definition from Example 2, we also
need to make sure that the proper include paths and linker dependencies are set:

\include frenchdictionary/CMakeLists.txt

After running the `usFrameworkExamplesDriver` program we should make sure that the
bundle from Example 1 is active. We can use the `status` shell command to get
a list of all bundles, their state, and their bundle identifier number.
If the Example 1 bundle is not active, we should start the bundle using the
start command and the bundle's identifier number or name that is displayed
by the `status` command. Now we can start our dictionary service bundle by typing
the `start frenchdictionary` command:

\verbatim
CppMicroServices-build> bin/usFrameworkExamplesDriver
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
> start frenchdictionary
Ex1: Service of type IDictionaryService registered.
> status
Id | Symbolic Name        | State
-----------------------------------
 0 | system_bundle        | ACTIVE
 1 | eventlistener        | ACTIVE
 2 | dictionaryservice    | INSTALLED
 3 | frenchdictionary     | ACTIVE
 4 | dictionaryclient     | INSTALLED
 5 | dictionaryclient2    | INSTALLED
 6 | dictionaryclient3    | INSTALLED
 7 | spellcheckservice    | INSTALLED
 8 | spellcheckclient     | INSTALLED

>
\endverbatim

To stop the bundle, use the `stop <id | name>` command. If the bundle from
\ref MicroServices_Example1 "Example 1" is still active,
then we should see it print out the details of the service event it receives
when our new bundle registers its dictionary service. Using the `usFrameworkExamplesDriver`
commands `stop` and `start` we can stop and start it at will, respectively. Each
time we start and stop our dictionary service bundle, we should see the details
of the associated service event printed from the bundle from Example 1. In
\ref MicroServices_Example3 "Example 3", we will create a client for our
dictionary service. To exit `usFrameworkExamplesDriver`, we use the `shutdown` command.

\note Because our french dictionary bundle has a link dependency on the
dictionary service bundle from Example 2, this bundle`s shared library
is automatically loaded by the operating system loader. However, its status
remains `INSTALLED` until it is exlicitly started.

Next: \ref MicroServices_Example3

Previous: \ref MicroServices_Example2
