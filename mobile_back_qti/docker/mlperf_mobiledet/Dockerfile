FROM tensorflow/tensorflow:1.15.0-py3
LABEL maintainer="quic_mcharleb@quicinc.com"

RUN pip3 install tf-slim scipy Pillow opencv-python setuptools matplotlib tensorflow_hub

# Create /opt/models for github.com/tensorflow/models
RUN curl -o /opt/tensorflow_models.tar.gz -L \
    https://github.com/tensorflow/models/archive/8a064338cf811494408089e0eb9a6e274784a6f9.tar.gz \
    && tar xzf /opt/tensorflow_models.tar.gz -C /opt \
    && rm /opt/tensorflow_models.tar.gz \
    && mv /opt/models-8a064338cf811494408089e0eb9a6e274784a6f9 /opt/models

RUN apt-get install -y --no-install-recommends protobuf-compiler 

# Compile tf_models/research protobufs
RUN ls /opt && cd /opt/models/research && protoc object_detection/protos/*.proto --python_out=.

RUN chmod -R 777 /opt/ssdlite_mobiledet
