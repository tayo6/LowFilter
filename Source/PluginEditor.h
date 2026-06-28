#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"
#include "FilmstripRotarySlider.h"

class LowFreqVSTAudioProcessorEditor final : public juce::AudioProcessorEditor
{
public:
    explicit LowFreqVSTAudioProcessorEditor(LowFreqVSTAudioProcessor&);
    ~LowFreqVSTAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    LowFreqVSTAudioProcessor& processorRef;

    // Filmstrip look-and-feel is shared across all three knobs - one decoded
    // image, reused for every rotary slider rather than reloading per-knob.
    FilmstripRotaryLookAndFeel filmstripLookAndFeel;

    FilmstripRotarySlider freqSlider;
    FilmstripRotarySlider qSlider;
    FilmstripRotarySlider gainSlider;

    juce::Label freqLabel { {}, "Freq" };
    juce::Label qLabel    { {}, "Q" };
    juce::Label gainLabel { {}, "Gain" };

    juce::ComboBox filterTypeBox;
    juce::Label filterTypeLabel { {}, "Filter Type" };

    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ComboBoxAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;

    std::unique_ptr<SliderAttachment> freqAttachment;
    std::unique_ptr<SliderAttachment> qAttachment;
    std::unique_ptr<SliderAttachment> gainAttachment;
    std::unique_ptr<ComboBoxAttachment> filterTypeAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LowFreqVSTAudioProcessorEditor)
};
