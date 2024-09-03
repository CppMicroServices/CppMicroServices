/*=============================================================================

  Library: CppMicroServices


Copyright Kevlin Henney, 2000, 2001, 2002. All rights reserved.
Extracted from Boost 1.46.1 and adapted for CppMicroServices.

Permission is hereby granted, free of charge, to any person or organization
obtaining a copy of the software and accompanying documentation covered by
this license (the "Software") to use, reproduce, display, distribute,
execute, and transmit the Software, and to prepare derivative works of the
Software, and to permit third-parties to whom the Software is furnished to
do so, all subject to the following:

The copyright notices in the Software and this entire statement, including
the above license grant, this restriction and the following disclaimer,
must be included in all copies of the Software, in whole or in part, and
all derivative works of the Software, unless such copies or derivative
works are solely in the form of machine-executable object code generated by
a source language processor.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.

=========================================================================*/

#ifndef CPPMICROSERVICES_THREADPOOLSAFEFUTURE_H
#define CPPMICROSERVICES_THREADPOOLSAFEFUTURE_H

#include "cppmicroservices/FrameworkExport.h"
#include <cstdint>
#include <future>

namespace cppmicroservices
{
    using ActualTask = std::packaged_task<void(bool)>;
    using PostTask = std::packaged_task<void()>;
    class US_Framework_EXPORT ThreadpoolSafeFuture
    {
      public:
        ThreadpoolSafeFuture(ThreadpoolSafeFuture const& other) = default;
        ThreadpoolSafeFuture& operator=(ThreadpoolSafeFuture const& other) = default;
        ThreadpoolSafeFuture(ThreadpoolSafeFuture&& other) noexcept;
        ThreadpoolSafeFuture& operator=(ThreadpoolSafeFuture&& other) noexcept = default;
        virtual ~ThreadpoolSafeFuture();

        /**
         * Compares this \c ThreadpoolSafeFuture object with the specified ThreadpoolSafeFuture.
         *
         * Valid \c ThreadpoolSafeFuture objects compare equal if and only if they
         * are installed in the same framework instance and their
         * ThreadpoolSafeFuturePrivate is equal. Invalid \c ThreadpoolSafeFuture objects are always
         * considered to be equal.
         *
         * @param rhs The \c ThreadpoolSafeFuture object to compare this object with.
         * @return \c true if this \c ThreadpoolSafeFuture object is equal to \c rhs,
         *         \c false otherwise.
         */
        bool operator==(ThreadpoolSafeFuture const& rhs) const = delete;

        /**
         * Compares this \c ThreadpoolSafeFuture object with the specified ThreadpoolSafeFuture
         * for inequality.
         *
         * @param rhs The \c ThreadpoolSafeFuture object to compare this object with.
         * @return Returns the result of <code>!(*this == rhs)</code>.
         */
        bool operator!=(ThreadpoolSafeFuture const& rhs) const = delete;

        bool operator<(ThreadpoolSafeFuture const& rhs) const = delete;

        /**
         * Tests this %ThreadpoolSafeFuture object for validity.
         *
         * Invalid \c ThreadpoolSafeFuture objects are created by the default constructor.
         *
         * @return \c true if this %ThreadpoolSafeFuture object is valid and can safely be used,
         *         \c false otherwise.
         */
        explicit operator bool() const;

        /**
         * Releases any resources held or locked by this
         * \c ThreadpoolSafeFuture and renders it invalid.
         */
        ThreadpoolSafeFuture& operator=(std::nullptr_t);

        // Method to get the result
        virtual void get() const = 0;
        virtual void wait() const = 0;
        virtual std::future_status wait_for(std::uint32_t const& timeout_duration_ms) const = 0;

      protected:
        ThreadpoolSafeFuture();
    };
} // namespace cppmicroservices

#endif // CPPMICROSERVICES_THREADPOOLSAFEFUTURE_H
