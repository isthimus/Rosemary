#include "DbCalculator.h"

namespace rosy {

DbCalculator::DbCalculator()
{
}

void DbCalculator::prepare(const juce::dsp::ProcessSpec& spec)
{
    sampleRate = static_cast<float>(spec.sampleRate);
    samplesPerSecond = static_cast<int64_t>(spec.sampleRate);
    reset();
}

void DbCalculator::process(const juce::dsp::ProcessContextReplacing<float>& context)
{
    auto& inputBlock = context.getInputBlock();
    const int numSamples = static_cast<int>(inputBlock.getNumSamples());
    
    // Find peak level across all channels in this block
    float blockPeak = 0.0f;
    for (int channel = 0; channel < static_cast<int>(inputBlock.getNumChannels()); ++channel)
    {
        const float* channelData = inputBlock.getChannelPointer(channel);
        for (int sample = 0; sample < numSamples; ++sample)
        {
            blockPeak = std::max(blockPeak, std::abs(channelData[sample]));
        }
    }
    
    // Update sample count
    int64_t currentSampleCount = samplesSincePeak.load();
    currentSampleCount += numSamples;
    
    // Only start decay after the delay period
    int64_t samplesForDelay = static_cast<int64_t>(decayDelay * samplesPerSecond);
    if (currentSampleCount > samplesForDelay)
    {
        // Check if we need to decay (every decayIntervalSeconds after the delay)
        int64_t samplesPerDecay = static_cast<int64_t>(decayIntervalSeconds * samplesPerSecond);
        int64_t samplesAfterDelay = currentSampleCount - samplesForDelay;
        
        if (samplesAfterDelay >= samplesPerDecay)
        {
            float currentPeak = peakLevel.load();
            float decayedPeak = currentPeak * decayFactor;
            peakLevel.store(decayedPeak);
            // Reset to just after delay so we maintain proper decay timing
            currentSampleCount = samplesForDelay + (samplesAfterDelay % samplesPerDecay);
        }
    }
    
    // If this block's peak is higher than current peak, update it and reset counter
    float currentPeak = peakLevel.load();
    if (blockPeak > currentPeak)
    {
        peakLevel.store(blockPeak);
        currentSampleCount = 0;  // Reset age counter for new peak
    }
    
    samplesSincePeak.store(currentSampleCount);
}

void DbCalculator::reset()
{
    resetPeak();
    samplesSincePeak.store(0);
}

float DbCalculator::getPeakDb() const
{
    float peak = peakLevel.load();
    if (peak < 1e-10f) // -200 dB
        return -200.0f;
    return 20.0f * std::log10(peak);
}

void DbCalculator::resetPeak()
{
    peakLevel.store(0.0f);
    samplesSincePeak.store(0);
}

} // namespace rosy 