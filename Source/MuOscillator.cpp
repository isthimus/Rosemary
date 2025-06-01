#include "MuOscillator.h"

namespace rosy {

MuOscillator::MuOscillator()
{
    // Initialize with a simple sine wave (first harmonic only)
    setHarmonicGains({1.0f});
}

void MuOscillator::prepare(const juce::dsp::ProcessSpec& spec)
{
    sampleRate = spec.sampleRate;
    currentPhase.store(0.0f);
    waveshaper.prepare(spec);
}

void MuOscillator::reset()
{
    currentPhase.store(0.0f);
    waveshaper.reset();
}

void MuOscillator::process(const juce::dsp::ProcessContextReplacing<float>& context)
{
    auto& outputBlock = context.getOutputBlock();
    const float phaseIncrement = frequency / static_cast<float>(sampleRate);

    // Generate phase-based sine wave
    for (size_t sample = 0; sample < outputBlock.getNumSamples(); ++sample)
    {
        float phase = currentPhase.load();
        
        // Generate sine wave using JUCE's fast approximation
        const float sineValue = juce::dsp::FastMathApproximations::sin(2.0f * juce::MathConstants<float>::pi * phase);
        
        // Write to all channels
        for (int channel = 0; channel < static_cast<int>(outputBlock.getNumChannels()); ++channel)
            outputBlock.setSample(channel, sample, sineValue);
        
        // Update phase
        phase += phaseIncrement;
        if (phase >= 1.0f)
            phase -= 1.0f;
            
        currentPhase.store(phase);
    }

    // Apply waveshaping
    waveshaper.process(context);
}

void MuOscillator::setHarmonicGains(const std::vector<float>& gains)
{
    polyEvaluator.setCoefficients(HarmonicProfileCalculator::calculateAllCoefficients(gains));
}

void MuOscillator::setFrequency(float freq)
{
    if (sampleRate > 0.0)
    {
        float nyquist = static_cast<float>(sampleRate) * 0.5f;
        frequency = std::min(freq, nyquist);
    }
    else
    {
        frequency = freq;  // Can't clamp yet, no sample rate
    }
}

} // namespace rosy 