/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
HelloMidiAudioProcessorEditor::HelloMidiAudioProcessorEditor (HelloMidiAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (400, 400);
    
    // Make the window resizable with a minimum and maximum size
    setResizable(true, true);
    setResizeLimits(300, 250, 800, 600);

    // slider bambino for the westy testy
    volSlider.setSliderStyle(juce::Slider::LinearBar);
    volSlider.setRange(0.0, 127.0, 1.0);
    volSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 90, 20);
    volSlider.setTextValueSuffix(" Volume");
    volSlider.setValue(1.0);

    // phase rotary slider setup
    phaseSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    phaseSlider.setRange(0.0, 360.0, 1.0);
    phaseSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 90, 20);
    phaseSlider.setTextValueSuffix(" Phase");
    phaseSlider.setValue(0.0);

    addAndMakeVisible(&volSlider);
    addAndMakeVisible(&phaseSlider);
}

HelloMidiAudioProcessorEditor::~HelloMidiAudioProcessorEditor()
{
}

//==============================================================================
void HelloMidiAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (juce::FontOptions (15.0f));
    g.drawFittedText ("I think this is title text?", getLocalBounds(), juce::Justification::centred, 1);
}

void HelloMidiAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..

    // Calculate proportional bounds for the sliders
    const int margin = 20;  // margin from edges
    const int volSliderHeight = 30;  // height for horizontal slider
    const int rotarySize = 100;  // size for the rotary slider

    // Position the volume slider at the top
    volSlider.setBounds(margin, margin, getWidth() - (margin * 2), volSliderHeight);
    
    // Position the phase slider below the volume slider
    const int phaseY = margin + volSliderHeight + margin;  // margin below volume slider
    const int phaseX = (getWidth() - rotarySize) / 2;  // center horizontally
    phaseSlider.setBounds(phaseX, phaseY, rotarySize, rotarySize);
}
