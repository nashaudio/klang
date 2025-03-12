## NB: A preview release of Klang Studio (v0.7.8) for Windows and Mac is now available from https://nash.audio/klang/studio. ##

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

>
> NOTE: The klang language is under active development, so syntax may change between pre-release versions.
>

## Getting Started

To use klang, simply include the **klang.h** header file in your C++ project. 

    #include <klang.h>
    using namespace klang::optimised; // optional

Audio objects are then accessible using the <code>klang</code> namespace (e.g. Effect, Sine). klang tries to use plain language to describe objects, so the namespace is used to avoid conflict with similar terms in other APIs or code. If this is not needed, add the <code>using namespace</code> directive as shown above.

The core data type in klang is <code>signal</code>, an extension of the basic C type <code>float</code> with additional audio and streaming functionality. In many cases, signals can be used interoperably with floats, facilitating integration of klang with other C++ code, while enabling signal-flow expressions, such as in the following wah-wah effect ...

```
// signal in, out;
// Sine lfo;
// LPF lpf;

signal mod = lfo(3) * 0.5 + 0.5;
in >> lpf(mod) >> out;
```

... where <code>lpf</code> is a low-pass filter, <code>in</code> is the input signal, <code>out</code> the output signal, and <code>mod</code> is a signal used to modulate the filter's parameter (~cutoff frequency), based on a low-frequency (3Hz) sine oscillator (<code>lfo</code>).

DSP components, like oscillators or filters, are declared as objects (using struct or class) as either a <code>Generator</code> (output only) or <code>Modifier</code> (input / output), supplying functions to handle parameter setting (<code>set()</code>) and signal processing (<code>process()</code>) ...

```
struct LPF : Modifier {
   param a, b;

   void set(param coeff) {
      a = coeff;
      b = 1 - a;
   }

   void process() {
      (a * in) + (b * out) >> out;
   }
};
```
Here, the <code>Modifier</code> parent class adapts the LPF code so it can be used as before. Parameters have type <code>param</code>, which is derived from <code>signal</code>, allowing constants, floats, or signals to be used interchangeably with or as parameters. Code may use either signal (<<, >>) or mathematical (=) operators interchangeable, to allow you to express the audio process in a manner best suited to the context or other reference material. Filters are often described in mathematical terms, so you could also write: `out = (a * in) + (b * out);`.

More complex audio processes are created by combining klang objects, as in this simple subtractive synthesiser:

```
struct Subtractive : Note {
   Saw osc;
   LPF lpf;
   Sine lfo;

   event on(Pitch pitch, Amplitude velocity) {
      const Frequency frequency(pitch -> Frequency);
      osc.set(frequency);
   }
	 
   void process() {
      signal mod = lfo(3) * 0.5 + 0.5;
      osc >> lpf(mod) >> out;     
   }
};
```

This class supplies the <code>Note</code> definition to be used as part of a synthesiser (<code>Synth</code> - see the Examples linked below). It processes audio, but also handles events such as a **note on**, where the supplied pitch is converted to a frequency (in Hz) for use in the oscillator. By default, without code to handle a **note off**, the note and audio processing will be automatically terminated when one is received. 

But most instruments continue making sound after a note is 'released'...

```
struct Subtractive : Note {
   Saw osc;
   LPF lpf;
   Sine lfo;
   ADSR adsr;

   event on(Pitch pitch, Amplitude velocity) {
      const Frequency frequency(pitch -> Frequency);
      osc.set(frequency);
      adsr.set(0.25, 0.25, 0.5, 5.0);
   }
	 
   event off() {
      adsr.release();
   }
	 
   void process() {
      signal mod = lfo(3) * 0.5 + 0.5;
      osc * adsr >> lpf(mod) >> out;     
      if (adsr.finished())
         stop();
   }
};
```

This example shapes the note and adds a release stage using an ADSR amplitude envelope. The <code>ADSR</code> is a type of <code>Envelope</code> that takes four parameters (attack, decay, sustain, release - set in <code>on()</code>) and uses its output to scale the (\*) amplitude of the signal. To add the 'release' stage and continue processing audio after a **note off**, we add the <code>off()</code> event and trigger the release of an ADSR envelope. Now, <code>process()</code> will continue to be called until you tell it to <code>stop()</code>, which we call when the ADSR is finished (that is, <code>adsr.finished()</code> is true).

For further techniques, see the Examples below.

## Examples

The /examples folder contains an evolving selection of examples (and tests):

- **Supersaw.k** - a basic, but capable JP-8000 SuperSaw synth
- **DX7.k** - a simple DX7 emulator, with five presets (TUB BELLS, E.PIANO 1, E.ORGAN 1, HARPSICH 1, STRINGS)
- **Guitar.k** - a Karplus-Strong-based exciter-resonator plucked string model
- **Banjo.k** - a variation of Guitar.k with extra twang
- **THX.k** - an versatile additive synthesiser based on stacked, detuned saw oscillators

See https://nash.audio/klang >> KLANG EXAMPLES for online, interactive demos of the above.

## Usage in a C++ project

This object defines the processing for a single synth note that can then be used in any audio C++ project, placing the following code fragments at appropriate points in your code (e.g. myEffect/mySynth mini-plugin or any AU/VST plugin, JUCE app, etc.):

```
  klang::Subtractive note;
 
  note.start(pitch, velocity);   // Note On
  note.release(pitch, velocity); // Note Off
    
  klang::Buffer buffer = { pfBuffer, numSamples };
  if(!note.process(buffer))
     note.stop();
```
For ready-made AU/VST compatible C++ templates for Xcode and Visual Studio, see the github.com/nashnet/myeffect and github.com/nashnet/mysynth repositories.
rapIDE (Klang Studio) plugins also support a pure Klang live coding mode.

## Usage in the **Klang Studio** plugin

Klang Studio plugins feature native support for klang, providing a pure klang mode for synthesiser and effect plugin development, and will ultimately support generating/exporting of VS/Xcode/JUCE/WASM projects.

**Klang Studio (aka rapIDE)** (https://nash.audio/klang - coming soon) is a rapid audio prototyping (RAP) integrated development environment (IDE): a state-of-the-art C++ audio development environment... inside a DAW plugin, enabling the live C++ development of realtime audio processes, where you can open, edit, and recompile the code of the running plugin, within the DAW itself plugin's own UI and without having to stop and reload it. 

## More information

For more information about the project, or to get involved, email Chris Nash (klang@nash.audio).
