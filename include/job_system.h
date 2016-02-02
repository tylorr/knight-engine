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

void initialize();
void shutdown();

bool has_job_completed(const Job *job);

Job *create_job(JobFunction function);
Job *create_job_as_child(Job *parent, JobFunction function);

template<typename ...Args>
Job *create_job(JobFunction function, Args&&... args) {
  auto job = create_job(function);
  memory_block::pack_data(job->data, std::forward<Args>(args)...);
  return job;
}

template<typename ...Args>
Job *create_job_as_child(Job *parent, JobFunction function, Args&&... args) {
  auto job = create_job_as_child(parent, function);
  memory_block::pack_data(job->data, std::forward<Args>(args)...);
  return job;
}

void run(Job *job);
void wait(const Job *job);

} // namespace JobSystem
} // namespace knight
