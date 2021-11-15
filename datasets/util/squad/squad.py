"""
Copyright (c) 2019 Intel Corporation

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
"""

from collections import namedtuple
import sys
sys.path.insert(1, 'py-bindings')
import numpy as np
import json

from _nlp_common import get_tokenizer, CLS_ID, SEP_ID

class SQUADConverter():
    # __provider__ = "squad"
    # annotation_types = (QuestionAnsweringAnnotation, )

    def __init__(self, testing_file, vocab_file, max_seq_length, max_query_length, doc_stride, lower_case ):
        self.testing_file = testing_file
        self.max_seq_length = max_seq_length
        self.max_query_length = max_query_length
        self.doc_stride = doc_stride
        self.lower_case = lower_case
        self.tokenizer = get_tokenizer(self.lower_case, vocab_file) #TODO: Check option for sentence_piece
        self.support_vocab = True #TODO: Check for vocab support

    @staticmethod
    def _load_examples(file):
        def _is_whitespace(c):
            if c == " " or c == "\t" or c == "\r" or c == "\n" or ord(c) == 0x202F:
                return True
            return False

        def read_json(test_file_path):
            with open(test_file_path, 'r') as content:
                return json.load(content)

        examples = []
        answers = []
        data = read_json(file)['data']

        for entry in data:
            for paragraph in entry['paragraphs']:
                paragraph_text = paragraph["context"]
                doc_tokens = []
                char_to_word_offset = []
                prev_is_whitespace = True
                for c in paragraph_text:
                    if _is_whitespace(c):
                        prev_is_whitespace = True
                    else:
                        if prev_is_whitespace:
                            doc_tokens.append(c)
                        else:
                            doc_tokens[-1] += c
                        prev_is_whitespace = False
                    char_to_word_offset.append(len(doc_tokens) - 1)

                for qa in paragraph["qas"]:
                    qas_id = qa["id"]
                    question_text = qa["question"]
                    orig_answer_text = qa["answers"]
                    is_impossible = False

                    example = {
                        'id': qas_id,
                        'question_text': question_text,
                        'tokens': doc_tokens,
                        'is_impossible': is_impossible
                    }
                    examples.append(example)
                    answers.append(orig_answer_text)
        return examples, answers

    def convert(self): #check_content=False, progress_callback=None, progress_interval=100, **kwargs):
        examples, answers = self._load_examples(self.testing_file)
        
        all_samples = []

        unique_id = 1000000000
        DocSpan = namedtuple("DocSpan", ["start", "length"])

        for (example_index, example) in enumerate(examples):
            query_tokens = self.tokenizer.tokenize(example['question_text'])
            if len(query_tokens) > self.max_query_length:
                query_tokens = query_tokens[:self.max_query_length]
            all_doc_tokens = []
            for (i, token) in enumerate(example['tokens']):
                sub_tokens = self.tokenizer.tokenize(token)
                for sub_token in sub_tokens:
                    all_doc_tokens.append(sub_token)
            max_tokens_for_doc = self.max_seq_length - len(query_tokens) - 3
            doc_spans = []
            start_offset = 0
            while start_offset < len(all_doc_tokens):
                length = len(all_doc_tokens) - start_offset
                if length > max_tokens_for_doc:
                    length = max_tokens_for_doc
                doc_spans.append(DocSpan(start_offset, length))
                if start_offset + length == len(all_doc_tokens):
                    break
                start_offset += min(length, self.doc_stride)
            for idx, doc_span in enumerate(doc_spans):
                sample_dict = {}
                tokens = []
                segment_ids = []
                tokens.append("[CLS]" if self.support_vocab else CLS_ID)
                segment_ids.append(0)
                for token in query_tokens:
                    tokens.append(token)
                    segment_ids.append(0)
                tokens.append("[SEP]" if self.support_vocab else SEP_ID)
                segment_ids.append(0)
                for i in range(doc_span.length):
                    split_token_index = doc_span.start + i
                    tokens.append(all_doc_tokens[split_token_index])
                    segment_ids.append(1)
                tokens.append("[SEP]" if self.support_vocab else SEP_ID)
                segment_ids.append(1)
                input_ids = self.tokenizer.convert_tokens_to_ids(tokens) if self.support_vocab else tokens
                input_mask = [1] * len(input_ids)
                while len(input_ids) < self.max_seq_length:
                    input_ids.append(0)
                    input_mask.append(0)
                    segment_ids.append(0)

                sample_dict['input_ids'] = input_ids
                sample_dict['input_mask'] = input_mask
                sample_dict['segment_ids'] = segment_ids
                sample_dict['tokens'] = tokens
                all_samples.append(sample_dict)
        return all_samples

    @staticmethod
    def _is_max_context(doc_spans, cur_span_index, position):
        best_score = None
        best_span_index = None
        for (span_index, doc_span) in enumerate(doc_spans):
            end = doc_span.start + doc_span.length - 1
            if position < doc_span.start:
                continue
            if position > end:
                continue
            num_left_context = position - doc_span.start
            num_right_context = end - position
            score = min(num_left_context, num_right_context) + 0.01 * doc_span.length
            if best_score is None or score > best_score:
                best_score = score
                best_span_index = span_index

        return cur_span_index == best_span_index
