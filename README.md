
# Table of Contents

1.  [Modules For Structure](#org84959db)
    1.  [Gko](#org85d0cba)
        1.  [Scene Selection](#orgea803b5)
        2.  [Recording with Gko](#orgd3c2304)
        3.  [Layer Attenuation](#orge8bf87a)
        4.  [Button Behaviour](#orgeb0d429)
        5.  [Gko Additional Uses Cases](#org553d53b)
    2.  [Gko Input/Output Modules](#org291c177)
        1.  [SIG](#org456bc5f)
        2.  [4SIG](#org2de341a)
        3.  [PLAY](#org8b0c698)
        4.  [Gko-X](#org792a3b4)
    3.  [TIME](#org7b1d09d)
2.  [Modules for a Sound Interface](#orgdfe4390)
    1.  [MACRO](#orgc33c75b)
    2.  [M-PARAM](#org6fc29b8)
    3.  [M-OUT](#orga32c19c)
    4.  [M-IN](#orgad2a709)
    5.  [INTERFACE](#orgf7c18a6)


<a id="org84959db"></a>

# Modules For Structure

These modules, provide the ability to record, loop, edit, playback, and sequence
sigs. I designed the interface for fluidity and simple controls, making them
suitable for live recording where one can not pause to click and clack.

The sig captured may be audio, in which case, these modules act as a loop
recorder that can create multi-layered musical sections, and then sequence these
sections. If capturing control voltage one could use these to record 
and sequence parameter automation, or as a keyframer. 

![img](img/structure_modules.png)


<a id="org85d0cba"></a>

## Gko

The function of `Gko` is to record and loop the input sigs of the modules on
it's left. These modules - `PLAY`, `SIG`, and `SIG4`, - define the inputs and
outputs of `Gko`. One can place multiple side by side to introduce more inputs
and outputs.

Despite the simple interface, one can use `Gko` as a multi-track looper, a
sequencer for musical sections, a delay unit, a sample & hold unit, a sequencer,
and more! 

There is also a non input-output expander for `Gko` called `Gko-X`. This module
provides additional `RATE` and `POS` parameters, as well as additional control
ports.  It enables more creative ways to use `Gko`.

`Gko` supports [polyphonic](https://vcvrack.com/manual/Polyphony) input.


<a id="orgea803b5"></a>

### Scene Selection

A scene is simply a collection of looping buffers, or, in frame's terminology, a
collection of 'layers'.

The `SCENE` parameter adjusts which scene `Gko` records and reads from.
The port directly under `SCENE` modulates the parameter, allowing for
interesting crossfade patterns, or voltage controlled scene sequencing.

One may use multiple `SCENEs` to sequence musical sections or, in the case `Gko`
is capturing control voltage, a keyframer for control voltage.

Scenes that have not recorded anything start with the layers of the scene
before, so one may progress to the next scene to leave behind a 'checkpoint',
that one may return to, or to just start developing a new scene by altering the
current one.


<a id="orgd3c2304"></a>

### Recording with Gko

When `DELTA` has moved a small threshold away from 12 o'clock, `Gko` will
start recording. The scene that `Gko` chooses for recording, is the scene that
is closest to the sum of the `SCENE` parameter and `SCENE` port.

If `Gko` detects a clock sig on the `CLK` port, it will snap the  recording start
and end points to mutiples of the clock period. 

Recording behaviour depends on whether `Gko` is in *global* mode, or *layer*
mode. The `DELTA_MODE` button (up-right of `DELTA`) toggles between these modes.
The upper LED in the delta section indicates its state.

Recording behaviour also depends on the direction of `DELTA`'s rotation. If it
moves clockwise, it activates the *extend* record mode. If `DELTA` moves
counter-clockwise, it activates *add* mode. When `DELTA` returns to 12 o'clock,
`Gko` finishes recording. The lower LED in the delta section indicates the
current recording mode.

The record and delta modes create four recording behaviours when combined.

1.  *global* and *extend* mode

    `Gko` records the input sig into a new layer, and loops the layer when
    `DELTA` returns to 12 o'clock.
    
    Used for dubbing a loop with elements that are longer than the current loop
    length - for example, adding a chord progression for a repeating phrase.

2.  *global* and *add* mode

    `Gko` records the input sig into a new layer with the same length as the
    active layer. On reaching the end, it repeats the process.
    
    Used for continuously recording multiple takes to audition and filter later
    on, or record new layers continuously.

3.  *layer* and *extend* mode

    `Gko` records the input sig into the active layer, and upon reaching the
    end, will continue recording as well as extend the active layer by looping
    the old contents.
    
    Used for creating variation at the xth repetition of a layer.

4.  *layer* and *add* mode

    `Gko` records the input sig into the active layer and when it reaches the
    loop end, it repeats the process.
    
    Used for continuously overdubbing a layer.


<a id="orge8bf87a"></a>

### Layer Attenuation

In all four mode combinations, if one twists further than the threshold
position, it affects the amplitude of previous layers at the current position,
or in other words, the 'attenuation power' of the recording. If delta mode is
*layer*, it only attenuates the active layer, if delta mode is *global*, it
attenuates all layers.

The attenuation power grows exponentially as `DELTA` twists, and when it reaches
a maximum, it will erase previous layers.

This attenuation behaviour allows for easily 'pushing back' previous layers in a
live-looping performance to create more movement. It also allows for editting
existing layers by re-recording certain parts. In the case there is no input
sig, it creates attenuation envelopes and when fully turned, erases parts of
previous layers.


<a id="orgeb0d429"></a>

### Button Behaviour

Excluding the `DELTA_MODE` button, there are four other buttons on `Gko`.

The button to the upper left of `DELTA` is the `UNDO` button. `Gko` keeps
track of states before and after engaging record modes, and `UNDO` recalls the
previous states. When one presses `UNDO` in a record mode, `Gko` will discard
any changes, and try again on the next loop start of the selected layer.

The `PREV` and `NEXT` buttons change the active layer, and the `PLAY` button
resets all layer positions to the beginning.


<a id="org553d53b"></a>

### Gko Additional Uses Cases

1.  Delay Unit

    `Gko` can function as a delay unit in the case *add* mode is consistently on
    in *layer* mode. `DELTA` would control the feedback in this case.
    
    One may use `Gko-X` to change the rate and offset of the delays to produce
    cool delay effects.


<a id="org291c177"></a>

## Gko Expansion Modules


<a id="org456bc5f"></a>

### SIG

`SIG` takes an arbitrary sig as input, sends it to
`Gko`, and outputs a mix of the input sig and output from `Gko`.

It also outputs `Gko`'s selected layer(s). This is useful in the case of
applying audio functions (or sig functions) to particular layers in
`Gko`. To do this, one would select a layer, route `SEL` into other VCV Rack
modules, route the output of those modules back into the input, and modify the
layer by engaging recording in *layer* mode.

1.  MIX

    -   At 7 O'clock, `SIG` only outputs `Gko` output, and the input sig is fully
        attenuated. This is useful to control the input power, but also in the case multiple expansion modules exist so to not record this input sig when `Gko` enters a record mode.
    -   At 12 O'clock, the input sig is not attenuated.
    -   At 5 O'clock, the input sig is still not attenuated, and `SIG` outputs 100% of
        `Gko`'s *active layer*. Used for auditioning multiple takes that were loop
        recorded, and for using *layer* mode without sonic clutter from other layers.

2.  VCA

    A VCA for the output. Used for setting or modulating the output volume.


<a id="org2de341a"></a>

### 4SIG

`4IGNAL` is `SIG`, just with 4 ports instead of 1. Its used for capturing
multiple sigs, as it saves space compared to 4 `SIG` modules set side by
side.


<a id="org8b0c698"></a>

### PLAY

`PLAY` takes 3 polyphonic (or monophonic) sigs associated with MIDI recording
as input.

`PLAY` functions just like `SIG`, with a difference in how it attenuates sigs.
Attenuation only affects VEL (velocity) sigs until max attenuation, where it
also removes GATE sigs and holds VOCT sigs.


<a id="org792a3b4"></a>

### Gko-X

This module is an expander for `Gko`. When placed on its right side, it gives
it extra `RATE`, and `POS` parameters, as well as ports for controlling `PREV`,
`NEXT`, and `PLAY`.

This module enables more ways to use `Gko`.

The `POS` parameter controls the start offset of the layers in the scene.

The `RATE` parameter controls the speed at which `Gko` plays back the layers
in the scene.

All the button ports react to rising edges. The ports underneath `POS` and
`RATE` modulate the parameters.

1.  Gko-X Usage Ideas

    1.  Pitch Shifter
    
        When one sets up `Gko` as a delay unit with a small layer size and adjusts
        the `RATE` of `Gko-X`, it will seem like the pitch of the sound is higher or
        lower.
    
    2.  Advanced Sample & Hold / Sequencer
    
        When one sets `RATE` to 0, `Gko` does not progress at all but still may record
        and read sigs. In this case, it acts as an advanced sample and hold module.
        Adjusting the `SCENE` knob smoothly transitions between samples.
        
        One may sequence samples in interesting ways using the `SCENE` modulation port.
    
    3.  Advanced 'MIDI' Looper
    
        When `Gko` is expanding `PLAY`, one may create interesting playback patterns
        by recording some GATE, VOCT, and VEL sigs, and varying or modulating the
        `RATE` and `POS` ports. One idea is to record a chord, and modulate `RATE` and
        `POS` with low frequency noise sources with channel variation to create
        fluctuating, dreamy note sequences.
    
    4.  An Instrument
    
        One may patch the `RATE` port with a VOCT sig, and the `PLAY` port with a
        GATE sig, patch the output VCA with a GATE controlled envelope, and play
        `Gko` as if it were an instrument.
        
        This use case applies to all the additional use cases below.
    
    5.  Wonky Audio Playback Unit
    
        One may patch the `RATE` port to modulate the speed of playback and recording,
        and one may patch the `POS` port to modulate the offset of `Gko` layers.
        Using these, one could get some cool sounds with `Gko` - especially if there
        is variation across channels. Have you ever wondered what playing back speech
        with a sin wave sounds like? I have.
    
    6.  Wavetable Oscillator with Additive and Subtractive Synthesis Capabilities
    
        `Gko` can be a wavetable oscillator if either the `CLK` rate is high, or a
        high frequency saw wave is input into `POS`.
        
        In this use case, the `SCENE` parameter morphs between recorded waves, and the
        `DELTA` parameter would add or subract from a `SCENEs` wave.
    
    7.  Granular Synthesis Engine Component
    
        To use `Gko` as a granulart synthesis engine component, one would record an
        audio sig, then patch a constant polyphonic sig with channel variation
        into `POS`.
        
        To create the grains, one would patch the `VCA` in `SIG` with short, repeating
        envelopes with phase variation across channels.


<a id="org7b1d09d"></a>

## TIME

Recall how `Gko` keeps track of it's state before and after a recording starts.
The `TIME` module is essentially a sequencer for all `Gko` module states. One
could leave `TIME` in record mode, record a live-looping performance with
`Gko`, rewind time via the `TIME` module, and play back the performance.
Rewinding / scrubbing time via the `TIME` module will also scrub the playback
position of all `Gko`'s so to preserve deterministic playback.

TODO


<a id="orgdfe4390"></a>

# Modules for a Sound Interface

These modules are for the [sound interface](https://github.com/gwatcha/sound-interface).


<a id="orgc33c75b"></a>

## MACRO

Defines a macro. A macro consists of one or multiple `M-PARAM`, and `M-IN` modules,
a strip of modules that follow it, and one or multiple `M-OUT` modules after the
strip, which touch the `MACRO` module.

User can enter a name for the macro, and save to file similar
to stoermelders `STRIP`. The user can also use it to load macro files. 


<a id="org6fc29b8"></a>

## M-PARAM

8 parameter mapping slots, with a place to enter an optional alias for the
parameter.

These mappings define the parameters of a macro.

Placed on the left side of a strip of modules.


<a id="orga32c19c"></a>

## M-OUT

8 in ports, with place to enter names, as well as labels about type of sig

-   gates (blue), clks (purple), triggers (light blue), control (yellow), audio
    (red), voct (green)

These ports define the output of the macro. they can be routed via OSC to any
`M-IN` module, as well as recorded via a `Gko` like interface.

Placed on the right side of a strip of modules.


<a id="orgad2a709"></a>

## M-IN

8 out ports, with place to enter names, as well as labels about type of sig.

These ports define the inputs for a macro. The sig on them can come from any `M-OUT` module.

Placed on the left side of a strip of modules.


<a id="orgf7c18a6"></a>

## INTERFACE

This  module is the brains of the Sound Interface. The user inputs an address
where it will listen for OSC messages from the controller. It will react to
these messages and do multiple tasks, such  as 

-   control the routings between `M-OUT` and `M-IN` modules. (control macro routing)
-   control the values of `M-PARAM` modules. (control macro parameters)
-   record and loop `M-OUT` module outputs ('frame' macros)
-   intelligently disable modules that have a recording downstream of
    the routing graph.
-   change active macros
-   control the timeline

