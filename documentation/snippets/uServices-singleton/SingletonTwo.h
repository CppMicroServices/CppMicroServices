#ifndef SINGLETONTWO_H
#define SINGLETONTWO_H

#include <usConfig.h>
#include <usServiceInterface.h>

#include US_BASECLASS_HEADER

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

class SingletonTwoService : public US_BASECLASS_NAME
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

US_DECLARE_SERVICE_INTERFACE(SingletonTwoService, "org.cppmicroservices.snippet.SingletonTwoService")

#endif // SINGLETONTWO_H
