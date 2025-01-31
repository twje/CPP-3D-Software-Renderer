#pragma once

//------------------------------------------------------------------------------
template<typename T>
class Vector2
{
public:
	Vector2()
		: x(static_cast<T>(0))
		, y(static_cast<T>(0))
	{ }

	Vector2(T x, T y)
		: x(x)
		, y(y)
	{ }

	// Template constructor to allow implicit conversion from another Vector2 type
	template<typename U>
	explicit Vector2(const Vector2<U>& other)
		: x(static_cast<T>(other.x))
		, y(static_cast<T>(other.y))
	{ }

	T x;
	T y;
};

//------------------------------------------------------------------------------
using Vector2f = Vector2<float>;
using Vector2u = Vector2<uint32_t>;
using Vector2i = Vector2<int32_t>;