#pragma once

#include <algorithm>
#include <memory>
#include <utility>
#include <typeinfo>

class Any {
private:

class any_base {
public:
  virtual std::shared_ptr<any_base> clone() = 0;
  virtual ~any_base() {}
};

template<typename T>
class any_class : public any_base {
public:
  template<typename U>
  any_class(U&& elem) : value_(std::forward<U>(elem)) {}
  const T& GetValue() const {
    return value_;
  }
  virtual std::shared_ptr<any_base> clone() override {
    return std::make_shared<any_class<T>>(value_);
  }
  ~any_class() {}
private:
  T value_;
};

std::shared_ptr<any_base> ptr_;

public:
    Any() : ptr_(nullptr) {}

    template<class T>
    Any(const T& value) : ptr_(std::make_shared<any_class<T>>(value)) {      
    }

    template<class T>
    Any& operator=(const T& value) {
      Any swapper(value);
      Swap(swapper);   
      return *this; 
    }

    Any(const Any& rhs) : ptr_(rhs.ptr_->clone()) {}
    // clone must be used
    // just increasing ctr amount in shared ptr is not proper

    Any& operator=(const Any& rhs) {
      // if (this == &rhs) {
      //  return *this;
      // }
      Any swapper(rhs);
      Swap(swapper);
      return *this;
    }

    ~Any() {}

    bool Empty() const {
        return ptr_ == nullptr;
    }

    void Clear() {
      ptr_ = nullptr;
    }

    void Swap(Any& rhs) {
      std::swap(ptr_, rhs.ptr_);
    }
    
    template<class T>
    const T& GetValue() const {
      return dynamic_cast<any_class<T>*>(ptr_.get())->GetValue();
    }
};
