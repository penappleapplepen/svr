#ifndef SVR_UNIQUE_PTR
#define SVR_UNIQUE_PTR

template <typename T>
class unique_ptr
{
private:
    T *d_ptr;
};

#endif