# klang | C++ for audio

klang is a language for the design and development of realtime audio processes in C++.

As a dialect of modern C++, klang enables synthesisers and effects to be written in clearer, more concise language, using the concept of streams and OOP to combine the performance of native C++ and clarity of visual/data-flow languages, such as block diagrams or patch-based tools like Max or PureData, and is designed to facilitate rapid prototyping of audio processing, as well as experimentation, knowledge sharing, and learning.

Designed for both professional and academic use, klang hopes to facilitate the sharing of DSP knowledge with the community-led development of an open library of reference examples.

- Concise, accessible, and extensible language for audio processing.
- Performance of native C++, with single sample or buffer-based processing.
- Primitives for all common DSP components (oscillators, filters, etc.).
- Natively compatible and compilable in all modern C++ compilers and IDEs.
- A single header file, easily integrated with any new or existing C++ project.
- Designed for knowledge sharing; supporting a growing repository of DSP examples.
- Permissive open-source (attribution) licence.

## Getting Started

To use klang, simply include the **klang.h** header file in your C++ project. 

    #include <klang.h>
    using namespace klang; // optional

Audio objects are then accessible using the <code>klang</code> namespace (e.g. Effect, Sine). klang tries to use plain language to describe objects, so the namespace is used to avoid conflict with similar terms in other APIs or code. If this is not needed, specify <code>using namespace klang;</code> 

The core data type in klang is <code>signal</code>, an extension of the basic C type <code>float</code> with additional audio and streaming functionality. In many cases, signals can be used interably with floats, facilitating integration of klang with other C++ code, while enabling signal-flow expressions, such as in the following auto-wah effect...

```
// signal in, out;
// Sine lfo;
// LPF lpf;

signal mod = lfo(3) * 0.5 + 0.5;
in >> lpf(mod) >> out;
```

... where <code>lpf</code> is a low-pass filter, <code>in</code> is the input signal, <code>out</code> the output signal, and <code>mod</code> is a signal used to modulate the filter's parameter (~cutoff frequency), based on a low-frequency (3Hz) sine oscillator (<code>lfo</code>).

DSP components, like oscillators or filters, are declared as objects (using struct or class) as either a <code>Generator</code> (output only) or <code>Modifier</code> (input / output), supplying functions to handle parameter setting (<code>set()</code>) and signal processing (<code>output()</code>)...

```
struct LPF : public Modifier {
   float a, b;

   void set(param coeff) {
      a = coeff;
      b = 1 - a;
   }

   signal output(const signal& in) {
      return out = (a * in) + (b * out);
   }
};
```
Here, the <code>Modifier</code> parent class adapts the LPF code so it can be used as before. Parameters have type <code>param</code>, which is derived from <code>signal</code>, allowing constants, floats, or signals to be used interchangeably with or as parameters. Code may use either signal (<<, >>) or mathematical (=) operators interchangeable, to allow you to express the audio process in a manner best suited to the context or other reference material. Filters are typically described in mathematical terms, as shown here.

More complex audio processes are created by combining klang objects, such as the following subtractive synthesiser:

```
class Subtractive : public Note {
   Saw osc;
   ADSR adsr;
   LPF lpf;
   Sine lfo;

   event on(Pitch pitch, Amplitude velocity) {
      const Frequency frequency(pitch > Type::Frequency);
      osc.set(frequency);
      adsr(0.25, 0.25, 0.5, 5.0);
      lfo.set(3);
   }
	 
   event off() {
      adsr.release();
   }
	 
   signal output() {
      signal mod = lfo * 0.5 + 0.5;
      signal mix = osc * adsr++ >> lpf(mod);
      if (adsr.finished())
         stop();
      return mix;
   }
};
```

## Usage in a C++ project

This object defines the processing for a single synth note that can then be used in any audio C++ project, placing the following code fragments at appropriate points in your code (e.g. myEffect/mySynth mini-plugin or any AU/VST plugin, JUCE app, etc.):

```
  klang::Subtractive note;
 
  note.start(pitch, velocity); 
  note.stop();
  
  klang::Buffer buffer = { pfBuffer, numSamples };
  note.process(buffer);
```
For ready-made AU/VST compatible C++ templates for Xcode and Visual Studio, see the github.com/myeffect and github.com/mysynth repositories.
rapide plugins also support a pure klang live coding mode.

## Usage in the **rapide** plugin

The rapide plugin features native support for klang, providing a pure klang mode for synthesiser and effect plugin development.

**rapide** (github.com/rapide) is a rapid audio prototyping (RAP) integrated development environment (IDE): a state-of-the-art C++ audio development environment... inside a DAW plugin, enabling the live C++ development of realtime audio processes, where you can open, edit, and recompile the code of the running plugin, within the DAW itself plugin's own UI and without having to stop and reload it. 

## More information

For more information about the project, or to get involved, email Chris Nash (klang@nash.audio).
