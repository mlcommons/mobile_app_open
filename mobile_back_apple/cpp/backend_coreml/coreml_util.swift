import Accelerate
import CoreML
import CoreVideo
import CryptoKit
import Foundation

struct MLFeature {
  public var data: UnsafeMutableRawPointer?
  public var pixelBuffer: CVPixelBuffer?
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

      if let pixelBuffer = input.pixelBuffer {
        featureValues[featureName] = MLFeatureValue(pixelBuffer: pixelBuffer)
        continue
      }

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
  case all = "cpu&gpu&ane"
  case cpuAndGPU = "cpu&gpu"
  case cpuAndNeuralEngine = "cpu&ane"
  case cpuOnly = "cpuOnly"
}

private class MLCommonsComputationUnit {
  private var unit: MLComputeUnits = .all

  init(unitName: ComputationUnitNameEnum) {
    unit = getComputationUnit(unitName)
  }

  private func getComputationUnit(_ computationUnitName: ComputationUnitNameEnum) -> MLComputeUnits
  {
    switch computationUnitName {
    case .all:
      if let hasANE = Device.hasANE, hasANE {
        return MLComputeUnits.all
      } else {
        return MLComputeUnits.cpuAndGPU
      }
    case .cpuAndGPU:
      return MLComputeUnits.cpuAndGPU
    case .cpuAndNeuralEngine:
      if let hasANE = Device.hasANE, hasANE, #available(iOS 16.0, macOS 13.0, *) {
        return MLComputeUnits.cpuAndNeuralEngine
      } else {
        return MLComputeUnits.cpuOnly
      }
    case .cpuOnly:
      return MLComputeUnits.cpuOnly
    }
  }

  private func getNameForComputationUnit(_ computationUnit: MLComputeUnits)
    -> ComputationUnitNameEnum
  {
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
      NSLog("Unknown MLComputeUnits: \(computationUnit)")
      return ComputationUnitNameEnum.all
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
  private var compiledModelURL: URL?
  private var compiledModelIsPersistent = false
  private var mlmodel: MLModel?
  private var batchSize: UInt = 0
  private var inputCount: UInt = 0
  private var outputCount: UInt = 0
  private var inputFeatures: [[MLFeature]] = []
  private var outputFeatures: [[MLFeature]] = []
  private var inputNames: [String] = []
  private var outputNames: [String] = []
  private var outputProvider: MLBatchProvider?
  private var singleOutputFeature: MLFeatureProvider?
  private var preallocInputArrays: [[String: MLMultiArray]] = []
  private var preallocSingleProvider: MLDictionaryFeatureProvider?
  private var preallocBatchProvider: MLArrayBatchProvider?
  private var accelerator: MLCommonsComputationUnit = MLCommonsComputationUnit(unitName: .all)

  @objc
  public init(
    modelPath: UnsafePointer<CChar>, batchSize: Int, acceleratorName: UnsafePointer<CChar>
  ) throws {
    let config = MLModelConfiguration()

    let unitName = String(validatingUTF8: acceleratorName)

    if let name = unitName, let computationUnit = ComputationUnitNameEnum(rawValue: name) {
      accelerator = MLCommonsComputationUnit(unitName: computationUnit)
      config.computeUnits = accelerator.computeUnits
    }

    guard let modelPathString = String(validatingUTF8: modelPath),
      let url = URL(string: modelPathString)
    else {
      throw MLError(message: "Failed to load Core ML model")
    }

    // Loading a freshly compiled .mlmodelc forces a full ANE compile through ANECompilerService on
    // every run. That helper is killed by the system once it has accumulated about 6 MB, which makes
    // a later benchmark's ANE compile fail and fall back to CPU. Reusing a persisted compiled model
    // lets the ANE compile cache hit, so the helper is only involved the first time.
    let persistentURL = CoreMLExecutor.persistentCompiledURL(for: url)
    var loadedModel: MLModel?
    var usedCompiledURL: URL?
    var persistent = false
    if let cached = persistentURL, FileManager.default.fileExists(atPath: cached.path) {
      if let model = try? MLModel(contentsOf: cached, configuration: config) {
        loadedModel = model
        usedCompiledURL = cached
        persistent = true
      } else {
        try? FileManager.default.removeItem(at: cached)
      }
    }
    if loadedModel == nil {
      guard let freshURL = try? MLModel.compileModel(at: url) else {
        throw MLError(message: "Failed to load Core ML model")
      }
      var compiledURL = freshURL
      if let target = persistentURL, CoreMLExecutor.moveCompiledModel(freshURL, to: target) {
        compiledURL = target
        persistent = true
      }
      guard let model = try? MLModel(contentsOf: compiledURL, configuration: config) else {
        throw MLError(message: "Failed to load Core ML model")
      }
      loadedModel = model
      usedCompiledURL = compiledURL
    }
    guard let mlmodel = loadedModel, let compiledModelURL = usedCompiledURL else {
      throw MLError(message: "Failed to load Core ML model")
    }

    let modelInputNames = mlmodel.modelDescription.inputDescriptionsByName.keys.sorted()
    let modelOutputNames = mlmodel.modelDescription.outputDescriptionsByName.keys.sorted()

    self.modelURL = url
    self.compiledModelURL = compiledModelURL
    self.compiledModelIsPersistent = persistent
    self.mlmodel = mlmodel
    self.batchSize = UInt(batchSize > 1 ? batchSize : 1)
    self.inputCount = UInt(modelInputNames.count)
    self.outputCount = UInt(modelOutputNames.count)
    self.inputNames = modelInputNames
    self.outputNames = modelOutputNames
    self.outputProvider = nil

    super.init()

    self.prepareFeatureVector()
    self.preparePreallocInputs()
  }

  // Key = OS version + model name + hash of the package's graph/manifest files and weight sizes, so
  // a changed model or an OS update compiles a new copy instead of reusing a stale one.
  private static func persistentCompiledURL(for source: URL) -> URL? {
    let fileManager = FileManager.default
    guard let caches = fileManager.urls(for: .cachesDirectory, in: .userDomainMask).first else {
      return nil
    }
    let resolved = source.resolvingSymlinksInPath()
    guard let enumerator = fileManager.enumerator(
      at: resolved, includingPropertiesForKeys: [.fileSizeKey, .isRegularFileKey])
    else {
      return nil
    }
    var hasher = SHA256()
    hasher.update(data: Data(ProcessInfo.processInfo.operatingSystemVersionString.utf8))
    var entries: [(String, URL, Int)] = []
    for case let file as URL in enumerator {
      let values = try? file.resourceValues(forKeys: [.fileSizeKey, .isRegularFileKey])
      guard values?.isRegularFile == true else { continue }
      entries.append((file.path.replacingOccurrences(of: resolved.path, with: ""), file, values?.fileSize ?? 0))
    }
    entries.sort { $0.0 < $1.0 }
    for (relativePath, file, size) in entries {
      hasher.update(data: Data("\(relativePath):\(size)".utf8))
      if file.pathExtension == "mlmodel" || file.lastPathComponent == "Manifest.json" {
        guard let data = try? Data(contentsOf: file, options: .mappedIfSafe) else { return nil }
        hasher.update(data: data)
      }
    }
    let key = hasher.finalize().prefix(16).map { String(format: "%02x", $0) }.joined()
    let name = resolved.deletingPathExtension().lastPathComponent
    return caches.appendingPathComponent("coreml_compiled/\(key)/\(name).mlmodelc")
  }

  private static func moveCompiledModel(_ source: URL, to target: URL) -> Bool {
    let fileManager = FileManager.default
    do {
      try fileManager.createDirectory(
        at: target.deletingLastPathComponent(), withIntermediateDirectories: true)
      try? fileManager.removeItem(at: target)
      try fileManager.moveItem(at: source, to: target)
      return true
    } catch {
      return false
    }
  }

  private static func isContiguous(_ array: MLMultiArray) -> Bool {
    let shape = array.shape
    let strides = array.strides
    guard shape.count == strides.count else { return false }
    var expected = 1
    for i in stride(from: shape.count - 1, through: 0, by: -1) {
      let dim = shape[i].intValue
      if dim > 1 && strides[i].intValue != expected {
        return false
      }
      expected *= dim
    }
    return true
  }

  private func preparePreallocInputs() {
    guard let mlmodel = self.mlmodel else { return }
    // Batched (offline) runs are faster with the per-query MLMultiArrayBatchProvider than with a
    // pre-allocated MLArrayBatchProvider (about 1000 vs 830 QPS on A18 for MobileNetV4 batch 32).
    if batchSize > 1 { return }
    var batchArrays: [[String: MLMultiArray]] = []
    var batchProviders: [MLDictionaryFeatureProvider] = []
    batchArrays.reserveCapacity(Int(batchSize))
    batchProviders.reserveCapacity(Int(batchSize))

    for _ in 0..<batchSize {
      var arrays: [String: MLMultiArray] = [:]
      var dict: [String: MLFeatureValue] = [:]
      for name in inputNames {
        guard let desc = mlmodel.modelDescription.inputDescriptionsByName[name],
          let constraint = desc.multiArrayConstraint,
          constraint.dataType != .int32,
          let arr = try? MLMultiArray(shape: constraint.shape, dataType: constraint.dataType),
          CoreMLExecutor.isContiguous(arr)
        else {
          return
        }
        arrays[name] = arr
        dict[name] = MLFeatureValue(multiArray: arr)
      }
      guard let provider = try? MLDictionaryFeatureProvider(dictionary: dict) else {
        return
      }
      batchArrays.append(arrays)
      batchProviders.append(provider)
    }

    self.preallocInputArrays = batchArrays
    if batchSize == 1, let single = batchProviders.first {
      self.preallocSingleProvider = single
    } else {
      self.preallocBatchProvider = MLArrayBatchProvider(array: batchProviders)
    }
  }

  @objc
  func prepareFeatureVector() {
    let modelInputFeatures = [[MLFeature]](
      repeating: [MLFeature](
        repeating: MLFeature(data: nil, pixelBuffer: nil, description: nil), count: Int(inputCount)),
      count: Int(batchSize))
    self.inputFeatures = modelInputFeatures
    let modelOutputFeatures = [[MLFeature]](
      repeating: [MLFeature](
        repeating: MLFeature(data: nil, pixelBuffer: nil, description: nil), count: Int(outputCount)),
      count: Int(batchSize))
    self.outputFeatures = modelOutputFeatures
  }

  private func clearFeatureVector() {
    inputFeatures.removeAll()
    outputFeatures.removeAll()
  }

  deinit {
    clearFeatureVector()
    if let compiledURL = compiledModelURL, !compiledModelIsPersistent {
      try? FileManager.default.removeItem(at: compiledURL)
    }
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
  public func isInputImageAt(_ i: Int) -> Bool {
    return getInputAt(i)?.imageConstraint != nil
  }

  @objc
  public func getInputSizeAt(_ i: Int) -> Int {
    let input = getInputAt(i)
    if let imageConstraint = input?.imageConstraint {
      let channels = imageConstraint.pixelFormatType == kCVPixelFormatType_OneComponent8 ? 1 : 3
      return imageConstraint.pixelsHigh * imageConstraint.pixelsWide * channels
    }
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

  private func createPixelBuffer(
    from data: UnsafeMutableRawPointer, constraint: MLImageConstraint
  ) -> CVPixelBuffer? {
    let width = constraint.pixelsWide
    let height = constraint.pixelsHigh
    let isGray = constraint.pixelFormatType == kCVPixelFormatType_OneComponent8
    let pixelFormat = isGray ? kCVPixelFormatType_OneComponent8 : kCVPixelFormatType_32BGRA

    var pixelBuffer: CVPixelBuffer?
    let attrs: [CFString: Any] = [
      kCVPixelBufferCGImageCompatibilityKey: true,
      kCVPixelBufferCGBitmapContextCompatibilityKey: true,
      kCVPixelBufferIOSurfacePropertiesKey: [:],
    ]
    let status = CVPixelBufferCreate(
      kCFAllocatorDefault, width, height, pixelFormat, attrs as CFDictionary, &pixelBuffer)
    guard status == kCVReturnSuccess, let pb = pixelBuffer else {
      return nil
    }

    CVPixelBufferLockBaseAddress(pb, [])
    defer { CVPixelBufferUnlockBaseAddress(pb, []) }
    guard let dstBase = CVPixelBufferGetBaseAddress(pb) else {
      return nil
    }
    let dstRowBytes = CVPixelBufferGetBytesPerRow(pb)
    if isGray {
      var srcBuf = vImage_Buffer(
        data: data, height: vImagePixelCount(height), width: vImagePixelCount(width),
        rowBytes: width)
      var dstBuf = vImage_Buffer(
        data: dstBase, height: vImagePixelCount(height), width: vImagePixelCount(width),
        rowBytes: dstRowBytes)
      vImageCopyBuffer(&srcBuf, &dstBuf, 1, vImage_Flags(kvImageNoFlags))
    } else {
      var srcBuf = vImage_Buffer(
        data: data, height: vImagePixelCount(height), width: vImagePixelCount(width),
        rowBytes: width * 3)
      var dstBuf = vImage_Buffer(
        data: dstBase, height: vImagePixelCount(height), width: vImagePixelCount(width),
        rowBytes: dstRowBytes)
      vImageConvert_RGB888toBGRA8888(
        &srcBuf, nil, 255, &dstBuf, false, vImage_Flags(kvImageNoFlags))
    }
    return pb
  }

  @objc
  public func setInputData(_ data: UnsafeMutableRawPointer, at i: Int, batchIndex: Int) -> Bool {
    let input = getInputAt(i)
    if let imageConstraint = input?.imageConstraint {
      guard let pixelBuffer = createPixelBuffer(from: data, constraint: imageConstraint) else {
        return false
      }
      inputFeatures[batchIndex][i] = MLFeature(
        data: data, pixelBuffer: pixelBuffer, description: input)
      return true
    }
    if batchIndex < preallocInputArrays.count,
      let name = input?.name,
      let arr = preallocInputArrays[batchIndex][name]
    {
      let elemBytes: Int
      switch arr.dataType {
      case .double: elemBytes = 8
      case .float32, .int32: elemBytes = 4
      default: elemBytes = 2
      }
      memcpy(arr.dataPointer, data, arr.count * elemBytes)
      return true
    }
    inputFeatures[batchIndex][i] = MLFeature(data: data, pixelBuffer: nil, description: input)
    return true
  }

  private func extractOutputs(from featureProvider: MLFeatureProvider?, batchIndex: Int) -> Bool {
    for (outputIndex, name) in outputNames.enumerated() {
      guard let outputValue = featureProvider?.featureValue(for: name) else {
        NSLog("Feature \(name) not found")
        return false
      }
      guard let outputArray = outputValue.multiArrayValue else {
        return false
      }
      outputFeatures[batchIndex][outputIndex] = MLFeature(
        data: outputArray.dataPointer, description: nil)
    }
    return true
  }

  @objc
  public func issueQueries() -> Bool {
    return autoreleasepool {
      do {
        if batchSize == 1, let singleProvider = preallocSingleProvider {
          let outFeature = try mlmodel?.prediction(from: singleProvider)
          singleOutputFeature = outFeature
          return extractOutputs(from: outFeature, batchIndex: 0)
        }

        let batchProvider: MLBatchProvider =
          preallocBatchProvider ?? MLMultiArrayBatchProvider(inputs: inputFeatures)
        outputProvider = try mlmodel?.predictions(fromBatch: batchProvider)

        for batchIndex in 0..<Int(batchSize) {
          let outputFeature = outputProvider?.features(at: batchIndex)
          if !extractOutputs(from: outputFeature, batchIndex: batchIndex) {
            return false
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
  public func getAccelerator() -> UnsafePointer<CChar>? {
    let name = accelerator.computationUnitName.rawValue
    let cs = (name as NSString).utf8String
    return UnsafePointer<Int8>(cs)
  }
}

class Device {

  // iPhone10,3
  static var firmwareIdentifier: String {
    var systemInfo = utsname()
    uname(&systemInfo)
    let machineMirror = Mirror(reflecting: systemInfo.machine)
    let identifier = machineMirror.children.reduce("") { identifier, element in
      guard let value = element.value as? Int8, value != 0
      else { return identifier }
      return identifier + String(UnicodeScalar(UInt8(value)))
    }
    return identifier
  }

  // firmwareIdentifier = iPhone10,3 -> firmwareNumber = (major: 10, minor: 3)
  static var firmwareNumber: (major: Int, minor: Int) {
    let numberElements =
      firmwareIdentifier
      .components(separatedBy: CharacterSet.decimalDigits.inverted)
      .suffix(2)
    let majorStr = numberElements.first ?? "0"
    let minorStr = numberElements.last ?? "0"
    let majorNum = Int(majorStr) ?? 0
    let minorNum = Int(minorStr) ?? 0
    return (major: majorNum, minor: minorNum)
  }

  // Map firmwareIdentifier to device name: https://gist.github.com/adamawolf/3048717
  // Devices with ANE: https://github.com/hollance/neural-engine/blob/master/docs/supported-devices.md
  static var hasANE: Bool? {
    if firmwareIdentifier.contains("iPhone") {
      // iPhone11,2 : iPhone XS
      return firmwareNumber.major >= 11
    }
    if firmwareIdentifier.contains("iPad") {
      // iPad8,1 : iPad Pro 11 inch 3rd Gen (WiFi)
      return firmwareNumber.major >= 8
    }
    if firmwareIdentifier.contains("arm64") {
      // arm64: Macs with Apple Silicon
      return true
    }
    return nil
  }

}
