#ifndef _HCASERVICE_SINGLETON_H_
#define _HCASERVICE_SINGLETON_H_

template<typename T>
class Singleton
{
public:
	virtual ~Singleton(){}
	static T& GetInstance() {
		static T ins;
		return ins;
	}
protected:
	Singleton(){}
};

#endif /*_HCASERVICE_SINGLETON_H_*/
