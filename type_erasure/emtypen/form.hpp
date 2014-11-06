%struct_prefix% {
 public:
  struct Concept {
    static const int kBufferSize = 64;
    virtual ~Concept() = default;
    virtual std::shared_ptr<Concept> clone(foundation::TempAllocator<kBufferSize> &allocator) const = 0;
    %pure_virtual_members%
  };

  template <class T_Holder__>
  struct Model : public T_Holder__, public virtual Concept {
    using T_Holder__::T_Holder__;

    virtual std::shared_ptr<Concept> clone(foundation::TempAllocator<Concept::kBufferSize> &allocator) const { 
      return Model::MakeNew(this->T_Holder__::get(), allocator);
    }

    %virtual_members%

    template<typename T_T__>
    static std::shared_ptr<Concept> MakeNew(T_T__ &&value, foundation::TempAllocator<Concept::kBufferSize> &allocator) {
      return std::allocate_shared<Model>(
        SmallBufferAllocator<Model, Concept::Concept::kBufferSize>(allocator),
        std::forward<T_T__>(value),
        nullptr
      );
    }
  };

  template <class Container>
  struct ExternalInterface : public Container {
    using Container::Container;

    %nonvirtual_members%
  };
};
