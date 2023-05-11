#include "InferenceEngine.hpp"
#include <FAST/Utility.hpp>

namespace fast {

void InferenceEngine::setModelAndWeights(std::vector<uint8_t> model, std::vector<uint8_t> weights) {
    m_model = model;
    m_weights = weights;
}

void InferenceEngine::setFilename(std::string filename) {
    m_filename = filename;
}

std::string InferenceEngine::getFilename() const {
    return m_filename;
}

bool InferenceEngine::isLoaded() const {
    return m_isLoaded;
}

void InferenceEngine::setIsLoaded(bool loaded) {
    m_isLoaded = loaded;
}

void InferenceEngine::addInputNode(NeuralNetworkNode node) {
    mInputNodes[node.name] = node;
}

void InferenceEngine::addOutputNode(NeuralNetworkNode node) {
    mOutputNodes[node.name] = node;
}

NeuralNetworkNode InferenceEngine::getInputNode(std::string name) const {
    return mInputNodes.at(name);
}

NeuralNetworkNode InferenceEngine::getOutputNode(std::string name) const {
    return mOutputNodes.at(name);
}

std::map<std::string, NeuralNetworkNode> InferenceEngine::getOutputNodes() const {
    return mOutputNodes;
}

std::map<std::string, NeuralNetworkNode> InferenceEngine::getInputNodes() const {
    return mInputNodes;
}


void InferenceEngine::setInputData(std::string nodeName, std::shared_ptr<Tensor> tensor) {
	mInputNodes.at(nodeName).data = tensor;
}

void InferenceEngine::setInputNodeShape(std::string name, TensorShape shape) {
    mInputNodes.at(name).shape = shape;
}

void InferenceEngine::setOutputNodeShape(std::string name, TensorShape shape) {
    mOutputNodes.at(name).shape = shape;
}

std::shared_ptr<fast::Tensor> InferenceEngine::getOutputData(std::string nodeName) {
    return mOutputNodes.at(nodeName).data;
}

void InferenceEngine::setDeviceType(InferenceDeviceType type) {
    m_deviceType = type;
}

void InferenceEngine::setDevice(int index, InferenceDeviceType type) {
    m_deviceIndex = index;
    m_deviceType = type;
}

std::vector<InferenceDeviceInfo> InferenceEngine::getDeviceList() {
    throw Exception("getDeviceList is not supported for the inference engine " + getName());
}

void InferenceEngine::setMaxBatchSize(int size) {
    m_maxBatchSize = size;
}

int InferenceEngine::getMaxBatchSize() {
    return m_maxBatchSize;
}

void InferenceEngine::loadCustomPlugins(std::vector<std::string> filenames) {
    throw NotImplementedException();
}

std::string getModelFileExtension(ModelFormat format) {
    std::map<ModelFormat, std::string> map = {
        {ModelFormat::PROTOBUF, "pb"},
        {ModelFormat::SAVEDMODEL, "/"}, // Saved model format is a directory
        {ModelFormat::ONNX, "onnx"},
        {ModelFormat::OPENVINO, "xml"},
        {ModelFormat::UFF, "uff"}
    };
    return map.at(format);
}

ModelFormat getModelFormat(std::string filename) {
    if(isDir(filename))
        return ModelFormat::SAVEDMODEL;

    auto pos = filename.rfind(".") + 1;
	if(pos == std::string::npos)
		throw Exception("Unable to determine model format because: Unable to get extension of file " + filename);
    auto extension = filename.substr(pos);
    extension = stringToLower(extension);
    std::map<std::string, ModelFormat> map = {
        {"pb", ModelFormat::PROTOBUF},
        {"onnx", ModelFormat::ONNX},
        {"xml", ModelFormat::OPENVINO},
        {"uff", ModelFormat::UFF}
    };
    if(map.count(extension) == 0)
        throw Exception("Unable to determine model format of file " + filename);

    return map[extension];
}

bool InferenceEngine::isModelFormatSupported(ModelFormat format) {
    auto formats = getSupportedModelFormats();
    auto pos = std::find(formats.begin(), formats.end(), format);
    return pos != formats.end();
}

void InferenceEngine::setImageOrdering(ImageOrdering ordering) {
    m_imageOrdering = ordering;
}

ImageOrdering InferenceEngine::detectImageOrdering(const TensorShape &shape, bool hasBatchDim) {
    if(shape.getDimensions() >= 4) { // If image; 2D or 3D
        if(shape[shape.getDimensions() - 1] <= 4 && shape[shape.getDimensions() - 1] > 0) {
            Reporter::info() << "Guessed image ordering to be channel last as shape was " << shape.toString() << Reporter::end();
            return ImageOrdering::ChannelLast;
        } else if(shape[hasBatchDim ? 1 : 0] <= 4 && shape[hasBatchDim ? 1 : 0] > 0) {
            Reporter::info() << "Guessed image ordering to be channel first as shape was " << shape.toString() << Reporter::end();
            return ImageOrdering::ChannelFirst;
        } else {
            // Unable to determine shape
            throw Exception("Unable to determine image ordering from shape " + shape.toString());
        }
    } else {
        throw Exception("Shape was not an image");
    }
}

NodeType InferenceEngine::detectNodeType(const TensorShape &shape) {
    if(shape.getDimensions() >= 4) {
        return NodeType::IMAGE;
    } else {
        return NodeType::TENSOR;
    }
}

std::string getModelFormatName(ModelFormat format) {
        std::map<ModelFormat, std::string> map = {
        {ModelFormat::SAVEDMODEL, "TensorFlow SavedModel"},
        {ModelFormat::PROTOBUF, "TensorFlow Protobuf"},
        {ModelFormat::ONNX, "ONNX"},
        {ModelFormat::OPENVINO, "OpenVINO"},
        {ModelFormat::UFF, "UFF"}
    };
    return map.at(format);
}

}