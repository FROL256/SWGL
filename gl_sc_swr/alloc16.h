#pragma once

#ifndef WIN32
  #include <stdlib.h>
#endif // WIN32

template<class T>
class aligned16
{
public:

  typedef size_t    size_type;
  typedef ptrdiff_t difference_type;
  typedef T*        pointer;
  typedef const T*  const_pointer;
  typedef T&        reference;
  typedef const T&  const_reference;
  typedef T         value_type;

  aligned16() {}
  aligned16(const aligned16&) {}

  pointer allocate(size_type n, const void* hint = nullptr)
  {
    #ifdef WIN32
    return (pointer)_aligned_malloc(n*sizeof(T), 16);
    #else
    return (pointer)aligned_alloc(16, n*sizeof(T));
    #endif
  }

  void deallocate(void* p, size_type n)
  {
    if (p != nullptr)
    {
    #ifdef WIN32
      _aligned_free(p);
    #else
      free(p);
    #endif // WIN32
    }
  }

  pointer           address(reference x) const { return &x; }
  const_pointer     address(const_reference x) const { return &x; }
  aligned16<T>&     operator=(const aligned16&) { return *this; }
  void              construct(pointer p, const T& val)
  {
    new ((T*)p) T(val);
  }

  void              destroy(pointer p) { p->~T(); }
  size_type         max_size() const { return size_t(-1); }

  template <class U>
  struct rebind { typedef aligned16<U> other; };

  template <class U>
  aligned16(const aligned16<U>&) {}

  template <class U>
  aligned16& operator=(const aligned16<U>&) { return *this; }

  bool operator==(const aligned16<T> a_rhs) { return true; } // sigleton, all instances are always equal

};


