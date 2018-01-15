#pragma once



template <typename T>
class List {
private:
  struct Node {
    Node(): prev_(nullptr), next_(nullptr){}
    Node* prev_;
    Node* next_;
    virtual ~Node() {prev_ = nullptr; next_ = nullptr;}
  };

  struct Item : public Node {
    Item() : elem_() {}
    Item(const T& elem) : elem_(elem) {}
    Item(T&& elem) : elem_(std::move(elem)) {}
    T elem_;
    virtual ~Item() {}
  };

public:
    class Iterator {
    public:
        Iterator(Node* node) : pointer_(node) {}

        Iterator& operator++() {
          pointer_ = pointer_->next_;
          return *this;
        }

        Iterator operator++(int) {
          Iterator current(pointer_);
          pointer_ = pointer_->next_;
          return current;
        }

        Iterator& operator--() {
          pointer_ = pointer_->prev_;
          return *this;
        }

        Iterator operator--(int) {
          Iterator current(pointer_);
          pointer_ = pointer_->prev_;
          return current;
        }

        T& operator*() const {
          return static_cast<Item*>(pointer_)->elem_;
        }

        T* operator->() const {
          return &(static_cast<Item*>(pointer_)->elem_);
        }

        bool operator==(const Iterator& rhs) const {
          return pointer_ == rhs.pointer_;
        }

        bool operator!=(const Iterator& rhs) const {
          return pointer_ != rhs.pointer_;
        }
    private:
        Node* pointer_;
    };

    List() {
      Init();
    }

    List(const List& other) {
      Init();
      Copy(other);
    }
    
    List(List&& other) {
      MoveCopy(std::move(other));
    }
    
    ~List() {
      RemoveSelf();
    }

    List& operator=(const List& other) {
      List new_list(other);
      Swap(new_list);
      return *this;
    }

    List& operator=(List&& other) {
      List new_list(std::move(other));
      Swap(new_list);
      return *this;
    }

    bool IsEmpty() const {
      return front_ == back_;
    }

    size_t Size() const {
      size_t size = 0;
      Node* current = front_;
      while (current != back_) {
        ++size;
        current = current->next_;
      }
      return size;
    }

    void PushBack(const T& elem) {
      Item* new_elem = new Item(elem);
      PushBack(static_cast<Node*>(new_elem));
    }

    void PushBack(T&& elem) {
      Item* new_elem = new Item(std::move(elem));
      PushBack(static_cast<Node*>(new_elem));
    }
    
    void PushFront(const T& elem) {
      Item* new_elem = new Item(elem);
      PushFront(static_cast<Node*>(new_elem));  
    }
    
    void PushFront(T&& elem) {
      Item* new_elem = new Item(std::move(elem));
      PushFront(static_cast<Node*>(new_elem));
    }

    T& Front() {
      return static_cast<Item*>(front_)->elem_;
    }

    const T& Front() const {
      return static_cast<const Item*>(front_)->elem_; 
    }

    T& Back() {
      return static_cast<Item*>(back_->prev_)->elem_;
    }

    const T& Back() const {
      return static_cast<const Item*>(back_->prev_)->elem_;
    }

    void PopBack() {
      Node* remove_elem = back_->prev_;
      UntieItem(remove_elem);
      if (front_ == remove_elem) {
        front_ = back_;
      }
      delete remove_elem;  
    }

    void PopFront() {
      Node* remove_elem = front_;
      UntieItem(remove_elem);
      front_ = front_->next_;
      delete remove_elem;
    }

    Iterator Begin() {
      return Iterator(front_);
    }

    Iterator End() {
      return Iterator(back_);
    }

private:

  void UntieItem(Node* item) {
    item->prev_->next_ = item->next_;
    item->next_->prev_ = item->prev_;
  }

  void RemoveSelf() {
    while (front_ != back_) {
      Node* next = front_->next_;
      delete front_;  
      front_ = next;
    }
    if (back_ != nullptr) {
      delete back_;
    }    
    back_ = nullptr;
    front_ = nullptr;
  }

  void Copy(const List& other) {
    Node* other_front = other.front_;
    while (other_front != other.back_) {
      PushBack(static_cast<Item*>(other_front)->elem_);
      other_front = other_front->next_;
    }  
  }

  void MoveCopy(List&& other) {
    front_ = other.front_;
    back_ = other.back_;
    other.front_ = nullptr;
    other.back_ = nullptr;
  }

  void PushFront(Node* elem) {
    elem->next_ = front_;
    elem->prev_ = back_;
    back_->next_ = elem;
    front_->prev_ = elem;
    front_ = elem;
  }

  void PushBack(Node* elem) {
    elem->next_ = back_;
    elem->prev_ = back_->prev_;
    back_->prev_->next_ = elem;
    back_->prev_ = elem;
    if (front_ == back_) {
      front_ = elem;
    }
  }
    
  void Init() {
    back_ = new Node();
    back_->prev_ = back_;
    back_->next_ = back_;
    front_ = back_;
  }

  void Swap(List& other) {
    std::swap(front_, other.front_);
    std::swap(back_, other.back_);
  }

  Node* front_;
  Node* back_;
};

template <typename T>
typename List<T>::Iterator begin(List<T>& list) {
  return list.Begin();
}

template <typename T>
typename List<T>::Iterator end(List<T>& list) {
  return list.End();
}
