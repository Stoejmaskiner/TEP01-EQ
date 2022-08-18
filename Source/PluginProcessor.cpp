/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
TEP01EQAudioProcessor::TEP01EQAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
}

TEP01EQAudioProcessor::~TEP01EQAudioProcessor()
{
}

//==============================================================================
const juce::String TEP01EQAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool TEP01EQAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool TEP01EQAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool TEP01EQAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double TEP01EQAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int TEP01EQAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int TEP01EQAudioProcessor::getCurrentProgram()
{
    return 0;
}

void TEP01EQAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String TEP01EQAudioProcessor::getProgramName (int index)
{
    return {};
}

void TEP01EQAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void TEP01EQAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    juce::dsp::ProcessSpec spec;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = 1;
    spec.sampleRate = sampleRate;

    left_chain.prepare(spec);
    right_chain.prepare(spec);

    auto chainSettings = getChainSettings(apvts);
    auto peakCoeffs = juce::dsp::IIR::Coefficients<float>::makePeakFilter(
        sampleRate,
        chainSettings.peakFreq,
        chainSettings.peakQuality,
        juce::Decibels::decibelsToGain(chainSettings.peakGainInDecibels));

    *left_chain.get<ChainPositions::Peak>().coefficients = *peakCoeffs;
    *right_chain.get<ChainPositions::Peak>().coefficients = *peakCoeffs;

    auto cutCoefficients = juce::dsp::FilterDesign<float>::designIIRHighpassHighOrderButterworthMethod(
        chainSettings.lowCutFreq,
        sampleRate,
        2 * (chainSettings.lowCutSlope + 1));

    auto& leftLowCut = left_chain.get<ChainPositions::LowCut>();

    leftLowCut.setBypassed<0>(true);
    leftLowCut.setBypassed<1>(true);
    leftLowCut.setBypassed<2>(true);
    leftLowCut.setBypassed<3>(true);

    switch (chainSettings.lowCutSlope) {
    case Slope12:
        leftLowCut.get<0>().coefficients = *cutCoefficients[0];
        leftLowCut.setBypassed<0>(false);
        break;
    case Slope24:
        leftLowCut.get<0>().coefficients = *cutCoefficients[0];
        leftLowCut.setBypassed<0>(false);
        leftLowCut.get<1>().coefficients = *cutCoefficients[1];
        leftLowCut.setBypassed<1>(false);
        break;
    case Slope36:
        leftLowCut.get<0>().coefficients = *cutCoefficients[0];
        leftLowCut.setBypassed<0>(false);
        leftLowCut.get<1>().coefficients = *cutCoefficients[1];
        leftLowCut.setBypassed<1>(false);
        leftLowCut.get<2>().coefficients = *cutCoefficients[2];
        leftLowCut.setBypassed<2>(false);
        break;
    case Slope48:
        leftLowCut.get<0>().coefficients = *cutCoefficients[0];
        leftLowCut.setBypassed<0>(false);
        leftLowCut.get<1>().coefficients = *cutCoefficients[1];
        leftLowCut.setBypassed<1>(false);
        leftLowCut.get<2>().coefficients = *cutCoefficients[2];
        leftLowCut.setBypassed<2>(false);
        leftLowCut.get<3>().coefficients = *cutCoefficients[3];
        leftLowCut.setBypassed<3>(false);
        break;
    }

    auto& rightLowCut = right_chain.get<ChainPositions::LowCut>();

    rightLowCut.setBypassed<0>(true);
    rightLowCut.setBypassed<1>(true);
    rightLowCut.setBypassed<2>(true);
    rightLowCut.setBypassed<3>(true);

    switch (chainSettings.lowCutSlope) {
    case Slope12:
        rightLowCut.get<0>().coefficients = *cutCoefficients[0];
        rightLowCut.setBypassed<0>(false);
        break;
    case Slope24:
        rightLowCut.get<0>().coefficients = *cutCoefficients[0];
        rightLowCut.setBypassed<0>(false);
        rightLowCut.get<1>().coefficients = *cutCoefficients[1];
        rightLowCut.setBypassed<1>(false);
        break;
    case Slope36:
        rightLowCut.get<0>().coefficients = *cutCoefficients[0];
        rightLowCut.setBypassed<0>(false);
        rightLowCut.get<1>().coefficients = *cutCoefficients[1];
        rightLowCut.setBypassed<1>(false);
        rightLowCut.get<2>().coefficients = *cutCoefficients[2];
        rightLowCut.setBypassed<2>(false);
        break;
    case Slope48:
        rightLowCut.get<0>().coefficients = *cutCoefficients[0];
        rightLowCut.setBypassed<0>(false);
        rightLowCut.get<1>().coefficients = *cutCoefficients[1];
        rightLowCut.setBypassed<1>(false);
        rightLowCut.get<2>().coefficients = *cutCoefficients[2];
        rightLowCut.setBypassed<2>(false);
        rightLowCut.get<3>().coefficients = *cutCoefficients[3];
        rightLowCut.setBypassed<3>(false);
        break;
    }
}

void TEP01EQAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool TEP01EQAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void TEP01EQAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    auto chainSettings = getChainSettings(apvts);
    auto peakCoeffs = juce::dsp::IIR::Coefficients<float>::makePeakFilter(
        getSampleRate(),
        chainSettings.peakFreq,
        chainSettings.peakQuality,
        juce::Decibels::decibelsToGain(chainSettings.peakGainInDecibels));

    *left_chain.get<ChainPositions::Peak>().coefficients = *peakCoeffs;
    *right_chain.get<ChainPositions::Peak>().coefficients = *peakCoeffs;

    auto cutCoefficients = juce::dsp::FilterDesign<float>::designIIRHighpassHighOrderButterworthMethod(
        chainSettings.lowCutFreq,
        getSampleRate(),
        2 * (chainSettings.lowCutSlope + 1));

    auto& leftLowCut = left_chain.get<ChainPositions::LowCut>();

    leftLowCut.setBypassed<0>(true);
    leftLowCut.setBypassed<1>(true);
    leftLowCut.setBypassed<2>(true);
    leftLowCut.setBypassed<3>(true);

    switch (chainSettings.lowCutSlope) {
    case Slope12:
        leftLowCut.get<0>().coefficients = *cutCoefficients[0];
        leftLowCut.setBypassed<0>(false);
        break;
    case Slope24:
        leftLowCut.get<0>().coefficients = *cutCoefficients[0];
        leftLowCut.setBypassed<0>(false);
        leftLowCut.get<1>().coefficients = *cutCoefficients[1];
        leftLowCut.setBypassed<1>(false);
        break;
    case Slope36:
        leftLowCut.get<0>().coefficients = *cutCoefficients[0];
        leftLowCut.setBypassed<0>(false);
        leftLowCut.get<1>().coefficients = *cutCoefficients[1];
        leftLowCut.setBypassed<1>(false);
        leftLowCut.get<2>().coefficients = *cutCoefficients[2];
        leftLowCut.setBypassed<2>(false);
        break;
    case Slope48:
        leftLowCut.get<0>().coefficients = *cutCoefficients[0];
        leftLowCut.setBypassed<0>(false);
        leftLowCut.get<1>().coefficients = *cutCoefficients[1];
        leftLowCut.setBypassed<1>(false);
        leftLowCut.get<2>().coefficients = *cutCoefficients[2];
        leftLowCut.setBypassed<2>(false);
        leftLowCut.get<3>().coefficients = *cutCoefficients[3];
        leftLowCut.setBypassed<3>(false);
        break;
    }

    auto& rightLowCut = right_chain.get<ChainPositions::LowCut>();

    rightLowCut.setBypassed<0>(true);
    rightLowCut.setBypassed<1>(true);
    rightLowCut.setBypassed<2>(true);
    rightLowCut.setBypassed<3>(true);

    switch (chainSettings.lowCutSlope) {
    case Slope12:
        rightLowCut.get<0>().coefficients = *cutCoefficients[0];
        rightLowCut.setBypassed<0>(false);
        break;
    case Slope24:
        rightLowCut.get<0>().coefficients = *cutCoefficients[0];
        rightLowCut.setBypassed<0>(false);
        rightLowCut.get<1>().coefficients = *cutCoefficients[1];
        rightLowCut.setBypassed<1>(false);
        break;
    case Slope36:
        rightLowCut.get<0>().coefficients = *cutCoefficients[0];
        rightLowCut.setBypassed<0>(false);
        rightLowCut.get<1>().coefficients = *cutCoefficients[1];
        rightLowCut.setBypassed<1>(false);
        rightLowCut.get<2>().coefficients = *cutCoefficients[2];
        rightLowCut.setBypassed<2>(false);
        break;
    case Slope48:
        rightLowCut.get<0>().coefficients = *cutCoefficients[0];
        rightLowCut.setBypassed<0>(false);
        rightLowCut.get<1>().coefficients = *cutCoefficients[1];
        rightLowCut.setBypassed<1>(false);
        rightLowCut.get<2>().coefficients = *cutCoefficients[2];
        rightLowCut.setBypassed<2>(false);
        rightLowCut.get<3>().coefficients = *cutCoefficients[3];
        rightLowCut.setBypassed<3>(false);
        break;
    }

    juce::dsp::AudioBlock<float> block (buffer);
    auto left_block = block.getSingleChannelBlock(0);
    auto right_block = block.getSingleChannelBlock(1);

    juce::dsp::ProcessContextReplacing<float> left_context(left_block);
    juce::dsp::ProcessContextReplacing<float> right_context(right_block);

    this->left_chain.process(left_context);
    this->right_chain.process(right_context);
}

//==============================================================================
bool TEP01EQAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* TEP01EQAudioProcessor::createEditor()
{
    //return new TEP01EQAudioProcessorEditor (*this);
    return new juce::GenericAudioProcessorEditor(*this);
}

//==============================================================================
void TEP01EQAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void TEP01EQAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new TEP01EQAudioProcessor();
}


// MY STUFF

juce::AudioProcessorValueTreeState::ParameterLayout TEP01EQAudioProcessor::create_parameter_layout() {

    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    juce::StringArray slope_options;
    for (int i = 0; i < 4; i++) {
        juce::String str;
        str << (12 + i * 12);
        str << " db/oct";
        slope_options.add(str);
    }

    layout.add(
        std::make_unique<juce::AudioParameterFloat>(
            "LOW_CUT_FREQ",
            "low cut freq",
            juce::NormalisableRange<float>(20.f, 20000.f, 1.f, .2f),
            20.f
            ),
        std::make_unique<juce::AudioParameterFloat>(
            "HIGH_CUT_FREQ",
            "high cut freq",
            juce::NormalisableRange<float>(20.f, 20000.f, 1.f, .2f),
            20000.f
            ),
        std::make_unique<juce::AudioParameterFloat>(
            "PEAK_FREQ",
            "peak freq",
            juce::NormalisableRange<float>(20.f, 20000.f, 1.f, .2f),
            650.f
            ),
        std::make_unique<juce::AudioParameterFloat>(
            "PEAK_GAIN",
            "peak gain",
            juce::NormalisableRange<float>(-24.f, 24.f, .5f, 1.f),
            0.f
            ),
        std::make_unique<juce::AudioParameterFloat>(
            "PEAK_Q",
            "peak Q",
            juce::NormalisableRange<float>(0.1f, 10.f, .05f, 1.f),
            1.f
            ),
        std::make_unique<juce::AudioParameterChoice>(
            "LOW_CUT_SLOPE",
            "low cut slope",
            slope_options,
            0
            ),
        std::make_unique<juce::AudioParameterChoice>(
            "HIGH_CUT_SLOPE",
            "high cut slope",
            slope_options,
            0
            )
    );

    return layout;
}

ChainSettings getChainSettings(juce::AudioProcessorValueTreeState& apvts)
{
    ChainSettings s;

    // removes some boilerplate
    auto getp_ = [&](const char* str)->auto {
        return apvts.getRawParameterValue(str)->load();
    };
    
    s.lowCutFreq = getp_("LOW_CUT_FREQ");
    s.highCutFreq = getp_("HIGH_CUT_FREQ");
    s.peakFreq = getp_("PEAK_FREQ");
    s.peakGainInDecibels = getp_("PEAK_GAIN");
    s.peakQuality = getp_("PEAK_Q");
    s.lowCutSlope = static_cast<Slope>(getp_("LOW_CUT_SLOPE"));
    s.highCutSlope = static_cast<Slope>(getp_("HIGH_CUT_SLOPE"));

    return s;
}
