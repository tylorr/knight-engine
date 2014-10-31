#pragma once

#include "small_buffer_allocator.h"

#include <temp_allocator.h>
#include <memory>

namespace detail {

template <class T>
class Holder {
 public:
  using Data = T;

  template <typename U = T>
  Holder(T value, typename std::enable_if<std::is_reference<U>::value>::type * = nullptr) 
    : data_(value) { }

  template <typename U = T>
  Holder(T value, typename std::enable_if<!std::is_reference<U>::value, int>::type * = nullptr) noexcept
    : data_(std::move(value)) { }

  virtual ~Holder() = default;

  T &get() { return data_; }
  const T &get() const { return data_; }

 private:
  T data_;
};

template <typename T>
struct Holder<std::reference_wrapper<T>> : Holder<T &> {
  Holder(std::reference_wrapper<T> ref, int *_ = nullptr) : Holder<T &>(ref.get()) { }
};

template <class Concept_, template <class> class Model>
class Container {
 public:
  using Concept = Concept_;

  Container() = default;

  template <class T>
  Container(T obj) 
    : allocator_(foundation::memory_globals::default_allocator()),
      self_(
        Model<Holder<typename std::remove_reference<T>::type>>::MakeNew(
          std::forward<T>(obj), 
          allocator_
        )
      ) { }

  Container(const Container &other) 
    : allocator_(foundation::memory_globals::default_allocator()), 
      self_(IsHeapAllocated(other) ? other.self_ : other.self_->clone(allocator_)) { }

  template <typename T>
  Container &operator=(T &&value)
  {
      Container temp(std::forward<T>(value));
      std::swap(temp.self_, self_);
      return *this;
  }

  const Concept &read() const { return *self_; }
  Concept &write() {
    if (!self_.unique()) {
      self_ = self_->clone(allocator_);
    }
    return *self_;
  }


 private:
  static bool IsHeapAllocated(const Container &container) {
    return !container.allocator_.is_buffer_allocated(container.self_.get());
  }

  foundation::TempAllocator<Concept_::kBufferSize> allocator_;
  std::shared_ptr<Concept> self_;
};

// Helpers for spec merging.
template <class Spec>
using ConceptOf = typename Spec::Concept;

template <class Spec, class Holder>
using ModelOf = typename Spec::template Model<Holder>;

template <class Spec, class Container>
using ExternalInterfaceOf = typename Spec::template ExternalInterface<Container>;

template <class Spec>
using ContainerOf = detail::Container<typename Spec::Concept, Spec::template Model>;

} // namspace detail

template <class Spec_>
class TypeErasure : public detail::ExternalInterfaceOf<Spec_, detail::ContainerOf<Spec_>> {
  using Base = detail::ExternalInterfaceOf<Spec_, detail::ContainerOf<Spec_>>;

 public:
  using Base::Base;
  using Spec = Spec_;
};

template <typename... Specs>
struct MergeSpecs;

template <typename SpecA, typename... Specs>
struct MergeSpecs<SpecA, Specs...> : MergeSpecs<SpecA, MergeSpecs<Specs...>> { };

template <class SpecA, class SpecB>
struct MergeSpecs<SpecA, SpecB> {
  struct Concept : public virtual detail::ConceptOf<SpecA>, public virtual detail::ConceptOf<SpecB> {};

  template <class Holder>
  struct Model : public detail::ModelOf<SpecA, detail::ModelOf<SpecB, Holder>>, public virtual Concept {
    using Base = detail::ModelOf<SpecA, detail::ModelOf<SpecB, Holder>>;
    using Base::Base;
  };

  template <class Container>
  struct ExternalInterface : public detail::ExternalInterfaceOf<SpecA, detail::ExternalInterfaceOf<SpecB, Container>> {
    using Base = detail::ExternalInterfaceOf<SpecA, detail::ExternalInterfaceOf<SpecB, Container>>;
    using Base::Base;
  };
};
