#pragma once

#include <JuceHeader.h>

class OddEvenOscillator : public juce::dsp::ProcessorBase
{
public:
    OddEvenOscillator();
    ~OddEvenOscillator() override = default;

    //==============================================================================
    void prepare(const juce::dsp::ProcessSpec& spec) override;
    void reset() override;
    void process(const juce::dsp::ProcessContextReplacing<float>& context) override;

private:
    std::atomic<float> currentPhase { 0.0f };
    double sampleRate { 0.0 };
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OddEvenOscillator)
}; 