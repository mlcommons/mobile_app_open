/* Copyright 2018 The MLPerf Authors. All Rights Reserved.
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at
    http://www.apache.org/licenses/LICENSE-2.0
Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/

#ifndef EDENMODEL_INCLUDE_EDEN_MODEL_TYPES_H_
#define EDENMODEL_INCLUDE_EDEN_MODEL_TYPES_H_

#include <cstdint>  // int8_t, uint8_t, int16_t, uint16_t, int32_t, uint32_t
#include <memory>   // unique_ptr
#include <vector>   // vector

#include "eden_types.h"  // EdenBuffer

#ifdef __cplusplus
extern "C" {
#endif

// #ifdef DSP_USERDRIVER
// Allocate Bridge Buffers for custom operations
// NPU
static const char* const NPU_NCP = "NCP";
static const char* const NCP_BINARY = "NCP_BINARY";
static const char* const NCP_NAME = "NCP_NAME";

// DSP Operation name -> TBD
// Name of DSP custom operator can be changed.
static const char* const DSP_OP = "DSP";
static const char* const DSP_BINARY = "DSP_BINARY";
static const char* const DSP_NAME = "DSP_NAME";
static const char* const DSP_DETECTION = "DSP_DETECTION";
static const char* const PRE_DSP_IV3 = "DSP_PRE_BGR_TO_BGRX";
static const char* const POST_DSP_IV3 = "DSP_POST_DEQUANT_SOFTMAX";
// #endif

/*! profile  */
typedef enum {
  EDEN_OP_CUSTOMS = 0,
  EDEN_OP_PROFILE1 = 1,
  EDEN_OP_PROFILE2 = 2,
  EDEN_OP_PROFILE3 = 3,
} EDEN_OP_PROFILE;

/*! operators */
typedef enum {
  EDEN_OP_ADD = 0,
  EDEN_OP_AVERAGE_POOL_2D = 1,
  EDEN_OP_CONCATENATION = 2,
  EDEN_OP_CONV_2D = 3,
  EDEN_OP_DEPTHWISE_CONV_2D = 4,

  EDEN_OP_DEPTH_TO_SPACE = 5,
  EDEN_OP_DEQUANTIZE = 6,
  EDEN_OP_EMBEDDING_LOOKUP = 7,
  EDEN_OP_FLOOR = 8,
  EDEN_OP_FULLY_CONNECTED = 9,

  EDEN_OP_HASHTABLE_LOOKUP = 10,
  EDEN_OP_L2_NORMALIZATION = 11,
  EDEN_OP_L2_POOL_2D = 12,
  EDEN_OP_LOCAL_RESPONSE_NORMALIZATION = 13,
  EDEN_OP_LOGISTIC = 14,

  EDEN_OP_LSH_PROJECTION = 15,
  EDEN_OP_LSTM = 16,
  EDEN_OP_MAX_POOL_2D = 17,
  EDEN_OP_MUL = 18,
  EDEN_OP_RELU = 19,

  EDEN_OP_RELU1 = 20,
  EDEN_OP_RELU6 = 21,
  EDEN_OP_RESHAPE = 22,
  EDEN_OP_RESIZE_BILINEAR = 23,
  EDEN_OP_RNN = 24,

  EDEN_OP_SOFTMAX = 25,
  EDEN_OP_SPACE_TO_DEPTH = 26,
  EDEN_OP_SVDF = 27,
  EDEN_OP_TANH = 28,
  EDEN_OP_BATCH_TO_SPACE_ND = 29,

  EDEN_OP_DIV = 30,
  EDEN_OP_MEAN = 31,
  EDEN_OP_CUSTOM = 32,
  EDEN_OP_SPACE_TO_BATCH_ND = 33,
  EDEN_OP_SQUEEZE = 34,

  EDEN_OP_STRIDED_SLICE = 35,
  EDEN_OP_SUB = 36,
  EDEN_OP_TRANSPOSE = 37,
  EDEN_OP_PRELU = 38,
  EDEN_OP_ELEMENTWISE_MAX = 39,

  EDEN_OP_ARGMAX = 40,
  EDEN_OP_SCALE = 41,
  EDEN_OP_CROP = 42,
  EDEN_OP_FLATTEN = 43,
  EDEN_OP_PERMUTE = 44,

  EDEN_OP_SLICE = 45,
  EDEN_OP_PRIORBOX = 46,
  EDEN_OP_POWER = 47,
  EDEN_OP_PAD = 48,
  EDEN_OP_DECONV_2D = 49,

  EDEN_OP_DETECTION = 50,
  EDEN_OP_ROIPOOL = 51,
  EDEN_OP_BIDIRECTIONAL_SEQUENCE_LSTM = 52,
  EDEN_OP_UNIDIRECTIONAL_SEQUENCE_LSTM = 53,
  EDEN_OP_BIDIRECTIONAl_RNN = 54,

  EDEN_OP_UNIDIRECTIONAL_SEQUENCE_RNN = 55,
  EDEN_OP_LAYER_NORM_LSTM = 56,
  EDEN_OP_TFDETECTION = 57,
  EDEN_OP_LOGICAL_NOT = 58,
  EDEN_OP_ROI_ALIGN = 59,

  EDEN_OP_GENERATE_PROPOSALS = 60,
  EDEN_OP_RESIZE_NEAREST_NEIGHBOR = 61,
  EDEN_OP_ABS = 62,
  EDEN_OP_GREATER = 63,
  EDEN_OP_GREATER_EQUAL = 64,

  EDEN_OP_EQUAL = 65,
  EDEN_OP_NOT_EQUAL = 66,
  EDEN_OP_MINIMUM = 67,
  EDEN_OP_NEG = 68,
  EDEN_OP_EXPAND_DIMS = 69,

  EDEN_OP_GATHER = 70,
  EDEN_OP_SELECT = 71,
  EDEN_OP_SPLIT = 72,
  EDEN_OP_POW = 73,
  EDEN_OP_LOG = 74,

  EDEN_OP_SIN = 75,
  EDEN_OP_RSQRT = 76,
  EDEN_OP_PAD_V2 = 77,
  EDEN_OP_TOPK_V2 = 78,
  EDEN_OP_TFLITEROIPOOL = 79,

  EDEN_OP_ARGMIN = 80,
  EDEN_OP_SQRT = 81,
  EDEN_OP_BOX_WITH_NMS_LIMIT = 82,
  EDEN_OP_EXP = 83,
  EDEN_OP_AXIS_ALIGNED_BBOX_TRANSFORM = 84,

  EDEN_OP_INSTANCE_NORMALIZATION = 85,
  EDEN_OP_QUANTIZED_16BIT_LSTM = 86,
  EDEN_OP_QUANTIZE = 87,
  EDEN_OP_DETECTION_POSTPROCESSING = 88,
  EDEN_OP_LOGICAL_AND = 89,

  EDEN_OP_LOGICAL_OR = 90,
  EDEN_OP_CAST = 91,
  EDEN_OP_HEATMAP_MAX_KEYPOINT_OP = 92,
  EDEN_OP_LOG_SOFTMAX = 93,
  EDEN_OP_CHANNEL_SHUFFLE = 94,

  EDEN_OP_RANDOM_MULTINOMIAL = 95,
  EDEN_OP_LESS_EQUAL = 96,
  EDEN_OP_REDUCE_SUM = 97,
  EDEN_OP_REDUCE_MIN = 98,
  EDEN_OP_REDUCE_MAX = 99,

  EDEN_OP_REDUCE_PROD = 100,
  EDEN_OP_REDUCE_ALL = 101,
  EDEN_OP_REDUCE_ANY = 102,
  EDEN_OP_TILE = 103,
  EDEN_OP_TF_SLICE = 104,
  EDEN_OP_LESS = 105,
  EDEN_OP_MERGE_CONVOLUTION_RESHAPE_CONCAT = 106,
  EDEN_OP_FILL = 107,
  EDEN_OP_ELU = 108,
  EDEN_OP_QUANTIZED_LSTM = 109,
  EDEN_OP_HARDSWISH = 110,
  EDEN_OP_RANK = 111,
  EDEN_OP_CONVERT_NCHW2NHWC = 112,
  EDEN_OP_CONVERT_NHWC2NCHW = 113,
  EDEN_OP_UNPACK = 114,

  // NOTICE: When you add new operation on EDEN_OP_NUM,
  //         you should add them on edenOpNames on tflite_converter.cpp too!!

  EDEN_OP_NUM_MAXIMUM
} EDEN_OP_NUM;

typedef enum {
  EDEN_FUSED_ACT_NONE = 0,
  EDEN_FUSED_ACT_RELU = 1,
  EDEN_FUSED_ACT_RELU1 = 2,
  EDEN_FUSED_ACT_RELU6 = 3,
  EDEN_FUSED_ACT_TANH = 4,
  EDEN_FUSED_ACT_LOGISTIC = 5,
} FusedActivation;

typedef enum {
  EDEN_PRIORBOX_CODING_TYPE_CORNER = 0,
  EDEN_PRIORBOX_CODING_TYPE_CENTER_SIZE = 1,
  EDEN_PRIORBOX_CODING_TYPE_CORNER_SIZE = 2,
} PriorBoxCodingType;

typedef enum {
  EDEN_NORM_REGION_ACROSS_CHANNELS = 0,
  EDEN_NORM_REGION_WITHIN_CHANNEL = 1,
} NormRegion;

typedef enum {
  EDEN_LSTM_KERNEL_TYPE_FULL = 0,
  EDEN_LSTM_KERNEL_TYPE_BASIC = 1,
  EDEN_LSTM_KERNEL_TYPE_LAYER_NORM = 2,
} LSTMKernelType;

typedef enum {
  CPU = 1,
  GPU = 2,
  NPU = 4,
  DSP = 8,
} EdenTargetDevice;

/*! performance dump */
typedef enum {
  EXECUTE_TIME = 1,
  USED_MEMORY = 2,
  CONSUMED_POWER = 4,
} EDEN_PERFORMANCE_DUMP;

/*! optimize way */
typedef enum { SGD = 0, MOMENTUM = 1, ADAGRAD = 2, ADAM = 3 } EDEN_OPTIMIZE_WAY;

/*! data type */
typedef enum {
  DATA_TYPE_INT8 = 0,
  DATA_TYPE_UINT8 = 1,
  DATA_TYPE_INT16 = 2,
  DATA_TYPE_UINT16 = 3,
  DATA_TYPE_INT32 = 4,
  DATA_TYPE_INT64 = 5,
  DATA_TYPE_FLOAT16 = 6,
  DATA_TYPE_FLOAT32 = 7,
  DATA_TYPE_FLOAT64 = 8,
  DATA_TYPE_BOOL8 = 9,
  DATA_TYPE_INVALID
} EdenDataType;

typedef enum {
  DATA_TYPE_INT8_SIZE = 1,
  DATA_TYPE_UINT8_SIZE = 1,
  DATA_TYPE_INT16_SIZE = 2,
  DATA_TYPE_UINT16_SIZE = 2,
  DATA_TYPE_INT32_SIZE = 4,
  DATA_TYPE_INT64_SIZE = 8,
  DATA_TYPE_FLOAT16_SIZE = 2,
  DATA_TYPE_FLOAT32_SIZE = 4,
  DATA_TYPE_FLOAT64_SIZE = 8,
  DATA_TYPE_BOOL8_SIZE = 1,
  DATA_TYPE_INVALID_SIZE = 0
} EdenDataTypeSize;

/*! buffer layout */
typedef enum {
  BUFFER_LAYOUT_NCHW = 0,
  BUFFER_LAYOUT_NHWC = 1,
  BUFFER_LAYOUT_CUSTOM = 2,
} EdenBufferLayout;

/*! buffer trait type */
typedef enum {
  IMAGE = 0,
  AUDIO = 1,
  TEXT = 2,
  VIDEO = 3,
  //...
  RAWDATA = 15,
  LZW = 16,
} EDEN_BUFFER_TRAIT_TYPE;

/*! image format */
typedef enum {
  RGB = 0,
  RGB_R = 1,
  RGB_G = 2,
  RGB_B = 3,
  RGB_A = 4,
  YUV = 5,
  Y = 6,
  U = 7,
  V = 8,
  //...
} EDEN_IMAGE_FORMAT;

/*! audio format */
typedef enum {
  RAW = 0,
  WAV = 1,
  //...
} EDEN_AUDIO_FORMAT;

/*! quantize data type */
typedef enum {
  QUANT_DATA_TYPE_INT8 = 0,
  QUANT_DATA_TYPE_INT16 = 1,
  QUANT_DATA_TYPE_INT32 = 2,
} EDEN_QUANT_DATA_TYPE;

/*! quantize way */
typedef enum {
  SCALE_ZEROPOINT = 0,
  FRACTIONAL_LENGTH = 1,
} EdenQuantWay;

/*! quantize way */
typedef enum {
  SYMM_PER_CHANNEL_QUANT = 0,
  EXTENSION = 1,
} EdenExtraParamType;

typedef enum {
  NORMAL = 0,
  // Will be added later
} EDEN_RATE_CONTROL_POLICY;

typedef enum {
  EDEN_MEM_TYPE_ION = 0,
  EDEN_MEM_TYPE_HEAP = 1,
} EdenMemType;

// Array index for dims in EdenShapeInfo
enum {
  N_NCHW = 0,
  C_NCHW = 1,
  H_NCHW = 2,
  W_NCHW = 3,
};

enum {
  N_NHWC = 0,
  H_NHWC = 1,
  W_NHWC = 2,
  C_NHWC = 3,
};

enum {
  N_CUSTOM = 0,
  C_CUSTOM = 1,
  H_CUSTOM = 2,
  W_CUSTOM = 3,
};

typedef struct __EdenString {
  int8_t* name;   /*!< character array ended with "\0" */
  int32_t length; /*!< length of name */
} EdenString;

typedef struct __EdenBufferInfo {
  EdenDataType dataType; /*!< data type of buffer (INT8, INT16, FLOAT32 etc) */
  EdenBufferLayout
      bufferLayout; /*!< data layout of buffer (NCHW, NHWC, CUSTOM etc) */
  int32_t numOfContentsInBuffer; /*!< N */
  int32_t channel;               /*!< C */
  int32_t height;                /*!< H */
  int32_t width;                 /*!< W */
} EdenBufferInfo;

typedef struct __EdenBufferNameMap {
  EdenString bufferName;      /*!< name of buffer */
  EdenBuffer* buffer;         /*!< buffer */
  EdenBufferInfo* bufferInfo; /*!< buffer information */
} EdenBufferNameMap;

typedef struct __EdenBufferSetInfo {
  int32_t numOfBuffers;          /*!< # of buffers */
  EdenBufferNameMap* bufferMaps; /*!< array of buffer mappings */
} EdenBufferSetInfo;

typedef struct __EdenBufferTraitInfo {
  EDEN_BUFFER_TRAIT_TYPE bufferTraitType; /*!< type of buffer */
  void* bufferTraitData;                  /*!< address to a buffer trait data */
  int32_t bufferTraitSize;                /*!< size of buffer trait data */
} EdenBufferTraitInfo;

typedef struct __EdenBufferQuantizeInfo {
  EDEN_QUANT_DATA_TYPE
  quantizeDataType;         /*!< quantization data type (INT8, INT16, INT32) */
  EdenQuantWay quantizeWay; /*!< quantization type (Qm.n, scale) */
  void* quantizeInfo;       /*!< address to quantization information */
} EdenBufferQuantizeInfo;

typedef struct __EdenScaleQuantInfo {
  float scale;        /*!< scale */
  int32_t zeroPoint;  /*!< zero point */
  float realValueMin; /*!< real value minimum */
  float realValueMax; /*!< real value maximum */
  int32_t bitLength;  /*!< bit length (8bit, 16bit etc) */
} EdenScaleQuantInfo;

typedef struct __EdenFLQuantInfo {
  uint8_t integerBitLength;    /*!< m, integer bit length */
  uint8_t fractionalBitLength; /*!< n, fractional bit length */
} EdenFLQuantInfo;

typedef struct __EdenExtraParamsSymmPerChannelQuant {
  std::vector<float> scales;
  uint32_t channelDim;
} EdenExtraParamsSymmPerChannelQuant;

typedef struct __EdenShapeInfo {
  EdenDataType dataType;
  EdenDataTypeSize typeSize;
  EdenBufferLayout bufferLayout;
  int32_t numOfDims;
  int32_t dims[4];
} EdenShapeInfo;

typedef struct __EdenQuantInfo {
  EdenQuantWay quantWay;
  void* data;
} EdenQuantInfo;

typedef struct __EdenExtraParams {
  EdenExtraParamType type;
  void* data;
} EdenExtraParams;

typedef struct __AddOptions {
  int32_t numOfCoeffs;
  std::unique_ptr<float[]> coeffs;
  FusedActivation fusedActivation;
} AddOptions;

typedef struct __ConcatenationOptions {
  int32_t axis;
} ConcatenationOptions;

typedef struct __Conv2DOptions {
  int32_t padLeft;
  int32_t padRight;
  int32_t padTop;
  int32_t padBottom;
  int32_t strideWidth;
  int32_t strideHeight;
  FusedActivation fusedActivation;
  bool NCHWDataLayout;
  int32_t dilationWidthFactor;
  int32_t dilationHeightFactor;
} Conv2DOptions;

typedef struct __DepthwiseConv2DOptions {
  int32_t padLeft;
  int32_t padRight;
  int32_t padTop;
  int32_t padBottom;
  int32_t strideWidth;
  int32_t strideHeight;
  int32_t depthMultiplier;
  FusedActivation fusedActivation;
  bool NCHWDataLayout;
  int32_t dilationWidthFactor;
  int32_t dilationHeightFactor;
} DepthwiseConv2DOptions;

typedef struct __DepthSpaceOptions {
  int32_t blockSize;
  bool NCHWDataLayout;
} DepthSpaceOptions;

typedef struct __Deconv2DOptions {
  int32_t padLeft;
  int32_t padRight;
  int32_t padTop;
  int32_t padBottom;
  int32_t strideWidth;
  int32_t strideHeight;
  int32_t group;
  FusedActivation fusedActivation;
} Deconv2DOptions;

typedef struct __FusedActivationOptions {
  FusedActivation fusedActivation;
} FusedActivationOptions;

typedef struct __Pool2DOptions {
  int32_t padLeft;
  int32_t padRight;
  int32_t padTop;
  int32_t padBottom;
  int32_t strideWidth;
  int32_t strideHeight;
  int32_t kernelWidth;
  int32_t kernelHeight;
  FusedActivation fusedActivation;
  bool NCHWDataLayout;
} Pool2DOptions;

typedef struct __LocalResponseNormalizationOptions {
  int32_t radius;
  float bias;
  float alpha;
  float beta;
  NormRegion normRegion;
  int32_t axis;
  int32_t numOfInputDims;
} LocalResponseNormalizationOptions;

typedef struct __LshProjectionOptions {
  int32_t numOfHashFunctions;
  int32_t numOfSeedsPerHashFunction;
  float* seeds;
  int32_t numOfWeight;
  float* weights;
  int32_t lshType;
} LshProjectionOptions;

typedef struct __LSTMOptions {
  // Parameters for LSTM version 1 or above.
  FusedActivation fusedActivation;
  float cellClip;  // Optional, 0.0 means no clipping
  float projClip;  // Optional, 0.0 means no clipping

  // Parameters for LSTM version 2 or above.
  LSTMKernelType kernelType;  // Default is FULL
} LSTMOptions;

typedef struct __MulOptions {
  int32_t numOfCoeffs;
  std::unique_ptr<float[]> coeffs;
  FusedActivation fusedActivation;
} MulOptions;

typedef struct __ReshapeOptions {
  int32_t numOfDims;
  std::unique_ptr<int32_t[]> dims;
} ReshapeOptions;

typedef struct __ResizeBilinearOptions {
  int32_t outputWidth;
  int32_t outputHeight;
  bool NCHWDataLayout;
  bool alignCorners;
  bool halfPixelCenters;
} ResizeBilinearOptions;

typedef struct __RNNOptions {
  FusedActivation fusedActivation;
} RNNOptions;

typedef struct __SoftmaxOptions {
  float beta;
  int32_t axis;
} SoftmaxOptions;

typedef struct __SVDFOptions {
  int32_t rank;
  FusedActivation fusedActivation;
} SVDFOptions;

typedef struct __StridedSliceOptions {
  int32_t beginMask;
  int32_t endMask;
  int32_t ellipsisMask;
  int32_t newAxisMask;
  int32_t shrinkAxisMask;
} StridedSliceOptions;

typedef struct __InstanceNormalizationOptions {
  float gamma;
  float beta;
  float epsilon;
} InstanceNormalizationOptions;

typedef struct __DivOptions {
  int32_t numOfCoeffs;
  std::unique_ptr<float[]> coeffs;
  FusedActivation fusedActivation;
} DivOptions;

typedef struct __SubOptions {
  int32_t numOfCoeffs;
  std::unique_ptr<float[]> coeffs;
  FusedActivation fusedActivation;
} SubOptions;

typedef struct __TransposeOptions {
  int32_t numOfPermIndex;
  std::unique_ptr<int32_t[]> permIndex;
} TransposeOptions;

typedef struct __PReLUOptions {
  int32_t numOfAlphas;
  std::unique_ptr<float[]> alphas;
} PReLUOptions;

typedef struct __ElementwiseMaxOptions {
  int32_t numOfCoeffs;
  std::unique_ptr<float[]> coeffs;
  FusedActivation fusedActivation;
} ElementwiseMaxOptions;

typedef struct __ArgmaxOptions {
  int32_t axis;
} ArgmaxOptions;

typedef struct __ScaleOptions {
  int32_t numOfChannels;
  std::unique_ptr<float[]> scales;
  std::unique_ptr<float[]> biass;
} ScaleOptions;

typedef struct __CropOptions {
  int32_t axis;
  int32_t numOfOffset;
  std::unique_ptr<int32_t[]> offsets;
} CropOptions;

typedef struct __FlattenOptions {
  int32_t startAxis;
  int32_t endAxis;
} FlattenOptions;

typedef struct __PermuteOptions {
  int32_t numOfOrders;
  std::unique_ptr<int32_t[]> permuteAxisOrders;
} PermuteOptions;

typedef struct __SliceOptions {
  int32_t axis;
  int32_t numOfSlicePoint;
  std::unique_ptr<int32_t[]> slicePoints;
} SliceOptions;

typedef struct __PriorBoxOptions {
  int32_t numOfMinMaxSize;
  std::unique_ptr<float[]> minSize;
  std::unique_ptr<float[]> maxSize;
  int32_t numOfAspectRatio;
  std::unique_ptr<float[]> aspectRatio;
  int32_t flip;
  int32_t clip;
  int32_t numOfVariance;
  std::unique_ptr<float[]> variance;
  int32_t imageWidth;
  int32_t imageHeight;
  float stepWidth;
  float stepHeight;
  float offset;
} PriorBoxOptions;

typedef struct __PowerOptions {
  float power;
  float scale;
  float shift;
} PowerOptions;

typedef struct __DetectionOptions {
  int32_t numClasses;
  int32_t shareLocation;
  int32_t backgroundLabelId;
  float nmsThreshold;
  int32_t nmsTopK;
  float nmsEta;
  PriorBoxCodingType codingType;
  int32_t varianceEncodedInTarget;
  float confidenceThreshold;
  int32_t keepTopK;
} DetectionOptions;

typedef struct __ROIpoolOptions {
  int32_t poolHeight;
  int32_t poolWidth;
  float scale;
} ROIpoolOptions;

typedef struct __TFliteRoiPoolOptions {
  int32_t output_height;
  int32_t output_width;
  float height_stride;
  float width_stride;
} TFliteRoiPoolOptions;

typedef struct __UnidirectionalSequenceLSTMOptions {
  // Parameters for LSTM version 1 or above.
  FusedActivation fusedActivation;
  float cellClip;  // Optional, 0.0 means no clipping
  float projClip;  // Optional, 0.0 means no clipping
  // If true then first dimension is sequence, otherwise batch.
  bool time_major;
} UnidirectionalSequenceLSTMOptions;

typedef struct __SequenceRNNOptions {
  bool time_major;
  FusedActivation fusedActivation;
} SequenceRNNOptions;

typedef struct __BidirectionalRNNOptions {
  FusedActivation fusedActivation;
  bool time_major;
  bool merge_outputs;
} BidirectionalRNNOptions;

typedef struct __BidirectionalSequenceLSTMOptions {
  FusedActivation fusedActivation;
  float cellClip;  // Optional, 0.0 means no clipping
  float projClip;  // Optional, 0.0 means no clipping

  // If true, store the outputs of both directions into the first output.
  bool merge_outputs;
  bool time_major;
} BidirectionalSequenceLSTMOptions;

typedef struct __TFliteDetectionOptions {
  uint32_t max_detections;
  uint32_t max_classes_per_detection;
  float nms_score_threshold;
  float nms_iou_threshold;
  uint32_t num_classes;
  float y_scale;
  float x_scale;
  float h_scale;
  float w_scale;
} TFliteDetectionOptions;

typedef struct __DetectionPostprocessingOptions {
  uint32_t max_detections;
  uint32_t max_classes_per_detection;
  uint32_t detections_per_class;
  float nms_score_threshold;
  float nms_iou_threshold;
  uint32_t num_classes;
  float y_scale;
  float x_scale;
  float h_scale;
  float w_scale;
  bool useRegularNms;
  bool isBGInLabel;
} DetectionPostprocessingOptions;

typedef struct __RoiAlignOptions {
  uint32_t output_height;
  uint32_t output_width;
  float stride_height;
  float stride_width;
  int32_t ratio_height;
  int32_t ratio_width;
} RoiAlignOptions;

typedef struct __TopK_V2Options {
  int32_t k;
  int32_t axis;
} TopK_V2Options;

typedef struct __GenerateProposalOptions {
  float height_stride;
  float width_stride;
  int32_t pre_nms;
  int32_t post_nms;
  float IoU_threshold;
  float min_size;
  bool layout;
} GenerateProposalOptions;

typedef struct __ResizeNearestNeighborOptions {
  // The following two scalars represent output shape if INT32, scale if
  // floating point.
  float height_scale;
  float width_scale;
  uint32_t type;
  bool align_corners;
  bool half_pixel_centers;
} ResizeNearestNeighborOptions;

typedef struct __ExpandDimsOptions {
  int32_t axis;
} ExpandDimsOptions;

typedef struct __GatherOptions {
  int32_t axis;
} GatherOptions;

typedef struct __SplitOptions {
  int32_t num_outputs;
  int32_t axis;
} SplitOptions;

typedef struct __L2NormalizationOptions {
  int32_t axis;
} L2NormalizationOptions;

typedef struct __BatchToSpaceNDOptions {
  bool NCHWDataLayout;
} BatchToSpaceNDOptions;

typedef struct __SpaceToBatchNDOptions {
  bool NCHWDataLayout;
} SpaceToBatchNDOptions;

typedef struct __ArgminOptions {
  int32_t axis;
} ArgminOptions;

typedef struct __BoxWithNmsLimitOptions {
  float scoreThreshold;
  int32_t maxNumDetection;
  int32_t nmsKernel;
  float ioUThreshold;
  float sigma;
  float nmsScoreThreshold;
} BoxWithNmsLimitOptions;

typedef struct __LogSoftmaxOptions {
  float beta;
  uint32_t axis;
} LogSoftmaxOptions;

typedef struct __ChannelShuffleOptions {
  uint32_t group;
  uint32_t axis;
} ChannelShuffleOptions;

typedef struct __RandomMultinomialOptions {
  int sample_count;
  std::unique_ptr<int32_t[]> seeds;
} RandomMultinomialOptions;

typedef struct __ReduceOptions {
  bool keep_dims;
} ReduceOptions;

typedef struct __RankOptions {
  uint32_t rank;
} RankOptions;

typedef struct __QuantizedLSTMOptions {
  // Clipping.
  float cell_clip;
  float projection_clip;
  // Scales of the result of matmul, i.e. input to layer normalization.
  float input_intermediate;
  float forget_intermediate;
  float cell_intermediate;
  float output_intermediate;
  // Zero point and scale of hidden state.
  int8_t hidden_state_zero_proint;
  float hidden_state_scale;
} QuantizedLSTMOptions;

typedef struct __UnpackOptions {
  uint16_t num_split;
  int16_t axis;
} UnpackOptions;

typedef struct __EdenOperand {
  EdenString name;
  EdenBuffer* buffer;
  EdenShapeInfo* shapeInfo;
  EdenQuantInfo* quantInfo;

  int32_t opType;
  EdenExtraParams* extraParams;
  uint32_t reserved[3];
  bool isNCHW;
} EdenOperand;

typedef struct __EdenOperation {
  int32_t opType;
  EdenString opName;
  int32_t numOfInputs;
  int32_t* inputOperandIndexes;
  int32_t numOfOutputs;
  int32_t* outputOperandIndexes;
  bool hasOptions;
  bool isNCHW;
  bool delegateNHWC;
#ifdef DSP_USERDRIVER
  // DSP user-driver template code by steven.oh
  uint8_t* custom_buffer;
  size_t custom_size;
#endif
} EdenOperation;

typedef struct __EdenNode {
  int32_t operationId; /*!< operation id */

  std::vector<struct __EdenNode*>
      forwardNodes; /*!< neighbors for forward path */
  // std::vector<struct __EdenNode*> backwardNodes;  /*!< neighbors for backward

  std::vector<int32_t> inputOperandIds;
  std::vector<int32_t> outputOperandIds;

  int32_t status;  // 0: Not yet visited, 1: Pused into queue, 2: Visited
} EdenNode;

#ifdef __cplusplus
}
#endif

#endif  // EDENMODEL_INCLUDE_EDEN_MODEL_TYPES_H_
