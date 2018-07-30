#ifndef __SIMPLEBEFFEREDPROXY__
#define __SIMPLEBEFFEREDPROXY__
#include <boost/lockfree/spsc_queue.hpp>
#include<IBufferedProxyProcessor.h>

//buffered samples, does not perform any processing
template <typename T, int N>
class SimpleBufferedProxy : public IBufferedProxyProcessor<T>
{
  public: /*methods*/
    virtual bool Push(const T &t) override;

    virtual int Push(const T *tArr, const int &size) override;

    virtual int Pop(T *ret, const int &size) override;

    virtual int ReadAvailable() override;

    virtual int WriteAvailable() override;

  private: /*members*/
    boost::lockfree::spsc_queue<T, boost::lockfree::capacity<N>> _ringBuffer;
};

template <typename T, int N>
bool SimpleBufferedProxy<T, N>::Push(const T &t)
{
    return _ringBuffer.push(t);
}

template <typename T, int N>
int SimpleBufferedProxy<T, N>::Push(const T *tArr, const int &size)
{
    return (int)_ringBuffer.push(tArr, size);
}

template <typename T, int N>
int SimpleBufferedProxy<T, N>::Pop(T *ret, const int &size)
{
    return (int)_ringBuffer.pop(ret,size);
}

template <typename T, int N>
int SimpleBufferedProxy<T, N>::ReadAvailable()
{
    return (int)_ringBuffer.read_available();
}

template <typename T, int N>
int SimpleBufferedProxy<T, N>::WriteAvailable()
{
    return (int)_ringBuffer.write_available();
}
#endif