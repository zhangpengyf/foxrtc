/* -----------------------------------------------------------------------------
 * std_vector.i
 * ----------------------------------------------------------------------------- */

%include <std_common.i>

%{
#include <vector>
#include <stdexcept>
struct DataWrapper
{
    const void* data;
    unsigned long len;
};
%}

namespace std {
    
    template<class T> class vector {
      public:
        typedef size_t size_type;
        typedef T value_type;
        typedef const value_type& const_reference;
        vector();
        vector(size_type n);
        size_type size() const;
        size_type capacity() const;
        void reserve(size_type n);
        %rename(isEmpty) empty;
        bool empty() const;
        void clear();
        %extend {
            void add(const_reference val)
            {
                self->push_back(val);
            }
            const_reference get(size_type i) throw (std::out_of_range) {
                if (i>=0 && i<self->size())
                    return (*self)[i];
                else
                    throw std::out_of_range("vector index out of range");
            }
            void set(size_type i, const value_type& val) throw (std::out_of_range) {
                if (i>=0 && i<self->size())
                    (*self)[i] = val;
                else
                    throw std::out_of_range("vector index out of range");
            }
        }
    };

    // bool specialization
    template<> class vector<bool> {
      public:
        typedef size_t size_type;
        typedef bool value_type;
        typedef bool const_reference;
        vector();
        vector(size_type n);
        size_type size() const;
        size_type capacity() const;
        void reserve(size_type n);
        %rename(isEmpty) empty;
        bool empty() const;
        void clear();
        %rename(add) push_back;
        %extend {
            const_reference get(size_type i) throw (std::out_of_range) {
                if (i>=0 && i<self->size())
                    return (*self)[i];
                else
                    throw std::out_of_range("vector index out of range");
            }
            void set(size_type i,value_type val) throw (std::out_of_range) {
                if (i>=0 && i<self->size())
                    (*self)[i] = val;
                else
                    throw std::out_of_range("vector index out of range");
            }
        }
    };
    // signed char specialization
    template<> class vector<signed char> {
      public:
        typedef size_t size_type;
        typedef signed char value_type;
        typedef signed char const_reference;
        vector();
        vector(size_type n);
        size_type size() const;
        size_type capacity() const;
        void reserve(size_type n);
        %rename(isEmpty) empty;
        bool empty() const;
        void clear();
        %extend {
            void add(value_type val)
            {
                self->push_back(val);
            }
            value_type get(size_type i) throw (std::out_of_range) {
                if (i>=0 && i<self->size())
                    return (*self)[i];
                else
                    throw std::out_of_range("vector index out of range");
            }
            void set(size_type i,value_type val) throw (std::out_of_range) {
                if (i>=0 && i<self->size())
                    (*self)[i] = val;
                else
                    throw std::out_of_range("vector index out of range");
            }
            void setData(const DataWrapper& data)
            {
                if (data.len>=0)
                {
                    self->resize(data.len);
                    memcpy(&(*self)[0],data.data,data.len);
                }
                else
                {
                    throw std::out_of_range("set data len < 0");
                }
            }
            DataWrapper getData()
            {
                DataWrapper dw;
                dw.data = 0;
                dw.len = 0;
                if (self->size()==0)
                {
                    return dw;
                }
                dw.data = &(*self)[0];
                dw.len = self->size();
                return dw;
            }
        }
    };
        // char specialization
    template<> class vector<char> {
      public:
        typedef size_t size_type;
        typedef char value_type;
        typedef char const_reference;
        vector();
        vector(size_type n);
        size_type size() const;
        size_type capacity() const;
        void reserve(size_type n);
        %rename(isEmpty) empty;
        bool empty() const;
        void clear();
        %extend {
            void add(value_type val)
            {
                self->push_back(val);
            }
            value_type get(size_type i) throw (std::out_of_range) {
                if (i>=0 && i<self->size())
                    return (*self)[i];
                else
                    throw std::out_of_range("vector index out of range");
            }
            void set(size_type i,value_type val) throw (std::out_of_range) {
                if (i>=0 && i<self->size())
                    (*self)[i] = val;
                else
                    throw std::out_of_range("vector index out of range");
            }
            void setData(const DataWrapper& data)
            {
                if (data.len>=0)
                {
                    self->resize(data.len);
                    memcpy(&(*self)[0],data.data,data.len);
                }
                else
                {
                    throw std::out_of_range("set data len < 0");
                }
            }
            DataWrapper getData()
            {
                DataWrapper dw;
                dw.data = 0;
                dw.len = 0;
                if (self->size()==0)
                {
                    return dw;
                }
                dw.data = &(*self)[0];
                dw.len = self->size();
                return dw;
            }
        }
    };
        // unsigned char specialization
    template<> class vector<unsigned char> {
      public:
        typedef size_t size_type;
        typedef unsigned char value_type;
        typedef unsigned char const_reference;
        vector();
        vector(size_type n);
        size_type size() const;
        size_type capacity() const;
        void reserve(size_type n);
        %rename(isEmpty) empty;
        bool empty() const;
        void clear();
        %extend {
            void add(value_type val)
            {
                self->push_back(val);
            }
            value_type get(size_type i) throw (std::out_of_range) {
                if (i>=0 && i<self->size())
                    return (*self)[i];
                else
                    throw std::out_of_range("vector index out of range");
            }
            void set(size_type i,value_type val) throw (std::out_of_range) {
                if (i>=0 && i<self->size())
                    (*self)[i] = val;
                else
                    throw std::out_of_range("vector index out of range");
            }
            void setData(const DataWrapper& data)
            {
                if (data.len>=0)
                {
                    self->resize(data.len);
                    memcpy(&(*self)[0],data.data,data.len);
                }
                else
                {
                    throw std::out_of_range("set data len < 0");
                }
            }
            DataWrapper getData()
            {
                DataWrapper dw;
                dw.data = 0;
                dw.len = 0;
                if (self->size()==0)
                {
                    return dw;
                }
                dw.data = &(*self)[0];
                dw.len = self->size();
                return dw;
            }
        }
    };

      // unsigned short specialization
    template<> class vector<unsigned short> {
      public:
        typedef size_t size_type;
        typedef unsigned short value_type;
        typedef unsigned short const_reference;
        vector();
        vector(size_type n);
        size_type size() const;
        size_type capacity() const;
        void reserve(size_type n);
        %rename(isEmpty) empty;
        bool empty() const;
        void clear();
        %extend {
            void add(value_type val)
            {
                self->push_back(val);
            }
            value_type get(size_type i) throw (std::out_of_range) {
                if (i>=0 && i<self->size())
                    return (*self)[i];
                else
                    throw std::out_of_range("vector index out of range");
            }
            void set(size_type i,value_type val) throw (std::out_of_range) {
                if (i>=0 && i<self->size())
                    (*self)[i] = val;
                else
                    throw std::out_of_range("vector index out of range");
            }
        }
    };
          // signed short specialization
    template<> class vector<signed short> {
      public:
        typedef size_t size_type;
        typedef signed short value_type;
        typedef signed short const_reference;
        vector();
        vector(size_type n);
        size_type size() const;
        size_type capacity() const;
        void reserve(size_type n);
        %rename(isEmpty) empty;
        bool empty() const;
        void clear();
        %extend {
            void add(value_type val)
            {
                self->push_back(val);
            }
            value_type get(size_type i) throw (std::out_of_range) {
                if (i>=0 && i<self->size())
                    return (*self)[i];
                else
                    throw std::out_of_range("vector index out of range");
            }
            void set(size_type i,value_type val) throw (std::out_of_range) {
                if (i>=0 && i<self->size())
                    (*self)[i] = val;
                else
                    throw std::out_of_range("vector index out of range");
            }
        }
    };
              // signed int specialization
    template<> class vector<signed int> {
      public:
        typedef size_t size_type;
        typedef signed int value_type;
        typedef signed int const_reference;
        vector();
        vector(size_type n);
        size_type size() const;
        size_type capacity() const;
        void reserve(size_type n);
        bool empty() const;
        void clear();
        %rename(add) push_back;
        %extend {
            void add(value_type val)
            {
                self->push_back(val);
            }
            value_type get(size_type i) throw (std::out_of_range) {
                if (i>=0 && i<self->size())
                    return (*self)[i];
                else
                    throw std::out_of_range("vector index out of range");
            }
            void set(size_type i,value_type val) throw (std::out_of_range) {
                if (i>=0 && i<self->size())
                    (*self)[i] = val;
                else
                    throw std::out_of_range("vector index out of range");
            }
        }
    };
                  // unsigned int specialization
    template<> class vector<unsigned int> {
      public:
        typedef size_t size_type;
        typedef unsigned int value_type;
        typedef unsigned int const_reference;
        vector();
        vector(size_type n);
        size_type size() const;
        size_type capacity() const;
        void reserve(size_type n);
        %rename(isEmpty) empty;
        bool empty() const;
        void clear();
        %extend {
            void add(value_type val)
            {
                self->push_back(val);
            }
            value_type get(size_type i) throw (std::out_of_range) {
                if (i>=0 && i<self->size())
                    return (*self)[i];
                else
                    throw std::out_of_range("vector index out of range");
            }
            void set(size_type i,value_type val) throw (std::out_of_range) {
                if (i>=0 && i<self->size())
                    (*self)[i] = val;
                else
                    throw std::out_of_range("vector index out of range");
            }
        }
    };
                      // unsigned long specialization
    template<> class vector<unsigned long> {
      public:
        typedef size_t size_type;
        typedef unsigned long value_type;
        typedef unsigned long const_reference;
        vector();
        vector(size_type n);
        size_type size() const;
        size_type capacity() const;
        void reserve(size_type n);
        %rename(isEmpty) empty;
        bool empty() const;
        void clear();
        %extend {
            void add(value_type val)
            {
                self->push_back(val);
            }
            value_type get(size_type i) throw (std::out_of_range) {
                if (i>=0 && i<self->size())
                    return (*self)[i];
                else
                    throw std::out_of_range("vector index out of range");
            }
            void set(size_type i,value_type val) throw (std::out_of_range) {
                if (i>=0 && i<self->size())
                    (*self)[i] = val;
                else
                    throw std::out_of_range("vector index out of range");
            }
        }
    };
                          // signed long specialization
    template<> class vector<signed long> {
      public:
        typedef size_t size_type;
        typedef signed long value_type;
        typedef signed long const_reference;
        vector();
        vector(size_type n);
        size_type size() const;
        size_type capacity() const;
        void reserve(size_type n);
        %rename(isEmpty) empty;
        bool empty() const;
        void clear();
        %extend {
            void add(value_type val)
            {
                self->push_back(val);
            }
            value_type get(size_type i) throw (std::out_of_range) {
                if (i>=0 && i<self->size())
                    return (*self)[i];
                else
                    throw std::out_of_range("vector index out of range");
            }
            void set(size_type i,value_type val) throw (std::out_of_range) {
                if (i>=0 && i<self->size())
                    (*self)[i] = val;
                else
                    throw std::out_of_range("vector index out of range");
            }
        }
    };
                          // unsigned long long  specialization
    template<> class vector<unsigned long long> {
      public:
        typedef size_t size_type;
        typedef unsigned long long value_type;
        typedef unsigned long long const_reference;
        vector();
        vector(size_type n);
        size_type size() const;
        size_type capacity() const;
        void reserve(size_type n);
        %rename(isEmpty) empty;
        bool empty() const;
        void clear();
        %extend {
            void add(value_type val)
            {
                self->push_back(val);
            }
            value_type get(size_type i) throw (std::out_of_range) {
                if (i>=0 && i<self->size())
                    return (*self)[i];
                else
                    throw std::out_of_range("vector index out of range");
            }
            void set(size_type i,value_type val) throw (std::out_of_range) {
                if (i>=0 && i<self->size())
                    (*self)[i] = val;
                else
                    throw std::out_of_range("vector index out of range");
            }
        }
    };
                          // signed long long specialization
    template<> class vector<signed long long> {
      public:
        typedef size_t size_type;
        typedef signed long long value_type;
        typedef signed long long const_reference;
        vector();
        vector(size_type n);
        size_type size() const;
        size_type capacity() const;
        void reserve(size_type n);
        %rename(isEmpty) empty;
        bool empty() const;
        void clear();
        %extend {
            void add(value_type val)
            {
                self->push_back(val);
            }
            value_type get(size_type i) throw (std::out_of_range) {
                if (i>=0 && i<self->size())
                    return (*self)[i];
                else
                    throw std::out_of_range("vector index out of range");
            }
            void set(size_type i,value_type val) throw (std::out_of_range) {
                if (i>=0 && i<self->size())
                    (*self)[i] = val;
                else
                    throw std::out_of_range("vector index out of range");
            }
        }
    };
}

%define specialize_std_vector(T)
#warning "specialize_std_vector - specialization for type T no longer needed"
%enddef

