Service Hooks    {#MicroServices_ServiceHooks}
=============

The CppMicroServices library implements the Service Hook Service Specification Version 1.1 from
OSGi Core Release 5 for C++. Below is a summary of the concept - consult the OSGi specifications
for more details.

Service hooks provide mechanisms for module writers to closely interact with the CppMicroServices
service registry. These mechanisms are not intended for use by application modules but rather
by modules in need of *hooking* into the service registry and modifying the behaviour of
application modules.

Specific use case for service hooks include proxying of existing services by hiding the original
service and registering a *proxy service* with the same properties or providing services
*on demand* based on registered service listeners from external modules.

## Event Listener Hook

A module can intercept events being delivered to other modules by registering a ServiceEventListenerHook
object as a service. The CppMicroServices library will send all service events to all the registered
hooks using the reversed ordering of their ServiceReference objects. Note that event listener hooks
are called *after* the event was created but *before* it is filtered by the optional filter expression
of the service listeners. Hence an event listener hook receives all \link ServiceEvent::REGISTERED
REGISTERED\endlink, \link ServiceEvent::MODIFIED MODIFIED\endlink, \link ServiceEvent::UNREGISTERING
UNREGISTERING\endlink, and \link ServiceEvent::MODIFIED_ENDMATCH MODIFIED_ENDMATCH\endlink events
regardelss of the presence of a service listener filter. It may then remove modules or specific
service listeners from the ServiceEventListenerHook::ShrinkableMapType object passed to the
ServiceEventListenerHook::Event method to hide
service events.

Implementers of the Event Listener Hook must ensure that modules continue to see a consistent set of
service events.

## Find Hook

Find Hook objects registered using the ServiceFindHook interface will be called when modules look up
service references via the ModuleContext::GetServiceReference or ModuleContext::GetServiceReferences
methods. The order in which the CppMicroServices library calls the find hooks is the reverse `operator<`
ordering of their ServiceReference objects. The hooks may remove service references from the
ShrinkableVector object passed to the ServiceFindHook::Find method to hide services from specific modules.

## Listener Hook

The CppMicroServices API provides information about the registration, unregistration, and modification
of services. However, it does not directly allow the introspection of modules to get information about
what services a module is waiting for. Waiting for a service to arrive (via a registered service listener)
before performing its function is a common pattern for modules. Listener Hooks provide a mechanism to
get informed about all existing, newly registerd, and removed service listeners.

A Listener Hook object registered using the ServiceListenerHook interface will be notified about service
listeners by being passed ServiceListenerHook::ListenerInfo objects. Each ListenerInfo object is related to
the registration / unregistration cycle of a specific service listener. That is, registering the same service
listener again, even with a different filter, will automatically unregister the previouse registration and
the newly registered service listener is related to a different ListenerInfo object. ListenerInfo objects
can be stored in unordered containers and compared with each other, e.g. to match ServiceListenerHook::Added
and ServiceListenerHook::Removed calls.

The Listener Hooks are called synchronously in the same order of their registration. However, in rare cases
the removal of a service listener may be reported before its corresponding addition. To handle this case,
the ListenerInfo::IsRemoved() method is provided which can be used in the ServiceListenerHook::Added
method to detect the out of order delivery. A simple strategy is to ignore removed events without
corresponding added events and ignore added events where the ListenerInfo object is already removed:

\snippet uServices-servicelistenerhook/main.cpp 1

## Architectural Notes

### Ordinary Services

All service hooks are treated as ordinary services. If the CppMicroServices library uses them, their
Service References will show that the CppMicroServices modules is using them, and if a hook is a
Service Factory, then the actual instance will be properly created.

The only speciality of the service hooks is that the CppMicroServices library does not use them for
the hooks themselves. That is, the Service Event and Service Find Hooks can not be used to hide the
services from the CppMicroServices library.

### Ordering

The hooks are very sensitive to ordering because they interact directly with the service registry.
In general, implementers of the hooks must be aware that other modules can be loaded before or after
the module which provides the hooks. To ensure early registration of the hooks, they should be registered
within the ModuleActivator::Load method of the program executable or a module being auto-loaded with
the executable.

### Multi Threading

All hooks must be thread-safe because the hooks can be called at any time. All hook methods must be
re-entrant, they can be entered at any time and in rare cases in the wrong order. The CppMicroServices
library calls all hook methods synchronously but the calls might be triggered from any user thread
interacting with the CppMicroServices API. The CppMicroServices API can be called from any of the
hook methods but implementers must be careful to not hold any lock while calling CppMicroServices methods.
