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
        slider.setRange(0.0, 1.0, 0.005);  // Finer step size for all sliders
        slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 90, 20);
        slider.setTextValueSuffix(suffix);
        slider.setMouseDragSensitivity(250);
        slider.setDoubleClickReturnValue(true, 0.5f);
        slider.setVelocityBasedMode(true);
        slider.setVelocityModeParameters(0.8, 1, 0.07, false);  // Smooth control for all sliders
    };

    // Setup volume slider with special range
    setupRotarySlider(volSlider, " Volume");
    volSlider.setRange(0.0, 0.25, 0.005);  // Only volume needs the lower max value
    volSliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getParameters(), "volume", volSlider);

    // Setup other sliders (they'll use the common settings from setupRotarySlider)
    setupRotarySlider(panSlider, " Pan");
    panSliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getParameters(), "pan", panSlider);

    setupRotarySlider(pitchSlider, " Pitch");
    pitchSliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getParameters(), "pitch", pitchSlider);

    setupRotarySlider(shapeXSlider, " Shape X");
    shapeXSliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getParameters(), "shapeX", shapeXSlider);

    setupRotarySlider(shapeYSlider, " Shape Y");
    shapeYSliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getParameters(), "shapeY", shapeYSlider);

    // Add all sliders to the editor
    addAndMakeVisible(&volSlider);
    addAndMakeVisible(&panSlider);
    addAndMakeVisible(&pitchSlider);
    addAndMakeVisible(&shapeXSlider);
    addAndMakeVisible(&shapeYSlider);

    // Setup harmonics display
    harmonicsLabel.setJustificationType(juce::Justification::left);
    harmonicsLabel.setFont(juce::Font(juce::FontOptions().withHeight(14.0f)));
    harmonicsLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(harmonicsLabel);
    
    // Setup peak level displays
    auto setupPeakLabel = [this](juce::Label& label) {
        label.setJustificationType(juce::Justification::left);
        label.setFont(juce::Font(juce::FontOptions().withHeight(14.0f)));
        label.setColour(juce::Label::textColourId, juce::Colours::white);
        addAndMakeVisible(label);
    };
    
    setupPeakLabel(preVolumePeakLabel);
    setupPeakLabel(postVolumePeakLabel);
    
    // Start timer to update display (10Hz)
    startTimerHz(10);
}

RosemaryAudioProcessorEditor::~RosemaryAudioProcessorEditor()
{
    stopTimer();
}

void RosemaryAudioProcessorEditor::timerCallback()
{
    // Get current gains and format them for display
    const auto& gains = audioProcessor.getCurrentHarmonicGains();
    juce::String text = "Harmonic Gains:\n";
    for (size_t i = 0; i < gains.size(); ++i)
    {
        text += "H" + juce::String(i) + ": " + juce::String(gains[i], 3);
        if (i < gains.size() - 1) text += "\n";
    }
    harmonicsLabel.setText(text, juce::dontSendNotification);
    
    // Update peak level displays
    float preVolumeDb = audioProcessor.getCurrentPreVolumeDb();
    juce::String preVolumeText = "Peak Level (pre volume):\n";
    preVolumeText += juce::String(preVolumeDb, 1) + " dBFS";
    preVolumePeakLabel.setText(preVolumeText, juce::dontSendNotification);
    
    float postVolumeDb = audioProcessor.getCurrentPostVolumeDb();
    juce::String postVolumeText = "Peak Level (post volume):\n";
    postVolumeText += juce::String(postVolumeDb, 1) + " dBFS";
    postVolumePeakLabel.setText(postVolumeText, juce::dontSendNotification);
    
    repaint();  // Ensure the display updates
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

    // Reserve space for harmonics and peak level displays on the right
    auto rightPanel = bounds.removeFromRight(150);
    
    // Layout the meters vertically
    auto preVolumeMeterArea = rightPanel.removeFromTop(40);
    auto postVolumeMeterArea = rightPanel.removeFromTop(40);
    auto harmonicsArea = rightPanel;
    
    preVolumePeakLabel.setBounds(preVolumeMeterArea);
    postVolumePeakLabel.setBounds(postVolumeMeterArea);
    harmonicsLabel.setBounds(harmonicsArea);

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
