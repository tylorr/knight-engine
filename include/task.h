#ifndef TASK_H_
#define TASK_H_

#include "common.h"

#include <vector>
#include <functional>

namespace knight {

class Task {
 public:
  Task() { }

  const std::vector<ID> &entities() const { return entities_; }
  void set_entities(std::vector<ID> &&r) { entities_ = std::move(r); }

 private:
  std::vector<ID> entities_;
  std::function<void()> run;
};

}; // namespace knight

#endif // TASK_H_
