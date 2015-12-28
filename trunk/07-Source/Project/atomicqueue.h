#ifndef ATOMICQUEUE_H
#define ATOMICQUEUE_H

#include <QMutex>
#include <QQueue>

template <class T>
class AtomicQueue : public QQueue<T> {
public:
   inline AtomicQueue() {}
   inline ~AtomicQueue() {}
   inline void enqueue(const T &t) {
      mutex.lock();
      QQueue<T>::enqueue(t);
      mutex.unlock();
   }
   inline T dequeue() {
      mutex.lock();
      T t = QQueue<T>::dequeue();
      mutex.unlock();
      return t;
   }
private:
   QMutex mutex;
};

#endif // ATOMICQUEUE_H
