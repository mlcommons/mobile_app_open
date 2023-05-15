import Foundation
import CoreML

struct MLFeature {
    public var data: UnsafeMutableRawPointer?
    public var description: MLFeatureDescription?
}


private class MLMultiArrayFeatureProvider: NSObject, MLFeatureProvider {
    private let inputs: [MLFeature]
    private var featureValues: [String: MLFeatureValue]
    
    var featureNames: Set<String> = Set<String>()
    
    init?(inputs: [MLFeature]) {
        self.inputs = inputs
        self.featureValues = [:]
        
        for input in inputs {
            let featureName = input.description?.name
            
            guard let featureName = featureName else { return nil }
            guard featureName.isEmpty else { return nil }
            
            featureNames.insert(featureName)
            
            let constraint = input.description?.multiArrayConstraint
            
            guard let constraint = constraint else { return nil }
            
            let shape = constraint.shape
            var strides = [NSNumber](repeating: 1, count: shape.count)
            
            for i in stride(from: shape.count - 1, to: 0, by: -1) {
                let stride = strides[i].intValue * shape[i].intValue
                strides[i - 1] = NSNumber(value: stride)
            }
            
            guard let mlArray = try? MLMultiArray(
                dataPointer: input.data!,
                shape: constraint.shape,
                dataType: constraint.dataType,
                strides: strides,
                deallocator: nil
            ) else {
                NSLog("Failed to create MLMultiArray for feature \(featureName)")
                return nil
            }
            
            let mlFeatureValue = MLFeatureValue(multiArray: mlArray)
            featureValues[featureName] = mlFeatureValue
        }
    }
    
    func featureValue(for featureName: String) -> MLFeatureValue? {
        let value = featureValues[featureName]
        if value == nil {
            NSLog("Feature \(featureName) not found")
        }
        return value
    }
}

private class MLMultiArrayBatchProvider: NSObject, MLBatchProvider {
    
    var count: Int
    
    private let inputs: [[MLFeature]]
    
    init(inputs: [[MLFeature]]) {
        self.inputs = inputs
        self.count = inputs.count
    }
    
    func features(at index: Int) -> MLFeatureProvider {
        let inputFeatures = inputs[index]
        return MLMultiArrayFeatureProvider(inputs: inputFeatures)!
    }
}

@objc
public class CoreMLExecutor: NSObject {
    private var modelURL: URL?
    private var mlmodel: MLModel?
    private var batchSize: UInt = 0
    private var inputCount: UInt = 0
    private var outputCount: UInt = 0
    private var inputFeatures: [[MLFeature]]?
    private var outputFeatures: [[MLFeature]]?
    private var inputNames: [String] = []
    private var outputNames: [String] = []
    private var outputProvider: MLBatchProvider?
    
    
    @objc
    init?(modelPath: UnsafePointer<CChar>, batchSize: Int) {
        pthread_set_qos_class_self_np(QOS_CLASS_USER_INITIATED, 0)
        
        let config = MLModelConfiguration()
        config.computeUnits = MLComputeUnits.all
        
        guard let modelPathString = String(validatingUTF8: modelPath), let url = URL(string: modelPathString),
              let compiledModelURL = try? MLModel.compileModel(at: url),
              let mlmodel = try? MLModel(contentsOf: compiledModelURL, configuration: config) else {
            NSLog("Failed to load Core ML model")
            return nil
        }
        
        let inputNames = mlmodel.modelDescription.inputDescriptionsByName.keys.sorted()
        let outputNames = mlmodel.modelDescription.outputDescriptionsByName.keys.sorted()
        
        self.modelURL = url
        self.mlmodel = mlmodel
        self.batchSize = UInt(batchSize > 1 ? batchSize : 1)
        self.inputCount = UInt(inputNames.count)
        self.outputCount = UInt(outputNames.count)
        self.inputNames = inputNames
        self.outputNames = outputNames
        self.inputFeatures = []
        self.outputFeatures = []
        self.outputProvider = nil
        
        super.init()
    
        self.prepareFeatureVector()
        
        NSLog("inputNames: \(inputNames)")
        NSLog("outputNames: \(outputNames)")
        NSLog("batchSize: \(batchSize)")
    }
    
    @objc
    func prepareFeatureVector() {
        let inputFeatures = [[MLFeature]](repeating: [MLFeature](repeating: MLFeature(data: nil, description: nil), count: Int(inputCount)), count: Int(batchSize))
      self.inputFeatures = inputFeatures
      
        let outputFeatures = [[MLFeature]](repeating: [MLFeature](repeating: MLFeature(data: nil, description: nil), count: Int(outputCount)), count: Int(batchSize))
      self.outputFeatures = outputFeatures
    }
    
    private func clearFeatureVector() {
        inputFeatures?.removeAll()
        outputFeatures?.removeAll()
    }
    
    deinit {
        clearFeatureVector()
        NSLog("[CoreMLExecutor dealloc]")
    }
    
    @objc
    func getInputCount() -> Int {
        return Int(inputCount)
    }
    
    
    func getInputAt(_ i: Int) -> MLFeatureDescription? {
        let name = inputNames[i]
        return mlmodel?.modelDescription.inputDescriptionsByName[name]
    }
    
    @objc
    func getInputSizeAt(_ i: Int) -> Int {
        let input = getInputAt(i)
        let shape = input?.multiArrayConstraint?.shape
        let inputSize = shape?.reduce(1, { $0 * $1.intValue }) ?? 0
        assert(inputSize > 0)
        return inputSize
    }
    
    @objc
    func getInputTypeAt(_ i: Int) -> NSNumber? {
        let input = getInputAt(i)
        
        if let inputType = input?.multiArrayConstraint?.dataType {
                return NSNumber(value: inputType.rawValue)
            }
        return nil
    }
    
    @objc
    func getOutputCount() -> Int {
        return Int(outputCount)
    }
    
    func getOutputAt(_ i: Int) -> MLFeatureDescription? {
        let name = outputNames[i]
        return mlmodel?.modelDescription.outputDescriptionsByName[name]
    }
    
    @objc
    func getOutputSizeAt(_ i: Int) -> Int {
        let output = getOutputAt(i)
        let shape = output?.multiArrayConstraint!.shape
        let outputSize = shape?.reduce(1, { $0 * $1.intValue }) ?? 0
        assert(outputSize > 0)
        return outputSize
    }
    
    @objc
    func getOutputTypeAt(_ i: Int) -> NSNumber? {
        let output = getOutputAt(i)
        if let outputType = output?.multiArrayConstraint?.dataType {
                return NSNumber(value: outputType.rawValue)
            }
        return nil
    }
    
    @objc
    func setInputData(_ data: UnsafeMutableRawPointer, at i: Int, batchIndex: Int) -> Bool {
        let input = getInputAt(i)
        inputFeatures?[batchIndex][i] = MLFeature(data: data, description: input)
        return true
    }
    
    @objc
    func issueQueries() -> Bool {
        autoreleasepool {
            do {
                let batchProvider = MLMultiArrayBatchProvider(inputs: inputFeatures!)
                let outputProvider = try mlmodel?.predictions(fromBatch: batchProvider)
                
                for batchIndex in 0..<batchSize {
                    let outputFeature = outputProvider?.features(at: Int(batchIndex))
                    
                    for (outputIndex, name) in outputNames.enumerated() {
                        guard let outputValue = outputFeature?.featureValue(for: name) else {
                            NSLog("Feature \(name) not found")
                            return false
                        }
                        
                        guard let outputArray = outputValue.multiArrayValue else {
                            return false
                        }
                        
                        let data = outputArray.dataPointer
                        outputFeatures?[Int(batchIndex)][outputIndex] = MLFeature(data: data, description: nil)
                    }
                }
                
                return true
            } catch let error as NSError {
                NSLog("Failed to predict with error: \(error.localizedDescription)")
                return false
            }
        }
    }
    
    @objc
    func flushQueries() -> Bool {
        clearFeatureVector()
        prepareFeatureVector()
        return true
    }
    
    @objc
    func getOutputData(_ data: UnsafeMutableRawPointer, at i: Int, batchIndex: Int) -> Bool {
        let outputFeature = outputFeatures?[batchIndex][i]
        guard let outputFeature = outputFeature, let data = outputFeature.data else {
            return false
        }
        data.copyMemory(from: data, byteCount: getOutputSizeAt(i))
        return true
    }
}
