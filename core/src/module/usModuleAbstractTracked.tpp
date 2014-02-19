/*=============================================================================

  Library: CppMicroServices

  Copyright (c) German Cancer Research Center,
    Division of Medical and Biological Informatics

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

=============================================================================*/

#include <usUtils_p.h>
#include <iterator>

US_BEGIN_NAMESPACE

template<class S, class TTT, class R>
const bool ModuleAbstractTracked<S,TTT,R>::DEBUG_OUTPUT = false;

template<class S, class TTT, class R>
ModuleAbstractTracked<S,TTT,R>::ModuleAbstractTracked()
{
  closed = false;
}

template<class S, class TTT, class R>
ModuleAbstractTracked<S,TTT,R>::~ModuleAbstractTracked()
{

}

template<class S, class TTT, class R>
void ModuleAbstractTracked<S,TTT,R>::SetInitial(const std::vector<S>& initiallist)
{
  std::copy(initiallist.begin(), initiallist.end(), std::back_inserter(initial));

  if (DEBUG_OUTPUT)
  {
    for(typename std::list<S>::const_iterator item = initial.begin();
      item != initial.end(); ++item)
    {
      US_DEBUG << "ModuleAbstractTracked::setInitial: " << (*item);
    }
  }
}

template<class S, class TTT, class R>
void ModuleAbstractTracked<S,TTT,R>::TrackInitial()
{
  while (true)
  {
    S item;
    {
      US_UNUSED(Lock(this));
      if (closed || (initial.size() == 0))
      {
        /*
         * if there are no more initial items
         */
        return; /* we are done */
      }
      /*
       * move the first item from the initial list to the adding list
       * within this synchronized block.
       */
      item = initial.front();
      initial.pop_front();
      if (TTT::IsValid(tracked[item]))
      {
        /* if we are already tracking this item */
        US_DEBUG(DEBUG_OUTPUT) << "ModuleAbstractTracked::trackInitial[already tracked]: " << item;
        continue; /* skip this item */
      }
      if (std::find(adding.begin(), adding.end(), item) != adding.end())
      {
        /*
         * if this item is already in the process of being added.
         */
        US_DEBUG(DEBUG_OUTPUT) << "ModuleAbstractTracked::trackInitial[already adding]: " << item;
        continue; /* skip this item */
      }
      adding.push_back(item);
    }
    US_DEBUG(DEBUG_OUTPUT) << "ModuleAbstractTracked::trackInitial: " << item;
    TrackAdding(item, R());
    /*
     * Begin tracking it. We call trackAdding
     * since we have already put the item in the
     * adding list.
     */
  }
}

template<class S, class TTT, class R>
void ModuleAbstractTracked<S,TTT,R>::Close()
{
  closed = true;
}

template<class S, class TTT, class R>
void ModuleAbstractTracked<S,TTT,R>::Track(S item, R related)
{
  T object = TTT::DefaultValue();
  {
    US_UNUSED(Lock(this));
    if (closed)
    {
      return;
    }
    object = tracked[item];
    if (!TTT::IsValid(object))
    { /* we are not tracking the item */
      if (std::find(adding.begin(), adding.end(),item) != adding.end())
      {
        /* if this item is already in the process of being added. */
        US_DEBUG(DEBUG_OUTPUT) << "ModuleAbstractTracked::track[already adding]: " << item;
        return;
      }
      adding.push_back(item); /* mark this item is being added */
    }
    else
    { /* we are currently tracking this item */
      US_DEBUG(DEBUG_OUTPUT) << "ModuleAbstractTracked::track[modified]: " << item;
      Modified(); /* increment modification count */
    }
  }

  if (!TTT::IsValid(object))
  { /* we are not tracking the item */
    TrackAdding(item, related);
  }
  else
  {
    /* Call customizer outside of synchronized region */
    CustomizerModified(item, related, object);
    /*
     * If the customizer throws an unchecked exception, it is safe to
     * let it propagate
     */
  }
}

template<class S, class TTT, class R>
void ModuleAbstractTracked<S,TTT,R>::Untrack(S item, R related)
{
  T object = TTT::DefaultValue();
  {
    US_UNUSED(Lock(this));
    std::size_t initialSize = initial.size();
    initial.remove(item);
    if (initialSize != initial.size())
    { /* if this item is already in the list
       * of initial references to process
       */
      US_DEBUG(DEBUG_OUTPUT) << "ModuleAbstractTracked::untrack[removed from initial]: " << item;
      return; /* we have removed it from the list and it will not be
               * processed
               */
    }

    std::size_t addingSize = adding.size();
    adding.remove(item);
    if (addingSize != adding.size())
    { /* if the item is in the process of
       * being added
       */
      US_DEBUG(DEBUG_OUTPUT) << "ModuleAbstractTracked::untrack[being added]: " << item;
      return; /*
           * in case the item is untracked while in the process of
           * adding
           */
    }
    object = tracked[item];
    /*
     * must remove from tracker before
     * calling customizer callback
     */
    tracked.erase(item);
    if (!TTT::IsValid(object))
    { /* are we actually tracking the item */
      return;
    }
    Modified(); /* increment modification count */
  }
  US_DEBUG(DEBUG_OUTPUT) << "ModuleAbstractTracked::untrack[removed]: " << item;
  /* Call customizer outside of synchronized region */
  CustomizerRemoved(item, related, object);
  /*
   * If the customizer throws an unchecked exception, it is safe to let it
   * propagate
   */
}

template<class S, class TTT, class R>
std::size_t ModuleAbstractTracked<S,TTT,R>::Size() const
{
  return tracked.size();
}

template<class S, class TTT, class R>
bool ModuleAbstractTracked<S,TTT,R>::IsEmpty() const
{
  return tracked.empty();
}

template<class S, class TTT, class R>
typename ModuleAbstractTracked<S,TTT,R>::T
ModuleAbstractTracked<S,TTT,R>::GetCustomizedObject(S item) const
{
  typename TrackingMap::const_iterator i = tracked.find(item);
  if (i != tracked.end()) return i->second;
  return T();
}

template<class S, class TTT, class R>
void ModuleAbstractTracked<S,TTT,R>::GetTracked(std::vector<S>& items) const
{
  for (typename TrackingMap::const_iterator i = tracked.begin();
       i != tracked.end(); ++i)
  {
    items.push_back(i->first);
  }
}

template<class S, class TTT, class R>
void ModuleAbstractTracked<S,TTT,R>::Modified()
{
  trackingCount.Ref();
}

template<class S, class TTT, class R>
int ModuleAbstractTracked<S,TTT,R>::GetTrackingCount() const
{
  return trackingCount;
}

template<class S, class TTT, class R>
void ModuleAbstractTracked<S,TTT,R>::CopyEntries(TrackingMap& map) const
{
  map.insert(tracked.begin(), tracked.end());
}

template<class S, class TTT, class R>
bool ModuleAbstractTracked<S,TTT,R>::CustomizerAddingFinal(S item, const T& custom)
{
  US_UNUSED(Lock(this));
  std::size_t addingSize = adding.size();
  adding.remove(item);
  if (addingSize != adding.size() && !closed)
  {
    /*
     * if the item was not untracked during the customizer
     * callback
     */
    if (TTT::IsValid(custom))
    {
      tracked[item] = custom;
      Modified(); /* increment modification count */
      this->NotifyAll(); /* notify any waiters */
    }
    return false;
  }
  else
  {
    return true;
  }
}

template<class S, class TTT, class R>
void ModuleAbstractTracked<S,TTT,R>::TrackAdding(S item, R related)
{
  US_DEBUG(DEBUG_OUTPUT) << "ModuleAbstractTracked::trackAdding:" << item;
  T object = TTT::DefaultValue();
  bool becameUntracked = false;
  /* Call customizer outside of synchronized region */
  try
  {
    object = CustomizerAdding(item, related);
    becameUntracked = this->CustomizerAddingFinal(item, object);
  }
  catch (...)
  {
    /*
     * If the customizer throws an exception, it will
     * propagate after the cleanup code.
     */
    this->CustomizerAddingFinal(item, object);
    throw;
  }

  /*
   * The item became untracked during the customizer callback.
   */
  if (becameUntracked && TTT::IsValid(object))
  {
    US_DEBUG(DEBUG_OUTPUT) << "ModuleAbstractTracked::trackAdding[removed]: " << item;
    /* Call customizer outside of synchronized region */
    CustomizerRemoved(item, related, object);
    /*
     * If the customizer throws an unchecked exception, it is safe to
     * let it propagate
     */
  }
}

US_END_NAMESPACE
