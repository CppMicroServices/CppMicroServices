Example 7 - Spell Checker Client Module   {#MicroServices_Example7}
=======================================

In this example we create a client for the spell checker service we implemented
in \ref MicroServices_Example6 "Example 6". This client monitors the dynamic
availability of the spell checker service using the Service Tracker and is very
similar in structure to the dictionary client we implemented in
\ref MicroServices_Example5 "Example 5". The functionality of the spell checker
client reads passages from standard input and spell checks them using the spell
checker service. Our module uses its module context to create a `ServiceTracker`
object to monitor spell checker services. The source code for our module is as
follows in a file called `spellcheckclient/Activator.cpp`:

\snippet spellcheckclient/Activator.cpp Activator

After running the `usCoreExamplesDriver` program use the `s` command to make sure that
only the modules from Example 2, Example 2b, and Example 6 are loaded; use the
load (`l`) and un-load (`u`) commands as appropriate to load and un-load the
various tutorial modules, respectively. Now we can load our spell checker client
module by entering `l spellcheckclient`:

\verbatim
CppMicroServices-build> bin/usCoreExamplesDriver
> l
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
> l spellcheckservice
> l frenchdictionary
> l spellcheckclient
Enter a blank line to exit.
Enter passage:
\endverbatim

When we start the module, it will use the main thread to prompt us for passages; a
passage is a collection or words separate by spaces, commas, periods, exclamation
points, question marks, colons, or semi-colons. Enter a passage and press the enter
key to spell check the passage or enter a blank line to stop spell checking passages.
To restart the module, we must use the command `s` command to get the module identifier
number for the module and first use the `u` command to un-load the module, then the
`l` command to re-load it.

Since this client uses the Service Tracker to monitor the dynamic availability of the
spell checker service, it is robust in the scenario where the spell checker service
suddenly departs. Further, when a spell checker service arrives, it automatically gets
the service if it needs it and continues to function. These capabilities are a little
difficult to demonstrate since we are using a simple single-threaded approach, but in
a multi-threaded or GUI-oriented application this robustness is very useful.

Previous: \ref MicroServices_Example6
