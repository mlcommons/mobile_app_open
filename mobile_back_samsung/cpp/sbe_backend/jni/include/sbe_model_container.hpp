#ifndef MODEL_CONTAINER_H_
#define MODEL_CONTAINER_H_

/**
 * @file mnodel_container.hpp
 * @brief model container for samsung specific model
 * @date 2021-12-29
 * @author soobong Huh (soobong.huh@samsung.com)
 */

#include "type.h"
#include "sbe_utils.hpp"

namespace sbe {
	enum MODEL_IDX {
		OBJECT_DETECTION = 0,
		IMAGE_CLASSIFICATION,
		IMAGE_SEGMENTATION,
		MOBILE_BERT,
		MAX_MODEL_IDX
	};

	enum ACCER_IDX {
		NPU = 0,
		GPU,
		DSP,
		NPUDSP,
		MAX_ACCER_IDX	
	};
	
	class model_container {
		public:
			int m_model_id;
			int m_width;
			int m_height;
			int m_channel;

			/* mdl config */
			int m_in_cnt;
			int m_out_cnt;
			int m_inbuf_size;
			int m_outbuf_size;

			/* perf config */
			int m_freeze;
			bool b_enable_lazy;
			bool b_enable_fpc;

			mlperf_data_t::Type  m_inbuf_type;
			mlperf_data_t::Type  m_outbuf_type;

			ACCER_IDX m_accer_idx;

		private:
			std::vector<mlperf_data_t> m_input_data;
			std::vector<mlperf_data_t> m_output_data;

		public:
			void set_input_data(mlperf_data_t v)
			{
				m_input_data.push_back(v);
			}

			void set_output_data(mlperf_data_t v)
			{
				m_output_data.push_back(v);
			}

			int get_buf_size()
			{
				return m_width * m_height * m_channel;
			}

			int get_input_size() { return m_input_data.size(); }
			int get_output_size() { return m_output_data.size(); }

			mlperf_data_t get_input_type(int idx) { return m_input_data.at(idx); }
			mlperf_data_t get_output_type(int idx) { return m_output_data.at(idx); }

			void unset_lazy() { b_enable_lazy = false; }
			virtual void init()=0;

			void deinit() {
				m_input_data.clear();
				m_input_data.shrink_to_fit();

				m_output_data.clear();
				m_output_data.shrink_to_fit();
			}

			model_container(int w, int h, int c, int in, int out, int mdl_id) {
				m_model_id = mdl_id;
				m_width = w;
				m_height = h;
				m_channel = c;
				m_in_cnt = in;
				m_out_cnt = out;
				m_freeze = 0;
				b_enable_lazy = false;
				b_enable_fpc = false;
				m_accer_idx = ACCER_IDX::NPU;
			}

			~model_container() {
				deinit();
			}
	};

	class mdl_attribute_template {
		public:
			/* model attribute */
			int m_width;
			int m_height;
			int m_channel;

			int m_in_cnt;
			int m_out_cnt;
			int m_inbuf_size;
			int m_outbuf_size;

			mlperf_data_t::Type m_inbuf_type;
			mlperf_data_t::Type m_outbuf_type;

			ACCER_IDX m_accer_idx;

			/* others */
			int m_preset_id;
			bool b_enable_fpc;
			int m_freeze;
			bool b_enable_lazy;

		public:
			int get_byte(mlperf_data_t::Type type) {
				switch (type) {
					case mlperf_data_t::Uint8:
						return 1;
					case mlperf_data_t::Int8:
						return 1;
					case mlperf_data_t::Float16:
						return 2;
					case mlperf_data_t::Int32:
					case mlperf_data_t::Float32:
						return 4;
					case mlperf_data_t::Int64:
						return 8;
				}
			}

			void update(model_container* ptr)
			{
				ptr->m_in_cnt = m_in_cnt;
				ptr->m_out_cnt = m_out_cnt;

				ptr->m_inbuf_size = m_inbuf_size;
				ptr->m_outbuf_size = m_outbuf_size;
				ptr->m_inbuf_type = m_inbuf_type;
				ptr->m_outbuf_type = m_outbuf_type;
				ptr->b_enable_fpc = b_enable_fpc;
				ptr->m_freeze = m_freeze;
				ptr->b_enable_lazy = b_enable_lazy;
				ptr->m_accer_idx = m_accer_idx;
			}

			void show() {
				MLOGV("mdl_attribute_template : ");
				MLOGV("m_preset_id : %d", m_preset_id);
				MLOGV("m_width : %d", m_width);
				MLOGV("m_height : %d", m_height);
				MLOGV("m_channel : %d", m_channel);
				MLOGV("m_in_cnt : %d", m_in_cnt);
				MLOGV("m_out_cnt : %d", m_out_cnt);
				MLOGV("m_inbuf_size : %d", m_inbuf_size);
				MLOGV("m_outbuf_size : %d", m_outbuf_size);
				MLOGV("m_inbuf_type : %d", m_inbuf_type);
				MLOGV("m_outbuf_type : %d", m_outbuf_type);
				MLOGV("b_enable_fpc : %d", b_enable_fpc);
				MLOGV("m_freeze : %d", m_freeze);
				MLOGV("b_lazy : %d", b_enable_lazy);
				MLOGV("m_accer_idx : %d", m_accer_idx);
			}

		public:
			mdl_attribute_template() {
				m_width=0;
				m_height=0;
				m_channel=0;
				m_in_cnt=1;
				m_out_cnt=1;
				m_inbuf_size=0;
				m_outbuf_size=0;
				m_inbuf_type=mlperf_data_t::Uint8;
				m_outbuf_type=mlperf_data_t::Uint8;
				m_preset_id=1001;
				b_enable_fpc=false;
				m_freeze=0;
				b_enable_lazy = false;
				m_accer_idx = ACCER_IDX::NPU;
			}
	};
	mdl_attribute_template mdl_attr;
	namespace sbeID2200 {
		class classification : public model_container {
			public :
				void init() override {
					MLOGV("on target model : classification");
					for(int i=0;i<m_in_cnt;i++)
						set_input_data({(m_inbuf_type), m_inbuf_size});

					for(int i=0;i<m_out_cnt;i++)
						set_output_data({m_outbuf_type, (int64_t)(m_outbuf_size / sizeof(float))});
				}
			public :
				classification():model_container(224, 224, 3, 1, 1, IMAGE_CLASSIFICATION) {}
		};

		class classification_offline : public model_container {
			public :
				void init() override {
					MLOGV("on target model : classification offline");
					for(int i=0;i<m_in_cnt;i++)
						set_input_data({m_inbuf_type, m_inbuf_size});

					for(int i=0;i<m_out_cnt;i++)
						set_output_data({m_outbuf_type, (int64_t)(m_outbuf_size / sizeof(float))});
				}				
			public :
				classification_offline():model_container(224, 224, 3, 1, 1, IMAGE_CLASSIFICATION) {}
		};

		class mobilebert : public model_container {
			public :				
				void init() override {
					MLOGV("on target model : mobile_bert");
					for(int i=0;i<m_in_cnt;i++)
						set_input_data({m_inbuf_type, 384});

					for(int i=0;i<m_out_cnt;i++)
						set_output_data({m_outbuf_type, 384});
				}
			public :
				mobilebert():model_container(384, 384, 3, 3, 2, MOBILE_BERT) {}
		};

		class segmentation : public model_container {
			public :				
				void init() override {
					MLOGV("on target model : segemenatation");
					for(int i=0;i<m_in_cnt;i++)
						set_input_data({m_inbuf_type, m_inbuf_size});

					for(int i=0;i<m_out_cnt;i++)
						set_output_data({m_outbuf_type, m_outbuf_size});
				}
			public :
				segmentation():model_container(512, 512, 3, 1, 1, IMAGE_SEGMENTATION) {}
		};

		class detection : public model_container {
			public :				
				void init() override {
					MLOGV("on target model : detection");
					for(int i=0;i<m_in_cnt;i++)
						set_input_data({m_inbuf_type, m_inbuf_size});
					
					set_output_data({m_outbuf_type, m_outbuf_size/7});
					set_output_data({m_outbuf_type, m_outbuf_size/28});
					set_output_data({m_outbuf_type, m_outbuf_size/28});
					set_output_data({m_outbuf_type, 1});
				}
			public :
				int det_block_cnt;
				int det_block_size;
			public :
				detection():model_container(320, 320, 3, 1, 1, OBJECT_DETECTION),
										det_block_cnt(11), det_block_size(7) {}
		};

		detection obj_od;
		classification obj_ic;
		classification_offline obj_ic_offline;
		segmentation obj_is;
		mobilebert obj_bert;
	}

	namespace sbeID1200 {
		class detection : public model_container {
			public :
				void init() override {
					MLOGV("on target model : detection");
					for(int i=0;i<m_in_cnt;i++)
						set_input_data({m_inbuf_type, m_inbuf_size});
					
					set_output_data({m_outbuf_type, m_outbuf_size/7});
					set_output_data({m_outbuf_type, m_outbuf_size/28});
					set_output_data({m_outbuf_type, m_outbuf_size/28});
					set_output_data({m_outbuf_type, 1});
				}
			public :
				int det_block_cnt;
				int det_block_size;
			public :
				detection():model_container(320, 320, 3, 1, 1, OBJECT_DETECTION),
										det_block_cnt(11), det_block_size(7) {}
		};

		class classification : public model_container {
			public :
				void init() override {
					MLOGV("on target model : classification");
					for(int i=0;i<m_in_cnt;i++)
						set_input_data({(m_inbuf_type), m_inbuf_size});

					for(int i=0;i<m_out_cnt;i++)
						set_output_data({m_outbuf_type, (int64_t)(m_outbuf_size / sizeof(float))});
				}
			public :
				classification():model_container(224, 224, 3, 1, 1, IMAGE_CLASSIFICATION) {}
		};

		class classification_offline : public model_container {
			public :
				void init() override {
					MLOGV("on target model : classification offline");
					for(int i=0;i<m_in_cnt;i++)
						set_input_data({m_inbuf_type, m_inbuf_size});

					for(int i=0;i<m_out_cnt;i++)
						set_output_data({m_outbuf_type, (int64_t)(m_outbuf_size / sizeof(float))});
				}				
			public :
				classification_offline():model_container(224, 224, 3, 1, 1, IMAGE_CLASSIFICATION) {}
		};

		class segmentation : public model_container {
			public :
				void init() override {
					MLOGV("on target model : segemenatation");
					for(int i=0;i<m_in_cnt;i++)
						set_input_data({m_inbuf_type, m_inbuf_size});

					for(int i=0;i<m_out_cnt;i++)
						set_output_data({m_outbuf_type, m_outbuf_size});
				}
			public :
				segmentation():model_container(512, 512, 3, 1, 1, IMAGE_SEGMENTATION) {}
		};

		detection obj_od;
		classification obj_ic;
		classification_offline obj_ic_offline;
		segmentation obj_is;
	}

	namespace sbeID2100 {
		class detection : public model_container {
			public :
				void init() override {
					MLOGV("on target model : detection");
					for(int i=0;i<m_in_cnt;i++)
						set_input_data({m_inbuf_type, m_inbuf_size});
					
					set_output_data({m_outbuf_type, m_outbuf_size/7});
					set_output_data({m_outbuf_type, m_outbuf_size/28});
					set_output_data({m_outbuf_type, m_outbuf_size/28});
					set_output_data({m_outbuf_type, 1});
				}
			public :
				int det_block_cnt;
				int det_block_size;
			public :
				detection():model_container(320, 320, 3, 1, 4, OBJECT_DETECTION),
										det_block_cnt(11), det_block_size(7) {}
		};

		class classification : public model_container {
			public :
				void init() override {
					MLOGV("on target model : classification");
					for(int i=0;i<m_in_cnt;i++)
						set_input_data({(m_inbuf_type), m_inbuf_size});

					for(int i=0;i<m_out_cnt;i++)
						set_output_data({m_outbuf_type, (int64_t)(m_outbuf_size / sizeof(float))});
				}
			public :
				classification():model_container(224, 224, 3, 1, 1, IMAGE_CLASSIFICATION) {}
		};

		class classification_offline : public model_container {
			public :
				void init() override {
					MLOGV("on target model : classification offline");
					for(int i=0;i<m_in_cnt;i++)
						set_input_data({m_inbuf_type, m_inbuf_size});

					for(int i=0;i<m_out_cnt;i++)
						set_output_data({m_outbuf_type, (int64_t)(m_outbuf_size / sizeof(float))});
				}				
			public :
				classification_offline():model_container(224, 224, 3, 1, 1, IMAGE_CLASSIFICATION) {}
		};

		class segmentation : public model_container {
			public :
				void init() override {
					MLOGV("on target model : segemenatation");
					for(int i=0;i<m_in_cnt;i++)
						set_input_data({m_inbuf_type, m_inbuf_size});

					for(int i=0;i<m_out_cnt;i++)
						set_output_data({m_outbuf_type, m_outbuf_size});
				}
			public :
				segmentation():model_container(512, 512, 3, 1, 1, IMAGE_SEGMENTATION) {}
		};

		class mobilebert : public model_container {
			public :				
				void init() override {
					MLOGV("on target model : mobile_bert");
					for(int i=0;i<m_in_cnt;i++)
						set_input_data({m_inbuf_type, 384});

					for(int i=0;i<m_out_cnt;i++)
						set_output_data({m_outbuf_type, 384});
				}
			public :
				mobilebert():model_container(384, 384, 3, 3, 2, MOBILE_BERT) {}
		};
		detection obj_od;
		classification obj_ic;
		classification_offline obj_ic_offline;
		segmentation obj_is;
		mobilebert obj_bert;
	}
}

#endif
