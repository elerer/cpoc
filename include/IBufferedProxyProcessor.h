#ifndef __IBUFFERPROCESSOR__
#define __IBUFFERPROCESSOR__
template <typename T>
class IBufferedProxyProcessor
{
  public:
    //push 1 element
    virtual bool Push(const T &t) = 0;
    //push max of size elements
    virtual int Push(const T *tArr, const int &size) = 0;
    //pop max of size elements into ret
    virtual int Pop(T *ret, const int &size) = 0;
    //get number of elements that are available for read
    virtual int ReadAvailable() = 0;
    //get write space to write elements
    virtual int WriteAvailable() = 0;

    virtual ~IBufferedProxyProcessor(){};
};
#endif