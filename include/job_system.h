#pragma once

#include "memory_block.h"

#include <atomic>

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

void Initialize();
void Shutdown();

bool HasJobCompleted(const Job *job);

Job *CreateJob(JobFunction function);
Job *CreateJobAsChild(Job *parent, JobFunction function);

template<typename ...Args>
Job *CreateJob(JobFunction function, Args&&... args) {
  auto job = CreateJob(function);
  memory_block::pack_data(job->data, std::forward<Args>(args)...);
  return job;
}

template<typename ...Args>
Job *CreateJobAsChild(Job *parent, JobFunction function, Args&&... args) {
  auto job = CreateJobAsChild(parent, function);
  memory_block::pack_data(job->data, std::forward<Args>(args)...);
  return job;
}

void Run(Job *job);
void Wait(const Job *job);

} // namespace JobSystem
} // namespace knight
