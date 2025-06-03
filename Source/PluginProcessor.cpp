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
                juce::NormalisableRange<float>(0.0f, 1.0f, 0.005f, 0.2f),  // range with step size and skew factor for logarithmic
                0.5f         // default value - starting at half volume
            ),
            std::make_unique<juce::AudioParameterFloat>(
                "pan",       // parameter ID
                "Pan",       // parameter name
                juce::NormalisableRange<float>(0.0f, 1.0f, 0.005f),  // range with step size
                0.5f        // default value (center)
            ),
            std::make_unique<juce::AudioParameterFloat>(
                "pitch",     // parameter ID
                "Pitch",     // parameter name
                juce::NormalisableRange<float>(0.0f, 1.0f, 0.005f),  // range with step size
                0.5f        // default value
            ),
            std::make_unique<juce::AudioParameterFloat>(
                "shapeX",    // parameter ID
                "Shape X",   // parameter name
                juce::NormalisableRange<float>(0.0f, 1.0f, 0.005f),  // range with step size
                0.5f        // default value
            ),
            std::make_unique<juce::AudioParameterFloat>(
                "shapeY",    // parameter ID
                "Shape Y",   // parameter name
                juce::NormalisableRange<float>(0.0f, 1.0f, 0.005f),  // range with step size
                0.5f        // default value
            )
        })
{
    // Get pointers to the atomic parameters for real-time audio processing
    volumeParameter = parameters.getRawParameterValue("volume");
    panParameter = parameters.getRawParameterValue("pan");

    // Add listeners for shape parameters
    parameters.addParameterListener("shapeX", this);
    parameters.addParameterListener("shapeY", this);
}

RosemaryAudioProcessor::~RosemaryAudioProcessor()
{
    parameters.removeParameterListener("shapeX", this);
    parameters.removeParameterListener("shapeY", this);
}

void RosemaryAudioProcessor::parameterChanged(const juce::String& parameterID, float newValue)
{
    if (parameterID == "shapeX")
        muOscillator.setShapeX(newValue);
    else if (parameterID == "shapeY")
        muOscillator.setShapeY(newValue);
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
    
    // Prepare peak level calculators
    preVolumePeakCalculator.prepare(spec);
    postVolumePeakCalculator.prepare(spec);
}

void RosemaryAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
    preVolumePeakCalculator.reset();
    postVolumePeakCalculator.reset();
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

    // Clear any output channels that don't contain input data
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    const float currentVol = *volumeParameter;
    const float pan = *panParameter;

    // Create an audio block and context for processing
    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);
    
    // Process the oscillator
    muOscillator.process(context);
    
    // Measure pre-volume peak level
    preVolumePeakCalculator.process(context);
    
    // Apply volume and panning
    if (totalNumOutputChannels == 2)
    {
        // Equal power panning using sin/cos for stereo
        const float panRadians = pan * juce::MathConstants<float>::halfPi;
        const float leftGain = std::cos(panRadians) * currentVol;
        const float rightGain = std::sin(panRadians) * currentVol;

        buffer.applyGain(0, 0, buffer.getNumSamples(), leftGain);
        buffer.applyGain(1, 0, buffer.getNumSamples(), rightGain);
    }
    else
    {
        // For mono, just apply volume
        buffer.applyGain(currentVol);
    }
    
    // Measure post-volume peak level
    postVolumePeakCalculator.process(context);
    
    // Reset peak meters every second (assuming 10Hz refresh rate in the UI)
    static int sampleCount = 0;
    sampleCount += buffer.getNumSamples();
    if (sampleCount >= getSampleRate() / 10)
    {
        preVolumePeakCalculator.resetPeak();
        postVolumePeakCalculator.resetPeak();
        sampleCount = 0;
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
