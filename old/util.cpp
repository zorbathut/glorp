
#include "util.h"

#include <cmath>
#include <sstream>
#include <algorithm>
#include <limits>

#include <stdarg.h>

using namespace std;

bool ffwd = false;

static bool inthread = false;

string StringPrintf(const char *bort, ...) {
  CHECK(!inthread);
  inthread = true;
  
  static vector< char > buf(2);
  va_list args;

  int done = 0;
  bool noresize = false;
  do {
    if(done && !noresize)
      buf.resize(buf.size() * 2);
    va_start(args, bort);
    done = vsnprintf(&(buf[0]), buf.size() - 1,  bort, args);
    if(done >= (int)buf.size()) {
      CHECK(noresize == false);
      CHECK(buf[buf.size() - 2] == 0);
      buf.resize(done + 2);
      done = -1;
      noresize = true;
    } else {
      CHECK(done < (int)buf.size());
    }
    va_end(args);
  } while(done == buf.size() - 1 || done == -1);

  CHECK(done < (int)buf.size());

  string rv = string(buf.begin(), buf.begin() + done);
  
  CHECK(inthread);
  inthread = false;
  
  return rv;
};

string stringFromLongdouble(long double x) {
  stringstream str;
  str << x;
  return str.str();
}

string prettyFloatFormat(float v) {
  string borf = StringPrintf("%.2f", v);
  while(borf.size() > 4 && borf[borf.size() - 1] != '.')
    borf.erase(borf.end() - 1);
  if(borf[borf.size() - 1] == '.')
    borf.erase(borf.end() - 1);
  
  if(count(borf.begin(), borf.end(), '.') == 0)
    for(int x = borf.size(); x > 3; x -= 3)
      borf.insert(borf.begin() + x - 3, ',');
  return borf;
}

/*************
 * Misc
 */

int modurot(int val, int mod) {
  if(val < 0)
    val += abs(val) / mod * mod + mod;
  return val % mod;
}

void checkEndian() {
  float j = 12;
  CHECK(reinterpret_cast<unsigned char*>(&j)[3] == 65);
}

string rawstrFromFloat(float x) {
  // This is awful.
  CHECK(sizeof(x) == 4);
  CHECK(numeric_limits<float>::is_iec559);  // yayz
  checkEndian();
  unsigned char *dat = reinterpret_cast<unsigned char*>(&x);
  string beef;
  for(int i = 0; i < 4; i++)
    beef += StringPrintf("%02x", dat[i]);
  CHECK(beef.size() == 8);
  return beef;
}

float floatFromString(const string &x) {
  // This is also awful.
  CHECK(sizeof(float) == 4);
  CHECK(numeric_limits<float>::is_iec559); // wootz
  checkEndian();
  CHECK(x.size() == 8);
  float rv;
  unsigned char *dat = reinterpret_cast<unsigned char*>(&rv);
  for(int i = 0; i < 4; i++) {
    int v;
    CHECK(sscanf(x.c_str() + i * 2, "%2x", &v));
    CHECK(v >= 0 && v < 256);
    dat[i] = v;
  }
  CHECK(rawstrFromFloat(rv) == x);
  return rv;
}

string roman_number(int rid) {
  CHECK(rid >= 0);
  rid++; // okay this is kind of grim
  CHECK(rid < 40); // lazy
  string rv;
  while(rid >= 10) {
    rid -= 10;
    rv += "X";
  }
  
  if(rid == 9) {
    rid -= 9;
    rv += "IX";
  }
  
  if(rid >= 5) {
    rid -= 5;
    rv += "V";
  }
  
  if(rid == 4) {
    rid -= 4;
    rv += "IV";
  }
  
  rv += string(rid, 'I');
  
  return rv;
}

int roman_max() {
  return 38;
}

bool withinEpsilon(float a, float b, float e) {
  CHECK(e >= 0);
  if(a == b)
    return true; // I mean even if they're both 0
  float diff = a / b;
  if(diff < 0)
    return false; // it's not even the same *sign*
  if(abs(diff) < 1.0)
    diff = 1 / diff;
  return 1 - diff <= e;
}

float lerp(float lhs, float rhs, float dist) {
  return lhs * (1.0 - dist) + rhs * dist;
}

/*
static vector<pair<int, string> > errs;

void addErrorMessage(const string &str) {
  errs.push_back(make_pair(frameNumber, str));
}
vector<string> returnErrorMessages() {
  while(errs.size() && errs[0].first < frameNumber + 600)
    errs.erase(errs.begin());
  vector<string> rv;
  for(int i = 0; i < errs.size(); i++)
    rv.push_back(errs[i].second);
  return rv;
}
*/
