/* Copyright 2018 The MLPerf Authors. All Rights Reserved.
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at
    http://www.apache.org/licenses/LICENSE-2.0
Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/

#ifndef COMMON_CLANG_THREADSAFETY_H_
#define COMMON_CLANG_THREADSAFETY_H_

#include <shared_mutex>

// Enable thread safety attributes only with clang.
// The attributes can be safely erased when compiling with other compilers.
#if defined(__clang__) && (!defined(SWIG))
#define THREAD_ANNOTATION_ATTRIBUTE__(x) __attribute__((x))
#else
#define THREAD_ANNOTATION_ATTRIBUTE__(x)  // no-op
#endif

#define CAPABILITY(x) THREAD_ANNOTATION_ATTRIBUTE__(capability(x))

#define SCOPED_CAPABILITY THREAD_ANNOTATION_ATTRIBUTE__(scoped_lockable)

#define GUARDED_BY(x) THREAD_ANNOTATION_ATTRIBUTE__(guarded_by(x))

#define PT_GUARDED_BY(x) THREAD_ANNOTATION_ATTRIBUTE__(pt_guarded_by(x))

#define ACQUIRED_BEFORE(...) \
  THREAD_ANNOTATION_ATTRIBUTE__(acquired_before(__VA_ARGS__))

#define ACQUIRED_AFTER(...) \
  THREAD_ANNOTATION_ATTRIBUTE__(acquired_after(__VA_ARGS__))

#define REQUIRES(...) \
  THREAD_ANNOTATION_ATTRIBUTE__(requires_capability(__VA_ARGS__))

#define REQUIRES_SHARED(...) \
  THREAD_ANNOTATION_ATTRIBUTE__(requires_shared_capability(__VA_ARGS__))

#define ACQUIRE(...) \
  THREAD_ANNOTATION_ATTRIBUTE__(acquire_capability(__VA_ARGS__))

#define ACQUIRE_SHARED(...) \
  THREAD_ANNOTATION_ATTRIBUTE__(acquire_shared_capability(__VA_ARGS__))

#define RELEASE(...) \
  THREAD_ANNOTATION_ATTRIBUTE__(release_capability(__VA_ARGS__))

#define RELEASE_SHARED(...) \
  THREAD_ANNOTATION_ATTRIBUTE__(release_shared_capability(__VA_ARGS__))

#define TRY_ACQUIRE(...) \
  THREAD_ANNOTATION_ATTRIBUTE__(try_acquire_capability(__VA_ARGS__))

#define TRY_ACQUIRE_SHARED(...) \
  THREAD_ANNOTATION_ATTRIBUTE__(try_acquire_shared_capability(__VA_ARGS__))

#define EXCLUDES(...) THREAD_ANNOTATION_ATTRIBUTE__(locks_excluded(__VA_ARGS__))

#define ASSERT_CAPABILITY(x) THREAD_ANNOTATION_ATTRIBUTE__(assert_capability(x))

#define ASSERT_SHARED_CAPABILITY(x) \
  THREAD_ANNOTATION_ATTRIBUTE__(assert_shared_capability(x))

#define RETURN_CAPABILITY(x) THREAD_ANNOTATION_ATTRIBUTE__(lock_returned(x))

#define NO_THREAD_SAFETY_ANALYSIS \
  THREAD_ANNOTATION_ATTRIBUTE__(no_thread_safety_analysis)

// Defines an annotated interface for mutexes.
// These methods can be implemented to use any internal mutex implementation.
// CAPABILITY is an attribute on classes, which specifies that objects of the
// class can be used as a capability.
// The analysis ensures that the calling thread cannot access the resource
// unless it has the capability
// The string argument specifies the kind of capability in error messages
class CAPABILITY("mutex") Mutex {
 private:
  std::shared_timed_mutex mut_;

 public:
  Mutex() {}

  // Acquire/lock this mutex exclusively.  Only one thread can have exclusive
  // access at any one time.  Write operations to guarded data require an
  // exclusive lock.
  void Lock() ACQUIRE() { mut_.lock(); }

  // Acquire/lock this mutex for read operations, which require only a shared
  // lock.  This assumes a multiple-reader, single writer semantics.  Multiple
  // threads may acquire the mutex simultaneously as readers, but a writer
  // must wait for all of them to release the mutex before it can acquire it
  // exclusively.
  void ReaderLock() ACQUIRE_SHARED() { mut_.lock_shared(); }

  // Release/unlock an exclusive mutex.
  void Unlock() RELEASE() { mut_.unlock(); }

  // Release/unlock a shared mutex.
  void ReaderUnlock() RELEASE_SHARED() { mut_.unlock_shared(); }

  // Try to acquire the mutex.  Returns true on success, and false on failure.
  void TryLock() TRY_ACQUIRE(true) { mut_.try_lock(); }

  // Try to acquire the mutex for read operations.
  void ReaderTryLock() TRY_ACQUIRE_SHARED(true) { mut_.try_lock_shared(); }
};

// MutexLocker is an RAII class that acquires a mutex in its constructor, and
// releases it in its destructor.
// SCOPED_CAPABILITY is an attribute on classes that implement RAII-style
// locking,
// in which a capability is acquired in the constructor, and released in the
// destructor.
class SCOPED_CAPABILITY MutexLocker {
 private:
  Mutex* mut;

 public:
  explicit MutexLocker(Mutex* mu) ACQUIRE(mu) : mut(mu) { mu->Lock(); }
  ~MutexLocker() RELEASE() { mut->Unlock(); }
};

// MutexLocker is an RAII class that acquires a mutex in its constructor, and
// releases it in its destructor.
// SCOPED_CAPABILITY is an attribute on classes that implement RAII-style
// locking,
// in which a capability is acquired in the constructor, and released in the
// destructor.
class SCOPED_CAPABILITY MutexReaderLocker {
 private:
  Mutex* mut;

 public:
  explicit MutexReaderLocker(Mutex* mu) ACQUIRE(mu) : mut(mu) {
    mu->ReaderLock();
  }
  ~MutexReaderLocker() RELEASE() { mut->ReaderUnlock(); }
};

#endif  // COMMON_CLANG_THREADSAFETY_H_
