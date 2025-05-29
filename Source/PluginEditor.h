/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class HelloMidiAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    HelloMidiAudioProcessorEditor (HelloMidiAudioProcessor&);
    ~HelloMidiAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    HelloMidiAudioProcessor& audioProcessor;
    juce::Slider volSlider;
    juce::Slider panSlider;

    // Slider attachments handle the connections between sliders and parameters
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> volSliderAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> panSliderAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HelloMidiAudioProcessorEditor)
};
