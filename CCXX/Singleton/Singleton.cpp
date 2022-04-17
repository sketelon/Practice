#include "sketelon.h"


class CTest
{
    SINGLETON_ANNOUNCEMENT(CTest);
public:
    void Print()
    {
        printf("Hello World!\r\n");
    }

};

int main()
{
    GET_SINGLETON_CLASS(CTest)->Print();

    RELEASE_SINGLETON_CLASS(CTest);

    return 0;
}