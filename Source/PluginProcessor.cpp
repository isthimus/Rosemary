/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
HelloMidiAudioProcessor::HelloMidiAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ),
#endif
    // Initialize the value tree state
    parameters(*this, nullptr, "PARAMETERS",
        {
            // Create parameter with std::make_unique
            std::make_unique<juce::AudioParameterFloat>(
                "volume",     // parameter ID
                "Volume",     // parameter name
                0.0f,        // minimum value
                1.0f,        // maximum value
                0.0f        // default value - starting with silence
            ),
            std::make_unique<juce::AudioParameterFloat>(
                "pan",       // parameter ID
                "Pan",       // parameter name
                0.0f,       // minimum value
                1.0f,       // maximum value
                0.5f       // default value (center)
            )
        })
{
    // Get pointers to the atomic parameters for real-time audio processing
    volumeParameter = parameters.getRawParameterValue("volume");
    panParameter = parameters.getRawParameterValue("pan");
}

HelloMidiAudioProcessor::~HelloMidiAudioProcessor()
{
}

//==============================================================================
const juce::String HelloMidiAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool HelloMidiAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool HelloMidiAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool HelloMidiAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double HelloMidiAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int HelloMidiAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int HelloMidiAudioProcessor::getCurrentProgram()
{
    return 0;
}

void HelloMidiAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String HelloMidiAudioProcessor::getProgramName (int index)
{
    return {};
}

void HelloMidiAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void HelloMidiAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Calculate phase increment for our sawtooth
    phaseIncrement = frequency / sampleRate;
}

void HelloMidiAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool HelloMidiAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

// Helper function to generate sawtooth wave
float HelloMidiAudioProcessor::getNextSample()
{
    // Update phase
    currentPhase += phaseIncrement;
    if (currentPhase >= 1.0)
        currentPhase -= 1.0;
    
    // Convert phase (0 to 1) to amplitude (-1 to 1)
    return (float)(2.0 * currentPhase - 1.0);
}

void HelloMidiAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // Clear any unused channels
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    const float currentVol = *volumeParameter;
    const float pan = *panParameter;

    // Generate one buffer of samples that we'll use for all channels
    juce::AudioBuffer<float> sawBuffer(1, buffer.getNumSamples());
    auto* sawData = sawBuffer.getWritePointer(0);
    for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
    {
        sawData[sample] = getNextSample();
    }

    if (totalNumOutputChannels == 1)
    {
        // Mono output - just apply volume
        auto* channelData = buffer.getWritePointer(0);
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            channelData[sample] = sawData[sample] * currentVol;
        }
    }
    else if (totalNumOutputChannels == 2)
    {
        // Stereo output - apply equal power panning
        auto* leftChannel = buffer.getWritePointer(0);
        auto* rightChannel = buffer.getWritePointer(1);
        
        // Equal power panning using sin/cos
        const float panRadians = pan * juce::MathConstants<float>::halfPi;
        const float leftGain = std::cos(panRadians) * currentVol;
        const float rightGain = std::sin(panRadians) * currentVol;

        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            leftChannel[sample] = sawData[sample] * leftGain;
            rightChannel[sample] = sawData[sample] * rightGain;
        }
    }
    else
    {
        // Multichannel output - spread the sawtooth across all channels
        for (int channel = 0; channel < totalNumOutputChannels; ++channel)
        {
            auto* channelData = buffer.getWritePointer(channel);
            // Even channels get left-biased gain, odd channels get right-biased
            const float panRadians = pan * juce::MathConstants<float>::halfPi;
            float channelGain = (channel % 2 == 0) 
                ? std::cos(panRadians) * currentVol
                : std::sin(panRadians) * currentVol;

            for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
            {
                channelData[sample] = sawData[sample] * channelGain;
            }
        }
    }
}

//==============================================================================
bool HelloMidiAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* HelloMidiAudioProcessor::createEditor()
{
    return new HelloMidiAudioProcessorEditor (*this);
}

//==============================================================================
void HelloMidiAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void HelloMidiAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new HelloMidiAudioProcessor();
}
