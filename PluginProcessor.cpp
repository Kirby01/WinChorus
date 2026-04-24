#include "PluginProcessor.h"
#include "PluginEditor.h"

WinChorusAudioProcessor::WinChorusAudioProcessor()
    : AudioProcessor (BusesProperties()
        .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
        .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "PARAMETERS", createParameterLayout())
{
}

WinChorusAudioProcessor::~WinChorusAudioProcessor() {}

const juce::String WinChorusAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool WinChorusAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* WinChorusAudioProcessor::createEditor()
{
    return new WinChorusAudioProcessorEditor (*this);
}

bool WinChorusAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    return layouts.getMainInputChannelSet()  == juce::AudioChannelSet::stereo()
        && layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
}

void WinChorusAudioProcessor::prepareToPlay (double sampleRate, int)
{
    sr = sampleRate;

    bufferSize = 4096;
    bufferL.assign ((size_t) bufferSize, 0.0f);
    bufferR.assign ((size_t) bufferSize, 0.0f);

    writePos = 0;
    phase = 0.0f;
    lowL = 0.0f;
    lowR = 0.0f;
}

void WinChorusAudioProcessor::releaseResources() {}

juce::AudioProcessorValueTreeState::ParameterLayout WinChorusAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back (std::make_unique<juce::AudioParameterFloat>("input",  "Input",  0.0f, 4.0f, 1.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat>("output", "Output", 0.0f, 4.0f, 1.0f));

    params.push_back (std::make_unique<juce::AudioParameterFloat>("mix",   "Mix",   0.0f, 1.0f, 0.75f));
    params.push_back (std::make_unique<juce::AudioParameterFloat>("depth", "Depth", 0.0f, 3.0f, 1.4f));

    params.push_back (std::make_unique<juce::AudioParameterFloat>("rate", "Rate Hz",
        juce::NormalisableRange<float> (0.01f, 5.0f, 0.01f), 0.35f));

    params.push_back (std::make_unique<juce::AudioParameterInt>("baseTaps", "Base Taps", 4, 64, 10));
    params.push_back (std::make_unique<juce::AudioParameterInt>("tapSwing", "Tap Swing", 0, 120, 60));

    params.push_back (std::make_unique<juce::AudioParameterFloat>("boost", "Boost", 0.0f, 5.0f, 1.8f));

    params.push_back (std::make_unique<juce::AudioParameterBool>("delta", "Delta", false));

    return { params.begin(), params.end() };
}

void WinChorusAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    const int numSamples = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();

    auto* left  = buffer.getWritePointer (0);
    auto* right = numChannels > 1 ? buffer.getWritePointer (1) : nullptr;

    const float input    = apvts.getRawParameterValue ("input")->load();
    const float output   = apvts.getRawParameterValue ("output")->load();
    const float mix      = apvts.getRawParameterValue ("mix")->load();
    const float depth    = apvts.getRawParameterValue ("depth")->load();
    const float rate     = apvts.getRawParameterValue ("rate")->load();
    const int baseTaps   = (int) apvts.getRawParameterValue ("baseTaps")->load();
    const int tapSwing   = (int) apvts.getRawParameterValue ("tapSwing")->load();
    const float boost    = apvts.getRawParameterValue ("boost")->load();
    const bool delta     = apvts.getRawParameterValue ("delta")->load() > 0.5f;

    const float twoPi = juce::MathConstants<float>::twoPi;

    for (int n = 0; n < numSamples; ++n)
    {
        float inL = left[n] * input;
        float inR = right != nullptr ? right[n] * input : inL;

        bufferL[(size_t) writePos] = inL;
        bufferR[(size_t) writePos] = inR;

        phase += twoPi * rate / (float) sr;
        if (phase > twoPi)
            phase -= twoPi;

        float lfoL = std::sin (phase) * 0.5f + 0.5f;
        float lfoR = std::sin (phase + juce::MathConstants<float>::pi) * 0.5f + 0.5f;

        int tapsL = (int) std::floor (juce::jlimit (4.0f, 128.0f,
            (float) baseTaps + (lfoL - 0.5f) * 2.0f * (float) tapSwing * depth));

        int tapsR = (int) std::floor (juce::jlimit (4.0f, 128.0f,
            (float) baseTaps + (lfoR - 0.5f) * 2.0f * (float) tapSwing * depth));

        auto hannRead = [&] (const std::vector<float>& delayBuffer, int taps)
        {
            float sum = 0.0f;
            float wsum = 0.0f;

            for (int i = 0; i < taps; ++i)
            {
                int idx = writePos - i;
                while (idx < 0)
                    idx += bufferSize;

                idx %= bufferSize;

                float denom = (float) juce::jmax (1, taps - 1);
                float w = 0.5f - 0.5f * std::cos (twoPi * (float) i / denom);

                sum += delayBuffer[(size_t) idx] * w;
                wsum += w;
            }

            return sum / juce::jmax (wsum, 0.000001f);
        };

        float wetL = hannRead (bufferL, tapsL);
        float wetR = hannRead (bufferR, tapsR);

        lowL += 0.03f * (wetL - lowL);
        lowR += 0.03f * (wetR - lowR);

        wetL = wetL - lowL * 0.55f;
        wetR = wetR - lowR * 0.55f;

        float dL = (wetL - inL) * boost;
        float dR = (wetR - inR) * boost;

        float outL = inL + dL * mix;
        float outR = inR + dR * mix;

        left[n] = (delta ? dL : outL) * output;

        if (right != nullptr)
            right[n] = (delta ? dR : outR) * output;

        writePos++;
        if (writePos >= bufferSize)
            writePos = 0;
    }
}

void WinChorusAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void WinChorusAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml (getXmlFromBinary (data, sizeInBytes));

    if (xml != nullptr && xml->hasTagName (apvts.state.getType()))
        apvts.replaceState (juce::ValueTree::fromXml (*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new WinChorusAudioProcessor();
}