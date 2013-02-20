////////////////////////////////////////////////////////////////////////////////
/// @brief input-output scheduler using libev
///
/// @file
///
/// DISCLAIMER
///
/// Copyright 2004-2012 triagens GmbH, Cologne, Germany
///
/// Licensed under the Apache License, Version 2.0 (the "License");
/// you may not use this file except in compliance with the License.
/// You may obtain a copy of the License at
///
///     http://www.apache.org/licenses/LICENSE-2.0
///
/// Unless required by applicable law or agreed to in writing, software
/// distributed under the License is distributed on an "AS IS" BASIS,
/// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
/// See the License for the specific language governing permissions and
/// limitations under the License.
///
/// Copyright holder is triAGENS GmbH, Cologne, Germany
///
/// @author Dr. Frank Celler
/// @author Achim Brandt
/// @author Copyright 2008-2012, triAGENS GmbH, Cologne, Germany
////////////////////////////////////////////////////////////////////////////////

#ifndef TRIAGENS_SCHEDULER_SCHEDULER_LIBEV_H
#define TRIAGENS_SCHEDULER_SCHEDULER_LIBEV_H 1

#include "Scheduler/Scheduler.h"

#include "BasicsC/locks.h"

// #define TRI_USE_SPIN_LOCK_SCHEDULER_LIBEV 1

// -----------------------------------------------------------------------------
// --SECTION--                                              class SchedulerLibev
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @addtogroup Scheduler
/// @{
////////////////////////////////////////////////////////////////////////////////

namespace triagens {
  namespace rest {

////////////////////////////////////////////////////////////////////////////////
/// @brief input-output scheduler using libev
////////////////////////////////////////////////////////////////////////////////

    class SchedulerLibev : public Scheduler {
      private:
        SchedulerLibev (SchedulerLibev const&);
        SchedulerLibev& operator= (SchedulerLibev const&);

////////////////////////////////////////////////////////////////////////////////
/// @}
////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------------------------------------
// --SECTION--                                             static public methods
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @addtogroup Scheduler
/// @{
////////////////////////////////////////////////////////////////////////////////

      public:

////////////////////////////////////////////////////////////////////////////////
/// @brief returns the available backends
////////////////////////////////////////////////////////////////////////////////

        static int availableBackends ();

////////////////////////////////////////////////////////////////////////////////
/// @brief switch the libev allocator
////////////////////////////////////////////////////////////////////////////////

        static void switchAllocator ();

////////////////////////////////////////////////////////////////////////////////
/// @}
////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------------------------------------
// --SECTION--                                      constructors and destructors
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @addtogroup Scheduler
/// @{
////////////////////////////////////////////////////////////////////////////////

      public:

////////////////////////////////////////////////////////////////////////////////
/// @brief creates a scheduler
////////////////////////////////////////////////////////////////////////////////

        explicit
        SchedulerLibev (size_t nrThreads = 1, int backend = BACKEND_AUTO);

////////////////////////////////////////////////////////////////////////////////
/// @brief deletes a scheduler
////////////////////////////////////////////////////////////////////////////////

        ~SchedulerLibev ();

////////////////////////////////////////////////////////////////////////////////
/// @}
////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------------------------------------
// --SECTION--                                                 Scheduler methods
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @addtogroup Scheduler
/// @{
////////////////////////////////////////////////////////////////////////////////

      public:

////////////////////////////////////////////////////////////////////////////////
/// {@inheritDoc}
////////////////////////////////////////////////////////////////////////////////

        void eventLoop (EventLoop);

////////////////////////////////////////////////////////////////////////////////
/// {@inheritDoc}
////////////////////////////////////////////////////////////////////////////////

        void wakeupLoop (EventLoop);

////////////////////////////////////////////////////////////////////////////////
/// {@inheritDoc}
////////////////////////////////////////////////////////////////////////////////

        EventToken installSocketEvent (EventLoop, EventType, Task*, socket_t);

////////////////////////////////////////////////////////////////////////////////
/// {@inheritDoc}
////////////////////////////////////////////////////////////////////////////////

        void startSocketEvents (EventToken);

////////////////////////////////////////////////////////////////////////////////
/// {@inheritDoc}
////////////////////////////////////////////////////////////////////////////////

        void stopSocketEvents (EventToken);

////////////////////////////////////////////////////////////////////////////////
/// {@inheritDoc}
////////////////////////////////////////////////////////////////////////////////

        EventToken installAsyncEvent (EventLoop, Task*);

////////////////////////////////////////////////////////////////////////////////
/// {@inheritDoc}
////////////////////////////////////////////////////////////////////////////////

        void sendAsync (EventToken);

////////////////////////////////////////////////////////////////////////////////
/// {@inheritDoc}
////////////////////////////////////////////////////////////////////////////////

        EventToken installTimerEvent (EventLoop, Task*, double timeout);

////////////////////////////////////////////////////////////////////////////////
/// {@inheritDoc}
////////////////////////////////////////////////////////////////////////////////

        void clearTimer (EventToken);

////////////////////////////////////////////////////////////////////////////////
/// {@inheritDoc}
////////////////////////////////////////////////////////////////////////////////

        void rearmTimer (EventToken, double timeout);

////////////////////////////////////////////////////////////////////////////////
/// {@inheritDoc}
////////////////////////////////////////////////////////////////////////////////

        EventToken installPeriodicEvent (EventLoop, Task*, double offset, double interval);

////////////////////////////////////////////////////////////////////////////////
/// {@inheritDoc}
////////////////////////////////////////////////////////////////////////////////

        void rearmPeriodic (EventToken, double offset, double timeout);

////////////////////////////////////////////////////////////////////////////////
/// {@inheritDoc}
////////////////////////////////////////////////////////////////////////////////

        EventToken installSignalEvent (EventLoop, Task*, int signal);

////////////////////////////////////////////////////////////////////////////////
/// {@inheritDoc}
////////////////////////////////////////////////////////////////////////////////

        void uninstallEvent (EventToken);

////////////////////////////////////////////////////////////////////////////////
/// @}
////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------------------------------------
// --SECTION--                                                   private methods
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @addtogroup Scheduler
/// @{
////////////////////////////////////////////////////////////////////////////////

      private:

////////////////////////////////////////////////////////////////////////////////
/// @brief looks up a watcher by event-token
////////////////////////////////////////////////////////////////////////////////

        void* lookupWatcher (EventToken);

////////////////////////////////////////////////////////////////////////////////
/// @brief looks up a watcher by event-token and event-type
////////////////////////////////////////////////////////////////////////////////

        void* lookupWatcher (EventToken, EventType&);

////////////////////////////////////////////////////////////////////////////////
/// @brief looks up an event lookup
////////////////////////////////////////////////////////////////////////////////

        void* lookupLoop (EventLoop);

////////////////////////////////////////////////////////////////////////////////
/// @brief registers a watcher
////////////////////////////////////////////////////////////////////////////////

        EventToken registerWatcher (void*, EventType);

////////////////////////////////////////////////////////////////////////////////
/// @brief unregisters a watcher
////////////////////////////////////////////////////////////////////////////////

        void unregisterWatcher (EventToken);

////////////////////////////////////////////////////////////////////////////////
/// @}
////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------------------------------------
// --SECTION--                                                 private variables
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @addtogroup Scheduler
/// @{
////////////////////////////////////////////////////////////////////////////////

      private:

////////////////////////////////////////////////////////////////////////////////
/// @brief backend to use
////////////////////////////////////////////////////////////////////////////////

        int _backend;

////////////////////////////////////////////////////////////////////////////////
/// @brief event loops
////////////////////////////////////////////////////////////////////////////////

        void* _loops;

////////////////////////////////////////////////////////////////////////////////
/// @brief event wakers
////////////////////////////////////////////////////////////////////////////////

        void* _wakers;

////////////////////////////////////////////////////////////////////////////////
/// @brief watchers lock
////////////////////////////////////////////////////////////////////////////////

#ifdef TRI_USE_SPIN_LOCK_SCHEDULER_LIBEV
        TRI_spin_t _watcherLock;
#else
        TRI_mutex_t _watcherLock;
#endif

////////////////////////////////////////////////////////////////////////////////
/// @brief watchers
////////////////////////////////////////////////////////////////////////////////

        vector<void*> _watchers;

////////////////////////////////////////////////////////////////////////////////
/// @brief free watchers
////////////////////////////////////////////////////////////////////////////////

        vector<size_t> _frees;

////////////////////////////////////////////////////////////////////////////////
/// @brief event-token to event-type
////////////////////////////////////////////////////////////////////////////////

        map<EventToken, EventType> _types;
    };
  }
}

////////////////////////////////////////////////////////////////////////////////
/// @}
////////////////////////////////////////////////////////////////////////////////

#endif

// Local Variables:
// mode: outline-minor
// outline-regexp: "^\\(/// @brief\\|/// {@inheritDoc}\\|/// @addtogroup\\|// --SECTION--\\|/// @\\}\\)"
// End:
