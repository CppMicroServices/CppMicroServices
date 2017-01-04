Example 4 - Robust Dictionary Client Bundle   {#MicroServices_Example4}
===========================================

In \ref MicroServices_Example3 "Example 3", we create a simple client bundle
for our dictionary service. The problem with that client was that it did not
monitor the dynamic availability of the dictionary service, thus an error
would occur if the dictionary service disappeared while the client was using it.
In this example we create a client for the dictionary service that monitors
the dynamic availability of the dictionary service. The result is a more robust
client.

The functionality of the new dictionary client is essentially the same as the
old client, it reads words from standard input and checks for their existence
in the dictionary service. Our bundle uses its bundle context to register itself
as a service event listener; monitoring service events allows the bundle to
monitor the dynamic availability of the dictionary service. Our client uses the
first dictionary service it finds. The source code for our bundle is as follows
in a file called `Activator.cpp`:

\snippet dictionaryclient2/Activator.cpp Activator

The client listens for service events indicating the arrival or departure of
dictionary services. If a new dictionary service arrives, the bundle will start
using that service if and only if it currently does not have a dictionary service.
If an existing dictionary service disappears, the bundle will check to see if the
disappearing service is the one it is using; if it is it stops using it and tries
to query for another dictionary service, otherwise it ignores the event.

As in Example 3, we must link our bundle to the `dictionaryservice` bundle:

\include dictionaryclient2/CMakeLists.txt

After running the `usFrameworkExamplesDriver` executable, and starting the event
listener bundle, we can use the `start dictionaryclient2` command to start
our robust dictionary client bundle:

\verbatim
CppMicroServices-debug> bin/usFrameworkExamplesDriver
> start eventlistener
Starting to listen for service events.
> start dictionaryclient2
Ex1: Service of type IDictionaryService registered.
Enter a blank line to exit.
Enter word:
\endverbatim

The above command starts the bundle and it will use the main thread to
prompt us for words. Enter one word at a time to check the words and enter a
blank line to stop checking words. To reload the bundle, we must first use the
`stop <id | name>` command to stop the bundle, then the `start <id | name>` command to re-start it.
To test the dictionary service, enter any of the words in the dictionary
(e.g., "welcome", "to", "the", "micro", "services", "tutorial") or any word not
in the dictionary.

Since this client monitors the dynamic availability of the dictionary service,
it is robust in the face of sudden departures of the dictionary service.
Further, when a dictionary service arrives, it automatically gets the service if
it needs it and continues to function. These capabilities are a little difficult
to demonstrate since we are using a simple single-threaded approach, but in a
multi-threaded or GUI-oriented application this robustness is very useful.

Next: \ref MicroServices_Example5

Previous: \ref MicroServices_Example3
