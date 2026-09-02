module;

#include <algorithm>
#include <condition_variable>
#include <cstddef>
#include <functional>
#include <latch>
#include <mutex>
#include <queue>
#include <stop_token>
#include <thread>
#include <type_traits>
#include <utility>
#include <vector>

export module thread;

namespace pbf {

export class Threads {
  mutable std::mutex mutex;
  std::condition_variable_any task_available;
  std::queue<std::function<void()>> tasks;
  std::vector<std::jthread> workers;

  void WorkerLoop(std::stop_token stop) {
    while (true) {
      std::function<void()> task;
      {
        std::unique_lock lock(mutex);
        task_available.wait(lock, stop, [this] { return !tasks.empty(); });
        if (tasks.empty())
          return;
        task = std::move(tasks.front());
        tasks.pop();
      }
      task();
    }
  }

  template <typename F> void Enqueue(F &&task) {
    {
      std::lock_guard lock(mutex);
      tasks.emplace(std::forward<F>(task));
    }
    task_available.notify_one();
  }

public:
  explicit Threads(std::size_t thread_count) {
    if (thread_count == 0)
      thread_count = std::thread::hardware_concurrency();
    workers.reserve(thread_count);
    for (std::size_t i = 0; i < thread_count; ++i)
      workers.emplace_back([this](std::stop_token stop) { WorkerLoop(stop); });
  }

  Threads(const Threads &) = delete;
  Threads &operator=(const Threads &) = delete;
  Threads(Threads &&) = delete;
  Threads &operator=(Threads &&) = delete;

  template <typename F>
    requires std::is_nothrow_invocable_v<F &, std::size_t, std::size_t>
  void ParallelFor(std::size_t count, F &&task) {
    if (workers.size() <= 1 || count == 0) {
      if (count > 0)
        task(0, count);
      return;
    }

    const std::size_t chunk = std::max<std::size_t>(1, count / workers.size());
    const std::size_t chunks = (count + chunk - 1) / chunk;

    std::latch done{static_cast<std::ptrdiff_t>(chunks)};

    std::size_t begin = 0;
    for (std::size_t c = 0; c + 1 < chunks; ++c, begin += chunk)
      Enqueue([&, begin, end = begin + chunk] {
        task(begin, end);
        done.count_down();
      });

    task(begin, count);
    done.count_down();
    done.wait();
  }
};

} // namespace pbf
