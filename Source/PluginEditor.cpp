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
    volSlider.setRange(0.0, 1.0, 0.01);  // Range 0-1 for direct volume control
    volSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 90, 20);
    volSlider.setTextValueSuffix(" Volume");
    volSlider.setMouseDragSensitivity(250); // Higher = more fine control

    // Create the attachment - this handles all the value sync automatically
    volSliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getParameters(), "volume", volSlider);

    // pan rotary slider setup
    panSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    panSlider.setRange(0.0, 1.0, 0.01);  // Range 0-1 for direct pan control
    panSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 90, 20);
    panSlider.setTextValueSuffix(" Pan");
    panSlider.setMouseDragSensitivity(250);  // Higher = more fine control
    panSlider.setDoubleClickReturnValue(true, 0.5f);  // Double-click to center

    // Create the attachment for the pan slider
    panSliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getParameters(), "pan", panSlider);

    addAndMakeVisible(&volSlider);
    addAndMakeVisible(&panSlider);
}

HelloMidiAudioProcessorEditor::~HelloMidiAudioProcessorEditor()
{
}

//==============================================================================
void HelloMidiAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    // g.setColour (juce::Colours::white);
    // g.setFont (juce::FontOptions (15.0f));
    // g.drawFittedText ("I think this is title text?", getLocalBounds(), juce::Justification::centred, 1);
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
    
    // Position the pan slider below the volume slider
    const int panY = margin + volSliderHeight + margin;  // margin below volume slider
    const int panX = (getWidth() - rotarySize) / 2;  // center horizontally
    panSlider.setBounds(panX, panY, rotarySize, rotarySize);
}
