#pragma once

#include <initializer_list>
#include <iterator>

template<class T>
class Vector {
  public:
    Vector() : 
      capacity_(0),
      size_(0), 
      data_(nullptr)
    {
    }

   
    Vector(size_t size) :
      capacity_(size),
      size_(size),
      data_(nullptr)
    {
      if (capacity_ != 0) {
        data_ = new T[capacity_];    
      }
    }    

    Vector(std::initializer_list<T> init_list) :
      Vector(init_list.begin(), init_list.end())
    {
    }

    template<typename Iterator>
    Vector(Iterator start, Iterator finish) {
      capacity_ = std::distance(start, finish);
      size_ = capacity_;
      data_ = nullptr;
      if (capacity_ != 0) {
        data_ = new T[capacity_];
        std::copy(start, finish, data_);
      }
    }

    Vector(const Vector& other) :
      capacity_(other.capacity_),
      size_(other.size_),
      data_(nullptr) 
    {
      if (capacity_ != 0) {
        data_ = new T[capacity_];
        std::copy(other.data_, other.data_ + other.capacity_, data_);
      }
    }

    Vector(Vector&& other) :
      capacity_(other.capacity_),
      size_(other.size_),
      data_(other.data_)
    {
      other.capacity_ = 0;
      other.size_ = 0;
      other.data_ = nullptr;        
    }

    Vector& operator=(const Vector& other) {
      Vector<T> vector(other);
      Swap(vector);
      return *this;
    }

    Vector& operator=(Vector&& other) {
      Vector<T> vector(std::move(other));
      Swap(vector);
      return *this; 
    }
    
    ~Vector() {
      capacity_ = 0;
      size_ = 0;
      if (data_ != nullptr) {
        delete[] data_;
      }
    }
    
    void Swap(Vector& other) {
      std::swap(capacity_, other.capacity_);
      std::swap(size_, other.size_);
      std::swap(data_, other.data_);
    }

    T& operator[] (size_t index) {
      return data_[index];
    }

    const T& operator[] (size_t index) const {
      return data_[index];
    }

    size_t Size() const {
      return size_;
    }

    size_t Capacity() const {
      return capacity_;
    }

    void PushBack(const T& elem) {
      if (capacity_ == 0) {
        capacity_ = 1;
        size_ = 1;
        data_ = new T[1];
        data_[0] = elem;
      } else {
        if (size_ == capacity_){          
          size_t new_capacity = 2 * capacity_;
          Vector<T> other(new_capacity);
          std::copy(data_, data_ + size_, other.data_);
          other.size_ = size_;
          Swap(other);
        } 
        data_[size_] = elem;
        ++size_;
      }
    }

    void PopBack() {
      if (size_ != 0) {
        --size_;
        data_[size_] = T();
      }
    }

    void Clear() {
      for (size_t iter = 0; iter < size_; ++iter) {
        data_[iter] = T();
      }
      size_ = 0;
    }

    void Reserve(size_t capacity) {
      if (capacity_ >= capacity) {
        return;
      }
      Vector<T> vector_new(capacity);
      std::copy(data_, data_ + size_, vector_new.data_);
      vector_new.size_ = size_;
      Swap(vector_new);
    }    

    class Iterator : public std::iterator<std::random_access_iterator_tag, T> {
      public:
        Iterator(){}
        Iterator(T* other) : pointer_(other) {}
  
        T& operator*() {
          return *pointer_;
        }   

        T* operator->() {
          return pointer_;
        }

        Iterator& operator++() {
          ++pointer_;
          return *this;
        }

        Iterator operator++(int) {
          Iterator save(*this);
          ++pointer_;
          return save;
        }

        Iterator& operator--() {
          --pointer_;
          return *this;
        }

        Iterator operator--(int) {
          Iterator save(*this);
          --pointer_;
          return save;
        }
  
        bool operator==(const Iterator& other) const {
          return pointer_ == other.pointer_;  
        }

        bool operator!=(const Iterator& other) const {
          return pointer_ != other.pointer_;
        }

        typename std::iterator<std::random_access_iterator_tag, T>::difference_type
        operator-(const Iterator& other) const {
          return pointer_ - other.pointer_;
        }
      
        typename std::iterator<std::random_access_iterator_tag, T>::difference_type
        operator+(const Iterator& other) const {
          return pointer_ + other.pointer_;
        }

        Iterator 
        operator-(int diff) const {
          return Iterator(pointer_ - diff);
        }
      
        Iterator
        operator+(int diff) const {
          return Iterator(pointer_ + diff);
        }

        Iterator& operator+=(const Iterator& other) {
          pointer_ = pointer_ + other.pointer_;
          return *this;
        }

        Iterator& operator-=(const Iterator& other) {
          pointer_ = pointer_ - other.pointer_;
          return *this;
        }

        Iterator& operator+=(int diff) {
          pointer_ = pointer_ + diff;
          return *this;
        }

        Iterator& operator-=(int diff) {
          pointer_ = pointer_ - diff;
          return *this;
        }


      private:
        T* pointer_;
    };

    Iterator Begin() {
      return Iterator(data_); 
    }

    Iterator End() {
      return Iterator(data_ + size_);
    }

    Iterator begin() {
      return Begin();
    }

    Iterator end() {
      return End();
    }

  private:
    size_t capacity_;
    size_t size_;
    T* data_;


};
