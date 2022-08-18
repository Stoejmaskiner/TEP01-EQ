/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

enum Slope {
    Slope12,
    Slope24,
    Slope36,
    Slope48
};

/// <summary>
/// an instant snapshot of the parameters
/// </summary>
struct ChainSettings {
    float peakFreq{ 0.f }, peakGainInDecibels{ 0.f }, peakQuality{ 1.f };
    float lowCutFreq{ 0.f }, highCutFreq{ 0.f };
    int lowCutSlope{ Slope::Slope12 }, highCutSlope{ Slope::Slope12 };
};

/// <summary>
/// transform an APVTS into a ChainSettings, which is a snapshot of the state of the parameters
/// </summary>
ChainSettings getChainSettings(juce::AudioProcessorValueTreeState& apvts);

//==============================================================================
/**
*/
class TEP01EQAudioProcessor  : public juce::AudioProcessor
                            #if JucePlugin_Enable_ARA
                             , public juce::AudioProcessorARAExtension
                            #endif
{
public:
    //==============================================================================
    TEP01EQAudioProcessor();
    ~TEP01EQAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    // MY STUFF
    juce::AudioProcessorValueTreeState apvts{
        *this,
        nullptr, // TODO
        "Parameters",
        this->create_parameter_layout()
    };

    static juce::AudioProcessorValueTreeState::ParameterLayout create_parameter_layout();


private:

    // a single filter has a slope of 12 dB oct
    using Filter = juce::dsp::IIR::Filter<float>;

    // up to 4 stages are needed to get 24, 36, 48 dB oct
    using CutFilter = juce::dsp::ProcessorChain<Filter, Filter, Filter, Filter>;

    using MonoChain = juce::dsp::ProcessorChain<CutFilter, Filter, CutFilter>;

    MonoChain left_chain, right_chain;

    enum ChainPositions {
        LowCut,
        Peak,
        HighCut
    };

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TEP01EQAudioProcessor)
};
