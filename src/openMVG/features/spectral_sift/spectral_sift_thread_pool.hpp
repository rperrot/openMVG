// This file is part of OpenMVG, an Open Multiple View Geometry C++ library.

// Copyright (c) 2026 openMVG authors.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENMVG_FEATURES_SPECTRAL_SIFT_SPECTRAL_SIFT_THREAD_POOL_HPP
#define OPENMVG_FEATURES_SPECTRAL_SIFT_SPECTRAL_SIFT_THREAD_POOL_HPP

#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <atomic>
#include <algorithm>

namespace openMVG {
namespace features {
namespace spectral_sift {

/**
 * @brief Thread-local flag identifying whether the current thread is a worker of a ThreadPool.
 *        Used to safely fall back to sequential execution and avoid deadlocks in nested parallel loops.
 */
inline bool & IsWorkerThread()
{
  static thread_local bool is_worker = false;
  return is_worker;
}

/**
 * @brief Lightweight, persistent C++11 thread pool for CPU multi-threading.
 *        Ensures cross-platform parallelism (particularly on macOS where OpenMP is absent by default).
 */
class ThreadPool
{
public:
  explicit ThreadPool(size_t num_threads = 0)
    : stop_(false)
    , active_tasks_(0)
  {
    if (num_threads == 0)
    {
      num_threads = std::thread::hardware_concurrency();
      if (num_threads == 0)
      {
        num_threads = 4;
      }
    }

    workers_.reserve(num_threads);
    for (size_t i = 0; i < num_threads; ++i)
    {
      workers_.emplace_back([this]() {
        IsWorkerThread() = true;
        while (true)
        {
          std::function<void()> task;
          {
            std::unique_lock<std::mutex> lock(this->queue_mutex_);
            this->cv_task_.wait(lock, [this]() {
              return this->stop_ || !this->tasks_.empty();
            });

            if (this->stop_ && this->tasks_.empty())
            {
              return;
            }

            task = std::move(this->tasks_.front());
            this->tasks_.pop();
          }

          task();

          {
            std::unique_lock<std::mutex> lock(this->queue_mutex_);
            --this->active_tasks_;
            if (this->tasks_.empty() && this->active_tasks_ == 0)
            {
              this->cv_finished_.notify_all();
            }
          }
        }
      });
    }
  }

  // Non-copyable, non-movable
  ThreadPool(const ThreadPool &) = delete;
  ThreadPool & operator=(const ThreadPool &) = delete;

  /**
   * @brief Destructor: joins all worker threads cleanly and frees resources
   */
  ~ThreadPool()
  {
    Shutdown();
  }

  /**
   * @brief Gracefully shut down all worker threads and join them.
   */
  void Shutdown()
  {
    {
      std::unique_lock<std::mutex> lock(queue_mutex_);
      if (stop_)
      {
        return;
      }
      stop_ = true;
    }
    cv_task_.notify_all();
    cv_finished_.notify_all();

    for (std::thread & worker : workers_)
    {
      if (worker.joinable())
      {
        worker.join();
      }
    }
    workers_.clear();
  }

  /**
   * @brief Enqueue a task for asynchronous execution
   */
  void Enqueue(std::function<void()> task)
  {
    {
      std::unique_lock<std::mutex> lock(queue_mutex_);
      if (stop_)
      {
        return;
      }
      ++active_tasks_;
      tasks_.push(std::move(task));
    }
    cv_task_.notify_one();
  }

  /**
   * @brief Block until all queued and active tasks are completed
   */
  void Wait()
  {
    std::unique_lock<std::mutex> lock(queue_mutex_);
    cv_finished_.wait(lock, [this]() {
      return this->stop_ || (this->tasks_.empty() && this->active_tasks_ == 0);
    });
  }

  size_t NumThreads() const
  {
    return workers_.size();
  }

  /**
   * @brief Global persistent singleton instance of ThreadPool
   */
  static ThreadPool & GlobalInstance()
  {
    static ThreadPool instance;
    return instance;
  }

private:
  std::vector<std::thread> workers_;
  std::queue<std::function<void()>> tasks_;

  std::mutex queue_mutex_;
  std::condition_variable cv_task_;
  std::condition_variable cv_finished_;

  bool stop_;
  size_t active_tasks_;
};

/**
 * @brief Helper template function to parallelize a 1D loop range [start, end) across thread pool workers
 * @param start Start index (inclusive)
 * @param end End index (exclusive)
 * @param fn Functor taking (int index)
 * @param pool Pointer to thread pool (uses GlobalInstance() if nullptr)
 */
template<typename Function>
inline void ParallelFor(int start, int end, Function fn, ThreadPool * pool = nullptr)
{
  if (start >= end)
  {
    return;
  }

  ThreadPool & tp = (pool != nullptr) ? *pool : ThreadPool::GlobalInstance();
  const int total_items = end - start;
  const int num_workers = static_cast<int>(tp.NumThreads());

  if (IsWorkerThread() || num_workers <= 1 || total_items <= 1)
  {
    for (int i = start; i < end; ++i)
    {
      fn(i);
    }
    return;
  }

  const int num_chunks = std::min(num_workers, total_items);
  const int chunk_size = (total_items + num_chunks - 1) / num_chunks;

  for (int c = 0; c < num_chunks; ++c)
  {
    const int c_start = start + c * chunk_size;
    const int c_end = std::min(end, c_start + chunk_size);
    if (c_start >= c_end)
    {
      continue;
    }

    tp.Enqueue([c_start, c_end, &fn]() {
      for (int i = c_start; i < c_end; ++i)
      {
        fn(i);
      }
    });
  }

  tp.Wait();
}

/**
 * @brief Helper template function that provides thread_id / worker_id to each chunk
 * @param start Start index (inclusive)
 * @param end End index (exclusive)
 * @param fn Functor taking (int index, int worker_id)
 * @param pool Pointer to thread pool (uses GlobalInstance() if nullptr)
 */
template<typename Function>
inline void ParallelForWithId(int start, int end, Function fn, ThreadPool * pool = nullptr)
{
  if (start >= end)
  {
    return;
  }

  ThreadPool & tp = (pool != nullptr) ? *pool : ThreadPool::GlobalInstance();
  const int total_items = end - start;
  const int num_workers = static_cast<int>(tp.NumThreads());

  if (IsWorkerThread() || num_workers <= 1 || total_items <= 1)
  {
    for (int i = start; i < end; ++i)
    {
      fn(i, 0);
    }
    return;
  }

  const int num_chunks = std::min(num_workers, total_items);
  const int chunk_size = (total_items + num_chunks - 1) / num_chunks;

  for (int c = 0; c < num_chunks; ++c)
  {
    const int c_start = start + c * chunk_size;
    const int c_end = std::min(end, c_start + chunk_size);
    if (c_start >= c_end)
    {
      continue;
    }

    tp.Enqueue([c, c_start, c_end, &fn]() {
      for (int i = c_start; i < c_end; ++i)
      {
        fn(i, c);
      }
    });
  }

  tp.Wait();
}

} // namespace spectral_sift
} // namespace features
} // namespace openMVG

#endif // OPENMVG_FEATURES_SPECTRAL_SIFT_SPECTRAL_SIFT_THREAD_POOL_HPP
