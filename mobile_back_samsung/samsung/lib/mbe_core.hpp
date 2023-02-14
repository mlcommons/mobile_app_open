/* Copyright 2020-2023 Samsung Electronics Co. LTD  All Rights Reserved.

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
#ifndef MBE_CORE_H_
#define MBE_CORE_H_

/**
 * @file mbe_core.hpp
 * @brief main core object for samsung backend
 * @date 2022-01-03
 * @author soobong Huh (soobong.huh@samsung.com)
 */

#include "backend_c.h"
#include "type.h"
#include "mbe_loader.hpp"
#include "mbe_helper.hpp"

namespace mbe {
class mbe_core_holder {
	public:
		~mbe_core_holder() {
			MLOGD("Destruct a mbe_core_holder object");
		}

		using backend_create_t = std::add_pointer<bool(const char *, mlperf_backend_configuration_t *, const char *)>::type;
		using backend_get_input_count_t = std::add_pointer<int(void)>::type;
		using backend_get_input_type_t = std::add_pointer<mlperf_data_t(int32_t)>::type;
		using backend_set_input_t = std::add_pointer<mlperf_status_t(int32_t, int32_t, void*)>::type;
		using backend_get_output_count_t = std::add_pointer<int32_t(void)>::type;
		using backend_get_output_type_t = std::add_pointer<mlperf_data_t(int32_t)>::type;
		using backend_get_output_t = std::add_pointer<mlperf_status_t(int32_t, int32_t, void**)>::type;
		using backend_issue_query_t = std::add_pointer<mlperf_status_t(void)>::type;
		using backend_convert_inputs_t = std::add_pointer<void(int, int, int, uint8_t*)>::type;
		using backend_delete_t = std::add_pointer<void(void)>::type;
		using backend_get_buffer_t = std::add_pointer<void*(size_t)>::type;
		using backend_release_buffer_t = std::add_pointer<void(void*)>::type;

		backend_create_t create_fp;
		backend_delete_t delete_fp;
		backend_issue_query_t issue_query_fp;
		backend_get_input_count_t get_input_count_fp;
		backend_get_input_type_t get_input_type_fp;
		backend_set_input_t set_input_fp;
		backend_get_output_count_t get_output_count_fp;
		backend_get_output_type_t get_output_type_fp;
		backend_get_output_t get_output_fp;
		backend_convert_inputs_t convert_inputs_fp;
		backend_get_buffer_t get_buffer_fp;
		backend_release_buffer_t release_buffer_fp;

		bool load_core_library(const char* lib_path)
		{
			char *error = nullptr;
			/* load mbe core */
			mbe::core_id = core_ctrl::get_core_id();
			MLOGD("acquired core id[%d]", mbe::core_id);

			if(core_id == CORE_INVALID) {
                MLOGE("fail to get mbe core libarary. core_id[%d]", mbe::core_id);
                return false;
			}

			std::string core_lib_path = std::string(lib_path) + "/" + mbe_core_libs[mbe::core_id];
			void *handle = dlopen(core_lib_path.c_str(), RTLD_NOW);
			MLOGD("native library path[%s], handle[%p]", core_lib_path.c_str(), handle);
			if(!handle) {
				MLOGE("fail to get handle of shared library");
				if ((error = dlerror()) != NULL) {
					MLOGE("dlopen error with %s\n", error);
				}
				return false;
			}

			create_fp = link_symbol(handle, backend_create);
			delete_fp = link_symbol(handle, backend_delete);
			issue_query_fp = link_symbol(handle, backend_issue_query);
			get_input_count_fp = link_symbol(handle, backend_get_input_count);
			get_input_type_fp = link_symbol(handle, backend_get_input_type);
			set_input_fp = link_symbol(handle, backend_set_input);
			get_output_count_fp = link_symbol(handle, backend_get_output_count);
			get_output_type_fp = link_symbol(handle, backend_get_output_type);
			get_output_fp = link_symbol(handle, backend_get_output);
			convert_inputs_fp = link_symbol(handle, backend_convert_inputs);
			get_buffer_fp = link_symbol(handle, backend_get_buffer);
			release_buffer_fp = link_symbol(handle, backend_release_buffer);
			return true;
		}

		void unload_core_library()
		{
			/* TODO: to something */
			MLOGD("unload native library");
		}

		mbe_core_holder():create_fp(nullptr), delete_fp(nullptr),
			issue_query_fp(nullptr),  get_input_count_fp(nullptr),
			get_input_type_fp(nullptr),  set_input_fp(nullptr),
			get_output_count_fp(nullptr),  get_output_type_fp(nullptr),
			get_output_fp(nullptr), convert_inputs_fp(nullptr),
			get_buffer_fp(nullptr),  release_buffer_fp(nullptr)
		{
			MLOGD("Construct a mbe_core_holder object");
		}
};
}	// namespace mbe
#endif
