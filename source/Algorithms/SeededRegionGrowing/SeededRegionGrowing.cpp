#include "SeededRegionGrowing.hpp"
#include "DeviceManager.hpp"
#include "SceneGraph.hpp"
#include <stack>

namespace fast {


void SeededRegionGrowing::setInput(ImageData::pointer input) {
    mInput = input;
    mIsModified = true;
    setParent(input);
    if(input->isDynamicData()) {
        mOutput = DynamicImage::New();
        DynamicImage::pointer(mOutput)->setStreamer(DynamicImage::pointer(mInput)->getStreamer());
    } else {
        mOutput = Image::New();
        input->retain(mDevice);
    }
    mOutput->setSource(mPtr.lock());
}

void SeededRegionGrowing::setIntensityRange(float min, float max) {
    if(min >= max)
        throw Exception("Min must be smaller than max intensity range in SeededRegionGrowing");

    mMinimumIntensity = min;
    mMaximumIntensity = max;
    mIsModified = true;
}

void SeededRegionGrowing::addSeedPoint(uint x, uint y) {
    Uint3 pos;
    pos[0] = x;
    pos[1] = y;
    pos[2] = 0;
    addSeedPoint(pos);
}

void SeededRegionGrowing::addSeedPoint(uint x, uint y, uint z) {
    Uint3 pos;
    pos[0] = x;
    pos[1] = y;
    pos[2] = z;
    addSeedPoint(pos);
}

void SeededRegionGrowing::addSeedPoint(Uint3 position) {
    mSeedPoints.push_back(position);
}

void SeededRegionGrowing::setDevice(ExecutionDevice::pointer device) {
    if(mInput.isValid() && !mInput->isDynamicData()) {
        Image::pointer(mInput)->release(mDevice);
        Image::pointer(mInput)->retain(device);
    }
    mDevice = device;
    mIsModified = true;
}

ImageData::pointer SeededRegionGrowing::getOutput() {
    if(!mOutput.isValid()) {
        throw Exception("Must call setInput before getOutput in SeededRegionGrowing");
    }
    return mOutput;
}

SeededRegionGrowing::SeededRegionGrowing() {
    mDevice = DeviceManager::getInstance().getDefaultComputationDevice();
    mDimensionCLCodeCompiledFor = 0;
}

void SeededRegionGrowing::recompileOpenCLCode(Image::pointer input) {
    // Check if there is a need to recompile OpenCL code
    if(input->getDimensions() == mDimensionCLCodeCompiledFor &&
            input->getDataType() == mTypeCLCodeCompiledFor)
        return;

    OpenCLDevice::pointer device = mDevice;
    std::string buildOptions = "";
    if(input->getDataType() == TYPE_FLOAT) {
        buildOptions = "-DTYPE_FLOAT";
    } else if(input->getDataType() == TYPE_INT8 || input->getDataType() == TYPE_INT16) {
        buildOptions = "-DTYPE_INT";
    } else {
        buildOptions = "-DTYPE_UINT";
    }
    std::string filename;
    if(input->getDimensions() == 2) {
        filename = "Algorithms/SeededRegionGrowing/SeededRegionGrowing2D.cl";
    } else {
        filename = "Algorithms/SeededRegionGrowing/SeededRegionGrowing3D.cl";
    }
    int programNr = device->createProgramFromSource(std::string(FAST_SOURCE_DIR) + filename, buildOptions);
    mKernel = cl::Kernel(device->getProgram(programNr), "seededRegionGrowing");
    mDimensionCLCodeCompiledFor = input->getDimensions();
    mTypeCLCodeCompiledFor = input->getDataType();
}

template <class T>
void SeededRegionGrowing::executeOnHost(T* input, Image::pointer output) {
    ImageAccess outputAccess = output->getImageAccess(ACCESS_READ_WRITE);
    uchar* outputData = (uchar*)outputAccess.get();
    // initialize output to all zero
    memset(outputData, 0, output->getWidth()*output->getHeight()*output->getDepth());
    std::stack<Uint3> queue;

    // Add seeds to queue
    for(int i = 0; i < mSeedPoints.size(); i++) {
        Uint3 pos = mSeedPoints[i];

        // Check if seed point is in bounds
        if(pos.x() < 0 || pos.y() < 0 || pos.z() < 0 ||
            pos.x() >= output->getWidth() || pos.y() >= output->getHeight() || pos.z() >= output->getDepth())
            throw Exception("One of the seed points given to SeededRegionGrowing was out of bounds.");

        queue.push(pos);
    }

    // Process queue
    while(!queue.empty()) {
        Uint3 pos = queue.top();
        queue.pop();

        // Add neighbors to queue
        for(int a = -1; a < 2; a++) {
        for(int b = -1; b < 2; b++) {
        for(int c = -1; c < 2; c++) {
            if(abs(a)+abs(b)+abs(c) != 1) // connectivity
                continue;
            Uint3 neighbor(pos.x()+a,pos.y()+b,pos.z()+c);
            // Check for out of bounds
            if(neighbor.x() < 0 || neighbor.y() < 0 || neighbor.z() < 0 ||
                neighbor.x() >= output->getWidth() || neighbor.y() >= output->getHeight() || neighbor.z() >= output->getDepth())
                continue;

            // Check that voxel is not already segmented
            if(outputData[neighbor.x()+neighbor.y()*output->getWidth()+neighbor.z()*output->getWidth()*output->getHeight()] == 1)
                continue;

            // Check condition
            T value = input[neighbor.x()+neighbor.y()*output->getWidth()+neighbor.z()*output->getWidth()*output->getHeight()];
            if(value >= mMinimumIntensity && value <= mMaximumIntensity) {
                // add it to segmentation
                outputData[neighbor.x()+neighbor.y()*output->getWidth()+neighbor.z()*output->getWidth()*output->getHeight()] = 1;

                // Add to queue
                queue.push(neighbor);
            }
        }}}
    }
}

void SeededRegionGrowing::execute() {
    Image::pointer input;
    if(!mInput.isValid())
        throw Exception("No input supplied to SeededRegionGrowing");

    if(mSeedPoints.size() == 0)
        throw Exception("No seed points supplied to SeededRegionGrowing");

    if(!mOutput.isValid()) {
        // output object is no longer valid
        return;
    }

    if(mInput->isDynamicData()) {
        input = DynamicImage::pointer(mInput)->getNextFrame();
    } else {
        input = mInput;
    }

    if(input->getNrOfComponents() != 1)
        throw Exception("Seeded region growing currently doesn't support images with several components.");

    Image::pointer output;
    if(mInput->isDynamicData()) {
        output = Image::New();
        DynamicImage::pointer(mOutput)->addFrame(output);
    } else {
        output = Image::pointer(mOutput);
    }

    // Initialize output image
    if(input->getDimensions() == 2) {
        output->create2DImage(
                input->getWidth(),
                input->getHeight(),
                TYPE_UINT8,
                1,
                mDevice
        );
    } else {
         output->create3DImage(
                input->getWidth(),
                input->getHeight(),
                input->getDepth(),
                TYPE_UINT8,
                1,
                mDevice
        );
    }

    if(mDevice->isHost()) {
        ImageAccess inputAccess = input->getImageAccess(ACCESS_READ);
        void* inputData = inputAccess.get();
        switch(input->getDataType()) {
            fastSwitchTypeMacro(executeOnHost<FAST_TYPE>((FAST_TYPE*)inputData, output));
        }
    } else {
        OpenCLDevice::pointer device = mDevice;

        recompileOpenCLCode(input);

        ImageAccess access = output->getImageAccess(ACCESS_READ_WRITE);
        uchar* outputData = (uchar*)access.get();
        // Initialize to all 0s
        memset(outputData,0,sizeof(uchar)*output->getWidth()*output->getHeight()*output->getDepth());

        // Add sedd points
        for(int i = 0; i < mSeedPoints.size(); i++) {
            Uint3 pos = mSeedPoints[i];

            // Check if seed point is in bounds
            if(pos.x() < 0 || pos.y() < 0 || pos.z() < 0 ||
                pos.x() >= output->getWidth() || pos.y() >= output->getHeight() || pos.z() >= output->getDepth())
                throw Exception("One of the seed points given to SeededRegionGrowing was out of bounds.");

            outputData[pos.x() + pos.y()*output->getWidth() + pos.z()*output->getWidth()*output->getHeight()] = 2;
        }
        access.release();

        cl::NDRange globalSize;
        if(output->getDimensions() == 2) {
            globalSize = cl::NDRange(input->getWidth(),input->getHeight());
            OpenCLImageAccess2D inputAccess = input->getOpenCLImageAccess2D(ACCESS_READ, device);
            mKernel.setArg(0, *inputAccess.get());
        } else {
            globalSize = cl::NDRange(input->getWidth(),input->getHeight(), input->getDepth());
            OpenCLImageAccess3D inputAccess = input->getOpenCLImageAccess3D(ACCESS_READ, device);
            mKernel.setArg(0, *inputAccess.get());
        }

        OpenCLBufferAccess outputAccess = output->getOpenCLBufferAccess(ACCESS_READ_WRITE, device);
        cl::Buffer stopGrowingBuffer = cl::Buffer(
                device->getContext(),
                CL_MEM_READ_WRITE,
                sizeof(char));
        cl::CommandQueue queue = device->getCommandQueue();
        mKernel.setArg(1, *outputAccess.get());
        mKernel.setArg(2, stopGrowingBuffer);
        mKernel.setArg(3, mMinimumIntensity);
        mKernel.setArg(4, mMaximumIntensity);

        bool stopGrowing = false;
        char stopGrowingInit = 1;
        char * stopGrowingResult = new char;
        int iterations = 0;
        do {
            iterations++;
            queue.enqueueWriteBuffer(stopGrowingBuffer, CL_TRUE, 0, sizeof(char), &stopGrowingInit);

            queue.enqueueNDRangeKernel(
                    mKernel,
                    cl::NullRange,
                    globalSize,
                    cl::NullRange
            );

            queue.enqueueReadBuffer(stopGrowingBuffer, CL_TRUE, 0, sizeof(char), stopGrowingResult);
            if(*stopGrowingResult == 1)
                stopGrowing = true;
        } while(!stopGrowing);
    }

    if(!mInput->isDynamicData())
        mInput->release(mDevice);

    // Update the timestamp of the output data
    output->updateModifiedTimestamp();
    SceneGraph::getInstance().setParentNode(output, input);
}

void SeededRegionGrowing::waitToFinish() {
    if(!mDevice->isHost()) {
        OpenCLDevice::pointer device = mDevice;
        device->getCommandQueue().finish();
    }
}

} // end namespace fast
