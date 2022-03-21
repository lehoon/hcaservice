#ifndef _HCASERVICE_COPY_DISABLE_H
#define _HCASERVICE_COPY_DISABLE_H

#define DISABLE_COPY(Type) \
	Type(const Type&) = delete;\
	Type operator=(const Type&) = delete;

#define DISABLE_MOVE(Type) \
	Type(Type&&) = delete; \
	Type& operator=(Type&&) = delete;

#define DISABLE_COPY_MOVE(Type) \
	DISABLE_COPY(Type); \
	DISABLE_COPY_MOVE(Type)


#define DISABLE_COPY_AND_MOVE(Type) \
	private: \
	    Type(const Type&);\
		Type& operator=(const Type&);\
		Type(Type&&);\
		Type& operator=(Type&&);

#endif /*_HCASERVICE_COPY_DISABLE_H*/