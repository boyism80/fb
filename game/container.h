#ifndef __CONTAINER_H__
#define	__CONTAINER_H__

namespace fb { namespace game {

class session;

template <typename T>
class container
{
private:
	session&				_owner;
	T**						_elements;
	uint8_t					_size;
	bool					_auto_release;

protected:
	container(session& owner, uint8_t size, bool auto_release = true);
	~container();

protected:
	T*						set(T* element, int position);

public:
	session&				owner();
	const session&			owner() const;
	virtual T*				at(uint8_t index) const;
	virtual bool			add(T& element);
	virtual uint8_t			add(T* element);
	virtual bool			remove(uint8_t index);
	virtual bool			swap(uint8_t src, uint8_t dest);

	bool					free() const;
	uint8_t					free_size() const;

public:
	T*						operator [] (int index);
	const T*				operator [] (int index) const;
};

#include "container.hpp"

} }

#endif // !__CONTAINER_H__
