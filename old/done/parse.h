#ifndef GLORP_PARSE
#define GLORP_PARSE

#include <map>
#include <string>
#include <vector>

using namespace std;

namespace Glorp {
  vector<string> tokenize(const string &in, const string &kar);
  vector<string> tokenize_withempty(const string &in, const string &kar);
  vector<int> sti(const vector<string> &foo);
  vector<float> stf(const vector<string> &foo);
    
  istream &getLineStripped(istream &ifs, string *out, int *line = NULL);
}

#endif
