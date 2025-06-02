/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "MuOscillator.h"

//==============================================================================
/**
*/
class RosemaryAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    RosemaryAudioProcessor();
    ~RosemaryAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState& getParameters() { return parameters; }
    float getVolume() const { return *volumeParameter; }

private:
    //==============================================================================
    // Value tree state for parameter management
    juce::AudioProcessorValueTreeState parameters;
    
    // References to parameters for real-time audio processing
    std::atomic<float>* volumeParameter = nullptr;
    std::atomic<float>* panParameter = nullptr;

    // Oscillator state
    double currentPhase = 0.0;
    double phaseIncrement = 0.0;
    const double frequency = 500.0; // Hz
    float getNextSample();  // Returns sawtooth wave

    // MuOscillator
    rosy::MuOscillator muOscillator;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RosemaryAudioProcessor)
};
