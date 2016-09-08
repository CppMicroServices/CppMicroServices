Example 5 - Service Tracker Dictionary Client Bundle   {#MicroServices_Example5}
====================================================

In \ref MicroServices_Example4 "Example 4", we created a more robust client bundle
for our dictionary service. Due to the complexity of dealing with dynamic service
availability, even that client may not sufficiently address all situations. To
deal with this complexity the C++ Micro Services library provides the `ServiceTracker`
utility class. In this example we create a client for the dictionary service that
uses the `ServiceTracker` class to monitor the dynamic availability of the dictionary
service, resulting in an even more robust client.

The functionality of the new dictionary client is essentially the same as the one
from Example 4. Our bundle uses its bundle context to create a `ServiceTracker`
instance to track the dynamic availability of the dictionary service on our behalf.
Our client uses the dictionary service returned by the `ServiceTracker`, which is
selected based on a ranking algorithm defined by the C++ Micro Services library.
The source code for our bundles is as follows in a file called
`dictionaryclient3/Activator.cpp`:

\snippet dictionaryclient3/Activator.cpp Activator

Since this client uses the `ServiceTracker` utility class, it will automatically
monitor the dynamic availability of the dictionary service. Again, we must link
our bundle to the `dictionaryservice` bundle:

\include dictionaryclient3/CMakeLists.txt

After running the `usFrameworkExamplesDriver` executable, and starting the event
listener bundle, we can use the `l dictionaryclient3` command to start
our robust dictionary client bundle:

\verbatim
CppMicroServices-debug> bin/usFrameworkExamplesDriver
> l eventlistener
Starting to listen for service events.
> l dictionaryclient3
Ex1: Service of type IDictionaryService/1.0 registered.
Enter a blank line to exit.
Enter word:
\endverbatim

The above command loads the bundle and its dependencies (the `dictionaryservice`
bundle) in a single step. When we load the bundle, it will use the main thread to
prompt us for words. Enter one word at a time to check the words and enter a
blank line to stop checking words. To reload the bundle, we must use the `s`
command to get the bundle identifier number for the bundle and first use the
`u <id>` command to unload the bundle, then the `l <id>` command to re-load it.
To test the dictionary service, enter any of the words in the dictionary
(e.g., "welcome", "to", "the", "micro", "services", "tutorial") or any word not
in the dictionary.

Since this client monitors the dynamic availability of the dictionary service,
it is robust in the face of sudden departures of the the dictionary service.
Further, when a dictionary service arrives, it automatically gets the service if
it needs it and continues to function. These capabilities are a little difficult
to demonstrate since we are using a simple single-threaded approach, but in a
multi-threaded or GUI-oriented application this robustness is very useful.

Next: \ref MicroServices_Example6

Previous: \ref MicroServices_Example4
