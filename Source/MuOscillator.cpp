#include "MuOscillator.h"

namespace rosy {

MuOscillator::MuOscillator()
{
    // Initialize with first harmonic only (fundamental frequency)
    currentHarmonicGains.resize(numHarmonics, 0.0f);
    currentHarmonicGains[0] = 1.0f;
    updatePolyEvalGains(currentHarmonicGains);
}

void MuOscillator::prepare(const juce::dsp::ProcessSpec& spec)
{
    sampleRate = spec.sampleRate;
    currentPhase.store(0.0f);
    waveshaper.prepare(spec);
    
    // Bind the polyEvaluator to the waveshaper
    waveshaper.functionToUse = [this](float x) { return polyEvaluator(x); };
    
    // Clamp frequency now that we have a valid sample rate
    float nyquist = static_cast<float>(sampleRate) * 0.5f;
    frequency = std::min(frequency, nyquist);
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

void MuOscillator::updatePolyEvalGains(const std::vector<float>& gains)
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

float MuOscillator::calculateHarmonicGain(int harmonicIndex, float shape) const
{
    // Clamp shape between 0 and 1
    shape = std::max(0.0f, std::min(1.0f, shape));
    
    // When shape is 0, gain is 0
    // When shape is 1, gain is 1/(i+1)
    // In between, we get a sharper rolloff controlled by rolloffSharpness
    return std::pow(shape, (harmonicIndex + 1) * rolloffSharpness / 2.0f) / (harmonicIndex + 1);
}

void MuOscillator::setShapeX(float x)
{
    // Shape even harmonics (indices 1, 3, 5, ...)
    for (int i = 1; i < numHarmonics; i += 2)
    {
        currentHarmonicGains[i] = calculateHarmonicGain(i, x);
    }
    updatePolyEvalGains(currentHarmonicGains);
}

void MuOscillator::setShapeY(float y)
{
    // Shape odd harmonics (indices 2, 4, 6, ...)
    // Note: We skip index 0 (fundamental) as it stays at 1.0
    for (int i = 2; i < numHarmonics; i += 2)
    {
        currentHarmonicGains[i] = calculateHarmonicGain(i, y);
    }
    updatePolyEvalGains(currentHarmonicGains);
}

} // namespace rosy 