/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
RosemaryAudioProcessorEditor::RosemaryAudioProcessorEditor (RosemaryAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (600, 400);
    
    // Make the window resizable with a minimum and maximum size
    setResizable(true, true);
    setResizeLimits(400, 300, 800, 600);

    // Common slider settings for all rotary sliders
    auto setupRotarySlider = [](juce::Slider& slider, const juce::String& suffix)
    {
        slider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
        slider.setRange(0.0, 1.0, 0.01);
        slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 90, 20);
        slider.setTextValueSuffix(suffix);
        slider.setMouseDragSensitivity(250);
        slider.setDoubleClickReturnValue(true, 0.5f);
    };

    // Setup volume slider
    setupRotarySlider(volSlider, " Volume");
    volSliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getParameters(), "volume", volSlider);

    // Setup pan slider
    setupRotarySlider(panSlider, " Pan");
    panSliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getParameters(), "pan", panSlider);

    // Setup pitch slider
    setupRotarySlider(pitchSlider, " Pitch");
    pitchSliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getParameters(), "pitch", pitchSlider);

    // Setup shapeX slider
    setupRotarySlider(shapeXSlider, " Shape X");
    shapeXSliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getParameters(), "shapeX", shapeXSlider);

    // Setup shapeY slider
    setupRotarySlider(shapeYSlider, " Shape Y");
    shapeYSliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getParameters(), "shapeY", shapeYSlider);

    // Add all sliders to the editor
    addAndMakeVisible(&volSlider);
    addAndMakeVisible(&panSlider);
    addAndMakeVisible(&pitchSlider);
    addAndMakeVisible(&shapeXSlider);
    addAndMakeVisible(&shapeYSlider);
}

RosemaryAudioProcessorEditor::~RosemaryAudioProcessorEditor()
{
}

//==============================================================================
void RosemaryAudioProcessorEditor::paint (juce::Graphics& g)
{
    // Fill the background
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}

void RosemaryAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();
    const int margin = 20;
    bounds.reduce(margin, margin);

    // Create the main vertical flexbox
    juce::FlexBox mainBox;
    mainBox.flexDirection = juce::FlexBox::Direction::column;
    mainBox.justifyContent = juce::FlexBox::JustifyContent::center;
    mainBox.alignContent = juce::FlexBox::AlignContent::center;

    // Create top row flexbox
    juce::FlexBox topRow;
    topRow.flexDirection = juce::FlexBox::Direction::row;
    topRow.justifyContent = juce::FlexBox::JustifyContent::spaceBetween;
    topRow.items.add(juce::FlexItem(volSlider).withFlex(1));
    topRow.items.add(juce::FlexItem(panSlider).withFlex(1));
    topRow.items.add(juce::FlexItem(pitchSlider).withFlex(1));

    // Create bottom row flexbox
    juce::FlexBox bottomRow;
    bottomRow.flexDirection = juce::FlexBox::Direction::row;
    bottomRow.justifyContent = juce::FlexBox::JustifyContent::center;
    bottomRow.items.add(juce::FlexItem(shapeXSlider).withFlex(1));
    bottomRow.items.add(juce::FlexItem(shapeYSlider).withFlex(1));

    // Add the rows to the main box with equal flex
    mainBox.items.add(juce::FlexItem(topRow).withFlex(1));
    mainBox.items.add(juce::FlexItem(bottomRow).withFlex(1));

    // Perform the layout
    mainBox.performLayout(bounds);
}
