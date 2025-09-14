#include <iostream>
#include <cmath>
using namespace std;

struct vect2 {
	float x, y;

	vect2(float x = 1.0f, float y = 1.0f) : x(x), y(y) {}

	vect2 operator+(const vect2& another) const{
		return vect2(x + another.x, y + another.y);
	}
	vect2& operator+=(const vect2& another){
		x += another.x;
	       	y += another.y;
		return *this;
	}
	vect2 operator*=(float modif) const{
		return vect2(x * modif, y * modif);
	}
	vect2& operator*=(float modif) {
		x*=modif;
		y*=modif;
		return *this;
	}
	static vect2 zero() { return vect2(0.0f, 0.0f);}
};

struct Body {
	vect2 pos;
	vect2 vel;
	vect2 acc;
	float mass;
	float radius;

	Body(vect2 pos, vect2 vel, float mass, float radius) : pos(pos), vel(vel), acc(vect2::zero()), mass(mass), radius(radius) {}

	void update(float dt) {
		vel += acc * dt;
		pos += vel * dt;
	}
}
