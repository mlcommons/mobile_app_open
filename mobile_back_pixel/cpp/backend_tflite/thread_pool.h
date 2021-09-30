/* Copyright 2021 The MLPerf Authors. All Rights Reserved.

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
/* Copyright (c) 2012 Jakob Progsch, Václav Zeman

This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
  claim that you wrote the original software. If you use this software
  in a product, an acknowledgment in the product documentation would be
  appreciated but is not required.

  2. Altered source versions must be plainly marked as such, and must not be
  misrepresented as being the original software.

  3. This notice may not be removed or altered from any source
  distribution.
==============================================================================*/
// reference:
// https://github.com/progschj/ThreadPool

#ifndef THREAD_POOL_H_
#define THREAD_POOL_H_

#include <atomic>
#include <condition_variable>
#include <functional>
#include <future>
#include <iostream>
#include <queue>
#include <stdexcept>
#include <thread>

class Threadpool {
 private:
  std::vector<std::thread> pool;
  std::queue<std::function<void()>> tasks;
  // synchronization variable
  std::mutex m_lock;
  std::condition_variable cond_var;
  std::atomic<bool> available;

 public:
  inline Threadpool(size_t thread_num) : available(true) {
    for (size_t i = 0; i < thread_num; i++) {
      pool.emplace_back([this] {
        while (available.load()) {
          std::function<void()> picked_task;
          // consumer in critical section
          {
            std::unique_lock<std::mutex> lock(this->m_lock);
            auto wait_until = [this]() -> bool {
              return !this->available.load() || !this->tasks.empty();
            };
            this->cond_var.wait(lock, wait_until);
            if (!this->available && this->tasks.empty()) return;
            picked_task = std::move(this->tasks.front());
            this->tasks.pop();
          }
          // invoke picked task
          picked_task();
        }
      });
    }
  }
  ~Threadpool() {
    available.store(false);
    cond_var.notify_all();
    for (auto& worker : pool) {
      if (worker.joinable()) worker.join();
    }
  }

  template <typename Fn, typename... Args>
  auto submit(Fn&& fn, Args&&... args) -> std::future<decltype(fn(args...))> {
    if (!available.load()) {
      std::cerr << "don't accept commit after stop." << std::endl;
      assert(false);
    }
    using FnReturnType = decltype(fn(args...));
    auto enq_task = std::make_shared<std::packaged_task<FnReturnType()>>(
        std::bind(std::forward<Fn>(fn), std::forward<Args>(args)...));
    std::future<FnReturnType> ret = enq_task->get_future();
    // producer in critical section
    {
      std::lock_guard<std::mutex> lock(m_lock);
      tasks.emplace([enq_task]() { (*enq_task)(); });
    }
    cond_var.notify_one();
    return ret;
  }
};
#endif  // THREAD_POOL_H_
