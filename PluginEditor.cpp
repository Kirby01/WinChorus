#include "PluginEditor.h"

WinChorusAudioProcessorEditor::WinChorusAudioProcessorEditor (WinChorusAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    setSize (760, 380);

    titleLabel.setText ("WinChorus", juce::dontSendNotification);
    titleLabel.setJustificationType (juce::Justification::centred);
    titleLabel.setColour (juce::Label::textColourId, juce::Colours::white);
    titleLabel.setFont (juce::Font (28.0f, juce::Font::bold));
    addAndMakeVisible (titleLabel);

    setupSlider (inputSlider,    inputLabel,    "Input");
    setupSlider (outputSlider,   outputLabel,   "Output");
    setupSlider (mixSlider,      mixLabel,      "Mix");
    setupSlider (depthSlider,    depthLabel,    "Depth");
    setupSlider (rateSlider,     rateLabel,     "Rate");
    setupSlider (baseTapsSlider, baseTapsLabel, "Base Taps");
    setupSlider (tapSwingSlider, tapSwingLabel, "Tap Swing");
    setupSlider (boostSlider,    boostLabel,    "Boost");

    deltaButton.setButtonText ("Delta");
    deltaButton.setColour (juce::ToggleButton::textColourId, juce::Colours::white);
    addAndMakeVisible (deltaButton);

    inputAttachment    = std::make_unique<SliderAttachment> (audioProcessor.apvts, "input", inputSlider);
    outputAttachment   = std::make_unique<SliderAttachment> (audioProcessor.apvts, "output", outputSlider);
    mixAttachment      = std::make_unique<SliderAttachment> (audioProcessor.apvts, "mix", mixSlider);
    depthAttachment    = std::make_unique<SliderAttachment> (audioProcessor.apvts, "depth", depthSlider);
    rateAttachment     = std::make_unique<SliderAttachment> (audioProcessor.apvts, "rate", rateSlider);
    baseTapsAttachment = std::make_unique<SliderAttachment> (audioProcessor.apvts, "baseTaps", baseTapsSlider);
    tapSwingAttachment = std::make_unique<SliderAttachment> (audioProcessor.apvts, "tapSwing", tapSwingSlider);
    boostAttachment    = std::make_unique<SliderAttachment> (audioProcessor.apvts, "boost", boostSlider);
    deltaAttachment    = std::make_unique<ButtonAttachment> (audioProcessor.apvts, "delta", deltaButton);
}

WinChorusAudioProcessorEditor::~WinChorusAudioProcessorEditor() {}

void WinChorusAudioProcessorEditor::setupSlider (juce::Slider& slider, juce::Label& label, const juce::String& text)
{
    slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 80, 22);
    slider.setColour (juce::Slider::rotarySliderFillColourId, juce::Colours::skyblue);
    slider.setColour (juce::Slider::thumbColourId, juce::Colours::white);
    slider.setColour (juce::Slider::textBoxTextColourId, juce::Colours::white);
    slider.setColour (juce::Slider::textBoxBackgroundColourId, juce::Colours::black);
    addAndMakeVisible (slider);

    label.setText (text, juce::dontSendNotification);
    label.setJustificationType (juce::Justification::centred);
    label.setColour (juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible (label);
}

void WinChorusAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (18, 20, 28));

    auto area = getLocalBounds().toFloat().reduced (12.0f);
    g.setColour (juce::Colour (35, 38, 50));
    g.fillRoundedRectangle (area, 18.0f);

    g.setColour (juce::Colours::skyblue.withAlpha (0.35f));
    g.drawRoundedRectangle (area, 18.0f, 2.0f);

    g.setColour (juce::Colours::white.withAlpha (0.55f));
    g.setFont (14.0f);
    g.drawText ("Harmonious Records",
                getLocalBounds().removeFromBottom (32),
                juce::Justification::centred);
}

void WinChorusAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced (24);

    titleLabel.setBounds (area.removeFromTop (50));

    auto row1 = area.removeFromTop (135);
    auto row2 = area.removeFromTop (135);

    auto place = [] (juce::Rectangle<int> r, juce::Slider& s, juce::Label& l)
    {
        l.setBounds (r.removeFromTop (24));
        s.setBounds (r);
    };

    int w1 = row1.getWidth() / 4;
    place (row1.removeFromLeft (w1).reduced (8), inputSlider, inputLabel);
    place (row1.removeFromLeft (w1).reduced (8), mixSlider, mixLabel);
    place (row1.removeFromLeft (w1).reduced (8), depthSlider, depthLabel);
    place (row1.reduced (8), rateSlider, rateLabel);

    int w2 = row2.getWidth() / 4;
    place (row2.removeFromLeft (w2).reduced (8), baseTapsSlider, baseTapsLabel);
    place (row2.removeFromLeft (w2).reduced (8), tapSwingSlider, tapSwingLabel);
    place (row2.removeFromLeft (w2).reduced (8), boostSlider, boostLabel);
    place (row2.reduced (8), outputSlider, outputLabel);

    deltaButton.setBounds (getWidth() / 2 - 45, getHeight() - 58, 90, 28);
}