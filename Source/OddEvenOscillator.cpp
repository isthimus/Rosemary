#include "OddEvenOscillator.h"

namespace rosy {

OddEvenOscillator::OddEvenOscillator()
{
}

void OddEvenOscillator::prepare(const juce::dsp::ProcessSpec& spec)
{
    sampleRate = spec.sampleRate;
    currentPhase.store(0.0f);
}

void OddEvenOscillator::reset()
{
    currentPhase.store(0.0f);
}

void OddEvenOscillator::process(const juce::dsp::ProcessContextReplacing<float>& context)
{
    auto& outputBlock = context.getOutputBlock();
    const double frequency = 500.0; // Hz
    const double phaseIncrement = frequency / sampleRate;

    for (size_t sample = 0; sample < outputBlock.getNumSamples(); ++sample)
    {
        float phase = currentPhase.load();
        
        // Generate sine wave using JUCE's fast approximation
        const float sineValue = juce::dsp::FastMathApproximations::sin(2.0f * juce::MathConstants<float>::pi * phase);
        
        // Write to all channels
        for (int channel = 0; channel < static_cast<int>(outputBlock.getNumChannels()); ++channel)
            outputBlock.setSample(channel, static_cast<int>(sample), sineValue);
        
        // Update phase
        phase += static_cast<float>(phaseIncrement);
        if (phase >= 1.0f)
            phase -= 1.0f;
            
        currentPhase.store(phase);
    }
}

} // namespace rosy 