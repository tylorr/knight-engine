// included by concurrent_queue.h to implement template methods

namespace knight {

template<typename T>
void ConcurrentQueue<T>::push(T item) {
  std::lock_guard<std::mutex> lk(mutex_);
  queue_.push(item);
  condition_.notify_one();
}

template<typename T>
T ConcurrentQueue<T>::ConcurrentQueue<T>::wait_pop() {
  std::unique_lock<std::mutex> lk(mutex_);
  condition_.wait(lk, [this]{ return queue_.size() > 0; });

  T item(std::move(queue_.front()));
  queue_.pop();
  return item;
}

template<typename T>
bool ConcurrentQueue<T>::try_pop(T &item) {
  std::lock_guard<std::mutex> lk(mutex_);

  bool result = false;
  if (!queue_.empty()) {
    item = queue_.front();
    queue_.pop();
    result = true;
  }

  return result;
}

template<typename T>
size_t ConcurrentQueue<T>::size() const {
  std::lock_guard<std::mutex> lock(mutex_);
  return queue_.size();
}

template<typename T>
bool ConcurrentQueue<T>::empty() const {
  std::lock_guard<std::mutex> lock(mutex_);
  return queue_.empty();
}

}; // namespace knight
