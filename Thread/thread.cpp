#include <iostream>
#include <thread>

int count(int n)
{
    for (int i = 0; i < n; ++i)
        std::cout << i;
}

int main()
{
    int n = 5;
    std::thread thread_obj(count, n);
    return 0;
}