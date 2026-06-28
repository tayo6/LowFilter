#include "PluginProcessor.h"
#include "PluginEditor.h"

LowFreqVSTAudioProcessor::LowFreqVSTAudioProcessor()
    : AudioProcessor(BusesProperties()
                          .withInput("Input", juce::AudioChannelSet::stereo(), true)
                          .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "PARAMETERS", createParameterLayout())
{
    filterTypeParam = apvts.getRawParameterValue(ParamIDs::filterType);
    freqParam       = apvts.getRawParameterValue(ParamIDs::freq);
    qParam          = apvts.getRawParameterValue(ParamIDs::q);
    gainParam       = apvts.getRawParameterValue(ParamIDs::gain);
}

LowFreqVSTAudioProcessor::~LowFreqVSTAudioProcessor() = default;

juce::AudioProcessorValueTreeState::ParameterLayout LowFreqVSTAudioProcessor::createParameterLayout()
{
    // NOTE: ParameterLayout has no iterator-pair constructor - parameters must
    // be added one at a time via .add(). (An earlier draft of this function
    // built a std::vector and tried to construct the layout from
    // {vec.begin(), vec.end()}, which does not compile - fixed here.)
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    // Filter type dropdown: index 0 = Low-pass, 1 = Low-shelf, 2 = High-pass.
    layout.add(std::make_unique<juce::AudioParameterChoice>(
        ParamIDs::filterType,
        "Filter Type",
        juce::StringArray { "Low-Pass", "Low-Shelf", "High-Pass" },
        0));

    // Frequency: 20 Hz - 2000 Hz, skewed so low end has more knob resolution
    // (typical for "low frequency" controls).
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        ParamIDs::freq,
        "Frequency",
        juce::NormalisableRange<float>(20.0f, 2000.0f, 0.1f, 0.3f),
        150.0f,
        "Hz"));

    // Q: resonance/bandwidth control, used by all three filter types.
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        ParamIDs::q,
        "Q",
        juce::NormalisableRange<float>(0.1f, 10.0f, 0.01f, 0.4f),
        0.707f));

    // Gain: only meaningful for Low-Shelf, but kept available for all modes
    // since the UI always shows 3 knobs. Harmless no-op for LP/HP.
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        ParamIDs::gain,
        "Gain",
        juce::NormalisableRange<float>(-24.0f, 24.0f, 0.1f),
        0.0f,
        "dB"));

    return layout;
}

void LowFreqVSTAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;

    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = static_cast<juce::uint32>(samplesPerBlock);
    spec.numChannels = 1; // each FilterBand instance handles one channel

    for (auto& f : filters)
    {
        f.prepare(spec);
        f.reset();
    }

    updateFilterCoefficients();
}

void LowFreqVSTAudioProcessor::releaseResources()
{
}

bool LowFreqVSTAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    const auto& mainOut = layouts.getMainOutputChannelSet();
    const auto& mainIn  = layouts.getMainInputChannelSet();

    if (mainOut != juce::AudioChannelSet::mono() && mainOut != juce::AudioChannelSet::stereo())
        return false;

    return mainIn == mainOut;
}

void LowFreqVSTAudioProcessor::updateFilterCoefficients()
{
    const auto typeIndex = static_cast<int>(filterTypeParam->load());
    const auto freqHz    = freqParam->load();
    const auto qVal      = qParam->load();
    const auto gainDb    = gainParam->load();

    const auto type = static_cast<LowFreqFilterType>(typeIndex);

    switch (type)
    {
        case LowFreqFilterType::LowPass:
            currentCoefficients = Coefficients::makeLowPass(currentSampleRate, freqHz, qVal);
            break;

        case LowFreqFilterType::LowShelf:
            currentCoefficients = Coefficients::makeLowShelf(
                currentSampleRate, freqHz, qVal, juce::Decibels::decibelsToGain(gainDb));
            break;

        case LowFreqFilterType::HighPass:
            currentCoefficients = Coefficients::makeHighPass(currentSampleRate, freqHz, qVal);
            break;
    }

    for (auto& f : filters)
        *f.coefficients = *currentCoefficients;
}

void LowFreqVSTAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    // Recompute coefficients once per block. Cheap enough, and avoids
    // needing a separate parameter-listener/dirty-flag mechanism for
    // this lesson-scope project.
    updateFilterCoefficients();

    const auto numChannels = juce::jmin(buffer.getNumChannels(), static_cast<int>(filters.size()));
    const auto numSamples  = buffer.getNumSamples();

    for (int ch = 0; ch < numChannels; ++ch)
    {
        auto* channelData = buffer.getWritePointer(ch);
        juce::dsp::AudioBlock<float> block(&channelData, 1, static_cast<size_t>(numSamples));
        juce::dsp::ProcessContextReplacing<float> context(block);
        filters[static_cast<size_t>(ch)].process(context);
    }
}

juce::AudioProcessorEditor* LowFreqVSTAudioProcessor::createEditor()
{
    return new LowFreqVSTAudioProcessorEditor(*this);
}

void LowFreqVSTAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    if (auto state = apvts.copyState(); state.isValid())
    {
        if (auto xml = state.createXml())
            copyXmlToBinary(*xml, destData);
    }
}

void LowFreqVSTAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary(data, sizeInBytes))
        apvts.replaceState(juce::ValueTree::fromXml(*xml));
}

// This creates new instances of the plugin
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new LowFreqVSTAudioProcessor();
}
