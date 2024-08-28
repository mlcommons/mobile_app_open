#include <fstream>
#include <iostream>
#include <map>
#include <regex>
#include <set>
#include <sstream>
#include <tuple>
#include <vector>

#ifndef _STABLE_DIFFUSION_BPE_H_
#define _STABLE_DIFFUSION_BPE_H_

using namespace std;

class bpe {
 private:
  string vocab_file = "vocab.txt";
  string merges_file = "merges.txt";
  std::map<std::string, int> vocab;
  std::vector<tuple<string, string>> merges;
  const int START_OF_TEXT = 49406;
  const int END_OF_TEXT = 49407;

  std::vector<int> _unconditioned_tokens{
      49406, 49407, 49407, 49407, 49407, 49407, 49407, 49407, 49407, 49407,
      49407, 49407, 49407, 49407, 49407, 49407, 49407, 49407, 49407, 49407,
      49407, 49407, 49407, 49407, 49407, 49407, 49407, 49407, 49407, 49407,
      49407, 49407, 49407, 49407, 49407, 49407, 49407, 49407, 49407, 49407,
      49407, 49407, 49407, 49407, 49407, 49407, 49407, 49407, 49407, 49407,
      49407, 49407, 49407, 49407, 49407, 49407, 49407, 49407, 49407, 49407,
      49407, 49407, 49407, 49407, 49407, 49407, 49407, 49407, 49407, 49407,
      49407, 49407, 49407, 49407, 49407, 49407, 49407, 49407, 49407, 49407,
      49407, 49407, 49407, 49407, 49407, 49407, 49407};

  vector<string> get_tokens(string line);
  void dump_string_vector(vector<string> to_dump);
  set<tuple<string, string>> get_pairs(vector<string> token);
  string concat_vector_to_string(vector<string> to_concat);
  string bpe_encode(string token);
  void init();

 public:
  bpe() { init(); }

  vector<int> encode(string line);
  vector<int> position_ids();
  vector<int> unconditioned_tokens() { return _unconditioned_tokens; }
};

#endif
