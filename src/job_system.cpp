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

  void Push(Job *job);
  Job *Pop();
  Job *Steal();

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

  uint32_t GetThreadIndex() {
    static std::atomic<uint32_t> thread_count;
    thread_local uint32_t thread_index = thread_count++;
    return thread_index;
  }

  Job *AllocateJob() {
    const uint32_t kMaxJobCount = 4096;
    thread_local Job job_allocator[kMaxJobCount];
    thread_local uint32_t allocated_jobs = 0u;
    auto index = allocated_jobs++;
    return &job_allocator[index & (kMaxJobCount-1u)];
  }

  WorkStealingQueue *GetWorkerThreadQueue() {
    return job_queues[GetThreadIndex()];
  }

  bool IsEmptyJob(const Job *job) {
    return job == nullptr || HasJobCompleted(job);
  }

  Job *GetJob() {
    auto *queue = GetWorkerThreadQueue();
    auto *job = queue->Pop();

    if (IsEmptyJob(job)) {
      auto index = 0;//random_in_range(0, (int)(job_queues.size() - 1));
      auto *steal_queue = job_queues[index];
      if (steal_queue == queue) {
        return nullptr;
      }

      auto *stolen_job = steal_queue->Steal();
      if (IsEmptyJob(stolen_job)) {
        return nullptr;
      }

      return stolen_job;
    }

    return job;
  }

  void NotifyAllWaiting() {
    for (auto &&wait_event : wait_events) {
      wait_event->Notify();
    }
  }

  void Finish(Job *job) {
    const int32_t unfinished_jobs = --job->unfinished_jobs;

    if (unfinished_jobs == 0 && job->parent != nullptr) {
      Finish(job->parent);
    }

    NotifyAllWaiting();
  }

  void Execute(Job *job) {
    (job->function)(job, job->data);
    Finish(job);
  }

  void WorkerThread(Semaphore &ready, bool &worker_thread_active) {
    ready.Notify();
    while(worker_thread_active) {
      auto *job = GetJob();
      if (job != nullptr) {
        Execute(job);
      }

      job_ready.Wait();
    }
  }
} // namespace

void WorkStealingQueue::Push(Job *job) {
  auto bottom = bottom_.load(std::memory_order_relaxed);
  jobs_[bottom & kMask] = job;

  bottom_.store(bottom + 1, std::memory_order_release);

  job_ready.Notify();
  NotifyAllWaiting();
}

Job *WorkStealingQueue::Pop() {
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

Job *WorkStealingQueue::Steal() {
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

void Initialize() {
  int worker_thread_count = std::thread::hardware_concurrency();

  GetThreadIndex();

  job_queues.emplace_back(new WorkStealingQueue());

  for (int i = 0; i < worker_thread_count + 1; i++) {
    job_queues.emplace_back(new WorkStealingQueue{});
    wait_events.emplace_back(new Semaphore{0, 1});
  }

  Semaphore workers_ready{1 - worker_thread_count};

  worker_thread_active = true;
  for (int i = 0; i < worker_thread_count; i++) {
    work_threads.emplace_back(WorkerThread, std::ref(workers_ready), std::ref(worker_thread_active));
  }

  workers_ready.Wait();
}

void Shutdown() {
  worker_thread_active = false;

  for (auto i = 0u; i < work_threads.size(); ++i) {
    job_ready.Notify();
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

bool HasJobCompleted(const Job *job) {
  return job->unfinished_jobs == 0;
}

Job *CreateJob(JobFunction function) {
  auto job = AllocateJob();
  job->function = function;
  job->parent = nullptr;
  job->unfinished_jobs = 1;
  return job;
}

Job *CreateJobAsChild(Job *parent, JobFunction function) {
  parent->unfinished_jobs++;

  auto job = CreateJob(function);
  job->parent = parent;

  return job;
}

void Run(Job *job) {
  auto *queue = GetWorkerThreadQueue();
  queue->Push(job);
}

void Wait(const Job *job) {
  while (!HasJobCompleted(job)) {
    auto *next_job = GetJob();
    if (next_job != nullptr) {
      Execute(next_job);
    }

    wait_events[GetThreadIndex()]->Wait();
  }
}

} // namespace JobSystem
} // namespace knight
