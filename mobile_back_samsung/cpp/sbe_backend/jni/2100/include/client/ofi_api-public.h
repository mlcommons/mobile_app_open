/*
 * Copyright (C) 2018 Samsung Electronics Co. LTD
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or
 * distributed, transmitted, transcribed, stored in a retrieval system or
 * translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed to third parties
 * without the express written permission of Samsung Electronics.
 *
 */

/**
 * @file ofi_api-public.h
 */

#ifndef SOURCE_DSP_NN_INCLUDE_OFI_API_PUBLIC_H_
#define SOURCE_DSP_NN_INCLUDE_OFI_API_PUBLIC_H_

#include <future>
#include <string>
#include "ofi_mm_memory-public.h"

typedef MemItem_t *ofi_memory;  ///< user type: encapsulation of MemItem_t
typedef int ofi_mm_type;        ///< user type: encapsulation of memory type
typedef int ofi_mm_opt;         ///< user type: encapsulation of memory option
typedef uint32_t ofi_model_id;  ///< user type: model id
typedef void *ofi_usr_image;  ///< user type: encapsulation of ofi image object

/**
 * @brief Buffer direction to set in/out buffer
 */
typedef enum ofi_buf_dir {
  OFI_BUF_DIR_IN,       ///< Input buffer of a graph
  OFI_BUF_DIR_OUT,      ///< Output buffer of a graph
  OFI_BUF_DIR_INTER,    ///< Buffer between nodes
  OFI_BUF_DIR_COREBUF,  ///< non-directional core Bufffer
} ofi_buf_dir_e;

/**
 * @brief ofi_usr_image_type
 * define image types enumeration as a parameter
 */
typedef enum ofi_usr_image_type_e {
  OFI_USR_IMAGE_NONE = 0x00,
  OFI_USR_IMAGE_TYPE_U8,              ///< g eneral data [1BYTE]
  OFI_USR_IMAGE_TYPE_U16,             ///< general data [2BYTE]
  OFI_USR_IMAGE_TYPE_U32,             ///< general data [4BYTE]
  OFI_USR_IMAGE_TYPE_S8,              ///< g eneral data [1BYTE]
  OFI_USR_IMAGE_TYPE_S16,             ///< general data [2BYTE]
  OFI_USR_IMAGE_TYPE_S32,             ///< general data [4BYTE]
  OFI_USR_IMAGE_TYPE_RGBX888,         ///< RGBX888
  OFI_USR_IMAGE_TYPE_RGB888,          ///< RGB888
  OFI_USR_IMAGE_TYPE_BGR8888,         ///< BGRX888
  OFI_USR_IMAGE_TYPE_BGR888,          ///< BGR888
  OFI_USR_IMAGE_TYPE_LUV888,          ///< LUV888
  OFI_USR_IMAGE_TYPE_YUV422_1P,       ///< YCbCr422 1p
  OFI_USR_IMAGE_TYPE_YUV422_2P,       ///< YCbCr422 2p
  OFI_USR_IMAGE_TYPE_YUV422_UV,       ///< YCbCr422 UV
  OFI_USR_IMAGE_TYPE_YUV420_NV21,     ///< YCbCr420 NV21
  OFI_USR_IMAGE_TYPE_YUV420_NV12,     ///< YCbCr420 NV12
  OFI_USR_IMAGE_TYPE_YUV420_NV21_UV,  ///< YCbCr420 NV21 UV
  OFI_USR_IMAGE_TYPE_YUV420_NV12_UV,  ///< YCbCr420 NV12 UV
  OFI_USR_IMAGE_TYPE_F8,
  OFI_USR_IMAGE_TYPE_F16,
  OFI_USR_IMAGE_TYPE_F32,
  OFI_USR_IMAGE_TYPE_MAX,
} ofi_usr_image_type;

/**
 * @brief ofi_usr_type_target
 * Indicates target type(CPU or DEVICE) that is used in memory allocation type
 */
typedef enum _ofi_usr_type_target_e {
  OFI_USR_TARGET_CPU = 0,
  OFI_USR_TARGET_DEVICE,
  OFI_USR_TARGET_MAX,
} ofi_usr_type_target;

/**
 * @brief Describe shape data
 * @see ofi_shape_info
 */
typedef struct ofi_shape_info_t {
  int channel;  ///< int: channel
  int height;   ///< int: height
  int width;    ///< int: width
} ofi_shape_info;

/**
 * @brief Describe buffer info data
 * @see ofi_buffer_info
 */
typedef struct ofi_buffer_info_t {
  ofi_shape_info info;
  int core_buffer_index;
} ofi_buffer_info;

/**
 * @brief Describe image meta data
 * @see ofi_usr_image_type
 * @see ofi_usr_type_target
 */
typedef struct ofi_usr_image_meta_t {
  ofi_usr_type_target target;  ///< enum ofi_usr_type_target
  ofi_usr_image_type type;     ///< enum ofi_usr_image_type
  int width;                   ///< int: width
  int height;                  ///< int: height
} ofi_usr_image_meta;

#if 1  // def OFI_USE_CALLBACK

/**
 * @brief Define Callback type
 *
 */
typedef enum ofi_cb_type {
  OFI_CB_NON_BLOCK = 0,  ///< Aynchronous callback function
  OFI_CB_BLOCK,          ///< Synchronous callback function
  OFI_CB_MAX,
} ofi_cb_type_e;

/**
 * @brief Prototype of callback function
 *
 */
typedef int (*ofi_usr_callback_func_p)(int, void **);

/**
 * @brief User can register callback with desc, param.
 * ofi_usr_callback_desc indicates function pointer in user area.
 */
struct ofi_usr_callback_desc {
  ofi_usr_callback_func_p func_p;
};

/**
 * @brief User can register callback with desc, param.
 * ofi_usr_callback_param indicates parameters of callback function
 */
struct ofi_usr_callback_param {
  int num_param;
  void **params;
};

#define N_SKIP_ID_MAX (20)
struct ofi_model_exe_attr {
  bool is_wait = true;
  int n_skip = 0;
  uint8_t skip_id[N_SKIP_ID_MAX] = {
      0,
  };
  uint64_t request_id = 0;
  // variables are used for async.
  int *result;
  bool is_async = false;
  int numOfIn = 0;
  int numOfOut = 0;
  int numOfUsrparam = 0;
  ofi_memory *in_buffers = nullptr;
  ofi_memory *out_buffers = nullptr;
  ofi_memory *usrparam_buffers = nullptr;  // array
  ofi_shape_info *in_buffers_shape = nullptr;
  ofi_shape_info *out_buffers_shape = nullptr;
  ofi_usr_callback_desc function_desc;
  ofi_usr_callback_param param;
};

#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Default NN_ID. Eventhough two graphs includes NN_ID_DEFAULTas a nn_id,
 * the graphs are independent in a service core
 */
#define NN_ID_DEFAULT (0xFFF0)

/**
 * @brief returns a version of framework.
 * format of version might depends on its device type.
 * @input char[256]
 */
extern int ofi_get_framework_version(char *ver_char_256);

/**
 * @brief returns a version of framework.
 * format of version might depends on its device type.
 * @input char[256]
 * @output int negative: failed, zero: success
 */
extern int ofi_get_model_compiler_version(ofi_model_id model_id,
                                          char *ver_char_256);

/**
 * @brief  Basic APi for OFI Framework.
 * @return returns 0 if success. if err, return negative value
 */
extern int ofi_initialize(void);

/**
 * @brief  Basic APi for OFI Framework for secure
 * @return returns 0 if success. if err, return negative value
 */
extern int ofi_initialize_secure(void);

/**
 *  * @brief  Basic APi for OFI Framework.
 *   * With this api, the service doesn't care the client's status.
 *    * We recommand use this, but in the special case, the client should use
 *    this. (RPC broken)
 *     * @return returns 0 if success. if err, return negative value
 *      */
extern int ofi_initialize_no_monitor(void);

/**
 * @brief ofi_load_model: load nn pre compiled model from file
 * @param compiled_graph_path pre-compiled graph from compiler
 * @return if err, returns 0
 */
extern ofi_model_id ofi_load_model(const char *compiled_graph_path);
/**
 * @brief ofi_load_model_from_memory: load nn pre compiled model from memory
 * @param va memory address that model loaded
 * @param size memory size that model loaded
 * @return if err, returns 0
 */
extern ofi_model_id ofi_load_model_from_memory(char *va, int size);

#ifdef OFI_USE_HIDL_INTERFACE
/**
 * @brief ofi_load_model_at_service: load nn pre compiled model from file at
 * service
 * @param compiled_graph_path pre-compiled graph from compiler
 * @return if err, returns 0
 */
extern ofi_model_id ofi_load_model_at_service(const char *compiled_graph_path);
#endif

/**
 * @brief Set nn_id. User manualy indicates multiple graph which has same
 * topologies with same nn_id, not DEFAULT_NN_ID.
 * @param model_id model ID
 * @param nn_id int(32 bits)
 * @return int 0 if succeed
 */
extern int ofi_set_model_nn_id(ofi_model_id model_id, uint32_t nn_id);
/**
 * @brief verify model and request preparation the model to the service
 *
 * @param model_id model ID
 * @return int 0 if succeed
 */
extern int ofi_verify_model(ofi_model_id model_id);
/**
 * @brief execute model. user should fill required field in a model such as
 * in/out buffers
 *
 * @param model_id model ID
 * @return int 0 if succeed
 */
extern int ofi_execute_model(ofi_model_id model_id);

/**
 * @brief wait for result
 *
 * @param req_id request ID issued by OFI framework
 * @return int 0 if succeed
 */
extern int ofi_execute_model_async_wait_for(ofi_model_id model_id,
                                            uint64_t req_id);
/**
 * @brief execute pre-model. user should fill required field in a model such as
 * in/out buffers
 *
 * @param model_id model ID
 * @return int 0 if succeed
 */
extern int ofi_execute_pre_model(ofi_model_id model_id);
/**
 * @brief close all models in a current context
 *
 * @return int 0 if succeed
 */
extern int ofi_close(void);
/**
 * @brief unload model. pair to ofi_load_model
 *
 * @param model_id model ID
 * @see ofi_load_model
 * @return int 0 if succeed
 */
extern int ofi_unload_model(ofi_model_id model_id);
/**
 * @brief deinitialize context
 * @see ofi_initialize
 * @return int 0 if succeed
 */
extern int ofi_deinitialize(void);

/**
 * @brief deinitialize context for secure
 * @see ofi_initialize
 * @return int 0 if succeed
 */
extern int ofi_deinitialize_secure(void);

/**
 * @brief Check weither this session has a context
 *
 * @return int 0 if succeed
 */
extern int ofi_context_available(void);
/**
 * @brief if there's user parameter in a current model, user can set the
 * parameter with this API.
 *
 * @param model_id model ID
 * @param user_p_buf memory object that contains user parameters
 * @param sg_id subgraph ID in a model
 * @return int 0 if succeed
 */
extern int ofi_set_execute_user_param(ofi_model_id model_id,
                                      ofi_memory user_p_buf, int sg_id);

/**
 * @brief set core buffer to execute model
 *
 * @param model model ID
 * @param mobj Memory Object contains in or out buffer
 * @param buf_idx buffer index of model
 * @return int 0 if succeed
 */
extern int ofi_model_set_core_buf(ofi_model_id graph_id, ofi_memory mobj,
                                  int buf_idx);

/**
 * @brief set in / out buffer to execute model
 *
 * @param model model ID
 * @param mobj Memory Object contains in or out buffer
 * @param dir direction
 * @param idx buffer index in model
 * @return int 0 if succeed
 */
extern int ofi_model_set_in_out_buf(ofi_model_id model, ofi_memory mobj,
                                    ofi_buf_dir_e dir, int idx);
/**
 * @brief set in / out buffer to execute pre-model
 *
 * @param model model ID
 * @param mobj Memory Object contains in or out buffer
 * @param dir direction
 * @param idx buffer index in model
 * @return int 0 if succeed
 */
extern int ofi_pre_model_set_in_out_buf(ofi_model_id model, ofi_memory mobj,
                                        ofi_buf_dir_e dir, int idx);
/**
 * @brief set in / out image to execute model
 *
 * @param model model ID
 * @param mobj image object that contains in or out image
 * @param dir direction
 * @param idx buffer index in model
 * @return int 0 if succeed. The API compares the meta data in a model and
 * parameters
 */
extern int ofi_model_set_in_out_image(ofi_model_id model, ofi_usr_image mobj,
                                      ofi_buf_dir_e dir, int idx);
/**
 * @brief Set shape information of "layer_name"
 *
 * @param model_id model ID
 * @param layer_name layer name (string)
 * @param channel number of channel
 * @param height height
 * @param width width
 * @return int 0 if succeed
 */
extern int ofi_inbuf_shape_overriding(ofi_model_id model_id, char *layer_name,
                                      uint32_t channel, uint32_t height,
                                      uint32_t width);
/**
 * @brief [NFD only] Set shape information of idx th in-buffer of model
 *
 * @param model model ID
 * @param mobj memory object (just check)
 * @param idx index of in buffer
 * @param shape [3]: channel, height, width
 * @return int 0 if succeed
 */
extern int ofi_model_shape_sync_in_buf(ofi_model_id model, ofi_memory mobj,
                                       int idx, ofi_shape_info shape);

/**
 * @brief [NFD only] Set shape information from image object to [idx] th
 * in-buffer of model
 *
 * @param model model ID
 * @param obj image object
 * @param idx index of in buffer
 * @return int 0 if success
 */
extern int ofi_model_shape_sync_in_image(ofi_model_id model, ofi_usr_image obj,
                                         int idx);
/**
 * @brief Not used
 *
 * @param model_id
 * @param idx
 * @param mobj
 * @return int
 */
extern int ofi_model_set_operand(ofi_model_id model_id, int idx,
                                 ofi_memory mobj);
#if 1  // def OFI_USE_CALLBACK

/**
 * @brief Register callback function to a (sg_id)th node in model
 *
 * @param model_id model ID
 * @param sg_id subgraph(node) ID
 * @param desc callback description. function pointer
 * @param params callback parameters.
 * @param cb_type callback type.
 * @return int 0 if succeed
 */
extern int ofi_model_register_callback(ofi_model_id model_id, int sg_id,
                                       ofi_usr_callback_desc desc,
                                       ofi_usr_callback_param params,
                                       ofi_cb_type_e cb_type);
#endif

/* TODO: write description */
extern int ofi_allocateInputs(ofi_model_id model, int *numOfBuffers,
                              ofi_memory **buffers);
extern int ofi_allocateOutputs(ofi_model_id model, int *numOfBuffers,
                               ofi_memory **buffers);
extern int ofi_freeInputs(ofi_model_id model, ofi_memory *buffers);
extern int ofi_freeOutputs(ofi_model_id model, ofi_memory *buffers);
extern int ofi_execute_model_attr(ofi_model_id model_id,
                                  ofi_model_exe_attr *attr);

/**
 * @brief Create memory object
 *
 * @param size request size
 * @param mm_type target type (to DEV or CPU)
 * @param mm_option memory allocation option
 * @return ofi_memory memory object
 */
extern ofi_memory ofi_create_memory(int32_t size, ofi_mm_type mm_type,
                                    ofi_mm_opt mm_option);

/**
 * @brief create memory object with external file descriptor
 *
 * @param size memory size
 * @param fd file descriptor which allocated from outside
 * @param predefined_va if already mapped, fill this field
 * @param memtype memory type
 * @return ofi_memory memory object
 */
extern ofi_memory ofi_create_memory_from_fd(int size, int fd,
                                            void *predefined_va,
                                            int32_t memtype);
/**
 * @brief create memory object with external file descriptor
 *
 * @param size memory size
 * @param fd file descriptor which allocated from outside
 * @param offset memory offset
 * @param predefined_va if already mapped, fill this field
 * @param memtype memory type
 * @return ofi_memory memory object
 */
extern ofi_memory ofi_create_memory_from_fd_with_offset(int size, int fd,
                                                        int offset,
                                                        void *predefined_va,
                                                        int32_t memtype);

/**
 * @brief release memory object generated by ofi_create_memory_from_fd
 *
 * @param mobj memory object
 * @see ofi_create_memory_from_fd
 * @return int 0 if succeed
 */
extern int ofi_release_memory_from_fd(ofi_memory mobj);

/**
 * @brief release memory object
 *
 * @param mobj memory object
 * @see ofi_create_memory
 * @return int 0 if succeed
 */
extern int ofi_release_memory(ofi_memory mobj);

/**
 * @brief create empty image object
 *
 * @return ofi_usr_image image object
 */
extern ofi_usr_image ofi_create_image(void);

/**
 * @brief destroy image object
 *
 * @param obj image object
 * @return int 0 if succeed
 */
extern int ofi_destroy_image(ofi_usr_image obj);

/**
 * @brief Allocate image with meta
 *
 * @param obj image object
 * @return int 0 if succeed
 */
extern int ofi_allocate_image(ofi_usr_image obj);
/**
 * @brief Free image buffer in image object
 *
 * @param obj image object
 * @see ofi_allocate_image
 * @return int 0 if succeed
 */
extern int ofi_free_image(ofi_usr_image obj);

/**
 * @brief import image buffer from external fd in image object
 *
 * @param obj image object
 * @param fd file descriptor
 * @param size buffer size in fd
 * @param mem_type memory type if set
 * @return int 0 if succeed
 */
extern int ofi_allocate_image_from_fd(ofi_usr_image obj, int fd, int size,
                                      int mem_type);

/**
 * @brief free image buffer in image object
 *
 * @param obj image object
 * @see ofi_allocate_image_from_fd
 * @return int 0 if succeed
 */
extern int ofi_free_image_from_fd(ofi_usr_image obj);

/**
 * @brief set image meta information in image object
 *
 * @param obj image object
 * @param meta meta data
 * @return int 0 if succeed
 */
extern int ofi_set_image_info(ofi_usr_image obj, ofi_usr_image_meta meta);

/**
 * @brief allocate and load buffer from file in image object <br>
 * User should set meta information before call this function.
 *
 * @param obj image object
 * @param filename file name (string)
 * @return int 0 if succeed
 */
extern int ofi_load_image_from_rawfile(ofi_usr_image obj, const char *filename);

/**
 * @brief allocate and load buffer with meta data from file in image object
 *
 * @param obj image object
 * @param filename file name (string)
 * @param meta meta data
 * @return int 0 if succeed
 */
extern int ofi_load_image_from_rawfile_with_meta(ofi_usr_image obj,
                                                 const char *filename,
                                                 ofi_usr_image_meta meta);

/**
 * @brief update image data from file in image object
 *
 * @param obj image object
 * @param filename file name (string)
 * @return int 0 if succeed
 */
extern int ofi_update_image_from_rawfile(ofi_usr_image obj,
                                         const char *filename);

/**
 * @brief Get base addresses of all plains in a image object
 *
 * @param img image object
 * @param addr_array expected 2-dim void* array that user allocated
 * @return int 0 if succeed
 */
extern int ofi_get_plain_base_addr(ofi_usr_image img, void **addr_array);
/**
 * @brief get base address of [index] plain in a image object
 *
 * @param img image object
 * @param index plain index
 * @return MemItem_t* Memory Object
 */
extern MemItem_t *ofi_get_plain_memory(ofi_usr_image img, int index);
/**
 * @brief get base address of buffer in image object
 *
 * @param img image object
 * @return MemItem_t* Memory Object
 */
extern MemItem_t *ofi_get_memory(ofi_usr_image img);

/**
 * @brief get meta information from image object
 *
 * @param meta meta data
 * @return int 0 if succeed
 */
extern int ofi_get_image_meta(ofi_usr_image, ofi_usr_image_meta *meta);

/**
 * @brief convert image format
 *
 * @param src source image object
 * @param dst target image type
 * @return ofi_usr_image target image oejct
 */
extern ofi_usr_image ofi_convert_image_format(ofi_usr_image src,
                                              ofi_usr_image_type type);
/**
 * @brief convert image format with tuning variables
 *
 * @param src source image object
 * @param dst target image type
 * @param mean mean values if needed
 * @param scale scale values if needed
 * @note Covertable formats:
 * Source | Target
 * ------ | ------
 * OFI_IMAGE_TYPE_RGB888 | OFI_IMAGE_TYPE_RGBF32_3P
 * OFI_IMAGE_TYPE_BGR888 | OFI_IMAGE_TYPE_RGBF32_3P
 * OFI_IMAGE_TYPE_RGB888 | OFI_IMAGE_TYPE_BGRF32_3P
 * OFI_IMAGE_TYPE_BGR888 | OFI_IMAGE_TYPE_BGRF32_3P
 * OFI_IMAGE_TYPE_RGBF32_3P | OFI_IMAGE_TYPE_RGB888
 * OFI_IMAGE_TYPE_RGB888 | OFI_IMAGE_TYPE_RGB888_3P
 *
 * @return ofi_usr_image target image oejct
 */
extern ofi_usr_image ofi_convert_image_format_with_tuning(
    ofi_usr_image src, ofi_usr_image_type type, void *, void *);

/**
 * @brief get the index of the Scalar by layer name
 *
 * @param model_id model ID
 * @param layer_name layer name (string)
 * @return index(int) if succeed
 */
extern int ofi_get_scalar_index_by_name(ofi_model_id model_id,
                                        char *layer_name);

/**
 * @brief get the index of IN/ OUT Buffer by layer name
 *
 * @param model_id model ID
 * @param layer_name layer name (string)
 * @param dir direction
 * @return index(int) if succeed
 */
extern int ofi_get_buffer_index_by_name(ofi_model_id model_id,
                                        const char *layer_name,
                                        ofi_buf_dir_e dir);

/**
 * @brief get the number of buffer by prefix
 *
 * @param model_id model ID
 * @param prefix_name prefix (string)
 * @return number of buffers(int) if succeed
 */
extern int ofi_get_num_buffer_info_by_prefix(ofi_model_id model_id,
                                             const char *prefix);

/**
 * @brief get shape information of In-buffer
 * the function fill the each parameters(width, height, channel)
 *
 * @param model_id model ID
 * @param layer_name layer name (string)
 * @param width width (pointer)
 * @param height height (pointer)
 * @param channel channel (pointer)
 * @return int 0 if succeed
 */
extern int ofi_get_inbuf_shape_info(ofi_model_id model_id,
                                    const char *layer_name, uint32_t *width,
                                    uint32_t *height, uint32_t *channel);

/**
 * @brief get buffer information by prefix
 * the function fill the each parameters(width, height, channel)
 *
 * @param model_id model ID
 * @param buffer index (string)
 * @param width width (pointer)
 * @param height height (pointer)
 * @param channel channel (pointer)
 * @return int 0 if succeed
 */
extern int ofi_get_buffer_infos_by_prefix(ofi_model_id model_id,
                                          const char *prefix,
                                          ofi_buffer_info *info);

/**
 * @brief get shape information of In-buffer
 * the function fill the each parameters(width, height, channel)
 *
 * @param model_id model ID
 * @param layer_name layer name (string)
 * @param width width (pointer)
 * @param height height (pointer)
 * @param channel channel (pointer)
 * @param source_type source type (pointer)
 * @param source_idx source index (pointer)
 * @return int 0 if succeed
 */
extern int ofi_get_inbuf_shape_info_with_idx(ofi_model_id model_id,
                                             const char *layer_name,
                                             uint32_t *width, uint32_t *height,
                                             uint32_t *channel,
                                             uint32_t *source_type,
                                             uint32_t *source_idx);
/**
 * @brief get shape information of Out-buffer
 * the function fill the each parameters(width, height, channel)
 *
 * @param model_id model ID
 * @param layer_name layer name (string)
 * @param width width (pointer)
 * @param height height (pointer)
 * @param channel channel (pointer)
 * @return int 0 if succeed
 */
extern int ofi_get_outbuf_shape_info(ofi_model_id model_id,
                                     const char *layer_name, uint32_t *width,
                                     uint32_t *height, uint32_t *channel);

/**
 * @brief get shape information of In-buffer
 * the function fill the each parameters(width, height, channel)
 *
 * @param model_id model ID
 * @param idx indx of input buffer
 * @param width width (pointer)
 * @param height height (pointer)
 * @param channel channel (pointer)
 * @return int 0 if succeed
 */
extern int ofi_get_inbuf_shape_info_by_index(ofi_model_id model_id,
                                             uint32_t index, uint32_t *width,
                                             uint32_t *height,
                                             uint32_t *channel);

/**
 * @brief get shape information of Out-buffer
 * the function fill the each parameters(width, height, channel)
 *
 * @param model_id model ID
 * @param idx index of output buffer
 * @param width width (pointer)
 * @param height height (pointer)
 * @param channel channel (pointer)
 * @return int 0 if succeed
 */
extern int ofi_get_outbuf_shape_info_by_index(ofi_model_id model_id,
                                              uint32_t index, uint32_t *width,
                                              uint32_t *height,
                                              uint32_t *channel);

/**
 * @brief get shape information of Out-buffer
 * the function fill the each parameters(width, height, channel)
 *
 * @param model_id model ID
 * @param layer_name layer name (string)
 * @param width width (pointer)
 * @param height height (pointer)
 * @param channel channel (pointer)
 * @param source_type source type (pointer)
 * @param source_idx source index (pointer)
 * @return int 0 if succeed
 */
extern int ofi_get_outbuf_shape_info_with_idx(ofi_model_id model_id,
                                              const char *layer_name,
                                              uint32_t *width, uint32_t *height,
                                              uint32_t *channel,
                                              uint32_t *source_type,
                                              uint32_t *source_idx);

/**
 * @brief [DEBUG] image object to file out
 *
 * @param img image object
 * @param filename filename to write
 * @return int 0 if succeed
 */
extern int ofi_export_image_to_file(ofi_usr_image img, const char *filename);

/**
 * @brief [DEBUG] binary compare between memory and file
 *
 * @param memory_base base address of memory
 * @param size size of memory
 * @param filename file to compare
 * @return int 0 if same
 */
extern int ofi_compare_memory_with_file(char *memory_base, int size,
                                        const char *filename);

/**
 * @brief [DEBUG] show the in / out buffer information of model
 *
 * @param model model ID
 * @return int 0 if succeed
 */
extern int ofi_model_dbg_show_in_out_buf(ofi_model_id model);

/**
 * @brief set specific environments in terms of hardware which can affect
 * to performance for a specific solutio
 *
 * @param name name of a specific solution
 * @return int 0 if succeed
 */
extern int ofi_set_preset(const char *name);

/**
 * @brief unset specific environments in terms of hardware which can affect
 * to performance for a specific solution
 *
 * @param name name of a specific solution
 * @return int 0 if succeed
 */
extern int ofi_unset_preset(const char *name);

#ifdef __cplusplus
}
#endif

#endif  // SOURCE_DSP_NN_INCLUDE_OFI_API_PUBLIC_H_
