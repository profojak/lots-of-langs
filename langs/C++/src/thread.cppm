module;

#include <condition_variable>
#include <cstddef>
#include <functional>
#include <future>
#include <mutex>
#include <queue>
#include <thread>
#include <type_traits>
#include <utility>
#include <vector>

export module thread;

namespace pbf {

export class Threads {
  std::vector<std::jthread> workers;
  std::queue<std::function<void()>> tasks;
  std::condition_variable task_available;
  mutable std::mutex mutex;
  bool stopping = false;

  void WorkerLoop() {
    while (true) {
      std::function<void()> task;
      {
        std::unique_lock lock(mutex);
        task_available.wait(lock,
                            [this] { return stopping || !tasks.empty(); });
        if (stopping && tasks.empty())
          return;
        task = std::move(tasks.front());
        tasks.pop();
      }
      task();
    }
  }

public:
  explicit Threads(auto thread_count = std::thread::hardware_concurrency()) {
    if (thread_count == 0)
      thread_count = 1;
    workers.reserve(thread_count);
    for (std::size_t i = 0; i < thread_count; ++i)
      workers.emplace_back([this] { WorkerLoop(); });
  }

  Threads(const Threads &) = delete;
  Threads &operator=(const Threads &) = delete;
  Threads(Threads &&) = delete;
  Threads &operator=(Threads &&) = delete;

  ~Threads() {
    {
      std::lock_guard lock(mutex);
      stopping = true;
    }
    task_available.notify_all();
  }

  template <typename F>
    requires std::invocable<F &>
  [[nodiscard]] std::future<std::invoke_result_t<F &>> Submit(F &&function) {
    using R = std::invoke_result_t<F &>;
    auto task =
        std::make_shared<std::packaged_task<R()>>(std::forward<F>(function));
    std::future<R> future = task->get_future();
    {
      std::lock_guard lock(mutex);
      tasks.emplace([task] { (*task)(); });
    }
    task_available.notify_one();
    return future;
  }

  template <typename F>
    requires std::invocable<F &, std::size_t, std::size_t>
  void ParallelFor(std::size_t count, F &&function) {
    if (workers.size() <= 1) {
      function(0, count);
      return;
    }

    std::size_t chunk = count / workers.size();
    if (chunk == 0)
      chunk = 1;

    std::vector<std::future<void>> futures;
    futures.reserve(workers.size());
    std::size_t begin = 0;
    for (; begin + chunk < count; begin += chunk)
      futures.push_back(Submit(
          [&function, begin, end = begin + chunk] { function(begin, end); }));
    function(begin, count);

    for (auto &future : futures)
      future.get();
  }
};

} // namespace pbf
