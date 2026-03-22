#ifndef QUAT
#define QUAT

#include <cmath>
#include <iostream>

template< typename T >
struct matrix_t
{
	T data[16];
};

template< typename T >
struct vector3_t
{
	T x, y, z;
};

template< typename T >
class Quat
{
  public:
	const T* data() const { return m_value; }
	Quat() : m_value{ 0, 0, 0, 0 } {}
	Quat(T a, T b, T c, T d) : m_value{ b, c, d, a } {}
	Quat(T angle, bool radians, const vector3_t< T >& axis)
	{
		const T half_angle = (radians ? angle / static_cast< const T >(2.0) : angle * static_cast< const T >(M_PI / 360.0));
		const T s = std::sin(half_angle);
		T nrm = 0;
		try
		{
			nrm = div(s, std::sqrt(sqr(axis.x) + sqr(axis.y) + sqr(axis.z)));
		} catch (std::logic_error& e)
		{
			throw;
		}
		m_value[3] = std::cos(half_angle);
		m_value[0] = axis.x * nrm;
		m_value[1] = axis.y * nrm;
		m_value[2] = axis.z * nrm;
	}
	Quat< T > operator+(const Quat< T >& rhs) const { return operation(m_value, rhs.m_value, 1); }
	Quat< T >& operator+=(const Quat< T >& rhs)
	{
		*this = *this + rhs;
		return *this;
	}
	Quat< T > operator-(const Quat< T >& rhs) const { return operation(m_value, rhs.m_value, -1); }
	Quat< T >& operator-=(const Quat< T >& rhs)
	{
		*this = *this - rhs;
		return *this;
	}
	Quat< T > operator*(const Quat< T >& rhs) const
	{
		return Quat< T >(
			m_value[3] * rhs.m_value[3] - rhs.m_value[0] * m_value[0] - m_value[1] * rhs.m_value[1] - rhs.m_value[2] * m_value[2],
			m_value[3] * rhs.m_value[0] + rhs.m_value[3] * m_value[0] + m_value[1] * rhs.m_value[2] - rhs.m_value[1] * m_value[2],
			m_value[3] * rhs.m_value[1] + rhs.m_value[3] * m_value[1] + m_value[2] * rhs.m_value[0] - rhs.m_value[2] * m_value[0],
			m_value[3] * rhs.m_value[2] + rhs.m_value[3] * m_value[2] + m_value[0] * rhs.m_value[1] - rhs.m_value[0] * m_value[1]);
	}
	Quat< T > operator*(const vector3_t< T >& coords) const
	{
		return *this * Quat< T >(0, coords.x, coords.y, coords.z);
	}
	Quat< T > operator*(const T cnst) const
	{
		return Quat< T >(m_value[3] * cnst, m_value[0] * cnst, m_value[1] * cnst, m_value[2] * cnst);
	}
	Quat< T > operator~() const { return Quat< T >(m_value[3], -m_value[0], -m_value[1], -m_value[2]); }
	bool operator==(const Quat< T >& rhs) const
	{
		for (int i = 0; i < 4; ++i)
			if (m_value[i] != rhs.m_value[i])
				return false;
		return true;
	}
	bool operator!=(const Quat< T >& rhs) const { return !(*this == rhs); }
	explicit operator T() const { return norm().second; }
	matrix_t< T > rotation_matrix() const
	{
		const T nrm = norm().first;
		T xx = 0, xy = 0, xz = 0, xw = 0, yy = 0, yz = 0, yw = 0, zz = 0, zw = 0;
		try
		{
			xx = div(m_value[0] * m_value[0], nrm);
			xy = div(m_value[0] * m_value[1], nrm);
			xz = div(m_value[0] * m_value[2], nrm);
			xw = div(m_value[0] * m_value[3], nrm);
			yy = div(m_value[1] * m_value[1], nrm);
			yz = div(m_value[1] * m_value[2], nrm);
			yw = div(m_value[1] * m_value[3], nrm);
			zz = div(m_value[2] * m_value[2], nrm);
			zw = div(m_value[2] * m_value[3], nrm);
		} catch (std::logic_error& e)
		{
			throw;
		}
		const T zero = static_cast< const T >(0);
		const T one = static_cast< const T >(1);
		const T two = static_cast< const T >(2);
		return {
			one - two * (yy + zz),
			two * (xy + zw),
			two * (xz - yw),
			zero,
			two * (xy - zw),
			one - two * (xx + zz),
			two * (yz + xw),
			zero,
			two * (xz + yw),
			two * (yz - xw),
			one - two * (xx + yy),
			zero,
			zero,
			zero,
			zero,
			one,
		};
	}
	matrix_t< T > matrix() const
	{
		const T a = m_value[3];
		const T b = m_value[0];
		const T c = m_value[1];
		const T d = m_value[2];

		return {
			a, -b, -c, -d, b, a, -d, c, c, d, a, -b, d, -c, b, a,
		};
	}
	T angle(bool radians = true) const
	{
		T angleRadians = 0;
		try
		{
			angleRadians = static_cast< const T >(2) * std::acos(div(m_value[3], norm().second));
		} catch (std::logic_error& e)
		{
			throw;
		}
		return (radians ? angleRadians : angleRadians * static_cast< const T >(180.0 / M_PI));
	}
	vector3_t< T > apply(const vector3_t< T >& vec) const
	{
		Quat< T > q = *this;
		try
		{
			q = q * div(static_cast< const T >(1.0), norm().second);
		} catch (std::logic_error& e)
		{
			throw;
		}
		q = q * vec * (~q);
		return {
			q.m_value[0],
			q.m_value[1],
			q.m_value[2],
		};
	}

  private:
	T m_value[4];
	T div(const T& a, const T& b) const
	{
		if (b == static_cast< const T >(0))
			throw std::logic_error("Division by zero Error");
		return a / b;
	}
	Quat< T > operation(const T* a, const T* b, const T sign) const
	{
		return Quat< T >(a[3] + b[3] * sign, a[0] + b[0] * sign, a[1] + b[1] * sign, a[2] + b[2] * sign);
	}
	T sqr(const T a) const { return a * a; }
	std::pair< T, T > norm() const
	{
		T s = 0;
		for (const T& i : m_value)
			s += sqr(i);
		return { s, std::sqrt(s) };
	}
};
#endif	  // QUAT
