import tensorflow as tf
import pandas as pd
import argparse

def parse_args():
    parser = argparse.ArgumentParser(description="Convert a Parquet of LLM prompts to TFRecord format.")
    parser.add_argument('--input_file', type=str, required=True, help="Path to the input Parquet (.parquet) file.")
    parser.add_argument('--output_file', type=str, required=True, help="Path to the output TFRecord file.")
    return parser.parse_args()

def map_answer(num):
    return {0: "A", 1: "B", 2: "C", 3: "D"}.get(num, "X")  # Use 'X' as fallback

def create_example(input_text, answer_letter):
    return tf.train.Example(features=tf.train.Features(feature={
        "input": tf.train.Feature(bytes_list=tf.train.BytesList(value=[str(input_text).encode()])),
        "answer": tf.train.Feature(bytes_list=tf.train.BytesList(value=[answer_letter.encode()])),
    }))

def main():
    args = parse_args()

    # Read Parquet (requires 'pyarrow' or 'fastparquet')
    try:
        df = pd.read_parquet(args.input_file)
    except ImportError as e:
        raise ImportError(
            "Reading Parquet requires 'pyarrow' or 'fastparquet'. "
            "Install one, e.g. `pip install pyarrow`."
        ) from e

    if "input_formatted" not in df.columns or "answer" not in df.columns:
        raise ValueError("Parquet must contain 'input_formatted' and 'answer' columns.")

    # Robustly map numeric answers to letters
    def to_letter(x):
        try:
            return map_answer(int(x))
        except (ValueError, TypeError):
            return "X"

    df["answer_letter"] = df["answer"].apply(to_letter)

    options = tf.io.TFRecordOptions(compression_type="ZLIB")
    with tf.io.TFRecordWriter(args.output_file, options=options) as writer:
        for _, row in df.iterrows():
            example = create_example(row["input_formatted"], row["answer_letter"])
            writer.write(example.SerializeToString())

    print(f"TFRecord written to: {args.output_file}")

if __name__ == "__main__":
    main()
