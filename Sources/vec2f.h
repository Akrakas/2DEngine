#ifndef __VEC2F_H__
#define __VEC2F_H__
//Vectors calcultions

#include <cmath>


class vec2f {
public:
	float x, y;

	vec2f() :x(0), y(0) {}
	vec2f(float x, float y) : x(x), y(y) {}
	vec2f(const vec2f& v) : x(v.x), y(v.y) {}

	vec2f& operator=(const vec2f& v) {
		x = v.x;
		y = v.y;
		return *this;
	}

	vec2f operator+(vec2f& v) {
		return vec2f(x + v.x, y + v.y);
	}
	vec2f operator-(vec2f& v) {
		return vec2f(x - v.x, y - v.y);
	}

	vec2f& operator+=(vec2f& v) {
		x += v.x;
		y += v.y;
		return *this;
	}
	vec2f& operator-=(vec2f& v) {
		x -= v.x;
		y -= v.y;
		return *this;
	}


	vec2f operator+(float s) {
		return vec2f(x + s, y + s);
	}
	vec2f operator-(float s) {
		return vec2f(x - s, y - s);
	}
	vec2f operator*(float s) {
		return vec2f(x * s, y * s);
	}
	vec2f operator/(float s) {
		return vec2f(x / s, y / s);
	}


	vec2f& operator+=(float s) {
		x += s;
		y += s;
		return *this;
	}
	vec2f& operator-=(float s) {
		x -= s;
		y -= s;
		return *this;
	}
	vec2f& operator*=(float s) {
		x *= s;
		y *= s;
		return *this;
	}
	vec2f& operator/=(float s) {
		x /= s;
		y /= s;
		return *this;
	}

	void set(float x, float y) {
		this->x = x;
		this->y = y;
	}

	void rotate(double deg) {
		double theta = deg / 180.0 * M_PI;
		double c = cos(theta);
		double s = sin(theta);
		double tx = x * c - y * s;
		double ty = x * s + y * c;
		x = tx;
		y = ty;
	}

	vec2f& normalize() {
		if (length() == 0) return *this;
		*this *= (1.0 / length());
		return *this;
	}

	vec2f normal() {
		vec2f v1(x,y);
		if (v1.length() == 0) return v1;
		v1 *= (1.0 / v1.length());
		return v1;
	}

	float dist(vec2f v) const {
		vec2f d(v.x - x, v.y - y);
		return d.length();
	}
	float length() const {
		return std::sqrt(x * x + y * y);
	}

	float sqrlength() const {
		return (x * x + y * y);
	}

	void truncate(double length) {
		double angle = atan2f(y, x);
		x = length * cos(angle);
		y = length * sin(angle);
	}

	vec2f ortho() const {
		return vec2f(y, -x);
	}

	float dot(vec2f v) {
		return x * v.x + y * v.y;
	}

	void reverse() {
		x = -x;
		y = -y;
	}

	static float cross(vec2f v1, vec2f v2) {
		return (v1.x * v2.y) - (v1.y * v2.x);
	}
};

#endif
