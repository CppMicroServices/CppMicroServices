/*=============================================================================

  Library: CppMicroServices

  Copyright (c) The CppMicroServices developers. See the COPYRIGHT
  file at the top-level directory of this distribution and at
  https://github.com/CppMicroServices/CppMicroServices/COPYRIGHT .

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
#ifndef CPPMICROSERVICES_GUARDEDOBJECT_H
#define CPPMICROSERVICES_GUARDEDOBJECT_H

#include <memory>
#include <mutex>

namespace cppmicroservices {
/**
         * Utility class to help developers avoid mistakes with locking data members
         * in a class. This code is based on the Guarded idiom as described in
         * "Mastering the C++17 STL" by Arthur O'Dwyer
         */
        template <class Data>
        class Guarded
        {
            std::mutex mtx;
            Data data;
            class Handle
            {
                std::unique_lock<std::mutex> lk;
                Data* ptr;

              public:
                Handle(std::unique_lock<std::mutex> lk, Data* p) : lk(std::move(lk)), ptr(p) {}

                Handle(Handle const&) = delete;
                Handle& operator=(Handle const&) = delete;

                Handle(Handle&& rhs) : lk(std::move(rhs.lk)), ptr(std::move(rhs.ptr)) {}

                Handle&
                operator=(Handle&& rhs)
                {
                    lk = std::move(rhs.lk);
                    ptr = std::move(rhs.ptr);
                }

                ~Handle() = default;

                Data*
                operator->() const
                {
                    return ptr;
                }
                Data&
                operator*() const
                {
                    return *ptr;
                }
            };

          public:
            /**
             * Locks the member mutex and returns an RAII object responsible for
             * unlocking the mutex.
             *
             * \return a RAII style wrapper object used to access the data
             */
            Handle
            lock()
            {
                std::unique_lock<std::mutex> lock(mtx);
                return Handle { std::move(lock), &data };
            }
        };
}

#endif