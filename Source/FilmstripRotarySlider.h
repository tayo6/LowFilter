#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

// ============================================================================
// FilmstripRotaryLookAndFeel
//
// Renders a rotary Slider by picking the correct frame out of a vertical
// filmstrip image, instead of drawing a knob procedurally. This is the
// standard JUCE filmstrip-knob technique:
//
//   1. The filmstrip is one tall PNG containing N frames stacked top to
//      bottom, each frame the same width/height (here: 360x360, 26 frames,
//      so the full image is 360x9360).
//   2. Frame 0 = the slider's minimum value, frame N-1 = maximum value.
//   3. On every repaint, we work out the slider's normalised position
//      (0.0 - 1.0) and pick frame = round(position * (N - 1)).
//   4. We blit just that one frame (a 360x360 sub-rectangle of the strip)
//      into the knob's bounds.
//
// The "clockwise from ~2 o'clock" rotary feel is just standard JUCE rotary
// parameters (rotaryStartAngle / rotaryEndAngle) — the filmstrip itself
// should already be drawn rotating clockwise across its frames, so we don't
// need to manually rotate anything here, just pick the right frame.
// ============================================================================
class FilmstripRotaryLookAndFeel final : public juce::LookAndFeel_V4
{
public:
    FilmstripRotaryLookAndFeel(const void* filmstripData, size_t filmstripDataSize, int numFramesInStrip);

    void drawRotarySlider(juce::Graphics& g,
                           int x, int y, int width, int height,
                           float sliderPosProportional,
                           float rotaryStartAngle,
                           float rotaryEndAngle,
                           juce::Slider& slider) override;

private:
    juce::Image filmstrip;
    int numFrames = 1;
    int frameWidth = 0;
    int frameHeight = 0;
};

// ============================================================================
// FilmstripRotarySlider
//
// Thin convenience wrapper around juce::Slider pre-configured as a rotary
// control with no built-in text box (the filmstrip image is the entire
// visual), with rotary angle range set so motion starts at ~2 o'clock and
// sweeps clockwise.
// ============================================================================
class FilmstripRotarySlider final : public juce::Slider
{
public:
    FilmstripRotarySlider();
};
