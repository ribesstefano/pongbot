#ifndef POINT_H_
#define POINT_H_

#include<cassert>

template<typename T, int nDimensions = 2>
class Point {
private:
  T (&elements_)[nDimensions];

public:
  typedef T ValueType;

  T& operator[](int const i) {
    return elements_[i];
  }

  T const& operator[](int const i) const {
    return elements_[i];
  }

  void operator+=(Point const& other) {
    for(int i = 0; i < nDimensions; ++i) {
      elements_[i] += other.elements_[i];
    }
  }

  void operator-=(Point const& other) {
    for(int i = 0; i < nDimensions; ++i) {
      elements_[i] -= other.elements_[i];
    }
  }

  void operator*=(Point const& other) {
    for(int i = 0; i < nDimensions; ++i) {
      elements_[i] *= other.elements_[i];
    }
  }

  void operator*(T const& a) {
    for(int i = 0; i < nDimensions; ++i) {
      elements_[i] *= a;
    }
  }

  friend Point operator+(Point const& a, Point const& b) {
    Point ret(a);
    ret += b;
    return ret;
  }

  friend Point operator-(Point const&a, Point const& b) {
    Point ret(a);
    ret -= b;
    return ret;
  }

  friend Point operator*(Point const&a, Point const& b) {
    Point ret(a);
    ret *= b;
    return ret;
  }

  void set_x(const T x) {
#pragma HLS INLINE
    elements_[0] = x;
  }

  void set_y(const T y) {
#pragma HLS INLINE
    elements_[1] = y;
  }

  void set_z(const T z) {
#pragma HLS INLINE
    assert(nDimensions == 2);
    elements_[2] = z;
  }

  T const& get_x() const {
#pragma HLS INLINE
    return elements_[0];
  }

  T const& get_y() const {
#pragma HLS INLINE
    return elements_[1];
  }

  T const& get_z() const {
    assert(nDimensions == 3);
    return elements_[2];
  }

  // NOTE: This is NOT synthesizeable, I gues because there's no assert.
  Point(T (&element)[nDimensions]): elements_(element) {}

  // NOTE: This is synthesizeable, I guess because there's an assert statement.
  Point(T (&element)[nDimensions], int x, int y): elements_(element) {
    assert(nDimensions == 2);
    elements_[0] = x;
    elements_[1] = y;
  }

  // NOTE: This is synthesizeable, I guess because there's an assert statement.
  Point(T (&element)[nDimensions], int x, int y, int z): elements_(element) {
    assert(nDimensions == 3);
    elements_[0] = x;
    elements_[1] = y;
    elements_[2] = z;
  }
};

typedef Point<int, 2> Point2D;
typedef Point<int, 3> Point3D;

#endif