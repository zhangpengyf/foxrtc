//
//  scoped_ptr.h
//  umcs
//
//  Created by zhangpeng on 15-3-18.
//  Copyright (c) 2015年 51talk. All rights reserved.
//

#ifndef scoped_ptr_h
#define scoped_ptr_h
#include <assert.h>
namespace foxrtc {
//#define scoped_ptr webrtc::scoped_ptr
    template <class C>
    class scoped_ptr {
    public:
        
        // The element type
        typedef C element_type;
        
        // Constructor.  Defaults to intializing with NULL.
        // There is no way to create an uninitialized scoped_ptr.
        // The input parameter must be allocated with new.
        explicit scoped_ptr(C* p = NULL) : ptr_(p) { }
        
        // Destructor.  If there is a C object, delete it.
        // We don't need to test ptr_ == NULL because C++ does that for us.
        ~scoped_ptr() {
            enum { type_must_be_complete = sizeof(C) };
            delete ptr_;
        }
        
        // Reset.  Deletes the current owned object, if any.
        // Then takes ownership of a new object, if given.
        // this->reset(this->get()) works.
        void reset(C* p = NULL) {
            if (p != ptr_) {
                enum { type_must_be_complete = sizeof(C) };
                delete ptr_;
                ptr_ = p;
            }
        }
        
        // Accessors to get the owned object.
        // operator* and operator-> will assert() if there is no current object.
        C& operator*() const {
            assert(ptr_ != NULL);
            return *ptr_;
        }
        C* operator->() const  {
            assert(ptr_ != NULL);
            return ptr_;
        }
        C* get() const { return ptr_; }
        
        // Comparison operators.
        // These return whether two scoped_ptr refer to the same object, not just to
        // two different but equal objects.
        bool operator==(C* p) const { return ptr_ == p; }
        bool operator!=(C* p) const { return ptr_ != p; }
        
        // Swap two scoped pointers.
        void swap(scoped_ptr& p2) {
            C* tmp = ptr_;
            ptr_ = p2.ptr_;
            p2.ptr_ = tmp;
        }
        
        // Release a pointer.
        // The return value is the current pointer held by this object.
        // If this object holds a NULL pointer, the return value is NULL.
        // After this operation, this object will hold a NULL pointer,
        // and will not own the object any more.
        C* release() {
            C* retVal = ptr_;
            ptr_ = NULL;
            return retVal;
        }
        
    private:
        C* ptr_;
        
        // Forbid comparison of scoped_ptr types.  If C2 != C, it totally doesn't
        // make sense, and if C2 == C, it still doesn't make sense because you should
        // never have the same object owned by two different scoped_ptrs.
        template <class C2> bool operator==(scoped_ptr<C2> const& p2) const;
        template <class C2> bool operator!=(scoped_ptr<C2> const& p2) const;
        
        // Disallow evil constructors
        scoped_ptr(const scoped_ptr&);
        void operator=(const scoped_ptr&);
    };

}

#endif
