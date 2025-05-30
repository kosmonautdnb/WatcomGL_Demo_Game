#ifndef __UTIL_HPP__
#define __UTIL_HPP__

#define PI 3.14159265358979323846

template<class T>
T min(const T&a, const T&b) {
  return a<b?a:b;
}

template<class T>
T max(const T&a, const T&b) {
  return a>b?a:b;
}

template<class T>
T clamp(const T&a, const T &b, const T &c) {
  return a<b?b:(a>c?c:a);
}

template<class T>
T saturate(const T&a) {
  return clamp(a,(T)0,(T)1);
}

float randomLike(const int index);

#endif //__UTIL_HPP__
