#include <exception>

#define ERR_ALREADY_EXISTS "Cannot create network; node is already connected!\n"

#define ERR_NODE_NOT_SET "Node URL is not set yet!"

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
