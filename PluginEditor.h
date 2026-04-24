#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class WinChorusAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit WinChorusAudioProcessorEditor (WinChorusAudioProcessor&);
    ~WinChorusAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;

    WinChorusAudioProcessor& audioProcessor;

    juce::Label titleLabel;

    juce::Slider inputSlider, outputSlider, mixSlider, depthSlider, rateSlider;
    juce::Slider baseTapsSlider, tapSwingSlider, boostSlider;

    juce::Label inputLabel, outputLabel, mixLabel, depthLabel, rateLabel;
    juce::Label baseTapsLabel, tapSwingLabel, boostLabel;

    juce::ToggleButton deltaButton;

    std::unique_ptr<SliderAttachment> inputAttachment, outputAttachment, mixAttachment;
    std::unique_ptr<SliderAttachment> depthAttachment, rateAttachment;
    std::unique_ptr<SliderAttachment> baseTapsAttachment, tapSwingAttachment, boostAttachment;
    std::unique_ptr<ButtonAttachment> deltaAttachment;

    void setupSlider (juce::Slider& slider, juce::Label& label, const juce::String& text);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WinChorusAudioProcessorEditor)
};