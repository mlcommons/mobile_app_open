#include "flutter/cpp/datasets/ifeval.h"

#include <fstream>
#include <iostream>

#include "flutter/cpp/datasets/mmlu_utils/sentencepiece_utils.h"
#include "tensorflow/core/example/example.pb.h"
#include "tensorflow/core/example/feature_util.h"

namespace mlperf {
namespace mobile {

IFEval::IFEval(Backend* backend, const std::string& input_tfrecord,
               const std::string& sp_path, bool loose_follow)
    : sample_reader_(input_tfrecord),
      loose_follow_(loose_follow),
      Dataset(backend) {
  sp_processor = std::unique_ptr<sentencepiece::SentencePieceProcessor>(
      LoadSentencePieceProcessor(sp_path));
  start_token_id = sp_processor->PieceToId(start_token);
  end_token_id = sp_processor->PieceToId(end_token);

  // Load all TFRecord samples into memory
  // NOTE this can be moved to LoadSamplesToRam, but will cause delays between
  // queries due to IO reads
  for (size_t i = 0; i < sample_reader_.Size(); i++) {
    tensorflow::tstring record = sample_reader_.ReadRecord(i);
    tensorflow::Example example;
    example.ParseFromString(record);
    int key = tensorflow::GetFeatureValues<int64_t>("key", example).Get(0);
    std::string prompt =
        tensorflow::GetFeatureValues<std::string>("prompt", example).Get(0);
    auto instructions = BuildInstructions(example);

    std::vector<int> input_tokens;
    sp_processor->Encode(prompt.c_str(), &input_tokens).ok();

    input_tokens.insert(input_tokens.begin(), start_token_id);

    auto sample = std::make_unique<ifeval::Sample>();
    sample->key = key;
    sample->prompt = prompt;
    sample->input_tokens = input_tokens;
    sample->instructions = std::move(instructions);

    samples_.push_back(std::move(sample));
    sample_output_tokens_.push_back(std::vector<int>());
  }
}

void IFEval::LoadSamplesToRam(const std::vector<QuerySampleIndex>& samples) {
  for (auto id : samples) {
    loaded_sample_ids_.insert(id);
  }
}

void IFEval::UnloadSamplesFromRam(
    const std::vector<QuerySampleIndex>& samples) {
  for (auto id : samples) {
    loaded_sample_ids_.erase(id);
  }
}

std::vector<void*> IFEval::GetData(int sample_idx) {
  std::vector<void*> data;

  if (sample_idx < samples_.size()) {
    data.push_back(reinterpret_cast<void*>(
        const_cast<std::vector<int>*>(&(samples_[sample_idx]->input_tokens))));
    data.push_back(reinterpret_cast<void*>(const_cast<int*>(&end_token_id)));
  }
  return data;
}

std::vector<uint8_t> IFEval::ProcessOutput(const int sample_idx,
                                           const std::vector<void*>& outputs) {
  if (sample_idx >= samples_.size() || outputs.empty()) return {0};

  const auto& output_tokens =
      *(reinterpret_cast<std::vector<int>*>(outputs[0]));

  sample_output_tokens_[sample_idx] = output_tokens;

  return {1};
}

int64_t IFEval::GetOutputTokenCount(const int sample_idx) {
  return sample_output_tokens_[sample_idx].size();
}

bool IFEval::HasAccuracy() { return true; }

bool IFEval::ComputeSampleAccuracy(const int sample_idx,
                                   ifeval::GroupAccuracy& accuracy) {
  std::string prediction;
  sp_processor->Decode(sample_output_tokens_[sample_idx], &prediction).ok();

  LOG(INFO) << "output(" << std::to_string(sample_idx) << "): " << prediction
            << std::endl;

  for (const auto& instruction : samples_[sample_idx]->instructions) {
    bool is_correct = instruction->IsFollowed(prediction, loose_follow_);
    ProcessResult(instruction->Group(), is_correct, accuracy);
  }
}

float IFEval::ComputeAccuracy() {
  uint16_t correct_sum;
  uint16_t total_sum;
  ifeval::GroupAccuracy accuracy;

  for (auto sample_id : used_sample_ids_) {
    ComputeSampleAccuracy(sample_id, accuracy);
  }

  correct_sum += accuracy.change_case_correct;
  correct_sum += accuracy.combination_correct;
  correct_sum += accuracy.detectable_content_correct;
  correct_sum += accuracy.detectable_format_correct;
  correct_sum += accuracy.keywords_correct;
  correct_sum += accuracy.language_correct;
  correct_sum += accuracy.length_constraints_correct;
  correct_sum += accuracy.punctuation_correct;
  correct_sum += accuracy.startend_correct;

  total_sum += accuracy.change_case_total;
  total_sum += accuracy.combination_total;
  total_sum += accuracy.detectable_content_total;
  total_sum += accuracy.detectable_format_total;
  total_sum += accuracy.keywords_total;
  total_sum += accuracy.language_total;
  total_sum += accuracy.length_constraints_total;
  total_sum += accuracy.punctuation_total;
  total_sum += accuracy.startend_total;

  return total_sum > 0 ? static_cast<float>(correct_sum) / total_sum : 0.0f;
}

std::string IFEval::ComputeAccuracyString() {
  float acc = ComputeAccuracy();
  return "Accuracy: " + std::to_string(acc * 100.0f) + "%";
}

inline std::vector<std::unique_ptr<ifeval::Instruction>>
IFEval::BuildInstructions(const tensorflow::Example& ex) {
  std::vector<std::unique_ptr<ifeval::Instruction>> out;

  // ---- helpers (local) ----
  auto parse_relation = [](const std::string& s) -> ifeval::Relation {
    return (s == "at least") ? ifeval::Relation::AT_LEAST
                             : ifeval::Relation::LESS_THAN;
  };

  auto add = [&](auto ptr) { out.emplace_back(std::move(ptr)); };

  auto get_strs = [&](const std::string& key,
                      std::vector<std::string>* vals) -> bool {
    const auto& sfield = tensorflow::GetFeatureValues<std::string>(key, ex);
    std::vector<std::string> svals(sfield.begin(), sfield.end());
    *vals = std::move(svals);
    return true;
  };
  auto get_ints = [&](const std::string& key,
                      std::vector<int64_t>* vals) -> bool {
    const auto& ifield = tensorflow::GetFeatureValues<int64_t>(key, ex);
    std::vector<int64_t> ivals(ifield.begin(), ifield.end());
    *vals = std::move(ivals);
    return true;
  };
  auto get_str = [&](const std::string& key, std::string* val) -> bool {
    std::vector<std::string> tmp;
    if (!get_strs(key, &tmp) || tmp.empty()) return false;
    *val = std::move(tmp[0]);
    return true;
  };
  auto get_int = [&](const std::string& key, int* val) -> bool {
    std::vector<int64_t> tmp;
    if (!get_ints(key, &tmp) || tmp.empty()) return false;
    *val = static_cast<int>(tmp[0]);
    return true;
  };

  // Read instruction_id_list (bytes_list of strings) without touching
  // ex.features().feature()
  const auto& id_field =
      tensorflow::GetFeatureValues<std::string>("instruction_id_list", ex);
  std::vector<std::string> ids(id_field.begin(), id_field.end());
  if (ids.empty()) return out;

  // Enum for switch (one case per instruction kind)
  enum class Kind {
    kCapitalWordFrequency,
    kEnglishCapital,
    kEnglishLowercase,
    kRepeatPrompt,
    kTwoResponses,
    kNumberPlaceholders,
    kPostscript,
    kConstrainedResponse,
    kJsonFormat,
    kMultipleSections,
    kNumberBulletLists,
    kNumberHighlightedSections,
    kTitle,
    kExistence,
    kForbiddenWords,
    kFrequency,
    kLetterFrequency,
    kResponseLanguage,
    kNthParagraphFirstWord,
    kNumberParagraphs,
    kNumberSentences,
    kNumberWords,
    kNoComma,
    kEndChecker,
    kQuotation,
    kUnknown
  };

  auto to_kind = [](const std::string& id) -> Kind {
    auto colon = id.find(':');
    std::string name = (colon == std::string::npos) ? id : id.substr(colon + 1);
    if (name == "capital_word_frequency") return Kind::kCapitalWordFrequency;
    if (name == "english_capital") return Kind::kEnglishCapital;
    if (name == "english_lowercase") return Kind::kEnglishLowercase;
    if (name == "repeat_prompt") return Kind::kRepeatPrompt;
    if (name == "two_responses") return Kind::kTwoResponses;
    if (name == "number_placeholders") return Kind::kNumberPlaceholders;
    if (name == "postscript") return Kind::kPostscript;
    if (name == "constrained_response") return Kind::kConstrainedResponse;
    if (name == "json_format") return Kind::kJsonFormat;
    if (name == "multiple_sections") return Kind::kMultipleSections;
    if (name == "number_bullet_lists") return Kind::kNumberBulletLists;
    if (name == "number_highlighted_sections")
      return Kind::kNumberHighlightedSections;
    if (name == "title") return Kind::kTitle;
    if (name == "existence") return Kind::kExistence;
    if (name == "forbidden_words") return Kind::kForbiddenWords;
    if (name == "frequency") return Kind::kFrequency;
    if (name == "letter_frequency") return Kind::kLetterFrequency;
    if (name == "response_language") return Kind::kResponseLanguage;
    if (name == "nth_paragraph_first_word") return Kind::kNthParagraphFirstWord;
    if (name == "number_paragraphs") return Kind::kNumberParagraphs;
    if (name == "number_sentences") return Kind::kNumberSentences;
    if (name == "number_words") return Kind::kNumberWords;
    if (name == "no_comma") return Kind::kNoComma;
    if (name == "end_checker") return Kind::kEndChecker;
    if (name == "quotation") return Kind::kQuotation;
    return Kind::kUnknown;
  };

  // Build each instruction from kwargs/<i>/* using
  // tensorflow::GetFeatureValues(ex, key, &vec)
  for (int i = 0; i < static_cast<int>(ids.size()); ++i) {
    const std::string& id = ids[i];
    const Kind kind = to_kind(id);

    auto K = [&](const std::string& key) {
      return "kwargs/" + std::to_string(i) + "/" + key;
    };

    switch (kind) {
      case Kind::kCapitalWordFrequency: {
        int pct = 0;
        std::string rel;
        get_int(K("capital_frequency"), &pct);
        get_str(K("capital_relation"), &rel);
        add(std::make_unique<ifeval::CapitalWordFrequency>(
            pct, parse_relation(rel)));
        break;
      }
      case Kind::kEnglishCapital: {
        add(std::make_unique<ifeval::EnglishCapital>());
        break;
      }
      case Kind::kEnglishLowercase: {
        add(std::make_unique<ifeval::EnglishLowercase>());
        break;
      }
      case Kind::kRepeatPrompt: {
        std::string p;
        get_str(K("prompt_to_repeat"), &p);
        add(std::make_unique<ifeval::RepeatPrompt>(p));
        break;
      }
      case Kind::kTwoResponses: {
        add(std::make_unique<ifeval::TwoResponses>());
        break;
      }
      case Kind::kNumberPlaceholders: {
        int n = 0;
        get_int(K("num_placeholders"), &n);
        add(std::make_unique<ifeval::NumberPlaceholders>(n));
        break;
      }
      case Kind::kPostscript: {
        std::string m;
        get_str(K("postscript_marker"), &m);
        add(std::make_unique<ifeval::Postscript>(m));
        break;
      }
      case Kind::kConstrainedResponse: {
        add(std::make_unique<ifeval::ConstrainedResponse>());
        break;
      }
      case Kind::kJsonFormat: {
        add(std::make_unique<ifeval::JsonFormat>());
        break;
      }
      case Kind::kMultipleSections: {
        int n = 0;
        std::string sep;
        get_int(K("num_sections"), &n);
        get_str(K("section_spliter"), &sep);
        add(std::make_unique<ifeval::MultipleSections>(n, sep));
        break;
      }
      case Kind::kNumberBulletLists: {
        int n = 0;
        get_int(K("num_bullets"), &n);
        add(std::make_unique<ifeval::NumberBulletLists>(n));
        break;
      }
      case Kind::kNumberHighlightedSections: {
        int n = 0;
        get_int(K("num_highlights"), &n);
        add(std::make_unique<ifeval::NumberHighlightedSections>(n));
        break;
      }
      case Kind::kTitle: {
        add(std::make_unique<ifeval::Title>());
        break;
      }
      case Kind::kExistence: {
        std::vector<std::string> kws;
        get_strs(K("keywords"), &kws);
        add(std::make_unique<ifeval::Existence>(kws));
        break;
      }
      case Kind::kForbiddenWords: {
        std::vector<std::string> bad;
        get_strs(K("forbidden_words"), &bad);
        add(std::make_unique<ifeval::ForbiddenWords>(bad));
        break;
      }
      case Kind::kFrequency: {
        int n = 0;
        std::string kw, rel;
        get_int(K("frequency"), &n);
        get_str(K("keyword"), &kw);
        get_str(K("relation"), &rel);
        add(std::make_unique<ifeval::Frequency>(n, kw, parse_relation(rel)));
        break;
      }
      case Kind::kLetterFrequency: {
        int n = 0;
        std::string letter, rel;
        get_int(K("let_frequency"), &n);
        get_str(K("letter"), &letter);
        get_str(K("let_relation"), &rel);
        char ch = letter.empty() ? 'a' : letter[0];
        add(std::make_unique<ifeval::LetterFrequency>(n, ch,
                                                      parse_relation(rel)));
        break;
      }
      case Kind::kResponseLanguage: {
        std::string lang;
        get_str(K("language"), &lang);
        add(std::make_unique<ifeval::ResponseLanguage>(lang));
        break;
      }
      case Kind::kNthParagraphFirstWord: {
        int nth = 0, total = 0;
        std::string fw;
        get_int(K("nth_paragraph"), &nth);
        get_int(K("num_paragraphs"), &total);
        get_str(K("first_word"), &fw);
        add(std::make_unique<ifeval::NthParagraphFirstWord>(nth, fw, total));
        break;
      }
      case Kind::kNumberParagraphs: {
        int n = 0;
        get_int(K("num_paragraphs"), &n);
        add(std::make_unique<ifeval::NumberParagraphs>(n));
        break;
      }
      case Kind::kNumberSentences: {
        int n = 0;
        std::string rel;
        get_int(K("num_sentences"), &n);
        get_str(K("relation"), &rel);
        add(std::make_unique<ifeval::NumberSentences>(n, parse_relation(rel)));
        break;
      }
      case Kind::kNumberWords: {
        int n = 0;
        std::string rel;
        get_int(K("num_words"), &n);
        get_str(K("relation"), &rel);
        add(std::make_unique<ifeval::NumberWords>(n, parse_relation(rel)));
        break;
      }
      case Kind::kNoComma: {
        add(std::make_unique<ifeval::NoComma>());
        break;
      }
      case Kind::kEndChecker: {
        std::string end;
        get_str(K("end_phrase"), &end);
        add(std::make_unique<ifeval::EndChecker>(end));
        break;
      }
      case Kind::kQuotation: {
        add(std::make_unique<ifeval::Quotation>());
        break;
      }
      case Kind::kUnknown:
      default: {
        // Unknown instruction id: skip (or handle as needed)
        break;
      }
    }
  }

  return out;
}

inline void IFEval::ProcessResult(ifeval::InstructionGroup group,
                                  bool is_correct,
                                  ifeval::GroupAccuracy& accuracy) {
  uint8_t correct_value = is_correct ? 1 : 0;
  switch (group) {
    case ifeval::InstructionGroup::CHANGE_CASE:
      accuracy.change_case_correct += correct_value;
      accuracy.change_case_total++;
      break;

    case ifeval::InstructionGroup::COMBINATION:
      accuracy.combination_correct += correct_value;
      accuracy.combination_total++;
      break;

    case ifeval::InstructionGroup::DETECTABLE_CONTENT:
      accuracy.detectable_content_correct += correct_value;
      accuracy.detectable_content_total++;
      break;

    case ifeval::InstructionGroup::DETECTABLE_FORMAT:
      accuracy.detectable_format_correct += correct_value;
      accuracy.detectable_format_total++;
      break;

    case ifeval::InstructionGroup::KEYWORDS:
      accuracy.keywords_correct += correct_value;
      accuracy.keywords_total++;
      break;

    case ifeval::InstructionGroup::LANGUAGE:
      accuracy.language_correct += correct_value;
      accuracy.language_total++;
      break;

    case ifeval::InstructionGroup::LENGTH_CONSTRAINTS:
      accuracy.length_constraints_correct += correct_value;
      accuracy.length_constraints_total++;
      break;

    case ifeval::InstructionGroup::PUNCTUATION:
      accuracy.punctuation_correct += correct_value;
      accuracy.punctuation_total++;
      break;

    case ifeval::InstructionGroup::STARTEND:
      accuracy.startend_correct += correct_value;
      accuracy.startend_total++;
      break;

    default:
      break;
  }
}

}  // namespace mobile
}  // namespace mlperf
