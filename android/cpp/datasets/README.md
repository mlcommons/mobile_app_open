# MLPerf datasets

This directory provides implementations of datasets used to evaluate the MLPerf
v1.0 benchmark.These implementations are initially developed to be used with
TFLite. Other backends may need to extend them.

## Imagenet

The full ImageNet dataset is available to MLCommons members individually. 
You can request access from the mobile chairs.
For testing purpose, you may download a substitute dataset from 
[https://github.com/mlcommons/mobile_models/tree/main/v0_7/datasets](https://github.com/mlcommons/mobile_models/tree/main/v0_7/datasets)

## COCO

Download the COCO 2017 dataset from
[http://cocodataset.org/#download](http://cocodataset.org/#download) and the script
[upscale_coco.py](https://github.com/mlcommons/mobile_app_open/blob/150470388e728894ccb0268b1cef422b410a83e7/mobile_back_qti/datasets/coco/upscale_coco.py).
Then use the script to process the images:

```bash
python upscale_coco.py --inputs /path-to-coco/ --outputs /output-path/ --size 300 300
```

The ground truth file is
[coco_val_full.pbtxt](https://github.com/mlcommons/mobile_models/blob/b2519b57de06c347a4600851b47e45ce45d5919f/v1_0/assets/coco_val_full.pbtxt).
If you want to use a subset of images, remember to use the first N images which appear
in the file instances_val2017.json. **Note** that the order of images in this
file and the order of images under the images directory are not the same.

## SQUAD

Download the
[Squad v1.1 evaluation set](https://rajpurkar.github.io/SQuAD-explorer/dataset/dev-v1.1.json)
and the
[vocab file](https://storage.googleapis.com/cloud-tpu-checkpoints/mobilebert/uncased_L-24_H-128_B-512_A-4_F-4_OPT.tar.gz).
Then, you can generate the groundtruth and input tfrecord files for running
inference as below:

1.  Install dependencies:

```
pip install tensorflow tensorflow_hub
```

2.  Download other dependencies from google-research and add its path to
    PYTHONPATH:

```
TMP_DIR=<your tmp directory>
pushd $TMP_DIR
curl -o google-research.tar.gz -L https://github.com/google-research/google-research/archive/256f678d1aeb7a4527031c8dd2f4a2c9f3833f93.tar.gz
tar -xzf google-research.tar.gz --transform s/google-research-256f678d1aeb7a4527031c8dd2f4a2c9f3833f93/google-research/
export PYTHONPATH="${PYTHONPATH}:`pwd`/google-research"
popd
```

3.  Generate tfrecord files for inference:

```
python cpp/datasets/squad_utils/generate_tfrecords.py \
  --vocab_file=<path to vocab.txt> \
  --predict_file=<path to dev-v1.1.json> \
  --output_dir=<output dir> \
  --max_seq_length=384 \
  --doc_stride=128 \
  --max_query_length=64
```

There are default tfrecord files at 
[https://github.com/mlcommons/mobile_models/tree/main/v1_0/datasets](https://github.com/mlcommons/mobile_models/tree/main/v1_0/datasets)
generated with above default parameters. By default, the app will use a
[mini version](https://github.com/mlcommons/mobile_models/raw/main/v1_0/datasets/squad_eval_mini.tfrecord)
of the dataset with 160 random questions. To evaluate using the full dataset,
you need to replace `squad_eval_mini.tfrecord` by `squad_eval.tfrecord` in the
[tasks_v2.pbtxt](https://raw.githubusercontent.com/mlcommons/mobile_models/main/v1_0/assets/tasks_v2.pbtxt) file or in the
`/storage/emulated/0/mlperf_datasets/tasks_v2.pbtxt` file if you use 'user-defined' tasks file.

## ADE20K
1. follow DeepLab's instruction to download the ADE20K dataset, https://github.com/tensorflow/models/blob/master/research/deeplab/g3doc/ade20k.md
2. prepare 512x512 images and and ground truth file with something like the following
```python
import os
import tensorflow as tf
import deeplab.input_preprocess
from PIL import Image as Image

tf.enable_eager_execution()

home = os.getenv("HOME")
ADE20K_PATH = home + '/tf-models/research/deeplab/datasets/ADE20K/ADEChallengeData2016/'

for i in range(1, 2001):
    image_jpeg = ADE20K_PATH+f'images/validation/ADE_val_0000{i:04}.jpg'
    label_png = ADE20K_PATH+f'annotations/validation/ADE_val_0000{i:04}.png'
    # print(image_jpeg)
    image_jpeg_data = tf.io.read_file(image_jpeg)
    image_tensor = tf.io.decode_jpeg(image_jpeg_data)
    label_png_data = tf.io.read_file(label_png)
    label_tensor = tf.io.decode_jpeg(label_png_data)
    o_image, p_image, p_label = deeplab.input_preprocess.preprocess_image_and_label(image_tensor, label_tensor, 512, 512, 512, 512, is_training=False)

    target_image_jpeg = f'/tmp/ade20k_512/images/validation/ADE_val_0000{i:04}.jpg'
    target_label_raw = f'/tmp/ade20k_512/annotations/raw/ADE_val_0000{i:04}.raw'

    resized_image = Image.fromarray(tf.reshape(tf.cast(p_image, tf.uint8), [512, 512, 3]).numpy())
    resized_image.save(target_image_jpeg)
    tf.reshape(tf.cast(p_label, tf.uint8), [512, 512]).numpy().tofile(target_label_raw)
```

3. Build command line tool to test performance and accuracy on x86 host

```
build  --cxxopt='--std=c++14' --host_cxxopt='--std=c++14' --copt=-march=native //cpp/binary:main
```
4. test with the command line tool
```
./bazel-bin/cpp/binary/main tflite ade20k \
  --mode=PerformanceOnly \
  --output_dir=/tmp/test_output \
  --model_file=/tmp/freeze_quant_ops16_32c_clean.tflite \
  --images_directory=/tmp/ade20k_512/images/validation  \
  --ground_truth_directory=/tmp/ade20k_512/annotations/raw  \
  --num_threads=4
```
