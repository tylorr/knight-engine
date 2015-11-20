#pragma once

#include "common.h"
#include "types.h"
#include "random.h"
#include "semaphore.h"

#include <memory.h>
#include <logog.hpp>

#include <limits>
#include <thread>
#include <mutex>
#include <atomic>
#include <vector>

namespace knight {

struct Job;

using JobFunction = void(*)(Job *, const void *);

// platform dependent
#define CACHE_LINE_SIZE 64

struct Job {
  alignas(CACHE_LINE_SIZE) JobFunction function;
  Job *parent;
  std::atomic<int32_t> unfinished_jobs;
  char data[
    CACHE_LINE_SIZE - 
    sizeof(JobFunction) - 
    sizeof(Job *) - 
    sizeof(std::atomic<int32_t>)];
};

namespace JobSystem {

class WorkStealingQueue {
  static const unsigned int kNumberOfJobs = 4096u;
  static const unsigned int kMask = kNumberOfJobs - 1u;

 public:
  WorkStealingQueue(std::condition_variable &job_ready) 
    : job_ready_{job_ready},
      bottom_{0}, 
      top_{0} { }

  void Push(Job *job) {
    std::lock_guard<std::mutex> guard(mutex_);
    jobs_[bottom_ & kMask] = job;
    ++bottom_;

    DBUG("Pushing new job");
    job_ready_.notify_all();
  }

  Job *Pop() {
    std::lock_guard<std::mutex> guard(mutex_);

    const int job_count = bottom_ - top_;
    if (job_count <= 0) {
      return nullptr;
    }

    --bottom_;
    return jobs_[bottom_ & kMask];
  }

  Job *Steal() {
    std::lock_guard<std::mutex> guard(mutex_);

    const int job_count = bottom_ - top_;
    if (job_count <= 0) {
      return nullptr;
    }

    auto *job = jobs_[top_ & kMask];
    ++top_;
    return job;
  }

 private:
  std::mutex mutex_;
  std::condition_variable &job_ready_;
  Job *jobs_[kNumberOfJobs];
  int32_t bottom_;
  int32_t top_;
};

std::vector<std::thread> work_threads;

#define MAX_JOB_COUNT 4096
thread_local Job job_allocator[MAX_JOB_COUNT];
thread_local uint32_t allocated_jobs = 0u;

std::vector<WorkStealingQueue *> job_queues;

std::atomic<bool> worker_thread_active;

std::mutex job_ready_mutex;
std::condition_variable job_ready;

Job *AllocateJob() {
  auto index = allocated_jobs++;
  return &job_allocator[index & (MAX_JOB_COUNT-1u)];
}

Job *CreateJob(JobFunction function) {
  auto job = AllocateJob();
  job->function = function;
  job->parent = nullptr;
  job->unfinished_jobs = 1;

  return job;
}

template<typename ...Args>
Job *CreateJobWithData(JobFunction function, Args&&... args) {
  auto job = CreateJob(function);
  pack_data(job->data, std::forward<Args>(args)...);
  return job;
}

Job *CreateJobAsChild(Job *parent, JobFunction function) {
  parent->unfinished_jobs++;

  auto job = CreateJob(function);
  job->parent = parent;

  return job;
}

uint32_t GetThreadIndex() {
  static std::atomic<uint32_t> thread_count;
  thread_local uint32_t thread_index = thread_count.fetch_add(1, std::memory_order_relaxed);
  return thread_index;
}

WorkStealingQueue *GetWorkerThreadQueue() {
  return job_queues[GetThreadIndex()];
}

bool HasJobCompleted(const Job *job) {
  //DBUG("%d", job->unfinished_jobs.load());
  return job->unfinished_jobs == 0;
}

bool IsEmptyJob(const Job *job) {
  return job == nullptr || HasJobCompleted(job);
}

Job *GetJob() {
  auto *queue = GetWorkerThreadQueue();
  auto *job = queue->Pop();

  if (IsEmptyJob(job)) {
    auto index = random_in_range(0, (int)(job_queues.size() - 1));
    auto *steal_queue = job_queues[index];
    if (steal_queue == queue) {
      //std::this_thread::yield();
      return nullptr;
    }

    auto *stolen_job = steal_queue->Steal();
    if (IsEmptyJob(stolen_job)) {
      // std::this_thread::yield();
      return nullptr;
    }

    //DBUG("%d stole job from thread %d", GetThreadIndex(), index);
    return stolen_job;
  }

  return job;
}

void Finish(Job *job) {
  const int32_t unfinished_jobs = --job->unfinished_jobs;

  //DBUG("thread %d finishing job %d", GetThreadIndex(), unfinished_jobs);

  if (unfinished_jobs == 0 && job->parent != nullptr) {
    //DBUG("thread %d finishing parent", GetThreadIndex());
    Finish(job->parent);
  }

  job_ready.notify_all();
}

void Execute(Job *job) {
  // DBUG("Thread %d running job", GetThreadIndex());
  (job->function)(job, job->data);
  Finish(job);
}

void WaitForJobReady() {
  std::unique_lock<std::mutex> lock(job_ready_mutex);
  job_ready.wait(lock);
}

void WorkerThread(Semaphore &ready) {
  ready.Notify();
  while(worker_thread_active) {
    auto *job = GetJob();
    if (job != nullptr) {
      Execute(job);
    }
    WaitForJobReady();
  }
}

void Run(Job *job) {
  auto *queue = GetWorkerThreadQueue();
  queue->Push(job);
}

void Wait(const Job *job) {
  while (job->unfinished_jobs > 0) {
    auto *next_job = GetJob();
    if (next_job != nullptr) {
      Execute(next_job);
    }
  }
}

void Initialize() {
  int worker_thread_count = std::thread::hardware_concurrency();
  INFO("Creating %d worker threads", worker_thread_count);

  GetThreadIndex();

  job_queues.emplace_back(new WorkStealingQueue(job_ready));

  for (int i = 0; i < worker_thread_count + 1; i++) {
    job_queues.emplace_back(new WorkStealingQueue(job_ready));
  }

  Semaphore workers_ready{1 - worker_thread_count};

  worker_thread_active = true;
  for (int i = 0; i < worker_thread_count; i++) {
    work_threads.emplace_back(WorkerThread, std::ref(workers_ready));
  }

  workers_ready.Wait();
}

void Shutdown() {
  worker_thread_active = false;

  job_ready.notify_all();

  for (auto &&thread : work_threads) {
    thread.join();
  }

  work_threads.clear();
}

} // namespace JobSystem
} // namespace knight
