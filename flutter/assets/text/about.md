# MLPerf Mobile App

The MLPerf Mobile App supports the following runtime backends to run AI tasks on different mobile devices.

## LiteRT Backend

LiteRT (formerly TensorFlow Lite) is Google’s cross-platform framework for deploying machine learning (ML) models on mobile and embedded systems. The runtime uses a set of custom operators that are optimized for efficiency, offering lower latency and smaller binary size compared to the full TensorFlow runtime. It can perform inference on CPUs using optimized kernels that take advantage of ARM’s Neon vector instructions on any platform. The runtime can also execute on a variety of accelerator delegates to take advantage of specialized APIs and hardware. For example, it can target GPUs for iOS and Android, Core ML for newer iOS devices, and the Android NNAPI delegate which can target GPUs, DSPs, NPUs, and custom back-ends.
When invoked on the CPU it will use 4 threads.

## Google Pixel

Google Pixel smartphones use the Android NNAPI run time. The Android Neural Networks API (NNAPI) is an Android C API designed for running computationally intensive operations for machine learning on Android devices. NNAPI is designed to provide a base layer of functionality for higher-level machine learning frameworks, such as TensorFlow Lite and Caffe2, that build and train neural networks.

The MLPerf app uses NNAPI on the following families:

* Pixel 10/9/8/7/6 and Pixel 10/9/8/7/6 Pro (Tensor G5/G4/G3/G2/G1 SoC)

<!-- markdown-link-check-disable-next-line -->
For More Info: [https://developer.android.com/ndk/guides/neuralnetworks](https://developer.android.com/ndk/guides/neuralnetworks)

## MediaTek Neuron Delegate

The MediaTek Neuron Delegate is a TensorFlow Lite delegate designed to work with the MediaTek Neuron runtime, providing access to more underlying capabilities and features for user applications. The Neuron Delegate is modeled after the TensorFlow Lite NNAPI delegate, but it communicates directly with MediaTek's Neuron runtime to deliver better performance. MediaTek processors include support for INT8, INT16, and FP16 calculations, and can work on dedicated AI tasks or hybrid tasks such as AI-ISP, AI-GPU, and AI-Video in conjunction with other dedicated compute resources.

The MLPerf app uses the Neuron delegate on the following families:

* Dimensity 9000 series (9000/9000+/9200/9200+/9300/9300+/9400)
* Dimensity 8000 series (8000/8020/8050/8100/8200/8300)

Link to MediaTek Neuron Delegate: [https://github.com/MediaTek-NeuroPilot/tflite-neuron-delegate](https://github.com/MediaTek-NeuroPilot/tflite-neuron-delegate)

Link to MediaTek Dimensity 9000/8000/7000 Series: [https://www.mediatek.com/products/smartphones/dimensity-5g](https://www.mediatek.com/products/smartphones/dimensity-5g)

## Qualcomm

The Qualcomm Neural Processing SDK (SNPE) is a software accelerated runtime and toolkit for optimizing and executing neural networks on devices with Snapdragon processors. It can take advantage of many different execution resources in Snapdragon processors including the Qualcomm Kryo™ CPU, Qualcomm Adreno™ GPU, Qualcomm Hexagon™ Processor. The Qualcomm backend provides the flexibility to users in choosing the best approach for their applications.

The MLPerf Application uses the SNPE Hexagon Processor runtimes on the following families:

* Snapdragon 8 Elite Gen 5
* Snapdragon 8 Elite
* Snapdragon 8 Gen 5
* Snapdragon 8 Gen 3
* Snapdragon 8s Gen 4
* Snapdragon 8s Gen 3
* Snapdragon 8 Gen 2
* Snapdragon 7 Gen 4
* Snapdragon 7 Gen 3
* Snapdragon 7s Gen 3
* Snapdragon 6 Gen 4
* Snapdragon 4 Gen 2
* Default fallback for all other Snapdragon mobile platforms

The Qualcomm backend utilizes the Snapdragon platform's AI hardware to accelerate neural network execution.

<!-- markdown-link-check-disable-next-line -->
Link to Qualcomm Neural Processing SDK: [https://developer.qualcomm.com/software/qualcomm-neural-processing-sdk](https://developer.qualcomm.com/software/qualcomm-neural-processing-sdk)

Link to Snapdragon Platform: [https://www.qualcomm.com/snapdragon](https://www.qualcomm.com/snapdragon)

## Samsung Exynos Neural Network (ENN)

Exynos Neural Network (ENN) is Samsung's Exynos runtime, toolkit, and set of libraries for optimizing and executing pre-trained neural networks on Samsung devices. The ENN SDK is designed to take maximum advantage of the underlying hardware components for AI tasks including CPU, GPU, DSP, and NPU.

Official website URL: [https://developer.samsung.com/neural/overview.html](https://developer.samsung.com/neural/overview.html)

The MLPerf Mobile Benchmarking App uses the Exynos Neural Network SDK on the following families of Exynos processors [https://semiconductor.samsung.com/processor/](https://semiconductor.samsung.com/processor/):

* Exynos 2600
* Exynos 2500
* Exynos 2400
* Exynos 2300
* Exynos 2200
* Exynos 2100

## iOS App

The MLPerf Mobile App is also available for iOS devices, optimized to leverage Apple's neural engine and Core ML framework. Core ML is Apple's machine learning framework that helps integrate machine learning models into iOS applications. On supported iOS devices, the app utilizes the Neural Engine available in Apple Silicon (A-series and M-series chips) to accelerate machine learning workloads.

The iOS version of the MLPerf Mobile App supports the same benchmarks as the Android version, allowing for consistent cross-platform performance comparisons. The app takes advantage of the Metal Performance Shaders (MPS) and Apple's Neural Engine to provide optimized performance on iOS devices.

For more information about Core ML, visit [https://developer.apple.com/documentation/coreml](https://developer.apple.com/documentation/coreml)

## Licensing Information

The MLPerf Mobile App code is licensed under the Apache 2.0 License. The models used in the benchmarks are subject to their respective licenses. For detailed licensing information, please refer to the [LICENSE.md](https://github.com/mlcommons/mobile_app_open/blob/master/LICENSE.md) file in the repository.

The datasets used for benchmarking are subject to their original licenses and terms of use. Please consult the documentation of each specific dataset for details on usage rights and restrictions.
