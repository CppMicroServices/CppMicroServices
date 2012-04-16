Implementing A Dictionary Service    {#MicroServices_DictionaryService}
=================================

This example demonstrates how to implement a service object. First, we must define the service interface
and then we define an implementation of that interface. In this particular example, we will create a
dictionary service that we can use to check if a word exists, which indicates if the word is spelled
correctly or not.

We start by defining the dictionary interface (for example in a file called DictionaryService.h):

\include uServices-dictionaryservice/DictionaryService.h

The service interface is quite simple, with only one method that needs to be implemented. Notice that the
file does only contain the interface declaration and no actual code.

In the following source code, the module uses its module context to register the dictionary service.
We implement the dictionary service as an inner class of the module activator class, but we could have also
put it in a separate file. Also note that there is no need to explicityl export any symbols from the module.

\snippet uServices-dictionaryservice/main.cpp Activator

Note that we do not need to unregister the service in the `Unload` method, because it will be done automatically
for us during static de-initialization of the module.

In the last line, we "export" the module activator, assuming that it is contained in a module named
`DictionaryServiceModule`. We can get the highest ranking service implementation for the DictionaryService interface
by querying the service registry via the module context:

\snippet uServices-dictionaryservice/main.cpp GetDictionaryService
