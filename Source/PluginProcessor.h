#pragma once

#include <array>
#include <atomic>
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>

// ============================================================================
// Parameter IDs - used by both the processor (to register parameters) and the
// editor (to attach knobs/combobox to them). Keeping them in one place avoids
// typo mismatches between the two files.
// ============================================================================
namespace ParamIDs
{
    static const juce::String filterType = "filterType";
    static const juce::String freq       = "freq";
    static const juce::String q          = "q";
    static const juce::String gain       = "gain";
}

// The three low-frequency filter modes, selectable via the UI dropdown.
enum class LowFreqFilterType
{
    LowPass = 0,
    LowShelf,
    HighPass
};

class LowFreqVSTAudioProcessor final : public juce::AudioProcessor
{
public:
    LowFreqVSTAudioProcessor();
    ~LowFreqVSTAudioProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;

    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }

    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    // Public so the editor can attach sliders/combobox directly to these.
    juce::AudioProcessorValueTreeState apvts;

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    void updateFilterCoefficients();

    // One IIR filter per channel (stereo). Using juce::dsp::IIR with a
    // coefficient pointer we swap out whenever a parameter changes.
    using FilterBand = juce::dsp::IIR::Filter<float>;
    using Coefficients = juce::dsp::IIR::Coefficients<float>;

    std::array<FilterBand, 2> filters;
    juce::ReferenceCountedObjectPtr<Coefficients> currentCoefficients;

    double currentSampleRate = 44100.0;

    // Cached atomic raw pointers to APVTS parameters for fast, lock-free
    // reads in processBlock/updateFilterCoefficients.
    std::atomic<float>* filterTypeParam = nullptr;
    std::atomic<float>* freqParam = nullptr;
    std::atomic<float>* qParam = nullptr;
    std::atomic<float>* gainParam = nullptr;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LowFreqVSTAudioProcessor)
};
