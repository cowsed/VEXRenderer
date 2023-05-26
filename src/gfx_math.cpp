
#include "gfx_math.h"
#include <iostream>
#include <ostream>
#include <math.h>

Vec2 Vec2::operator+(const Vec2 b) const
{
	return {x + b.x, y + b.y};
}
Vec2 Vec2::operator-(const Vec2 b) const
{
	return {x - b.x, y - b.y};
}
Vec2 Vec2::operator/(const float s) const
{
	return {x / s, y / s};
}
Vec2 Vec2::operator*(const float s) const
{
	return {x * s, y * s};
}
Vec2 operator/(const float s, const Vec2 v)
{
	return {s / v.x, s / v.y};
}

Vec2 operator*(const float s, const Vec2 v)
{
	return {s * v.x, s * v.y};
}

float lerpS(const float a, const float b, const float t)
{
	return (a * (1 - t)) + (b * t);
}

Vec2 Vec2::lerp(const Vec2 a, const Vec2 b, const float t)
{
	return Vec2{
		lerpS(a.x, b.x, t),
		lerpS(a.y, b.y, t),
	};
}

Vec2 Vec3::toVec2()
{
	return {x, y};
}

Vec4 Vec3::toVec4(const float w) const
{
	return {x, y, z, w};
}

float Vec3::length() const
{
	return sqrt(x * x + y * y + z * z);
}

Vec3 Vec3::operator-(const Vec3 b) const
{
	return {x - b.x, y - b.y, z - b.z};
}
Vec3 Vec3::operator+(const Vec3 b) const
{
	return {x + b.x, y + b.y, z + b.z};
}

Vec3 Vec3::operator/(const float s) const
{
	return {x / s, y / s, z / s};
}
Vec3 Vec3::operator*(const float s) const
{
	return {x * s, y * s, z * s};
}
Vec3 Vec3::operator*(const Vec3 v) const
{
	return {x * v.x, y * v.y, z * v.z};
}

Vec3 Vec3::lerp(const Vec3 a, const Vec3 b, const float t)
{
	return Vec3{
		lerpS(a.x, b.x, t),
		lerpS(a.y, b.y, t),
		lerpS(a.z, b.z, t),
	};
}

Vec3 Vec3::RotateY(float radians) const
{
	float nx = cos(radians) * x - sin(radians) * z;
	float nz = sin(radians) * x + cos(radians) * z;
	return {nx, y, nz};
}
Vec3 Vec3::RotateZ(float radians) const
{
	float ny = cos(radians) * y - sin(radians) * z;
	float nz = sin(radians) * y + cos(radians) * z;
	return {x, ny, nz};
}

Vec3 const Vec3::Normalize() const
{
	return (*this) / length();
}

float Vec3::Dot(const Vec3 b) const
{
	return x * b.x + y * b.y + z * b.z;
}

Vec3 const Vec4::toVec3()
{
	return {x, y, z};
}

std::ostream &operator<<(std::ostream &os, const Vec3 m)
{
	os << "(" << m.x << ", " << m.y << ", " << m.z << ")";
	return os;
}

Vec3 Mat4::Mul4xV3(Vec3 v)
{
	float x = X00 * v.x + X01 * v.y + X02 * v.z + X03;
	float y = X10 * v.x + X11 * v.y + X12 * v.z + X13;
	float z = X20 * v.x + X21 * v.y + X22 * v.z + X23;
	return {x, y, z};
}

uint32_t const Vec3::toIntColor() const
{
	uint32_t ir = static_cast<uint32_t>(255.0 * r);
	uint32_t ig = static_cast<uint32_t>(255.0 * g);
	uint32_t ib = static_cast<uint32_t>(255.0 * b);

	return 0x00000000 + (ir << 16) + (ig << 8) + (ib);
}

Vec4 Mat4::Mul4xV4(const Vec4 v) const
{
	float x = X00 * v.x + X01 * v.y + X02 * v.z + X03 * v.w;
	float y = X10 * v.x + X11 * v.y + X12 * v.z + X13 * v.w;
	float z = X20 * v.x + X21 * v.y + X22 * v.z + X23 * v.w;
	float w = X30 * v.x + X31 * v.y + X32 * v.z + X33 * v.w;
	return {x, y, z, w};
}

Mat4 Mat4::Mul4x4(const Mat4 b) const
{
	Mat4 m;
	m.X00 = X00 * b.X00 + X01 * b.X10 + X02 * b.X20 + X03 * b.X30;
	m.X10 = X10 * b.X00 + X11 * b.X10 + X12 * b.X20 + X13 * b.X30;
	m.X20 = X20 * b.X00 + X21 * b.X10 + X22 * b.X20 + X23 * b.X30;
	m.X30 = X30 * b.X00 + X31 * b.X10 + X32 * b.X20 + X33 * b.X30;
	m.X01 = X00 * b.X01 + X01 * b.X11 + X02 * b.X21 + X03 * b.X31;
	m.X11 = X10 * b.X01 + X11 * b.X11 + X12 * b.X21 + X13 * b.X31;
	m.X21 = X20 * b.X01 + X21 * b.X11 + X22 * b.X21 + X23 * b.X31;
	m.X31 = X30 * b.X01 + X31 * b.X11 + X32 * b.X21 + X33 * b.X31;
	m.X02 = X00 * b.X02 + X01 * b.X12 + X02 * b.X22 + X03 * b.X32;
	m.X12 = X10 * b.X02 + X11 * b.X12 + X12 * b.X22 + X13 * b.X32;
	m.X22 = X20 * b.X02 + X21 * b.X12 + X22 * b.X22 + X23 * b.X32;
	m.X32 = X30 * b.X02 + X31 * b.X12 + X32 * b.X22 + X33 * b.X32;
	m.X03 = X00 * b.X03 + X01 * b.X13 + X02 * b.X23 + X03 * b.X33;
	m.X13 = X10 * b.X03 + X11 * b.X13 + X12 * b.X23 + X13 * b.X33;
	m.X23 = X20 * b.X03 + X21 * b.X13 + X22 * b.X23 + X23 * b.X33;
	m.X33 = X30 * b.X03 + X31 * b.X13 + X32 * b.X23 + X33 * b.X33;
	return m;
}
Mat4 Mat4::operator*(const Mat4 other) const
{
	return Mul4x4(other);
}

Mat4 Mat4::GetNoPos() const
{
	Mat4 m = *this;
	m.X03 = 0;
	m.X13 = 0;
	m.X23 = 0;
	m.X33 = 0;
	return m;
}


Vec3 Mat4::GetPos() const
{
	return {X03, X13, X23};
}
void Mat4::SetPos(Vec3 v)
{
	X03 = v.x;
	X13 = v.y;
	X23 = v.z;
}

std::ostream &operator<<(std::ostream &os, const Mat4 &m)
{
	os << "[" << m.X00 << "\t" << m.X01 << "\t" << m.X02 << "\t" << m.X03 << "]" << std::endl;
	os << "[" << m.X10 << "\t" << m.X11 << "\t" << m.X12 << "\t" << m.X13 << "]" << std::endl;
	os << "[" << m.X20 << "\t" << m.X21 << "\t" << m.X22 << "\t" << m.X23 << "]" << std::endl;
	os << "[" << m.X30 << "\t" << m.X31 << "\t" << m.X32 << "\t" << m.X33 << "]" << std::endl;
	return os;
}

Mat4 Mat4Identity()
{
	// clang-format off
	return {
		1,0,0,0,
		0,1,0,0,
		0,0,1,0,
		0,0,0,1,
	};
	// clang-format on
}

Mat4 Translate3D(const Vec3 v)
{
	// clang-format off
	return {
		1, 0, 0, v.x,
		0, 1, 0, v.y,
		0, 0, 1, v.z,
		0, 0, 0, 1};
	// clang-format on
}

Mat4 RotateX(const float rad)
{
	float c = cos(rad);
	float s = sin(rad);

	// clang-format off
	return {
		1, 0,  0, 0,
		0, c, -s, 0,
		0, s,  c, 0,
		0, 0,  0, 1,
	};
	// clang-format on
}

Mat4 RotateY(const float rad)
{
	float c = cos(rad);
	float s = sin(rad);

	// clang-format off
	return {
		c, 0, -s, 0,
		0, 1,  0, 0,
		s, 0,  c, 0,
		0, 0,  0, 1,
	};
	// clang-format on
}
Mat4 RotateZ(const float rad)
{
	float c = cosf(rad);
	float s = sinf(rad);
	// clang-format off
	return {
		c, -s, 0, 0,
		s,  c, 0, 0,
		0,  0, 1, 0,
		0,  0, 0, 1,
	};
	// clang-format on
}
