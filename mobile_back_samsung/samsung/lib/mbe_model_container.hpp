#ifndef MODEL_CONTAINER_H_
#define MODEL_CONTAINER_H_

/**
 * @file mnodel_container.hpp
 * @brief model container for samsung specific model
 * @date 2021-12-29
 * @author soobong Huh (soobong.huh@samsung.com)
 */

#include "type.h"
#include "mbe_utils.hpp"
#include <vector>
#include <fstream>

namespace mbe {
	enum MODEL_IDX {
		OBJECT_DETECTION = 0,
		IMAGE_CLASSIFICATION,
        IMAGE_CLASSIFICATION_OFFLINE,
		IMAGE_SEGMENTATION,
		MOBILE_BERT,
        SUPER_RESOLUTION,
		MAX_MODEL_IDX
	};

	enum ACCER_IDX {
		NPU = 0,
		GPU,
		DSP,
		NPUDSP,
		MAX_ACCER_IDX
	};

    typedef struct _config_info {
        int n;
        int h;
        int w;
        int c;
        int size;

        int get_size() {
            return n*h*w*c;
        }
    }config_info;

    #define UPDATE_CONFIG(a, b) \
        a = b?b:1;  \

    #define UPDATE_MDL(a, b) \
        a = b?b:a;  \

	class model_container {
		public:
			int m_model_id;
            int m_batch;
			int m_width;
			int m_height;
			int m_channel;

            int gid =0;

			/* mdl config */
			int m_in_cnt;
			int m_out_cnt;
			int m_inbuf_size;
			int m_outbuf_size;

			/* perf config */
			bool b_enable_lazy;
			bool b_extension;

			mlperf_data_t::Type  m_inbuf_type;
			mlperf_data_t::Type  m_outbuf_type;

			ACCER_IDX m_accer_idx;

		private:
			std::vector<mlperf_data_t> m_input_data;
			std::vector<mlperf_data_t> m_output_data;

		public:
			void set_input_data(mlperf_data_t v) {
                v.size /= m_batch;
				m_input_data.push_back(v);
			}

			void set_output_data(mlperf_data_t v) {
                v.size /= m_batch;
				m_output_data.push_back(v);
			}

			int get_buf_size() { return m_batch * m_width * m_height * m_channel; }
			int get_input_size() { return m_input_data.size(); }
			int get_output_size() { return m_output_data.size(); }

			mlperf_data_t get_input_type(int idx) { return m_input_data.at(idx); }
			mlperf_data_t get_output_type(int idx) { return m_output_data.at(idx); }

			void unset_lazy() { b_enable_lazy = false; }
			virtual void init()=0;
            virtual void* get_ofm(std::vector<void *> & ,uint32_t, int32_t) { return nullptr; }
            virtual void* get_ofm_eden(void*, void *, uint32_t, int32_t) { return nullptr; }

			model_container(config_info config, int in, int out, int mdl_id) {
				m_model_id = mdl_id;
                m_batch = config.n;
				m_width = config.w;
				m_height = config.h;
				m_channel = config.c;
				m_in_cnt = in;
				m_out_cnt = out;
                m_inbuf_type=mlperf_data_t::Float32;
				m_outbuf_type=mlperf_data_t::Float32;
				b_enable_lazy = false;
				b_extension = false;
				m_accer_idx = ACCER_IDX::NPU;
			}

			virtual ~model_container() {
				m_input_data.clear();
				m_input_data.shrink_to_fit();

				m_output_data.clear();
				m_output_data.shrink_to_fit();
			}
	};

	class mdl_config_parser {
		public:
            int m_batch;
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

			int m_preset_id;
			bool b_extension;
			bool b_enable_lazy;

        private:
            std::vector<std::pair<config_info, MODEL_IDX>> conf;

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
                    default:
                        return 1;
				}
			}

            void show() {
                MLOGV("mdl_attribute : ");
				MLOGV("m_preset_id : %d", m_preset_id);
                MLOGV("m_batch : %d", m_batch);
				MLOGV("m_width : %d", m_width);
				MLOGV("m_height : %d", m_height);
				MLOGV("m_channel : %d", m_channel);
				MLOGV("m_in_cnt : %d", m_in_cnt);
				MLOGV("m_out_cnt : %d", m_out_cnt);
				MLOGV("m_inbuf_size : %d", m_inbuf_size);
				MLOGV("m_outbuf_size : %d", m_outbuf_size);
				MLOGV("m_inbuf_type : %d", m_inbuf_type);
				MLOGV("m_outbuf_type : %d", m_outbuf_type);
				MLOGV("b_extension : %d", b_extension);
				MLOGV("b_lazy : %d", b_enable_lazy);
				MLOGV("m_accer_idx : %d", m_accer_idx);
            }

            void update(model_container* ptr) {
                IS_VALID(ptr->m_batch, m_batch);
			    IS_VALID(ptr->m_width, m_width);
			    IS_VALID(ptr->m_height, m_height);
			    IS_VALID(ptr->m_channel, m_channel);

				UPDATE_MDL(ptr->m_in_cnt, m_in_cnt);
                UPDATE_MDL(ptr->m_in_cnt, m_in_cnt);
				UPDATE_MDL(ptr->m_out_cnt, m_out_cnt);
				UPDATE_MDL(ptr->m_inbuf_size, m_inbuf_size);
				UPDATE_MDL(ptr->m_outbuf_size, m_outbuf_size);
				UPDATE_MDL(ptr->m_inbuf_type, m_inbuf_type);
				UPDATE_MDL(ptr->m_outbuf_type, m_outbuf_type);
				UPDATE_MDL(ptr->b_extension, b_extension);
				UPDATE_MDL(ptr->b_enable_lazy, b_enable_lazy);
				UPDATE_MDL(ptr->m_accer_idx, m_accer_idx);
			}
            void set_configuration(config_info config, MODEL_IDX mdl_id) {
                conf.push_back({config, mdl_id});
            }
            std::vector<std::pair<config_info, MODEL_IDX>> get_configuration() {
                return conf;
            }
		public:
			mdl_config_parser() {
                m_batch=0;
				m_width=0;
				m_height=0;
				m_channel=0;
				m_in_cnt=0;
				m_out_cnt=0;
				m_inbuf_size=0;
				m_outbuf_size=0;
				m_inbuf_type=mlperf_data_t::Float32;
				m_outbuf_type=mlperf_data_t::Float32;
				m_preset_id=1001;
				b_extension=false;
				b_enable_lazy = false;
				m_accer_idx = ACCER_IDX::NPU;
			}
            ~mdl_config_parser() {
                conf.clear();
                conf.shrink_to_fit();
            }
	};

    mdl_config_parser *mdl_config = nullptr;
	namespace soc2300 {
		class classification : public model_container {
			public :
				void init() override {
					MLOGV("on target model : classification");
					for(int i=0;i<m_in_cnt;i++)
						set_input_data({(m_inbuf_type), m_inbuf_size});

					for(int i=0;i<m_out_cnt;i++)
						set_output_data({m_outbuf_type, (int64_t)(m_outbuf_size / sizeof(float))});
				}
                void* get_ofm(std::vector<void *> &ofm_buf, uint32_t batch_idx, int32_t idx) override {
                    return ofm_buf[0];
                }
			public :
				classification(config_info config):model_container(config, 1, 1, IMAGE_CLASSIFICATION) {}
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
                void* get_ofm(std::vector<void *> &ofm_buf, uint32_t batch_idx, int32_t idx) override {
                    uint8_t *buf = (uint8_t *)(ofm_buf[0]);
                    return (void *)(buf + m_outbuf_size / m_batch * batch_idx);
                }
			public :
				classification_offline(config_info config):model_container(config, 1, 1, IMAGE_CLASSIFICATION) {}
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
                void* get_ofm(std::vector<void *> &ofm_buf, uint32_t batch_idx, int32_t idx) override {
                    return (void *)(ofm_buf[idx]);
                }
			public :
				mobilebert(config_info config):model_container(config, 3, 2, MOBILE_BERT) {}
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
                void* get_ofm(std::vector<void *> &ofm_buf, uint32_t batch_idx, int32_t idx) override {
                    return ofm_buf[0];
                }
			public :
				segmentation(config_info config):model_container(config, 1, 1, IMAGE_SEGMENTATION) {}
		};
        class super_resolution : public model_container {
			public :
				void init() override {
					MLOGV("on target model : super_resolution");
					for(int i=0;i<m_in_cnt;i++)
						set_input_data({(m_inbuf_type), m_inbuf_size});

					for(int i=0;i<m_out_cnt;i++)
						set_output_data({m_outbuf_type, (int64_t)(m_outbuf_size / mdl_config->get_byte(mdl_config->m_outbuf_type))});
				}
                void* get_ofm(std::vector<void *> &ofm_buf, uint32_t batch_idx, int32_t idx) override {
                    return ofm_buf[0];
                }
			public :
				super_resolution(config_info config):model_container(config, 1, 1, SUPER_RESOLUTION) {}
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

                    det_lbl_boxes = std::vector<float>(m_outbuf_size / 7);
                    det_lbl_indices = std::vector<float>(m_outbuf_size / 28);
                    det_lbl_prob = std::vector<float>(m_outbuf_size / 28);
                    det_num = std::vector<float>(1);
				}
                void* get_ofm(std::vector<void *> &ofm_buf, uint32_t batch_idx, int32_t idx) override {
                    void *data = nullptr;
                    float *buf = (float *)(ofm_buf[0]);
                    float det_idx = 0.0;
                    int det_cnt = 0;

                    for (int i = 0, j = 0; j < det_block_cnt; j++) {
                        det_idx = buf[j * det_block_size + 1];
                        if (det_idx > 0) {
                            switch (idx) {
                            case 0:
                                det_lbl_boxes[i++] = buf[j * det_block_size + 4];
                                det_lbl_boxes[i++] = buf[j * det_block_size + 3];
                                det_lbl_boxes[i++] = buf[j * det_block_size + 6];
                                det_lbl_boxes[i++] = buf[j * det_block_size + 5];
                                break;
                            case 1:
                                det_lbl_indices[j] = det_idx - 1;
                            case 2:
                                det_lbl_prob[j] = buf[j * det_block_size + 2];
                            case 3:
                                det_cnt++;
                            default:
                                break;
                            }
                        }
                    }

                    switch (idx) {
                    case 0:
                        data = (void *)(det_lbl_boxes.data());
                        break;
                    case 1:
                        data = (void *)(det_lbl_indices.data());
                        break;
                    case 2:
                        data = (void *)(det_lbl_prob.data());
                        break;
                    case 3:
                        det_num[0] = det_cnt;
                        data = (void *)(det_num.data());
                        memset(buf, 0, sizeof(float) * det_block_size * det_block_cnt);
                        break;
                    default:
                        break;
                    }
                    return data;
                }
            private :
                int det_block_cnt;
				int det_block_size;
                std::vector<float> det_lbl_boxes;
                std::vector<float> det_lbl_indices;
                std::vector<float> det_lbl_prob;
                std::vector<float> det_num;

			public :
				detection(config_info config):model_container(config, 1, 1, OBJECT_DETECTION),
										det_block_cnt(10), det_block_size(7) {}
                ~detection() {
                    det_lbl_boxes.clear();
                    det_lbl_indices.clear();
                    det_lbl_prob.clear();
                    det_num.clear();
                    det_lbl_boxes.shrink_to_fit();
                    det_lbl_indices.shrink_to_fit();
                    det_lbl_prob.shrink_to_fit();
                    det_num.shrink_to_fit();
                }
		};
        void update_configuration() {
            mdl_config->set_configuration({1, 224, 224, 3}, IMAGE_CLASSIFICATION);
            mdl_config->set_configuration({4, 224, 224, 3}, IMAGE_CLASSIFICATION_OFFLINE);
            mdl_config->set_configuration({1, 1, 1, 384}, MOBILE_BERT);
            mdl_config->set_configuration({1, 512, 512, 3}, IMAGE_SEGMENTATION);
            mdl_config->set_configuration({1, 320, 320, 3}, OBJECT_DETECTION);
            mdl_config->set_configuration({1, 540, 960, 3}, SUPER_RESOLUTION);
        }

        model_container* get_mdl_container() {
            update_configuration();
            model_container * cntr = nullptr;
            auto configuration = mdl_config->get_configuration();
            for(auto e:configuration) {
                if(mdl_config->m_inbuf_size == e.first.get_size() * mdl_config->get_byte(mdl_config->m_inbuf_type)) {
                    auto config = e.first;
                    switch(e.second) {
                        case IMAGE_CLASSIFICATION:
                            cntr = new classification(config);
                            break;
                        case IMAGE_CLASSIFICATION_OFFLINE:
                            cntr = new classification_offline(config);
                            break;
                        case OBJECT_DETECTION:
                            cntr = new detection(config);
                            break;
                        case IMAGE_SEGMENTATION:
                            cntr = new segmentation(config);
                            break;
                        case MOBILE_BERT:
                            cntr = new mobilebert(config);
                            break;
                        case SUPER_RESOLUTION:
                            cntr = new super_resolution(config);
                            break;
                        default:
                            cntr = nullptr;
                            break;
                    }
                    mdl_config->update(cntr);
                    return cntr;
                }
            }
            return cntr;
        }
	}

	namespace soc2200 {
		class classification : public model_container {
			public :
				void init() override {
					MLOGV("on target model : classification");
					for(int i=0;i<m_in_cnt;i++)
						set_input_data({(m_inbuf_type), m_inbuf_size});

					for(int i=0;i<m_out_cnt;i++)
						set_output_data({m_outbuf_type, (int64_t)(m_outbuf_size / sizeof(float))});
				}
                void* get_ofm(std::vector<void *> &ofm_buf, uint32_t batch_idx, int32_t idx) override {
                    return ofm_buf[0];
                }
			public :
				classification(config_info config):model_container(config, 1, 1, IMAGE_CLASSIFICATION) {}
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
                void* get_ofm(std::vector<void *> &ofm_buf, uint32_t batch_idx, int32_t idx) override {
                    uint8_t *buf = (uint8_t *)(ofm_buf[0]);
                    return (void *)(buf + m_outbuf_size * batch_idx);
                }
			public :
				classification_offline(config_info config):model_container(config, 1, 1, IMAGE_CLASSIFICATION) {}
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
                void* get_ofm(std::vector<void *> &ofm_buf, uint32_t batch_idx, int32_t idx) override {
                    if (idx == 1) {
                        return ofm_buf[0];
                    }
                    else if (idx == 0) {
                        return ofm_buf[1];
                    }
                    return nullptr;
                }
			public :
				mobilebert(config_info config):model_container(config, 3, 2, MOBILE_BERT) {}
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
                void* get_ofm(std::vector<void *> &ofm_buf, uint32_t batch_idx, int32_t idx) override {
                    return ofm_buf[0];
                }
			public :
				segmentation(config_info config):model_container(config, 1, 1, IMAGE_SEGMENTATION) {}
		};
        class super_resolution : public model_container {
			public :
				void init() override {
					MLOGV("on target model : super_resolution");
					for(int i=0;i<m_in_cnt;i++)
						set_input_data({(m_inbuf_type), m_inbuf_size});

					for(int i=0;i<m_out_cnt;i++)
						set_output_data({m_outbuf_type, (int64_t)(m_outbuf_size / mdl_config->get_byte(mdl_config->m_outbuf_type))});
				}
                void* get_ofm(std::vector<void *> &ofm_buf, uint32_t batch_idx, int32_t idx) override {
                    return ofm_buf[0];
                }
			public :
				super_resolution(config_info config):model_container(config, 1, 1, SUPER_RESOLUTION) {}
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

                    det_lbl_boxes = std::vector<float>(m_outbuf_size / 7);
                    det_lbl_indices = std::vector<float>(m_outbuf_size / 28);
                    det_lbl_prob = std::vector<float>(m_outbuf_size / 28);
                    det_num = std::vector<float>(1);
				}
                void* get_ofm(std::vector<void *> &ofm_buf, uint32_t batch_idx, int32_t idx) override {
                    void *data = nullptr;
                    float *buf = (float *)(ofm_buf[0]);
                    float det_idx = 0.0;
                    int det_cnt = 0;

                    for (int i = 0, j = 0; j < det_block_cnt; j++) {
                        det_idx = buf[j * det_block_size + 1];
                        if (det_idx > 0) {
                            switch (idx) {
                            case 0:
                                det_lbl_boxes[i++] = buf[j * det_block_size + 4];
                                det_lbl_boxes[i++] = buf[j * det_block_size + 3];
                                det_lbl_boxes[i++] = buf[j * det_block_size + 6];
                                det_lbl_boxes[i++] = buf[j * det_block_size + 5];
                                break;
                            case 1:
                                det_lbl_indices[j] = det_idx - 1;
                            case 2:
                                det_lbl_prob[j] = buf[j * det_block_size + 2];
                            case 3:
                                det_cnt++;
                            default:
                                break;
                            }
                        }
                    }

                    switch (idx) {
                        case 0:
                            data = (void *)(det_lbl_boxes.data());
                            break;
                        case 1:
                            data = (void *)(det_lbl_indices.data());
                            break;
                        case 2:
                            data = (void *)(det_lbl_prob.data());
                            break;
                        case 3:
                            det_num[0] = det_cnt;
                            data = (void *)(det_num.data());
                            memset(buf, 0, sizeof(float) * det_block_size * det_block_cnt);
                            break;
                        default:
                            break;
                    }
                    return data;
                }
			private :
                int det_block_cnt;
				int det_block_size;
                std::vector<float> det_lbl_boxes;
                std::vector<float> det_lbl_indices;
                std::vector<float> det_lbl_prob;
                std::vector<float> det_num;
			public :
				detection(config_info config):model_container(config, 1, 1, OBJECT_DETECTION),
										det_block_cnt(11), det_block_size(7) {}
                ~detection() {
                    det_lbl_boxes.clear();
                    det_lbl_indices.clear();
                    det_lbl_prob.clear();
                    det_num.clear();
                    det_lbl_boxes.shrink_to_fit();
                    det_lbl_indices.shrink_to_fit();
                    det_lbl_prob.shrink_to_fit();
                    det_num.shrink_to_fit();
                }
		};
        void update_configuration() {
            mdl_config->set_configuration({1, 224, 224, 3}, IMAGE_CLASSIFICATION);
            mdl_config->set_configuration({1, 1, 1, 384}, MOBILE_BERT);
            mdl_config->set_configuration({1, 512, 512, 3}, IMAGE_SEGMENTATION);
            mdl_config->set_configuration({1, 320, 320, 3}, OBJECT_DETECTION);
        }

		model_container* get_mdl_container(int batch) {
            update_configuration();
            model_container * cntr = nullptr;
            auto configuration = mdl_config->get_configuration();
            for(auto e:configuration) {
                if(mdl_config->m_inbuf_size == e.first.get_size() * mdl_config->get_byte(mdl_config->m_inbuf_type)) {
                    auto config = e.first;
                    switch(e.second) {
                        case IMAGE_CLASSIFICATION:
                        if(batch > 1) {
                            cntr = new classification_offline(config);
                        }
                        else {
                            cntr = new classification(config);
                        }
                        break;
                    case OBJECT_DETECTION:
                        cntr = new detection(config);
                        break;
                    case IMAGE_SEGMENTATION:
                        cntr = new segmentation(config);
                        break;
                    case MOBILE_BERT:
                        cntr = new mobilebert(config);
                        break;
                    default:
                        cntr = nullptr;
                    }
                    mdl_config->update(cntr);
                    return cntr;
                }
            }
            return cntr;
        }
	}

	namespace soc1200 {
        class classification : public model_container {
			public :
				void init() override {
					MLOGV("on target model : classification");
					for(int i=0;i<m_in_cnt;i++)
						set_input_data({(m_inbuf_type), m_inbuf_size});

					for(int i=0;i<m_out_cnt;i++)
						set_output_data({m_outbuf_type, (int64_t)(m_outbuf_size / sizeof(float))});
				}
                void* get_ofm_eden(void *ofm_buf, void *batch_buf, uint32_t batch_idx, int32_t idx) override {
                    return ofm_buf;
                }
			public :
				classification(config_info config):model_container(config, 1, 1, IMAGE_CLASSIFICATION) {}
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
                void* get_ofm_eden(void *ofm_buf, void *batch_buf, uint32_t batch_idx, int32_t idx) override {
                    uint8_t *buf = (uint8_t *)(batch_buf);
                    return (void *)(buf + m_outbuf_size * batch_idx);
                }
			public :
				classification_offline(config_info config):model_container(config, 1, 1, IMAGE_CLASSIFICATION) {}
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
                void* get_ofm_eden(void *ofm_buf, void *batch_buf, uint32_t batch_idx, int32_t idx) override {
                    return ofm_buf;
                }
			public :
				segmentation(config_info config):model_container(config, 1, 1, IMAGE_SEGMENTATION) {}
		};
        class super_resolution : public model_container {
			public :
				void init() override {
					MLOGV("on target model : super_resolution");
					for(int i=0;i<m_in_cnt;i++)
						set_input_data({(m_inbuf_type), m_inbuf_size});

					for(int i=0;i<m_out_cnt;i++)
						set_output_data({m_outbuf_type, (int64_t)(m_outbuf_size / mdl_config->get_byte(mdl_config->m_outbuf_type))});
				}
                void* get_ofm_eden(void *ofm_buf, void *batch_buf, uint32_t batch_idx, int32_t idx) override {
                    return ofm_buf;
               }
			public :
				super_resolution(config_info config):model_container(config, 1, 1, SUPER_RESOLUTION) {}
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

                    det_lbl_boxes = std::vector<float>(m_outbuf_size / 7);
                    det_lbl_indices = std::vector<float>(m_outbuf_size / 28);
                    det_lbl_prob = std::vector<float>(m_outbuf_size / 28);
                    det_num = std::vector<float>(1);
				}
                void* get_ofm_eden(void *ofm_buf, void *batch_buf, uint32_t batch_idx, int32_t idx) override {
                    void *data = nullptr;
                    float *buf = (float *)(ofm_buf);
                    float det_idx = 0.0;
                    int det_cnt = 0;

                    for (int i = 0, j = 0; j < det_block_cnt; j++) {
                        det_idx = buf[j * det_block_size + 1];
                        if (det_idx > 0) {
                            switch (idx) {
                            case 0:
                                det_lbl_boxes[i++] = buf[j * det_block_size + 4];
                                det_lbl_boxes[i++] = buf[j * det_block_size + 3];
                                det_lbl_boxes[i++] = buf[j * det_block_size + 6];
                                det_lbl_boxes[i++] = buf[j * det_block_size + 5];
                                break;
                            case 1:
                                det_lbl_indices[j] = det_idx - 1;
                            case 2:
                                det_lbl_prob[j] = buf[j * det_block_size + 2];
                            case 3:
                                det_cnt++;
                            default:
                                break;
                            }
                        }
                    }

                    switch (idx) {
                    case 0:
                        data = (void *)(det_lbl_boxes.data());
                        break;
                    case 1:
                        data = (void *)(det_lbl_indices.data());
                        break;
                    case 2:
                        data = (void *)(det_lbl_prob.data());
                        break;
                    case 3:
                        det_num[0] = det_cnt;
                        data = (void *)(det_num.data());
                        memset(buf, 0, sizeof(float) * det_block_size * det_block_cnt);
                        break;
                    default:
                        break;
                    }
                    return data;
                }
			private :
                int det_block_cnt;
				int det_block_size;
                std::vector<float> det_lbl_boxes;
                std::vector<float> det_lbl_indices;
                std::vector<float> det_lbl_prob;
                std::vector<float> det_num;
			public :
				detection(config_info config):model_container(config, 1, 1, OBJECT_DETECTION),
										det_block_cnt(11), det_block_size(7) {}
                ~detection() {
                    det_lbl_boxes.clear();
                    det_lbl_indices.clear();
                    det_lbl_prob.clear();
                    det_num.clear();
                    det_lbl_boxes.shrink_to_fit();
                    det_lbl_indices.shrink_to_fit();
                    det_lbl_prob.shrink_to_fit();
                    det_num.shrink_to_fit();
                }
		};
        void update_configuration() {
            mdl_config->set_configuration({1, 224, 224, 3}, IMAGE_CLASSIFICATION);
            mdl_config->set_configuration({1, 512, 512, 3}, IMAGE_SEGMENTATION);
            mdl_config->set_configuration({1, 320, 320, 3}, OBJECT_DETECTION);
        }

        model_container* get_mdl_container(int batch) {
            update_configuration();
            model_container * cntr = nullptr;
            auto configuration = mdl_config->get_configuration();
            for(auto e:configuration) {
                if(mdl_config->m_inbuf_size == e.first.get_size() * mdl_config->get_byte(mdl_config->m_inbuf_type)) {
                    auto config = e.first;
                    switch(e.second) {
                        case IMAGE_CLASSIFICATION:
                            if(batch > 1) {
                                cntr = new classification_offline(config);
                            }
                            else {
                                cntr = new classification(config);
                            }
                            break;
                        case OBJECT_DETECTION:
                            cntr = new detection(config);
                            break;
                        case IMAGE_SEGMENTATION:
                            cntr = new segmentation(config);
                            break;
                        case MOBILE_BERT:
                        default:
                            cntr = nullptr;
                    }
                    mdl_config->update(cntr);
                    return cntr;
                }
            }
            return cntr;
        }
	}

	namespace soc2100 {
        class classification : public model_container {
			public :
				void init() override {
					MLOGV("on target model : classification");
					for(int i=0;i<m_in_cnt;i++)
						set_input_data({(m_inbuf_type), m_inbuf_size});

					for(int i=0;i<m_out_cnt;i++)
						set_output_data({m_outbuf_type, (int64_t)(m_outbuf_size / sizeof(float))});
				}
                void* get_ofm_eden(void *ofm_buf, void *batch_buf, uint32_t batch_idx, int32_t idx) override {
                    return ofm_buf;
                }
			public :
				classification(config_info config):model_container(config, 1, 1, IMAGE_CLASSIFICATION) {}
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
                void* get_ofm_eden(void *ofm_buf, void *batch_buf, uint32_t batch_idx, int32_t idx) override {
                    uint8_t *buf =(uint8_t*)(batch_buf);
                    return (void*)(buf + m_outbuf_size * batch_idx);
                }
			public :
				classification_offline(config_info config):model_container(config, 1, 1, IMAGE_CLASSIFICATION) {}
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
                void* get_ofm_eden(void *ofm_buf, void *batch_buf, uint32_t batch_idx, int32_t idx) override {
                    return ofm_buf;
                }
			public :
				segmentation(config_info config):model_container(config, 1, 1, IMAGE_SEGMENTATION) {}
		};
        class super_resolution : public model_container {
			public :
				void init() override {
					MLOGV("on target model : super_resolution");
					for(int i=0;i<m_in_cnt;i++)
						set_input_data({(m_inbuf_type), m_inbuf_size});

					for(int i=0;i<m_out_cnt;i++)
						set_output_data({m_outbuf_type, (int64_t)(m_outbuf_size / mdl_config->get_byte(mdl_config->m_outbuf_type))});
				}
                void* get_ofm_eden(void *ofm_buf, void *batch_buf, uint32_t batch_idx, int32_t idx) override {
                    return ofm_buf;
               }
			public :
				super_resolution(config_info config):model_container(config, 1, 1, SUPER_RESOLUTION) {}
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
                void* get_ofm_eden(void *ofm_buf, void *batch_buf, uint32_t batch_idx, int32_t idx) override {
                    /* DO NOT ANYTHING */
                    MLOGD("for limitation of Eden, can not use get_ofm");
                    return nullptr;
                }
			public :
				mobilebert(config_info config):model_container(config, 3, 2, MOBILE_BERT) {}
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

                    det_lbl_boxes = std::vector<float>(m_outbuf_size / 7);
                    det_lbl_indices = std::vector<float>(m_outbuf_size / 28);
                    det_lbl_prob = std::vector<float>(m_outbuf_size / 28);
                    det_num = std::vector<float>(1);
				}
                void* get_ofm_eden(void *ofm_buf, void *batch_buf, uint32_t batch_idx, int32_t idx) override {
                    void *data = nullptr;
                    float *buf = (float *)(ofm_buf);
                    float det_idx = 0.0;
                    int det_cnt = 0;

                    for (int i=0, j=0; j<det_block_cnt; j++) {
                        det_idx = buf[j * det_block_size + 1];
                        if(det_idx > 0) {
                            switch (idx) {
                                case 0:
                                    det_lbl_boxes[i++] = buf[j * det_block_size + 4];
                                    det_lbl_boxes[i++] = buf[j * det_block_size + 3];
                                    det_lbl_boxes[i++] = buf[j * det_block_size + 6];
                                    det_lbl_boxes[i++] = buf[j * det_block_size + 5];
                                    break;
                                case 1:
                                    det_lbl_indices[j] = det_idx-1;
                                case 2:
                                    det_lbl_prob[j] = buf[j * det_block_size + 2];
                                case 3:
                                    det_cnt++;
                                default:
                                    break;
                            }
                        }
                    }

                    switch (idx) {
                        case 0:
                            data = (void *)(det_lbl_boxes.data());
                            break;
                        case 1:
                            data = (void *)(det_lbl_indices.data());
                            break;
                        case 2:
                            data = (void *)(det_lbl_prob.data());
                            break;
                        case 3:
                            det_num[0] = det_cnt;
                            data = (void *)(det_num.data());
                            memset(buf, 0, sizeof(float) * det_block_size * det_block_cnt);
                            break;
                        default:
                            break;
                    }
                    return data;
                }
			private :
                int det_block_cnt;
				int det_block_size;
                std::vector<float> det_lbl_boxes;
                std::vector<float> det_lbl_indices;
                std::vector<float> det_lbl_prob;
                std::vector<float> det_num;
			public :
				detection(config_info config):model_container(config, 1, 4, OBJECT_DETECTION),
										det_block_cnt(11), det_block_size(7) {}
                ~detection() {
                    det_lbl_boxes.clear();
                    det_lbl_indices.clear();
                    det_lbl_prob.clear();
                    det_num.clear();
                    det_lbl_boxes.shrink_to_fit();
                    det_lbl_indices.shrink_to_fit();
                    det_lbl_prob.shrink_to_fit();
                    det_num.shrink_to_fit();
                }
		};
        void update_configuration() {
            mdl_config->set_configuration({1, 224, 224, 3}, IMAGE_CLASSIFICATION);
            mdl_config->set_configuration({1, 1, 1, 384}, MOBILE_BERT);
            mdl_config->set_configuration({1, 512, 512, 3}, IMAGE_SEGMENTATION);
            mdl_config->set_configuration({1, 320, 320, 3}, OBJECT_DETECTION);
        }

        model_container* get_mdl_container(int batch) {
            update_configuration();
            model_container * cntr = nullptr;
            auto configuration = mdl_config->get_configuration();
            for(auto e:configuration) {
                if(mdl_config->m_inbuf_size == e.first.get_size() * mdl_config->get_byte(mdl_config->m_inbuf_type)) {
                    auto config = e.first;
                    switch(e.second) {
                        case IMAGE_CLASSIFICATION:
                            if(batch > 1) {
                                cntr = new classification_offline(config);
                            }
                            else {
                                cntr = new classification(config);
                            }
                            break;
                        case OBJECT_DETECTION:
                            cntr = new detection(config);
                            break;
                        case IMAGE_SEGMENTATION:
                            cntr = new segmentation(config);
                            break;
                        case MOBILE_BERT:
                            cntr = new mobilebert(config);
                            break;
                        default:
                            cntr = nullptr;
                    }
                    mdl_config->update(cntr);
                    return cntr;
                }
            }
            return cntr;
        }
	}
}

#endif
