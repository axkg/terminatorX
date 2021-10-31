Tuning terminatorX' performance
===============================

There are several things you can do to improve terminatorX performance:

+ Don't use fancy gtk+ themes
+ Install suid root
+ Compile an optimized binary
+ Optimize your terminatorX setup

In Detail:

Don't use pixmapped nor other "fat" gtk+-themes
-----------------------------------------------

terminatorX involves quite some GUI activity, so it's desirable that drawing
happens fast. Some gtk+ engines can slow down drawing signifcantly.

2. Install suid root
--------------------

Note: Installing a program suid-root is always potentially dangerous. However,
a program needs special privileges to acquire realtime scheduling (which 
improves playback quality signifcantly). However, only when running suid-root
terminatorX can acquire realtime scheduling priorty to avoid buffer underruns
with low latency settings and the mouse motion events can be captured directly
from the hardware improving scratching precision.

When running setuid-root, terminatorX makes use of Linux' POSIX capabilities:
Right after start-up it acquires the CAP_SYS_NICE capabilty and accesses the
Linux input interface before dropping root privileges for good.

While the capabilities based approach seems much more secure than the approach
that was previously implemented, it might still be exploitable. So, for 100%
security you have to do without realtime scheduling and not install terminatorX
suid root.

3. Compile an optimized binary
------------------------------

This issue is covered in the seperate INSTALL file. Setting good optimization
flags is a good idea although this step will proabably have the least effect
and the defaults should be reasonable.

4. Optimize your terminatorX setup
----------------------------------

The default settings for the GUI updates provide good realtime feedback. This
may cause problems on slower machines or slow gtk+-themes. If the GUI-thread
causes dropouts in the audio-engine you should increase the Update-Delay value 
in the options dialog.
