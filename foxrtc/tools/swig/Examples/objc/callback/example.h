/* File : example.h */

#include <cstdio>
#include <iostream>
#include <string>
#include <vector>
class Callback {
public:
	virtual ~Callback() { std::cout << "Callback::~Callback()" << std:: endl; }
	virtual void run() { std::cout << "Callback::run()" << std::endl; }
};


class Caller {
private:
	Callback *_callback;
public:
	Caller(): _callback(0) {}
	~Caller() { delCallback(); }
	void delCallback() { delete _callback; _callback = 0; }
	void setCallback(Callback *cb) { delCallback(); _callback = cb; }
	void call() { if (_callback) _callback->run(); }
};



class Test
{
    Test()
    {}
};

class EventArgs
{
	public:
	const std::vector<Caller>& getVector()
	{
		return _all;
	}
	public:
	std::string GetName()
	{
		return "";
	}
	static const std::string StaticName()
	{
		return "123";
	}
	private:
	std::vector<Caller> _all;
};
