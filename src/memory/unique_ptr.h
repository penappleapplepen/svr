#ifndef SVR_UNIQUE_PTR
#define SVR_UNIQUE_PTR

#include <memory>
#include <utility>
#include <type_traits>
#include "templates/forward.h"
#include "helpers/test.h"
#include "templates/move.h"


/**
 Be mindful of these things
 a) Can a function be called on a const object as well? Try to make it
 const because a const function can be called by both const and non-const
 b) In assignment operators, always check if the new object is same as old object
 c) When you call delete, make sure that pointer actually exists to be deleted
 d) Function returning pointer can always be const
 e) Function returning reference needs to return const reference for const function
 and non-const reference for non-const function
 f) Also consider adding a r-value overload for a function which returns by value, but moves it
 g) For array specialization of unique_ptr, only difference is make_unique and operator[]. It gives
 opportunity to allocate array at run-time. Compile time constants are not allowed because why not
 just do it on stack?
 */
namespace svr
{
    template<typename T, typename Deleter = std::default_delete<T>>
    class unique_ptr
    {
        private:
            T* d_ptr;
            // Used for base class optimization
            // If Deleter does not have member variables
            // unique_ptr will just inherit from Deleter
            // This reduces storage 
            [[no_unique_address]] Deleter d_deleter;
        
        public:
            // Default constructor
            unique_ptr() : d_ptr{nullptr} {}
            // Constructor with std::nullptr_t
            unique_ptr(std::nullptr_t ptr) : d_ptr{ptr} {}
            // Constructor taking in plain pointer
            unique_ptr(T* ptr): d_ptr(ptr) {}
            // Constructor taking in plain pointer and deleter
            unique_ptr(T* ptr, Deleter&& deleter): d_ptr(ptr), d_deleter(std::forward<Deleter>(deleter)){}
            // Copy constructor
            unique_ptr(const unique_ptr&) = delete;
            // Copy assignment operator
            unique_ptr& operator=(const unique_ptr&) = delete;
            // Move constructor
            unique_ptr(unique_ptr&& other) noexcept : d_ptr(other.d_ptr), d_deleter(svr::move(other.d_deleter))
            {
                other.d_ptr = nullptr;
            }
            // Move assignment operator
            unique_ptr& operator=(unique_ptr&& other) noexcept
            {
                if(this != &other)
                {
                    if(d_ptr)
                    {
                        get_deleter()(d_ptr);
                    }
                    d_ptr = other.d_ptr;
                    d_deleter = svr::move(other.d_deleter);
                    other.d_ptr = nullptr;
                }
                return *this;
            }
            // Swap function
            void swap(unique_ptr& other)
            {
                std::swap(d_ptr, other.d_ptr);
                std::swap(d_deleter, other.d_deleter);
            }
            // Get pointer
            T* get() const
            {
                return d_ptr;
            }
            // -> operator
            T* operator->() const
            {
                return d_ptr;
            }
            // * operator
            T& operator*() const
            {
                return *d_ptr;
            }
            // Release pointer
            T* release()
            {
                T* temp = d_ptr;
                d_ptr = nullptr;
                return temp;
            }
            // Reset pointer
            void reset(T* newPtr = nullptr)
            {
                if(d_ptr == newPtr)
                {
                    return;
                }
                if(d_ptr)
                {
                    get_deleter()(d_ptr);
                }
                d_ptr = newPtr;
            }
            // operator bool
            explicit operator bool() const
            {
                return get();
            }
            // Get deleter function
            const Deleter& get_deleter() const
            {
                return d_deleter;
            }
            // Get deleter function
            Deleter& get_deleter()
            {
                return d_deleter;
            }
            // Destructor
            ~unique_ptr()
            {
                if(d_ptr)
                {
                    get_deleter()(d_ptr);
                }   
            }
    };

    // Note that we don't have a custom deleter because
    // we use operator new here which will be paired with
    // std::default_delete<T>
    // Only enabled for non-array types
    template<typename T, typename... Args,
             typename = std::enable_if_t<!std::is_array<T>::value>>
    unique_ptr<T> make_unique(Args&&... args)
    {
        return unique_ptr<T>(new T(svr::forward<Args>(args)...));
    }

    template<typename T, typename Deleter>
    class unique_ptr<T[], Deleter>
    {
        private:
            T* d_ptr;
            // Used for base class optimization
            // If Deleter does not have member variables
            // unique_ptr will just inherit from Deleter
            // This reduces storage 
            [[no_unique_address]] Deleter d_deleter;
        
        public:
            // Default constructor
            unique_ptr() : d_ptr{nullptr} {}
            // Constructor with std::nullptr_t
            unique_ptr(std::nullptr_t ptr) : d_ptr{ptr} {}
            // Constructor taking in plain pointer
            unique_ptr(T* ptr): d_ptr(ptr) {}
            // Copy constructor
            unique_ptr(const unique_ptr&) = delete;
            // Copy assignment operator
            unique_ptr& operator=(const unique_ptr&) = delete;
            // Move constructor
            unique_ptr(unique_ptr&& other) noexcept : d_ptr(other.d_ptr), d_deleter(svr::move(other.d_deleter))
            {
                other.d_ptr = nullptr;
            }
            // Move assignment operator
            unique_ptr& operator=(unique_ptr&& other) noexcept
            {
                if(this != &other)
                {
                    if(d_ptr)
                    {
                        get_deleter()(d_ptr);
                    }
                    d_ptr = other.d_ptr;
                    d_deleter = svr::move(other.d_deleter);
                    other.d_ptr = nullptr;
                }
                return *this;
            }
            // Swap function
            void swap(unique_ptr& other)
            {
                std::swap(d_ptr, other.d_ptr);
                std::swap(d_deleter, other.d_deleter);
            }
            // Get pointer
            T* get() const
            {
                return d_ptr;
            }
            // Index access operator
            T& operator[](size_t index)
            {
                return d_ptr[index];
            }
            const T& operator[](size_t index) const
            {
                return d_ptr[index];
            }
            // Release pointer
            T* release()
            {
                T* temp = d_ptr;
                d_ptr = nullptr;
                return temp;
            }
            // Reset pointer
            void reset(T* newPtr = nullptr)
            {
                if(d_ptr == newPtr)
                {
                    return;
                }
                if(d_ptr)
                {
                    get_deleter()(d_ptr);
                }
                d_ptr = newPtr;
            }
            // operator bool
            explicit operator bool() const
            {
                return get();
            }
            // Get deleter function
            const Deleter& get_deleter() const
            {
                return d_deleter;
            }
            // Get deleter function
            Deleter& get_deleter()
            {
                return d_deleter;
            }
            // Destructor
            ~unique_ptr()
            {
                if(d_ptr)
                {
                    get_deleter()(d_ptr);
                }   
            }
    };

    // Array version of make_unique, only enabled for unbounded array types
    template<typename T,
             typename = std::enable_if_t<std::is_unbounded_array<T>::value>>
    unique_ptr<T> make_unique(std::size_t n)
    {
        using ElementType = typename std::remove_extent<T>::type;
        return unique_ptr<T>(new ElementType[n]());
    }
}

#endif