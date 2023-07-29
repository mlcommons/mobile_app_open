import CoreML
import Foundation

struct MLFeature {
    public var data: UnsafeMutableRawPointer?
    public var description: MLFeatureDescription?
}

struct MLError: Error {
    let message: String
}

private class EmptyFeatureProvider: MLFeatureProvider {
    var featureNames: Set<String> {
        return []
    }
    
    func featureValue(for featureName: String) -> MLFeatureValue? {
        return nil
    }
}

private class MLMultiArrayFeatureProvider: NSObject, MLFeatureProvider {
    private let inputs: [MLFeature]
    private var featureValues: [String: MLFeatureValue]
    
    var featureNames: Set<String> = Set<String>()
    
    init(inputs: [MLFeature]) throws {
        self.inputs = inputs
        self.featureValues = [:]
        
        for input in inputs {
            guard let featureName = input.description?.name, !featureName.isEmpty else {
                throw MLError(message: "Feature name is missing")
            }
            
            featureNames.insert(featureName)
            
            guard let constraint = input.description?.multiArrayConstraint else {
                throw MLError(message: "Constraint is missing")
            }
            
            let shape = constraint.shape
            var strides = [NSNumber](repeating: 1, count: shape.count)
            
            for i in stride(from: shape.count - 1, to: 0, by: -1) {
                let stride = strides[i].intValue * shape[i].intValue
                strides[i - 1] = NSNumber(value: stride)
            }
            
            guard let data = input.data,
                  let mlArray = try? MLMultiArray(
                    dataPointer: data,
                    shape: constraint.shape,
                    dataType: constraint.dataType,
                    strides: strides,
                    deallocator: nil
                  )
            else {
                throw MLError(message: "Failed to create MLMultiArray for feature \(featureName)")
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
        do {
            let result = try MLMultiArrayFeatureProvider(inputs: inputFeatures)
            return result
        } catch {
            NSLog("Empty feature provider is used")
            return EmptyFeatureProvider()
        }
    }
}

enum ComputationUnitNameEnum: String {
    case all = "all"
    case cpuAndGPU = "cpu&gpu"
    case cpuAndNeuralEngine = "cpu&ne"
    case cpuOnly = "cpuOnly"
    case unknown = "uknown"
}

private class MLCommonsComputationUnit {
    private var unit: MLComputeUnits = .all
    
    init(unitName: ComputationUnitNameEnum) {
        unit = getComputationUnit(unitName)
    }
    
    private func getComputationUnit(_ computationUnitName: ComputationUnitNameEnum) -> MLComputeUnits {
        switch computationUnitName {
        case .all:
            return MLComputeUnits.all
        case .cpuAndGPU:
            return MLComputeUnits.cpuAndGPU
        case .cpuAndNeuralEngine:
            if #available(iOS 16.0, *) {
                return MLComputeUnits.all
            }
            return MLComputeUnits.cpuAndGPU
        case .cpuOnly:
            return MLComputeUnits.cpuOnly
        case .unknown:
            return MLComputeUnits.all
        }
    }
    
    private func getNameForComputationUnit(_ computationUnit: MLComputeUnits) -> ComputationUnitNameEnum {
        switch computationUnit {
        case .all:
            return ComputationUnitNameEnum.all
        case .cpuAndNeuralEngine:
            return ComputationUnitNameEnum.cpuAndNeuralEngine
        case .cpuAndGPU:
            return ComputationUnitNameEnum.cpuAndGPU
        case .cpuOnly:
            return ComputationUnitNameEnum.cpuOnly
        @unknown default:
            return ComputationUnitNameEnum.unknown
        }
    }
    
    var computeUnits: MLComputeUnits {
        return unit
    }
    
    var computationUnitName: ComputationUnitNameEnum {
        return getNameForComputationUnit(unit)
    }
}


@objc
public class CoreMLExecutor: NSObject {
    private var modelURL: URL?
    private var mlmodel: MLModel?
    private var batchSize: UInt = 0
    private var inputCount: UInt = 0
    private var outputCount: UInt = 0
    private var inputFeatures: [[MLFeature]] = []
    private var outputFeatures: [[MLFeature]] = []
    private var inputNames: [String] = []
    private var outputNames: [String] = []
    private var outputProvider: MLBatchProvider?
    private var accelerator: MLCommonsComputationUnit = MLCommonsComputationUnit(unitName: .all);
    
    @objc
    public init(modelPath: UnsafePointer<CChar>, batchSize: Int, acceleratorName: UnsafePointer<CChar>) throws {
        pthread_set_qos_class_self_np(QOS_CLASS_USER_INITIATED, 0)
        let config = MLModelConfiguration()
        
        let unitName = String(validatingUTF8: acceleratorName)
        
        if let name = unitName, let computationUnit = ComputationUnitNameEnum(rawValue: name)  {
            accelerator = MLCommonsComputationUnit(unitName: computationUnit)
            config.computeUnits = accelerator.computeUnits
        }
        
        guard let modelPathString = String(validatingUTF8: modelPath),
              let url = URL(string: modelPathString),
              let compiledModelURL = try? MLModel.compileModel(at: url),
              let mlmodel = try? MLModel(contentsOf: compiledModelURL, configuration: config)
        else {
            throw MLError(message: "Failed to load Core ML model")
        }
        
        let modelInputNames = mlmodel.modelDescription.inputDescriptionsByName.keys.sorted()
        let modelOutputNames = mlmodel.modelDescription.outputDescriptionsByName.keys.sorted()
        
        self.modelURL = url
        self.mlmodel = mlmodel
        self.batchSize = UInt(batchSize > 1 ? batchSize : 1)
        self.inputCount = UInt(modelInputNames.count)
        self.outputCount = UInt(modelOutputNames.count)
        self.inputNames = modelInputNames
        self.outputNames = modelOutputNames
        self.outputProvider = nil
        
        super.init()
        
        self.prepareFeatureVector()
    }
    
    @objc
    func prepareFeatureVector() {
        let modelInputFeatures = [[MLFeature]](
            repeating: [MLFeature](
                repeating: MLFeature(data: nil, description: nil), count: Int(inputCount)),
            count: Int(batchSize))
        self.inputFeatures = modelInputFeatures
        let modelOutputFeatures = [[MLFeature]](
            repeating: [MLFeature](
                repeating: MLFeature(data: nil, description: nil), count: Int(outputCount)),
            count: Int(batchSize))
        self.outputFeatures = modelOutputFeatures
    }
    
    private func clearFeatureVector() {
        inputFeatures.removeAll()
        outputFeatures.removeAll()
    }
    
    deinit {
        clearFeatureVector()
    }
    
    @objc
    public func getInputCount() -> Int {
        return Int(inputCount)
    }
    
    func getInputAt(_ i: Int) -> MLFeatureDescription? {
        let name = inputNames[i]
        return mlmodel?.modelDescription.inputDescriptionsByName[name]
    }
    
    @objc
    public func getInputSizeAt(_ i: Int) -> Int {
        let input = getInputAt(i)
        let shape = input?.multiArrayConstraint?.shape
        let inputSize = shape?.reduce(1, { $0 * $1.intValue }) ?? 0
        assert(inputSize > 0)
        return inputSize
    }
    
    @objc
    public func getInputTypeAt(_ i: Int) -> NSNumber? {
        let input = getInputAt(i)
        
        if let inputType = input?.multiArrayConstraint?.dataType {
            return NSNumber(value: inputType.rawValue)
        }
        return nil
    }
    
    @objc
    public func getOutputCount() -> Int {
        return Int(outputCount)
    }
    
    func getOutputAt(_ i: Int) -> MLFeatureDescription? {
        let name = outputNames[i]
        return mlmodel?.modelDescription.outputDescriptionsByName[name]
    }
    
    @objc
    public func getOutputSizeAt(_ i: Int) -> Int {
        let output = getOutputAt(i)
        let shape = output?.multiArrayConstraint?.shape
        let outputSize = shape?.reduce(1, { $0 * $1.intValue }) ?? 0
        assert(outputSize > 0)
        return outputSize
    }
    
    @objc
    public func getOutputTypeAt(_ i: Int) -> NSNumber? {
        let output = getOutputAt(i)
        if let outputType = output?.multiArrayConstraint?.dataType {
            return NSNumber(value: outputType.rawValue)
        }
        return nil
    }
    
    @objc
    public func setInputData(_ data: UnsafeMutableRawPointer, at i: Int, batchIndex: Int) -> Bool {
        let input = getInputAt(i)
        inputFeatures[batchIndex][i] = MLFeature(data: data, description: input)
        return true
    }
    
    @objc
    public func issueQueries() -> Bool {
        autoreleasepool {
            do {
                let batchProvider = MLMultiArrayBatchProvider(inputs: inputFeatures)
                outputProvider = try mlmodel?.predictions(fromBatch: batchProvider)
                
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
                        outputFeatures[Int(batchIndex)][outputIndex] = MLFeature(data: data, description: nil)
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
    public func flushQueries() -> Bool {
        clearFeatureVector()
        prepareFeatureVector()
        return true
    }
    
    @objc
    public func getOutputData(
        _ data: UnsafeMutablePointer<UnsafeMutableRawPointer?>, at i: Int, batchIndex: Int
    ) -> Bool {
        let outputFeature = outputFeatures[batchIndex][i]
        data.pointee = outputFeature.data
        return true
    }
    
    @objc
    public func acceleratorName() -> String {
        return accelerator.computationUnitName.rawValue;
    }
}
