Example 2 - Dictionary Service Bundle    {#MicroServices_Example2}
=====================================

This example creates a bundle that implements a service. Implementing a
service is a two-step process, first we must define the interface of the service
and then we must define an implementation of the service interface. In this
particular example, we will create a dictionary service that we can use to check
if a word exists, which indicates if the word is spelled correctly or not. First,
we will start by defining a simple dictionary service interface in a file called
`dictionaryservice/IDictionaryService.h`:

\snippet dictionaryservice/IDictionaryService.h service

The service interface is quite simple, with only one method that needs to be
implemented. Because we provide an empty out-of-line destructor (defined in the
file `IDictionaryService.cpp`) we must export the service interface by using the
bundle specific `DICTIONARYSERVICE_EXPORT` macro.

In the following source code, the bundle uses its bundle context
to register the dictionary service. We implement the dictionary service as an
inner class of the bundle activator class, but we could have also put it in a
separate file. The source code for our bundle is as follows in a file called
`dictionaryservice/Activator.cpp`:

\snippet dictionaryservice/Activator.cpp Activator

Note that we do not need to unregister the service in the Stop() method,
because the C++ Micro Services library will automatically do so for us. The
dictionary service that we have implemented is very simple; its dictionary
is a set of only five words, so this solution is not optimal and is only
intended for educational purposes.

\note In this example, the service interface and implementation are both
contained in one bundle which exports the interface class. However, service
implementations almost never need to be exported and in many use cases
it is beneficial to provide the service interface and its implementation(s)
in separate bundles. In such a scenario, clients of a service will only
have a link-time dependency on the shared library providing the service interface
(because of the out-of-line destructor) but not on any bundles containing
service implementations. This often leads to bundles which do not export
any symbols at all and hence need to be loaded into the running process
manually.

For an introduction how to compile our source code, see \ref MicroServices_Example1.

After running the `usFrameworkExamplesDriver` program we should make sure that the
bundle from Example 1 is active. We can use the `s` shell command to get
a list of all bundles, their state, and their bundle identifier number.
If the Example 1 bundle is not active, we should start the bundle using the
start command and the bundle's identifier number or name that is displayed
by the `s` command. Now we can start our dictionary service bundle by typing
the `l dictionaryservice` command:

\verbatim
CppMicroServices-build> bin/usFrameworkExamplesDriver
> s
Id | Name                 | Status
-----------------------------------
 - | dictionaryservice    | -
 - | eventlistener        | -
 1 | CppMicroServices     | ACTIVE
> l eventlistener
Starting to listen for service events.
> l dictionaryservice
Ex1: Service of type IDictionaryService registered.
> s
Id | Name                 | Status
-----------------------------------
 1 | CppMicroServices     | ACTIVE
 2 | Event Listener       | ACTIVE
 3 | Dictionary Service   | ACTIVE
>
\endverbatim

To stop the bundle, use the `u <id>` command. If the bundle from
\ref MicroServices_Example1 "Example 1" is still active,
then we should see it print out the details of the service event it receives
when our new bundle registers its dictionary service. Using the `usFrameworkExamplesDriver`
commands `u` and `l` we can stop and start it at will, respectively. Each
time we start and stop our dictionary service bundle, we should see the details
of the associated service event printed from the bundle from Example 1. In
\ref MicroServices_Example3 "Example 3", we will create a client for our
dictionary service. To exit `usFrameworkExamplesDriver`, we use the `q` command.

Next: \ref MicroServices_Example2b

Previous: \ref MicroServices_Example1
