#ifndef SINGLETONONE_H
#define SINGLETONONE_H

#include <usServiceInterface.h>
#include <usServiceReference.h>
#include <usServiceRegistration.h>


//![s1]
class SingletonOne
{
public:

  static SingletonOne& GetInstance();

  // Just some member
  int a;

private:

  SingletonOne();
  ~SingletonOne();

  // Disable copy constructor and assignment operator.
  SingletonOne(const SingletonOne&);
  SingletonOne& operator=(const SingletonOne&);
};
//![s1]

class SingletonTwoService;

//![ss1]
class SingletonOneService
{
public:

  // This will return a SingletonOneService instance with the
  // lowest service id at the time this method was called the first
  // time and returned a non-null value (which is usually the instance
  // which was registered first). A null-pointer is returned if no
  // instance was registered yet.
  static SingletonOneService* GetInstance();

  int a;

private:

  // Only our module activator class should be able to instantiate
  // a SingletonOneService object.
  friend class MyActivator;

  SingletonOneService();
  ~SingletonOneService();

  // Disable copy constructor and assignment operator.
  SingletonOneService(const SingletonOneService&);
  SingletonOneService& operator=(const SingletonOneService&);
};
//![ss1]

#endif // SINGLETONONE_H
