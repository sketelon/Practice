#include "stdio.h"

#define  SINGLE_INSTANCE(T)\
static T* Instance(bool bRelease = false)\
{\
    static T* m_pApp = NULL;\
    if (!bRelease)\
    {\
        if (m_pApp == NULL)\
        {\
            m_pApp = new T();\
        }\
    }\
    else\
    {\
        if (m_pApp != NULL)\
        {\
            delete m_pApp;\
            m_pApp = NULL;\
        }\
    }\
    return m_pApp;\
}\

#define SINGLETON_ANNOUNCEMENT(T)\
private:\
    T() {};\
    ~T() {};\
    SINGLE_INSTANCE(T)\
public:\
    static T* GetInstance()\
    {\
        return Instance();\
    }\
    static T* ReleaseInstance()\
    {\
        return Instance(true);\
    }\
public:\


#define GET_SINGLETON_CLASS(T)      T::GetInstance()
#define RELEASE_SINGLETON_CLASS(T)  T::ReleaseInstance()

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