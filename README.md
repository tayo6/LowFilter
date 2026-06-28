# LowFreqVST

A JUCE 7.0.12 lesson VST3 plugin: a low-frequency filter (Low-Pass / Low-Shelf
/ High-Pass, selectable via dropdown) with three knobs — **Freq**, **Q**, and
**Gain** — each rendered as a custom rotary control using a vertical filmstrip
PNG instead of a procedurally-drawn knob.

Builds automatically as a Windows VST3 via GitHub Actions on every push.

## Where to upload the filmstrip PNG

**Path in the repo: `Assets/KnobFilmstrip.png`**

```
LowFreqVST/
├── Assets/
│   └── KnobFilmstrip.png   <-- upload your real filmstrip here, this exact filename
├── Source/
├── CMakeLists.txt
└── .github/workflows/build-vst3-windows.yml
```

Steps on GitHub.com (no local git needed):

1. Open your repo in the browser.
2. Navigate into the `Assets` folder.
3. Click **Add file → Upload files**.
4. Drag in your PNG, and **rename it to exactly `KnobFilmstrip.png`** if it
   isn't already (case-sensitive, must match exactly — the CMake build and
   the C++ code both reference this exact filename).
5. Commit directly to `main`.

That's it. The next push (this upload counts as one) triggers the GitHub
Actions workflow, which:
- checks that `Assets/KnobFilmstrip.png` exists (fails fast with a clear
  message if you forgot to upload it or misnamed it),
- fetches JUCE 7.0.12 automatically via CMake,
- compiles the filmstrip PNG into the plugin binary (no loose files at
  runtime — it's embedded via `juce_add_binary_data`),
  builds the VST3, and uploads it as a downloadable workflow artifact named
  `LowFreqVST-VST3-Windows`.

A placeholder filmstrip is already included so the project builds
successfully out of the box. It's a 360×360 × 26-frame strip (9360 px tall
total) that mimics the same "ring shrinking to a dot, clockwise from ~2
o'clock" style as your reference image — just swap it out by uploading your
real PNG to the same path and filename.

## Filmstrip spec (must match if you swap the PNG)

| Property        | Value          |
|------------------|----------------|
| Frame size       | 360 × 360 px   |
| Frame count      | 26             |
| Total strip size | 360 × 9360 px  |
| Orientation      | Vertical, frame 0 at top |
| Frame 0          | Knob minimum value |
| Frame 25         | Knob maximum value |

If you ever change the frame count, update the `26` passed into
`filmstripLookAndFeel(...)` in `Source/PluginEditor.cpp` to match.

## How the filmstrip-to-knob mapping works

This is the core JUCE technique demonstrated here, in
`Source/FilmstripRotarySlider.cpp`:

1. `juce::Slider` already computes a normalized 0.0–1.0 position
   (`sliderPosProportional`) for wherever the knob currently sits in its
   parameter range — JUCE does this math for you.
2. We multiply that by `(numFrames - 1)` and round, to pick a single frame
   index out of the 26 stacked in the PNG.
3. We crop just that one 360×360 frame out of the tall strip
   (`getClippedImage`) and draw it into the slider's bounds.
4. Because the artwork itself already depicts the knob rotating clockwise
   frame-by-frame (full ring → shrinking arc → dot), we don't rotate
   anything in code — we just pick the matching drawn frame.

The rotary travel itself (where "empty" and "full" sit on the dial) is set
in `FilmstripRotarySlider`'s constructor via `setRotaryParameters(...)`,
configured to start near 2 o'clock and sweep clockwise.

## Project layout

```
Source/
├── PluginProcessor.h/.cpp     DSP: 3 filter modes (LP/Low-Shelf/HP) via juce::dsp::IIR,
│                              parameter layout (filterType, freq, q, gain)
├── PluginEditor.h/.cpp        UI layout: dropdown + 3 filmstrip knobs, APVTS attachments
└── FilmstripRotarySlider.h/.cpp   Custom LookAndFeel that paints knob frames from the PNG
```

## Building locally (optional — Actions does this for you on Windows)

Requires CMake ≥ 3.22 and a C++17 compiler.

```bash
cmake -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release --target LowFreqVST_VST3
```

The built `.vst3` will be somewhere under `build/LowFreqVST_artefacts/Release/VST3/`.

## Parameters

| Knob/Control | Range            | Notes |
|--------------|-------------------|-------|
| Filter Type  | Low-Pass / Low-Shelf / High-Pass | Dropdown above the knobs |
| Freq         | 20 Hz – 2000 Hz   | Skewed for finer control at low frequencies |
| Q            | 0.1 – 10.0        | Resonance/bandwidth, used by all 3 filter types |
| Gain         | -24 dB – +24 dB   | Only audible in Low-Shelf mode; harmless in LP/HP |

## Notes for first-time VST builders

- You don't need JUCE installed locally — `CMakeLists.txt` pulls JUCE 7.0.12
  straight from GitHub via `FetchContent`, both locally and in CI.
- You don't need a VST3 SDK install either — JUCE bundles what it needs.
- The GitHub Actions workflow is the easiest way to get a working `.vst3`
  without installing Visual Studio yourself: just push to `main` (or run the
  workflow manually from the **Actions** tab → "Build VST3 (Windows)" → **Run
  workflow**), then download the artifact from the completed run.
