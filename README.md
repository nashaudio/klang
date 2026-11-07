[Download Klang Studio for Windows or Mac](https://nash.audio/klang/studio/).

# klang | C++ for audio

klang is a language for the design and development of realtime audio processes in C++.

As a dialect of modern C++, klang enables synthesisers and effects to be written in clearer, more concise language, using the concept of streams and OOP to combine the performance of native C++ and clarity of visual/data-flow languages, such as block diagrams or patch-based tools like Max or PureData, and is designed to facilitate rapid prototyping of audio processing, as well as experimentation, knowledge sharing, and learning.

Designed for both professional and academic use, klang hopes to facilitate the sharing of DSP knowledge with the community-led development of an open library of reference examples.

- Concise, accessible, and extensible language for audio processing.
- Performance of native C++, with single sample or buffer-based processing.
- Primitives for all common DSP components (oscillators, filters, etc.).
- Compatible with C++17 compilers and IDEs.
- A single header file, easily integrated with any new or existing C++ project.
- Designed for knowledge sharing; supporting a growing repository of DSP examples.
- Permissive open-source (attribution) licence.

>
> NOTE: The klang language is under active development, so syntax may change between pre-release versions.
>

## Getting Started

To use klang, simply include the **klang.h** header file in your C++ project.

    #include <klang.h>
    using namespace klang::optimised; // optional

Audio objects can be qualified with their namespaces (e.g. <code>klang::Effect</code> and <code>klang::optimised::Sine</code>). klang tries to use plain language to describe objects, so the namespace is used to avoid conflict with similar terms in other APIs or code. If this is not needed, add the <code>using namespace</code> directive as shown above.

The core data type in klang is <code>signal</code>, an extension of the basic C type <code>float</code> with additional audio and streaming functionality. In many cases, signals can be used interoperably with floats, facilitating integration of klang with other C++ code, while enabling signal-flow expressions, such as in the following wah-wah effect. Declare <code>Sine lfo;</code> and <code>LPF lpf;</code> as members of an <code>Effect</code> and put these lines in its <code>process()</code> method.

```cpp
signal mod = lfo(3) * 900 + 1200;
in >> lpf(mod) >> out;
```

... where <code>lpf</code> is a low-pass filter, <code>in</code> is the input signal, <code>out</code> the output signal, and <code>mod</code> is a signal used to modulate the filter's cutoff frequency between 300 and 2,100 Hz, based on a low-frequency (3 Hz) sine oscillator (<code>lfo</code>).

DSP components, like oscillators or filters, are declared as objects (using struct or class) as either a <code>Generator</code> (output only) or <code>Modifier</code> (input / output), supplying functions to handle parameter setting (<code>set()</code>) and signal processing (<code>process()</code>) ...

```cpp
struct OnePoleLPF : Modifier {
   param a = 0.1, b = 0.9;

   void set(param coeff) override {
      a = coeff;
      b = 1 - a;
   }

   void process() override {
      (a * in) + (b * out) >> out;
   }
};
```
Here, the <code>Modifier</code> parent class adapts the one-pole filter so it can be used in a signal chain. Its <code>set()</code> method takes a coefficient between 0 and 1. Parameters have type <code>param</code>, which is derived from <code>signal</code>, allowing constants, floats, or signals to be used interchangeably with or as parameters. Filters are often described in mathematical terms, so you could also write `out = (a * in) + (b * out);`.

More complex audio processes are created by combining klang objects, as in this voice for a simple subtractive synthesiser:

```cpp
struct Subtractive : Note {
   Saw osc;
   LPF lpf;
   Sine lfo;

   event on(Pitch pitch, Amplitude velocity) override {
      const Frequency frequency(pitch -> Frequency);
      osc.set(frequency);
   }

   void process() override {
      signal mod = lfo(3) * 900 + 1200;
      osc >> lpf(mod) >> out;
   }
};
```

This class supplies the <code>Note</code> definition to be used as part of a synthesiser (<code>Synth</code> - see the Examples linked below). It processes audio, but also handles events such as a **note on**, where the supplied pitch is converted to a frequency (in Hz) for use in the oscillator. By default, without code to handle a **note off**, the note and audio processing will be automatically terminated when one is received. In the synth constructor, use <code>notes.add&lt;Subtractive&gt;(16)</code> to register a pool of 16 voices.

But most instruments continue making sound after a note is 'released'...

```cpp
struct Subtractive : Note {
   Saw osc;
   LPF lpf;
   Sine lfo;
   ADSR adsr;

   event on(Pitch pitch, Amplitude velocity) override {
      const Frequency frequency(pitch -> Frequency);
      osc.set(frequency);
      adsr.set(0.25, 0.25, 0.5, 5.0);
   }

   event off(Amplitude) override {
      adsr.release();
   }

   void process() override {
      signal mod = lfo(3) * 900 + 1200;
      osc * adsr >> lpf(mod) >> out;
      if (adsr.finished())
         stop();
   }
};
```

This example shapes the note and adds a release stage using an ADSR amplitude envelope. The <code>ADSR</code> is a type of <code>Envelope</code> that takes four parameters (attack, decay, sustain, release - set in <code>on()</code>) and uses its output to scale the amplitude of the signal. The times are in seconds and sustain is a linear level. Reading <code>adsr</code> in the expression advances it by one sample. To add the 'release' stage and continue processing audio after a **note off**, we override the <code>off(Amplitude)</code> event and trigger the release of an ADSR envelope. Now, <code>process()</code> will continue to be called until you tell it to <code>stop()</code>, which we call when the ADSR is finished (that is, <code>adsr.finished()</code> is true).

For further techniques, see the Examples below.

## Examples

The /examples folder contains an evolving selection of examples (and tests). Start with these core examples:

- [Gain/Gain.k](examples/Gain/Gain.k) - a basic gain effect with a single control
- [Filtering/WahWah.k](examples/Filtering/WahWah.k) - a low-pass filter swept by a sine oscillator
- [Delay/Feedback.k](examples/Delay/Feedback.k) - a repeating echo built with a feedback delay
- [Additive/Saw.k](examples/Additive/Saw.k) - a sawtooth built by summing harmonics
- [Subtractive/Filter.k](examples/Subtractive/Filter.k) - a sawtooth voice shaped by a filter envelope
- [Subtractive/Release.k](examples/Subtractive/Release.k) - a synth voice with an envelope release stage
- [Physical/String.k](examples/Physical/String.k) - a plucked string model built around a delay line

See the [Examples page](https://nash.audio/klang/help/api/html/md_examples.html) for the full collection. For more complex patches, see [SuperSaw.k](examples/SuperSaw.k), [DX7.k](examples/DX7.k) and [SynTHX.k](examples/SynTHX.k) in the root of the examples folder.

## Usage in a C++ project

This object defines the processing for a single synth note that can then be used in any audio C++ project. Place the following code fragments at the appropriate points in your host code. Set <code>klang::fs</code> to the host sample rate in the translation unit that runs the DSP.

```cpp
Subtractive note;
klang::Note& voice = note;

voice.start(pitch, velocity);  // Note On
voice.release();               // Note Off

klang::buffer buffer = { pfBuffer, numSamples };
if (!voice.finished())
   voice.process(buffer);
```

The buffer wraps the host’s mono sample array. The voice adds its output to the buffer, so clear it before mixing voices into an empty output block.

## Usage in the **Klang Studio** plugin

Klang Studio plugins feature native support for klang, providing a pure klang mode for sound, effect and synthesiser development. Open a <code>.k</code> file in the matching mode and select **Build** to compile it.

**Klang Studio** is a rapid audio prototyping (RAP) integrated development environment (IDE): a state-of-the-art C++ audio development environment... inside a DAW plugin, enabling the live C++ development of realtime audio processes, where you can open, edit, and recompile the code of the running plugin, within the plugin's own UI and without having to stop and reload it.

## More information

For more information about the project, or to get involved, email Chris Nash (klang@nash.audio).
