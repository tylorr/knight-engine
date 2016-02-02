#pragma once

#include <mutex>
#include <condition_variable>
#include <limits>
#include <algorithm>

namespace knight {

class Semaphore {
 public:
  Semaphore(int count = 0, int max = std::numeric_limits<int>::max())
    : count_{count},
      max_{max} { }

  void notify() {
    {
      std::lock_guard<std::mutex> lock{mutex_};
      count_ = std::min(count_ + 1, max_);
    }
    condition_.notify_one();
  }

  void wait() {
    std::unique_lock<std::mutex> lock{mutex_};
    condition_.wait(lock, [this]{ return count_ > 0; });
    count_--;
  }

  bool try_wait() {
    std::lock_guard<std::mutex> lock{mutex_};
    if (count_ > 0) {
      count_--;
      return true;
    } else {
      return false;
    }
  }

 private:
  std::mutex mutex_;
  std::condition_variable condition_;
  int count_;
  const int max_;
};

} // namespace knight
