#ifndef SINGLETONTWO_H
#define SINGLETONTWO_H

#include <usGlobalConfig.h>
#include <usServiceInterface.h>


class SingletonTwo
{
public:

  static SingletonTwo& GetInstance();

  int b;

private:

  SingletonTwo();
  ~SingletonTwo();

  // Disable copy constructor and assignment operator.
  SingletonTwo(const SingletonTwo&);
  SingletonTwo& operator=(const SingletonTwo&);
};

class SingletonTwoService
{
public:

  static SingletonTwoService* GetInstance();

  int b;

private:

  friend class MyActivator;

  SingletonTwoService();
  ~SingletonTwoService();

  // Disable copy constructor and assignment operator.
  SingletonTwoService(const SingletonTwoService&);
  SingletonTwoService& operator=(const SingletonTwoService&);
};

#endif // SINGLETONTWO_H
