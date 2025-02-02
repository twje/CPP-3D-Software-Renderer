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
template<typename T>
class Vector3
{
public:
	Vector3()
		: x(static_cast<T>(0))
		, y(static_cast<T>(0))
		, z(static_cast<T>(0))
	{ }

	Vector3(T x, T y, T z)
		: x(x)
		, y(y)
		, z(z)
	{ }

	// Template constructor to allow implicit conversion from another Vector2 type
	template<typename U>
	explicit Vector3(const Vector3<U>& other)
		: x(static_cast<T>(other.x))
		, y(static_cast<T>(other.y))
		, z(static_cast<T>(other.z))
	{ }

	Vector3<T> operator+(const Vector3<T>& other) const noexcept 
	{
		return { x + other.x, y + other.y, z + other.z };
	}

	Vector3<T>& operator+=(const Vector3<T>& other) noexcept 
	{
		x += other.x;
		y += other.y;
		z += other.z;
		return *this;
	}

	Vector3<T> operator-(const Vector3<T>& other) const noexcept
	{
		return { x - other.x, y - other.y, z - other.z };
	}

	Vector3<T>& operator-=(const Vector3<T>& other) noexcept
	{
		x -= other.x;
		y -= other.y;
		z -= other.z;
		return *this;
	}
	
	T operator*(float factor) const noexcept
	{
		return { x * factor, y * factor, z * factor };
	}

	Vector3<T>& operator*=(float factor) noexcept
	{
		x *= factor;
		y *= factor;
		z *= factor;
		return *this;
	}

	T Length() const noexcept
	{
		return sqrt(x * x + y * y + z * z);
	}

	T Dot(const Vector3<T>& other) const noexcept
	{
		return x * other.x + y * other.y + z * other.z;
	}

	Vector3<T> Cross(const Vector3<T>& other) const noexcept
	{
		return {
			y * other.z - z * other.y,
			z * other.x - x * other.z,
			x * other.y - y * other.x
		};
	}

	void Normalize() noexcept
	{
		T length = Length();
		x /= length;
		y /= length;
		z /= length;
	}

	T x;
	T y;
	T z;
};

//------------------------------------------------------------------------------
using Vector2f = Vector2<float>;
using Vector2u = Vector2<uint32_t>;
using Vector2i = Vector2<int32_t>;

using Vector3f = Vector3<float>;
using Vector3u = Vector3<uint32_t>;
using Vector3i = Vector3<int32_t>;