#pragma once

#include "NeuralNetwork.hpp"

namespace fast {

/**
 * @brief Neural network process object for bounding box detection
 *
 * This class is a convenience class for a neural network which performs bounding box prediction.
 *
 * @ingroup neural-network
 */
class FAST_EXPORT BoundingBoxNetwork : public NeuralNetwork {
    FAST_OBJECT(BoundingBoxNetwork)
    public:
        void setThreshold(float threshold);
        void loadAttributes() override;
        void setAnchors(std::vector<std::vector<Vector2f>> anchors);
    private:
        BoundingBoxNetwork();
        void execute() override;

        float m_threshold;
        std::vector<std::vector<Vector2f>> m_anchors;


};

}
