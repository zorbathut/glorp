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
  
class kvData {
public:
  
  string category;
  map<string, string> kv;

  string debugOutput() const;

  string consume(const string &key);
  const string &read(const string &key) const;  // guarantees that it exists otherwise kablooey
  string saferead(const string &key) const;
  bool isDone() const;
  void shouldBeDone() const;

};

istream &getLineStripped(istream &ifs, string *out, int *line = NULL);
istream &getkvData(istream &ifs, kvData *out, int *line = NULL, int *endline = NULL);

string stringFromKvData(const kvData &kvd);

char fromHex(char in);

#endif
