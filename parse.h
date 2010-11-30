#ifndef DNET_PARSE
#define DNET_PARSE






#include <map>
#include <string>
#include <vector>

using namespace std;

vector<string> tokenize(const string &in, const string &kar);
vector<string> tokenize_withempty(const string &in, const string &kar);
vector<int> sti(const vector<string> &foo);
vector<float> stf(const vector<string> &foo);
  
istream &getLineStripped(istream &ifs, string *out, int *line = NULL);

#endif
