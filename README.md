# terminatorX

Copyright (C) 1999-2024 by Alexander König <alex@lisas.de> - https://terminatorX.org

## About

terminatorX is a realtime audio synthesizer that allows “scratching” on
digitally sampled audio data (*.wav, *.mp3, etc.) the way hip-hop DJs scratch on
vinyl records. 

The GTK+ user interface provides many features, most notably:
* multiple turntables
* realtime effects (buit-in as well as [LADSPA plugin effects](https://www.ladspa.org/))
* a sequencer and MIDI interface

This software is designed to run under Linux, FreeBSD and the like.

terminatorX is **free software**; see the [COPYING file](COPYING) that came with this
distribution for details.

WARNING: terminatorX comes with *ABSOLUTELY NO WARRANTY*. This software may
lock up your machine or cause other problems under rare conditions. It is not
recommended to run it on production servers.

## Documentation

 * [INSTALL](INSTALL.md)
   Information on installing terminatorX
 * [help/C/index.docbook](help/C/index.docbook)
   The user manual browsable with "yelp" (yelp is the gnome help browser).
 * [COPYING](COPYING)
   The license (GPL V2)

## Performance

The following measures can help to improve the responsiveness as well as the
precision when using terminatorX.

### Install Setuid 'root'

DISCLAIMER: *Installing a program suid-root is always potentially dangerous*.
However, a program will require root privileges to acquire realtime scheduling
(which improves playback quality signifcantly). Only when running suid-root can
terminatorX acquire realtime scheduling priorty to avoid buffer underruns with
low latency settings and the mouse motion events can be captured directly from
the hardware improving scratching precision.

When running setuid-root, terminatorX makes use of Linux' POSIX capabilities:
Right after start-up it acquires the CAP_SYS_NICE capabilty and accesses the
Linux input interface before dropping root privileges for good.

While the capabilities based approach seems much more secure than the approach
that was previously implemented, it might still be exploitable. So, for 100%
security you have to do without realtime scheduling and not install terminatorX
suid root.

### Compile an Optimized Binary

This issue is covered in the seperate [INSTALL file](INSTALL.md). Setting good
optimization flags is a good idea although this step will proabably have the
least effect and the defaults should be reasonable.

### Optimize Your terminatorX Setup

The default settings for the GUI updates provide good realtime feedback. This
may cause problems on slower machines or slow gtk+-themes. If the GUI-thread
causes dropouts in the audio-engine you should increase the Update-Delay value 
in the options dialog.

If you experience issues with the performance of the user interface, consider
using a simpler GTK+ theme.