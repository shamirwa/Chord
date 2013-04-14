#ifndef _EXCEPTION_H
#define _EXCEPTION_H

#include <exception>

class MyException:public exception
{

	public:
	string exceptionString;
	
	MyException() throw()
	{

	
	}

	virtual const char* what() const throw()
  {
		return exceptionString.c_str();
	}
	~MyException() throw()
	{
		
	}
	
	void setErrorMessage(string message) throw()
	{
		exceptionString = message;
	}

	string displayMessage() throw()
	{
		return exceptionString;
	}

}myError;

#endif
