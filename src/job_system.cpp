#include "job_system.h"
#include "common.h"
#include "random.h"
#include "semaphore.h"

#include <limits>
#include <thread>
#include <mutex>
#include <vector>

namespace knight {
namespace JobSystem {

class WorkStealingQueue {
  static const int64_t kNumberOfJobs = 4096u;
  static const int64_t kMask = kNumberOfJobs - 1u;

 public:
  WorkStealingQueue()
    : bottom_{0},
      top_{0} { }

  void push(Job *job);
  Job *pop();
  Job *steal();

 private:
  std::mutex mutex_;
  Job *jobs_[kNumberOfJobs];
  std::atomic<int64_t> bottom_;
  std::atomic<int64_t> top_;
};

namespace {
  std::vector<std::thread> work_threads;
  std::vector<WorkStealingQueue *> job_queues;
  std::vector<Semaphore *> wait_events;

  bool worker_thread_active;

  Semaphore job_ready;
  Semaphore job_finished{0, 1};

  uint32_t get_thread_index() {
    static std::atomic<uint32_t> thread_count;
    thread_local uint32_t thread_index = thread_count++;
    return thread_index;
  }

  Job *allocate_job() {
    const uint32_t kMaxJobCount = 4096;
    thread_local Job job_allocator[kMaxJobCount];
    thread_local uint32_t allocated_jobs = 0u;
    auto index = allocated_jobs++;
    return &job_allocator[index & (kMaxJobCount-1u)];
  }

  WorkStealingQueue *get_worker_thread_queue() {
    return job_queues[get_thread_index()];
  }

  bool is_empty_job(const Job *job) {
    return job == nullptr || has_job_completed(job);
  }

  Job *get_job() {
    auto *queue = get_worker_thread_queue();
    auto *job = queue->pop();

    if (is_empty_job(job)) {
      auto index = 0;//random_in_range(0, (int)(job_queues.size() - 1));
      auto *steal_queue = job_queues[index];
      if (steal_queue == queue) {
        return nullptr;
      }

      auto *stolen_job = steal_queue->steal();
      if (is_empty_job(stolen_job)) {
        return nullptr;
      }

      return stolen_job;
    }

    return job;
  }

  void notify_all_waiting() {
    for (auto &&wait_event : wait_events) {
      wait_event->notify();
    }
  }

  void finish(Job *job) {
    const int32_t unfinished_jobs = --job->unfinished_jobs;

    if (unfinished_jobs == 0 && job->parent != nullptr) {
      finish(job->parent);
    }

    notify_all_waiting();
  }

  void execute(Job *job) {
    (job->function)(job, job->data);
    finish(job);
  }

  void worker_thread(Semaphore &ready, bool &worker_thread_active) {
    ready.notify();
    while(worker_thread_active) {
      auto *job = get_job();
      if (job != nullptr) {
        execute(job);
      }

      job_ready.wait();
    }
  }
} // namespace

void WorkStealingQueue::push(Job *job) {
  auto bottom = bottom_.load(std::memory_order_relaxed);
  jobs_[bottom & kMask] = job;

  bottom_.store(bottom + 1, std::memory_order_release);

  job_ready.notify();
  notify_all_waiting();
}

Job *WorkStealingQueue::pop() {
  auto bottom = bottom_.fetch_sub(1, std::memory_order_seq_cst) - 1;
  auto top = top_.load(std::memory_order_relaxed);
  if (top <= bottom) {
    auto *job = jobs_[bottom & kMask];
    if (top != bottom) {
      return job;
    }

    if (!top_.compare_exchange_strong(top, top + 1)) {
      job = nullptr;
    }

    bottom_.store(top + 1, std::memory_order_relaxed);
    return job;
  } else {
    bottom_.store(top, std::memory_order_relaxed);
    return nullptr;
  }
}

Job *WorkStealingQueue::steal() {
  auto top = top_.load(std::memory_order_relaxed);
  auto bottom = bottom_.load(std::memory_order_acquire);
  if (top < bottom) {
    auto *job = jobs_[top & kMask];
    if (!top_.compare_exchange_strong(top, top + 1)) {
      return nullptr;
    }
    return job;
  }

  return nullptr;
}

void initialize() {
  int worker_thread_count = std::thread::hardware_concurrency();

  get_thread_index();

  job_queues.emplace_back(new WorkStealingQueue());

  for (int i = 0; i < worker_thread_count + 1; i++) {
    job_queues.emplace_back(new WorkStealingQueue{});
    wait_events.emplace_back(new Semaphore{0, 1});
  }

  Semaphore workers_ready{1 - worker_thread_count};

  worker_thread_active = true;
  for (int i = 0; i < worker_thread_count; i++) {
    work_threads.emplace_back(worker_thread, std::ref(workers_ready), std::ref(worker_thread_active));
  }

  workers_ready.wait();
}

void shutdown() {
  worker_thread_active = false;

  for (auto i = 0u; i < work_threads.size(); ++i) {
    job_ready.notify();
  }

  for (auto &&thread : work_threads) {
    thread.join();
  }

  work_threads.clear();

  for (auto &&queue : job_queues) {
    delete queue;
  }

  for (auto &&wait_event : wait_events) {
    delete wait_event;
  }
}

bool has_job_completed(const Job *job) {
  return job->unfinished_jobs == 0;
}

Job *create_job(JobFunction function) {
  auto job = allocate_job();
  job->function = function;
  job->parent = nullptr;
  job->unfinished_jobs = 1;
  return job;
}

Job *create_job_as_child(Job *parent, JobFunction function) {
  parent->unfinished_jobs++;

  auto job = create_job(function);
  job->parent = parent;

  return job;
}

void run(Job *job) {
  auto *queue = get_worker_thread_queue();
  queue->push(job);
}

void wait(const Job *job) {
  while (!has_job_completed(job)) {
    auto *next_job = get_job();
    if (next_job != nullptr) {
      execute(next_job);
    }

    wait_events[get_thread_index()]->wait();
  }
}

} // namespace JobSystem
} // namespace knight
