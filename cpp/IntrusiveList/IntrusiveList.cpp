#pragma once

#include <algorithm>

class ListHook {
public:
    ListHook() {
      HookSelf();
    }

    bool IsLinked() const {
      return left_ != this;
    }

    void Unlink() {
      if (!IsLinked()) {
        return;
      }
      this->left_->right_ = this->right_;
      this->right_->left_ = this->left_;
      HookSelf();
    }

    // Must unlink element from list
    ~ListHook() {
      Unlink();
    }

    ListHook(const ListHook&) = delete;

private:
    template <class T>
    friend class List;

    ListHook* left_;
    ListHook* right_;

    // that helper function might be useful
    void LinkBefore(ListHook* other) {
      this->left_->right_ = other;
      other->left_ = this->left_;
      other->right_ = this;
      this->left_ = other;
    }

    void HookSelf() {
      left_ = this;
      right_ = this;
    }
    
    ListHook(ListHook&& other) {
      HookSelf();      
      if (other.IsLinked()) {
        other.left_->right_ = this;
        left_ = other.left_;
        other.right_->left_ = this;
        right_ = other.right_;
        other.HookSelf();
      }
    }

    void Swap(ListHook& other) {
      bool self_empty = !IsLinked();
      bool other_empty = !other.IsLinked();
      if (self_empty && other_empty) {        
        return;        
      }
      if (self_empty && !other_empty) {
        other.left_->right_ = this;
        other.right_->left_ = this;
        left_ = other.left_;
        right_ = other.right_;
        other.HookSelf();
        return;        
      }
      if (!self_empty && other_empty) {
        left_->right_ = &other;
        right_->left_ = &other;
        other.left_ = left_;
        other.right_ = right_;
        HookSelf();
        return;
      }
      left_->right_ = &other;
      right_->left_ = &other;
      other.left_->right_ = this;
      other.right_->left_ = this;
      std::swap(left_, other.left_);
      std::swap(right_, other.right_);
    }
};

template <typename T>
class List {
public:
    class Iterator : public std::iterator<std::bidirectional_iterator_tag, T> {
    public:
        Iterator(ListHook* hook) : ptr_(hook) {}

        Iterator& operator++() {
          ptr_ = ptr_->right_;
          return *this;
        }

        Iterator operator++(int) {
          Iterator old(ptr_);
          ptr_ = ptr_->right_;
          return old;
        }

        T& operator*() const {
          return *static_cast<T*>(ptr_);
        }

        T* operator->() const {
          return static_cast<T*>(ptr_);
        }

        bool operator==(const Iterator& rhs) const {
          return ptr_ == rhs.ptr_;
        }
        
        bool operator!=(const Iterator& rhs) const {
          return ptr_ != rhs.ptr_;
        }
    private:
        ListHook* ptr_;
    };

    List() : dummy_() {      
    }

    List(const List&) = delete;
    
    List(List&& other) : dummy_(std::move(other.dummy_)) {
    }

    // must unlink all elements from list
    ~List() {
      UnlinkAll();   
    }

    List& operator=(const List&) = delete;
    
    List& operator=(List&& other) {
      List lst(std::move(other));   
      Swap(lst);
      return *this;
    }

    bool IsEmpty() const {
      return !dummy_.IsLinked();
    }

    // that method is allowed to be O(n)
    size_t Size() const {
      if (IsEmpty()) {
        return 0u;
      }  
      size_t size = 0;
      ListHook* elem = dummy_.right_;
      while (elem != &dummy_) {
        ListHook* next_elem = elem->right_;
        ++size;
        elem = next_elem;
      }
      return size;
    }

    // note that IntrusiveList doesn't own elements,
    // and never copies or moves T
    void PushBack(T* elem) {
      ListHook* new_elem = static_cast<ListHook*>(elem);
      dummy_.LinkBefore(new_elem);  
    }

    void PushFront(T* elem) {
      ListHook* new_elem = static_cast<ListHook*>(elem);
      dummy_.left_->LinkBefore(new_elem);    
    }

    T& Front() {
      return *static_cast<T*>(dummy_.right_);
    }
    const T& Front() const {
      return *static_cast<const T*>(dummy_.right_);
    }

    T& Back() {
      return *static_cast<T*>(dummy_.left_);
    }

    const T& Back() const {
      return *static_cast<const T*>(dummy_.left_);
    }

    void PopBack() {
      dummy_.left_->Unlink();      
    }

    void PopFront() {
      dummy_.right_->Unlink();
    }

    Iterator Begin() {
      return Iterator(dummy_.right_);
    }

    Iterator End() {
      return Iterator(&dummy_);
    }

    // complexity of this function must be O(1)
    Iterator IteratorTo(T* element) {
      return Iterator(static_cast<ListHook*>(element));    
    }

private:    
    void Swap(List& other) {
      dummy_.Swap(other.dummy_);
    }

    void UnlinkAll() {
      while (dummy_.left_ != &dummy_) {
        dummy_.left_->Unlink();
      }
    }

    ListHook dummy_;
};

template <typename T>
typename List<T>::Iterator begin(List<T>& list) {
    return list.Begin();
}

template <typename T>
typename List<T>::Iterator end(List<T>& list) {
    return list.End();
}
