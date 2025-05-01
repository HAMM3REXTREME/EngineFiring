#include <string>
#include <vector>
// Stores an audio file as a large vector of floats.
class AudioVector{
    public:
        std::vector<float> samples;
        AudioVector(const std::string& filename){}; // Load from wav
        AudioVector(const std::vector<float>& m_samples){
            samples = m_samples;
        };
};