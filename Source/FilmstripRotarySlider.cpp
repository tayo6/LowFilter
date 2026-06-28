#include "FilmstripRotarySlider.h"
#include <cmath>

FilmstripRotaryLookAndFeel::FilmstripRotaryLookAndFeel(const void* filmstripData,
                                                        size_t filmstripDataSize,
                                                        int numFramesInStrip)
    : numFrames(numFramesInStrip)
{
    filmstrip = juce::ImageCache::getFromMemory(filmstripData, static_cast<int>(filmstripDataSize));

    jassert(filmstrip.isValid()); // PNG failed to decode - check BinaryData was generated correctly
    jassert(numFrames > 0);

    if (filmstrip.isValid() && numFrames > 0)
    {
        frameWidth = filmstrip.getWidth();
        frameHeight = filmstrip.getHeight() / numFrames;
    }
}

void FilmstripRotaryLookAndFeel::drawRotarySlider(juce::Graphics& g,
                                                    int x, int y, int width, int height,
                                                    float sliderPosProportional,
                                                    float rotaryStartAngle,
                                                    float rotaryEndAngle,
                                                    juce::Slider& /*slider*/)
{
    if (!filmstrip.isValid() || frameWidth <= 0 || frameHeight <= 0)
    {
        // Fallback so a broken/missing image doesn't leave a blank, useless
        // UI: draw a value arc using the slider's own rotary angle range, so
        // the control still visibly reflects its value even without the PNG.
        auto bounds = juce::Rectangle<int>(x, y, width, height).toFloat().reduced(8.0f);
        const auto radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) * 0.5f;
        const auto centre = bounds.getCentre();
        const auto valueAngle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);

        juce::Path track;
        track.addCentredArc(centre.x, centre.y, radius, radius, 0.0f, rotaryStartAngle, rotaryEndAngle, true);
        g.setColour(juce::Colours::darkgrey);
        g.strokePath(track, juce::PathStrokeType(6.0f));

        juce::Path value;
        value.addCentredArc(centre.x, centre.y, radius, radius, 0.0f, rotaryStartAngle, valueAngle, true);
        g.setColour(juce::Colours::darkred);
        g.strokePath(value, juce::PathStrokeType(6.0f));
        return;
    }

    // sliderPosProportional is already 0.0 - 1.0 (JUCE normalises this for
    // us based on the slider's range), so frame selection is a direct
    // multiply-and-round against the frame count. Note this is the path that
    // actually runs whenever the filmstrip image loaded correctly - the
    // rotary angle parameters above are NOT used here, because the artwork
    // itself already depicts the rotation; the slider's rotary angle range
    // (set in FilmstripRotarySlider's constructor) only affects the fallback
    // drawing above, not this image-based path.
    const int frameIndex = juce::jlimit(
        0, numFrames - 1,
        static_cast<int>(std::round(sliderPosProportional * static_cast<float>(numFrames - 1))));

    juce::Rectangle<int> sourceFrame(0, frameIndex * frameHeight, frameWidth, frameHeight);

    // Destination: largest square that fits the knob's bounds, centred.
    const int side = juce::jmin(width, height);
    juce::Rectangle<int> destBounds(
        x + (width - side) / 2,
        y + (height - side) / 2,
        side, side);

    juce::Image frame = filmstrip.getClippedImage(sourceFrame);
    g.drawImage(frame,
                destBounds.toFloat(),
                juce::RectanglePlacement::centred);
}

FilmstripRotarySlider::FilmstripRotarySlider()
    : juce::Slider(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag,
                    juce::Slider::TextEntryBoxPosition::NoTextBox)
{
    // Rotary travel: starts at 2 o'clock and sweeps clockwise, down through
    // 6 o'clock, up to 10 o'clock - a 240 degree travel, the classic analog
    // knob range. Angles are in radians, measured clockwise from 12 o'clock
    // (JUCE's convention for rotary sliders).
    //
    //   2 o'clock  = 60 degrees  = pi/3   (start)
    //   10 o'clock = 300 degrees = 5*pi/3 (end - the long way round through
    //                the bottom, not backwards through 12)
    //
    // This range only drives the LookAndFeel's fallback arc drawing (used if
    // the filmstrip image fails to load) - the normal, working knob visual
    // comes entirely from the filmstrip artwork's own frames, independent of
    // this setting.
    const float startAngle = juce::MathConstants<float>::pi * (1.0f / 3.0f); // 60 deg
    const float endAngle   = juce::MathConstants<float>::pi * (5.0f / 3.0f); // 300 deg

    setRotaryParameters(startAngle, endAngle, true);
    setMouseDragSensitivity(150);
}
