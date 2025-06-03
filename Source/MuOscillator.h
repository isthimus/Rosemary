#pragma once

#include <JuceHeader.h>
#include "HarmonicProfileCalculator.h"

namespace rosy {

class MuOscillator : public juce::dsp::ProcessorBase
{
public:
    class PolyEvaluator
    {
    public:
        PolyEvaluator() = default;
        
        void setCoefficients(const std::vector<float>& newCoeffs) {
            coefficients = newCoeffs;
        }
        
        float operator()(float x) const {
            float result = 0.0f;
            float xPow = 1.0f;  // x^0
            
            for (float coeff : coefficients) {
                result += coeff * xPow;
                xPow *= x;
            }
            
            return result;
        }
        
    private:
        std::vector<float> coefficients;
    };

    MuOscillator();

    //==============================================================================
    void prepare(const juce::dsp::ProcessSpec& spec) override;
    void reset() override;
    void process(const juce::dsp::ProcessContextReplacing<float>& context) override;

    //==============================================================================
    void setFrequency(float freq);
    void setShapeX(float x);
    void setShapeY(float y);
    
    // Get current harmonic gains for display/debugging
    const std::vector<float>& getCurrentHarmonicGains() const { return currentHarmonicGains; }

protected:
    float calculateHarmonicGain(int harmonicIndex, float shape) const;
    
private:
    void updatePolyEvalGains(const std::vector<float>& gains);
    
    std::atomic<float> currentPhase { 0.0f };
    float frequency { 440.0f };
    double sampleRate { 0.0 };
    
    static constexpr int numHarmonics { 16 };
    std::vector<float> currentHarmonicGains;
    
    // Controls how quickly harmonics roll off when shape parameter is < 1.0
    // Has no effect when shape = 1.0 (pure reciprocal rolloff)
    // Higher values = sharper rolloff
    float rolloffSharpness { 1.2f };
    
    PolyEvaluator polyEvaluator;
    juce::dsp::WaveShaper<float, std::function<float(float)>> waveshaper;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MuOscillator)
};

} // namespace rosy 