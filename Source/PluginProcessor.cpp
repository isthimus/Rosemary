/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
RosemaryAudioProcessor::RosemaryAudioProcessor()
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
                0.5f        // default value - starting at half volume
            ),
            std::make_unique<juce::AudioParameterFloat>(
                "pan",       // parameter ID
                "Pan",       // parameter name
                0.0f,       // minimum value
                1.0f,       // maximum value
                0.5f       // default value (center)
            ),
            std::make_unique<juce::AudioParameterFloat>(
                "pitch",     // parameter ID
                "Pitch",     // parameter name
                0.0f,       // minimum value
                1.0f,       // maximum value
                0.5f       // default value
            ),
            std::make_unique<juce::AudioParameterFloat>(
                "shapeX",    // parameter ID
                "Shape X",   // parameter name
                0.0f,       // minimum value
                1.0f,       // maximum value
                0.5f       // default value
            ),
            std::make_unique<juce::AudioParameterFloat>(
                "shapeY",    // parameter ID
                "Shape Y",   // parameter name
                0.0f,       // minimum value
                1.0f,       // maximum value
                0.5f       // default value
            )
        })
{
    // Get pointers to the atomic parameters for real-time audio processing
    volumeParameter = parameters.getRawParameterValue("volume");
    panParameter = parameters.getRawParameterValue("pan");
}

RosemaryAudioProcessor::~RosemaryAudioProcessor()
{
}

//==============================================================================
const juce::String RosemaryAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool RosemaryAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool RosemaryAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool RosemaryAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double RosemaryAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int RosemaryAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int RosemaryAudioProcessor::getCurrentProgram()
{
    return 0;
}

void RosemaryAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String RosemaryAudioProcessor::getProgramName (int index)
{
    return {};
}

void RosemaryAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void RosemaryAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Calculate phase increment for our sawtooth
    phaseIncrement = frequency / sampleRate;

    // Prepare MuOscillator
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = getTotalNumOutputChannels();
    
    muOscillator.prepare(spec);
    muOscillator.setFrequency(static_cast<float>(frequency));
}

void RosemaryAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool RosemaryAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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
float RosemaryAudioProcessor::getNextSample()
{
    // Update phase
    currentPhase += phaseIncrement;
    if (currentPhase >= 1.0)
        currentPhase -= 1.0;
    
    // Convert phase (0 to 1) to amplitude (-1 to 1)
    return (float)(2.0 * currentPhase - 1.0);
}

void RosemaryAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // Only clear input channels if we have more outputs than inputs
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    const float currentVol = *volumeParameter;
    const float pan = *panParameter;

    // Convert AudioBuffer to AudioBlock
    juce::dsp::AudioBlock<float> block(buffer);
    
    // Create a ProcessContext that will replace the output
    juce::dsp::ProcessContextReplacing<float> context(block);
    
    // Process the audio through muOscillator
    muOscillator.process(context);

    // Debug: Check if we have any non-zero samples after oscillator
    bool hasSignal = false;
    for (int channel = 0; channel < totalNumOutputChannels && !hasSignal; ++channel) {
        auto* channelData = buffer.getReadPointer(channel);
        for (int sample = 0; sample < buffer.getNumSamples() && !hasSignal; ++sample) {
            if (std::abs(channelData[sample]) > 0.0001f) {
                hasSignal = true;
                DBG("Signal detected after oscillator: " << channelData[sample] << " at sample " << sample);
            }
        }
    }
    
    // Apply volume and panning
    if (totalNumOutputChannels == 2)
    {
        // Equal power panning using sin/cos for stereo
        const float panRadians = pan * juce::MathConstants<float>::halfPi;
        const float leftGain = std::cos(panRadians) * currentVol;
        const float rightGain = std::sin(panRadians) * currentVol;

        DBG("Volume: " << currentVol << " Pan: " << pan << " Left gain: " << leftGain << " Right gain: " << rightGain);

        buffer.applyGain(0, 0, buffer.getNumSamples(), leftGain);
        buffer.applyGain(1, 0, buffer.getNumSamples(), rightGain);

        // Debug: Check if we have any non-zero samples after volume
        hasSignal = false;
        for (int channel = 0; channel < totalNumOutputChannels && !hasSignal; ++channel) {
            auto* channelData = buffer.getReadPointer(channel);
            for (int sample = 0; sample < buffer.getNumSamples() && !hasSignal; ++sample) {
                if (std::abs(channelData[sample]) > 0.0001f) {
                    hasSignal = true;
                    DBG("Signal detected after volume: " << channelData[sample] << " at sample " << sample);
                }
            }
        }
    }
    else
    {
        buffer.applyGain(currentVol);
    }
}

//==============================================================================
bool RosemaryAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* RosemaryAudioProcessor::createEditor()
{
    return new RosemaryAudioProcessorEditor (*this);
}

//==============================================================================
void RosemaryAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void RosemaryAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new RosemaryAudioProcessor();
}
