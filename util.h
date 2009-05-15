#ifndef DNET_UTIL
#define DNET_UTIL

#include "debug.h"

#include <vector>

using namespace std;

extern bool ffwd;

/*************
 * Text processing
 */

#ifdef printf
#define PFDEFINED
#undef printf
#endif

string StringPrintf(const char *bort, ...) __attribute__((format(printf,1,2)));

#ifdef PFDEFINED
#define printf FAILURE
#undef PFDEFINED
#endif

string stringFromLongdouble(long double ld);
string prettyFloatFormat(float v);  // this formats it to have two decimal places at most, and cut those off once it reaches or passes four decimal digits

/*************
 * Misc
 */

int modurot(int val, int mod);

namespace detail {
  template <typename type, ::std::size_t size> char (&array_size_impl(type(&)[size]))[size];
}
#define ARRAY_SIZE(array) sizeof(::detail::array_size_impl(array))

#define VECTORIZE(array) vector<int>((array), (array) + ARRAY_SIZE(array))

string rawstrFromFloat(float x);
float floatFromString(const string &x);

template <typename T> vector<T*> ptrize(vector<T> *vt) {
  vector<T*> rv;
  for(int i = 0; i < vt->size(); i++)
    rv.push_back(&(*vt)[i]);
  return rv;
}

template <typename T> vector<const T*> ptrize(const vector<T> &vt) {
  vector<const T*> rv;
  for(int i = 0; i < vt.size(); i++)
    rv.push_back(&vt[i]);
  return rv;
}

template<typename T, typename U> vector<T> vdc(const vector<U> &vt) {
  vector<T> rv;
  for(int i = 0; i < vt.size(); i++)
    rv.push_back(vt[i]);
  return rv;
}

template<typename T, typename U, typename V> T clamp(T x, U min, V max) {
  CHECK(min <= max);
  if(x < min)
    return min;
  if(x > max)
    return max;
  return x;
}

template<typename T> T approach(const T &start, const T &target, const T &delta) {
  CHECK(delta >= 0);
  if(abs(start - target) <= delta)
    return target;
  else if(start < target)
    return start + delta;
  else if(start > target)
    return start - delta;
  else
    CHECK(0);  // oh god bear is driving car how can this be
}

// roman_number returns the roman equivalent of rid+1. This is retarded. If you're having trouble with that, for the love of god please fix it.
string roman_number(int rid);
int roman_max();

bool withinEpsilon(float a, float b, float e);

float lerp(float lhs, float rhs, float dist);

void addErrorMessage(const string &str);
vector<string> returnErrorMessages();

#endif
