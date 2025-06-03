#pragma once
#include <JuceHeader.h>

namespace rosy {

class DbCalculator : public juce::dsp::ProcessorBase
{
public:
    DbCalculator();
    
    void prepare(const juce::dsp::ProcessSpec& spec) override;
    void process(const juce::dsp::ProcessContextReplacing<float>& context) override;
    void reset() override;
    
    // Get the peak level in dBFS since last reset
    float getPeakDb() const;
    
    // Reset the peak level
    void resetPeak();
    
private:
    std::atomic<float> peakLevel{0.0f};
    float sampleRate{44100.0f};
    
    // Track peak age
    std::atomic<int64_t> samplesSincePeak{0};
    int64_t samplesPerSecond{44100};
    
    // Decay constants
    static constexpr float decayDelay = 0.5f;     // Wait this long before starting decay
    static constexpr float decayFactor = 0.9f;    // How much to reduce peak per decay step
    static constexpr float decayIntervalSeconds = 0.1f;  // How often to apply decay
};

} // namespace rosy 