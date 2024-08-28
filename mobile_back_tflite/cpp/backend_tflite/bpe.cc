#include "bpe.h"

using namespace std;

vector<string> bpe::get_tokens(string line) {
  // simple alphanum+whitespace and punctions pattern
  std::string pattern("(\\w+)</w>|(\\w+)|([[:punct:]])</w>|([[:punct:]])");
  std::regex re(pattern, std::regex_constants::icase);
  std::sregex_token_iterator i(line.begin(), line.end(), re);
  std::sregex_token_iterator end_of_string;

  vector<string> tokens;
  while (i != end_of_string) {
    tokens.push_back(*i);
    i++;
  }
  return tokens;
}

void bpe::dump_string_vector(vector<string> to_dump) {
  for (size_t i = 0; i < to_dump.size(); i++) {
    if (i != (to_dump.size() - 1))
      cout << to_dump[i] << ", ";
    else
      cout << to_dump[i] << "\n";
  }
}

set<tuple<string, string>> bpe::get_pairs(vector<string> token) {
  set<tuple<string, string>> pairs;

  for (size_t i = 0; i < token.size() - 1; i++) {
    pairs.insert(pairs.end(), make_tuple(token[i], token[i + 1]));
  }
  return pairs;
}

string bpe::concat_vector_to_string(vector<string> to_concat) {
  string word_to_return;

  for (auto i = to_concat.begin(); i < to_concat.end(); i++) {
    word_to_return.append(*i);
  }
  return word_to_return;
}

string bpe::bpe_encode(string token) {
  vector<string> word;
  for (auto i : token) {
    word.push_back(string(1, i));
  }
  word[token.size() - 1] = word[token.size() - 1] + "</w>";
  set<tuple<string, string>> pairs = get_pairs(word);
  if (pairs.size() == 0) {
    return token + "</w>";
  }

  while (true) {
    map<tuple<string, string>, int> can_merges;

    for (tuple<string, string> pair : pairs) {
      for (size_t i = 0; i < merges.size(); i++) {
        if (merges[i] == pair) {
          can_merges[pair] = i;
        }
      }
    }
    if (can_merges.size() == 0) break;

    auto min = min_element(can_merges.begin(), can_merges.end(),
                           [](const auto& lhs, const auto& rhs) {
                             return lhs.second < rhs.second;
                           });
    auto first = get<0>(min->first);
    auto second = get<1>(min->first);

    vector<string> new_word;
    size_t i = 0;
    while (i < word.size()) {
      vector<string> remaining(word.begin() + i, word.end());

      auto found_i = find(word.begin() + i, word.end(), first);
      auto found = -1;
      if (found_i != word.end()) found = distance(word.begin(), found_i);

      if (found != -1) {
        vector<string> prefix(word.begin() + i, word.begin() + found);
        new_word.insert(new_word.end(), prefix.begin(), prefix.end());

        if ((i < (word.size() - 1)) && word[found + 1] == second) {
          new_word.push_back(word[found] + word[found + 1]);
          i = found + 2;
        } else {
          new_word.push_back(word[found]);
          i = found + 1;
        }
      } else {
        new_word.insert(new_word.end(), remaining.begin(), remaining.end());
        break;
      }
    }

    word = new_word;
    pairs = get_pairs(new_word);
  }

  string word_to_return;
  if (word.size() == 1) {
    word_to_return = word[0];
  } else {
    for (auto i = word.begin(); i < word.end() - 1; i++) {
      word_to_return = word_to_return + (*i + " ");
    }
    word_to_return = word_to_return + word[word.size() - 1];
  }

  // cout << "word_to_return: " << word_to_return << "\n";
  return word_to_return;
}

void bpe::init() {
  std::ifstream merges_stream(merges_file, std::ifstream::binary);
  std::ifstream vocab_stream(vocab_file, std::ifstream::binary);

  string k, v;
  auto index = 0;

  while (vocab_stream >> v >> index) {
    vocab[v] = index;
  }

  while (merges_stream >> k >> v) {
    merges.push_back(make_tuple(k, v));
  }
}

// bpe::bpe() { init(); }

vector<int> bpe::encode(string line) {
  // clean up whitespace, replacing all consective ws with single " "
  regex ws("\\s+");
  line = regex_replace(line, ws, " ");

  // to lower
  transform(line.begin(), line.end(), line.begin(), ::tolower);
  auto tokens = get_tokens(line);
  if (tokens.size() > 77) {
    tokens = {tokens.begin(), tokens.begin() + 76};
  }
  vector<int> codes;
  codes.push_back(START_OF_TEXT);
  for (auto t : tokens) {
    auto returned = get_tokens(bpe_encode(t));
    for (size_t i = 0; i < returned.size(); i++) {
      codes.push_back(vocab[returned[i]]);
    }
  }
  codes.push_back(END_OF_TEXT);
  if (codes.size() < 77) {
    auto padding_size = 77 - codes.size();
    std::vector<int> padding(padding_size, END_OF_TEXT);
    codes.insert(codes.end(), padding.begin(), padding.end());
  }
  return codes;
}

vector<int> bpe::position_ids() {
  vector<int> position_ids;
  for (int i = 0; i < 77; i++) {
    position_ids.push_back(i);
  }
  return position_ids;
}

#ifdef __TEST_BPE__
int main(int argc, char* argv[]) {
  bpe bpe_encoder;

  string prompt = "a photo of an astronaut riding a horse on Mars";
  if (argc == 2) prompt = argv[1];
  auto encoded = bpe_encoder.encode(prompt);

  cout << "[";
  for (auto i = encoded.begin(); i < encoded.end(); i++) {
    if (i != (encoded.end() - 1))
      cout << *i << ", ";
    else
      cout << *i;
  }
  cout << "]"
       << "\n";

  auto pos_ids = bpe_encoder.position_ids();
  cout << "[";
  for (auto i = pos_ids.begin(); i < pos_ids.end(); i++) {
    if (i != (pos_ids.end() - 1))
      cout << *i << ", ";
    else
      cout << *i;
  }
  cout << "]"
       << "\n";
}
#endif
