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
class RosemaryAudioProcessorEditor  : public juce::AudioProcessorEditor,
                                     public juce::Timer
{
public:
    RosemaryAudioProcessorEditor (RosemaryAudioProcessor&);
    ~RosemaryAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    
    // Timer callback to update the harmonic gains display
    void timerCallback() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    RosemaryAudioProcessor& audioProcessor;
    
    // Sliders
    juce::Slider volSlider;
    juce::Slider panSlider;
    juce::Slider pitchSlider;
    juce::Slider shapeXSlider;
    juce::Slider shapeYSlider;

    juce::Label harmonicsLabel;  // Display for harmonic gains
    juce::Label preVolumePeakLabel;   // Display for pre-volume peak level
    juce::Label postVolumePeakLabel;  // Display for post-volume peak level

    // Slider attachments handle the connections between sliders and parameters
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> volSliderAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> panSliderAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> pitchSliderAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> shapeXSliderAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> shapeYSliderAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RosemaryAudioProcessorEditor)
};
