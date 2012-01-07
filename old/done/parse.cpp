
#include "parse.h"

#include "util.h"

#include <iostream>
#include <algorithm>

using namespace std;

namespace Glorp {
  vector<string> tokenize(const string &in, const string &kar) {
    string::const_iterator cp = in.begin();
    vector<string> oot;
    while(cp != in.end()) {
      while(cp != in.end() && count(kar.begin(), kar.end(), *cp))
        cp++;
      if(cp != in.end())
        oot.push_back(string(cp, find_first_of(cp, in.end(), kar.begin(), kar.end())));
      cp = find_first_of(cp, in.end(), kar.begin(), kar.end());
    };
    return oot;
  };

  vector<string> tokenize_withempty(const string &in, const string &kar) {
    vector<string> rv;
    
    const char *pt = in.c_str();
    while(*pt) {
      const char *nd = pt;
      while(*nd && !count(kar.begin(), kar.end(), *nd))
        nd++;
      
      rv.push_back(string(pt, nd));
      pt = nd;
      if(*pt)
        pt++;
    }
    
    return rv;
  };

  vector<int> sti(const vector<string> &foo) {
    vector<int> bar;
    for(int i = 0; i < foo.size(); i++)
      bar.push_back(atoi(foo[i].c_str()));
    return bar;
  };

  vector<float> stf(const vector<string> &foo) {
    vector<float> bar;
    for(int i = 0; i < foo.size(); i++)
      bar.push_back(atof(foo[i].c_str()));
    return bar;
  };

  istream &getLineStripped(istream &ifs, string *out, int *line) {
    while(getline(ifs, *out)) {
      if(line)
        (*line)++;
      *out = string(out->begin(), find(out->begin(), out->end(), '#'));
      while(out->size() && isspace(*out->begin()))
        out->erase(out->begin());
      while(out->size() && isspace((*out)[out->size()-1]))
        out->erase(out->end() - 1);
      if(out->size())
        return ifs;
    }
    return ifs;
  }
}
