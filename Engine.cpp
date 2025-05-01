#include <string>
#include <vector>
#include "AudioVector.h"
class Engine{
    public:
        std::string name;
        float shownRPM;
        float rpmMultiplier; // rpmMultiplier * shownRPM = 'audio RPM'
        void setIntervalsFromDegrees(const std::vector<float>& degreesInterval){}
        float getAudioRPM(){
            return shownRPM*rpmMultiplier;
        }
        void setShownRPM(float newRpm){
            shownRPM = newRpm;
        }
        Engine(std::string m_name, float m_rpmMultiplier, std::vector<AudioVector>& m_pistons, const std::vector<float>& m_degreesIntervals){
            name = m_name;
            rpmMultiplier = m_rpmMultiplier;
            pistons = m_pistons;
            setIntervalsFromDegrees(m_degreesIntervals);
        }

        std::vector<AudioVector> pistons;
        int cylinderCount;
        std::vector<int> firing_order;
        std::vector<float> interval_factors;  // Intervals (not rpm corrected, just factors) (should all be 1 for even fire)


};