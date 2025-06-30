#ifndef SVR_TEST
#define SVR_TEST

#include "iostream"

namespace svr
{
    struct Test
    {
        Test()
        {
            std::cout << "Constructor\n";
        }
        ~Test()
        {
            std::cout << "Destructor\n";
        }
        Test(const Test &)
        {
            std::cout << "Copy constructor\n";
        }
        Test(Test&&)
        {
            std::cout << "Move constructor\n";
        }
        Test& operator=(const Test&)
        {
            std::cout << "Copy assignment operator\n";
            return *this;
        }
        Test& operator=(Test&&)
        {
            std::cout << "Move assignment operator\n";
            return *this;
        }
    };
}

#endif