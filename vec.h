#pragma once

#include <cmath>
#include <cassert>
#include <algorithm>
#include <initializer_list>
#include <iostream>

// TODO
// Union for getting .x .y and .z

/*
Use explicit when writing constructors that take a single parameter, unless implicit conversions are specifically desired. 
(This applies to everything in C++, not just vectors/matrices.) Done

Provide a constructor that fills a whole vector with a single value. Done

Provide vector constructors that take the most common combinations of vector and scalar values (for instance, a 
4-vector constructor taking a 3-vector for xyz and a scalar for w). As with swizzles, it’s possible to heroically support 
every possible combination, but it’s really overkill and you’ll never use most of them.

For matrices, provide an easy way to get zero and identity matrices.

For matrices (at least the most common sizes), please do provide componentwise constructors. Everyone has these for vectors,
but not so often for matrices; still, they do come in handy now and then (projection matrices, for example).

If you’re using C++11, consider supporting array initialization syntax (by adding a constructor from std::initializer_list<T>), 
as this solves the componentwise issue nicely.

Provide ways to build a matrix out of vectors, both row-wise and column-wise. These belong as free functions, not constructors,
though, due to the ambiguity of what’s meant when you just see Matrix(vec0, vec1, vec2, vec3).

Don’t use constructors for operations more complex than just filling in components—for example, building a rotation matrix 
from Euler angles or a quaternion.

Provide constructors that take an array of floats, as that will ease conversion between other math libs’ vector/matrix types and yours. DONE

If you’re using C++11, write constexpr constructors where applicable, to hint to the compiler that it can optimize the constructor away.
*/

static const double PI = 3.14159265358979323846264338327950288;
static const double EPS = 1e-8;
static const double EPS2 = EPS * EPS;
static const double EPS3 = EPS * EPS * EPS;

template <typename T, int n>
struct Vec
{
	T data[n];

	Vec()
	{
		for (int i = 0; i < n; i++)
		{
			data[i] = 0;
		}
	}

	explicit Vec(T t)
	{
		for (int i = 0; i < n; i++)
		{
			data[i] = t;
		}
	}

	Vec(const T& t0, const T& t1, const T& t2)
	{
		assert(n == 3);
		data[0] = t0;
		data[1] = t1;
		data[2] = t2;
	}

	Vec(const T& t0, const T& t1, const T& t2, const T& t3)
	{
		assert(n == 4);
		data[0] = t0;
		data[1] = t1;
		data[2] = t2;
		data[3] = t3;
	}

	template <int m>
	explicit Vec(const Vec<T, m>& v, const T& extendValue = T(0))
	{
		int min = std::min(m, n);
		for (int i = 0; i < min; ++i)
		{
			data[i] = v[i];
		}
		for (int i = min; i < n; ++i)
		{
			data[i] = extendValue;
		}
	}

	Vec(const T a[], const int len, const T& extendValue = T(0))
	{
		int min = std::min(n, len);
		for (int i = 0; i < min; ++i)
		{
			data[i] = a[i];
		}
		for (int i = min; i < n; ++i)
		{
			data[i] = extendValue;
		}
	}

    template <int m>
    T& operator =(const Vec<T, m>& v)
    {
        int min = std::min(m, n);
        for(int i = 0; i < min; ++i)
        {
            data[i] = v[i];
        }
    }

	T& operator [](const int i)
	{
		return data[i];
	}

	const T& operator [](const int i) const
	{
		return data[i];
	}

	Vec operator -() const
	{
		return Vec(*this) *= -1;
	}

	Vec& operator +=(const Vec& v)
	{
		for (int i = 0; i < n; ++i)
		{
			data[i] += v[i];
		}
		return *this;
	}

	Vec& operator -=(const Vec& v)
	{
		for (int i = 0; i < n; ++i)
		{
			data[i] -= v[i];
		}
		return *this;
	}

	Vec& operator *=(const T a)
	{
		for (int i = 0; i < n; ++i)
		{
			data[i] *= a;
		}
		return *this;
	}

	Vec& operator /= (const T a)
	{
		const T inv(1 / a);
		for (int i = 0; i < n; ++i)
		{
			data[i] *= inv;
		}
		return *this;
	}

	Vec& operator *=(const Vec& v)
	{
		for (int i = 0; i < n; ++i)
		{
			data[i] *= v[i];
		}
		return *this;
	}

	Vec& operator /=(const Vec& v)
	{
		for (int i = 0; i < n; ++i)
		{
			data[i] /= v[i];
		}
		return *this;
	}

	Vec operator +(const Vec& v) const
	{
		return Vec(*this) += v;
	}

	Vec operator -(const Vec& v) const 
	{
		return Vec(*this) -= v;
	}

	Vec operator *(const T a) const 
	{
		return Vec(*this) *= a;
	}

	Vec operator /(const T a) const
	{
		return Vec(*this) /= a;
	}

	Vec operator *(const Vec& v) const
	{
		return Vec(*this) *= v;
	}

	Vec operator /(const Vec& v) const 
	{
		return Vec(*this) /= v;
	}

	Vec& normalize()
	{
		assert(dot(*this, *this) > EPS2);
		return *this /= sqrt(dot(*this, *this));
	}

	T length() const
	{
		assert(dot(*this, *this) > EPS2);
		return sqrt(dot(*this, *this));
	}

    void print()
    {
        for(int i = 0; i < n; i++)
        {
            std::cout << data[i] << " ";
        }
        std::cout << std::endl;
    }
};

typedef Vec<float, 2> Vec2;
typedef Vec<float, 3> Vec3;
typedef Vec<float, 4> Vec4;

template <typename T, int n>
inline T dot(const Vec<T, n>& a, const Vec<T, n>& b)
{
	T r(0);
	for (int i = 0; i < n; ++i)
	{
		r += a[i] * b[i];
	}
	return r;
}

template <typename T>
inline Vec<T, 3> cross(const Vec<T, 3>& a, const Vec<T, 3>& b)
{
	return	Vec<T, 3>(a[1] * b[2] - a[2] * b[1],
					  a[2] * b[0] - a[0] * b[2],
					  a[0] * b[1] - a[1] * b[0]);
}

template <typename T, int n>
inline Vec<T, n> normalize(const Vec<T, n>& v)
{
	return Vec<T, n>(v).normalize();
}

