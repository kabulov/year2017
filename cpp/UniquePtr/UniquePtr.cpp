#pragma once

class UniquePtr {
public:
    // see http://en.cppreference.com/w/cpp/memory/unique_ptr

    UniquePtr() : ptr_(nullptr) {
    }

    UniquePtr(TestClass* param) : ptr_(param) {    
    }

    UniquePtr(UniquePtr&& other) : ptr_(other.ptr_) {
      other.ptr_ = nullptr;
    }

    UniquePtr& operator=(UniquePtr&& other) {
      if (this == &other) {
        return *this;
      }
      remove();
      ptr_ = other.ptr_;
      other.ptr_ = nullptr;
      return *this;
    }

    TestClass* operator->() {
      return ptr_;
    }

    void reset(TestClass* param = nullptr) {
      remove();
      ptr_ = param;
    }

    ~UniquePtr() {
      remove();
    }

private:
    void remove() {
      if (ptr_) {
        delete ptr_;
        ptr_ = nullptr;
      }
    }

    TestClass* ptr_;
};
