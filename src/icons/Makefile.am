
tX_knob_images = knob0.png knob1.png knob2.png\
		knob3.png knob4.png knob5.png knob6.png knob7.png knob8.png \
		knob9.png knob10.png knob11.png knob12.png knob13.png knob14.png\
		knob15.png knob16.png knob17.png knob18.png knob19.png knob20.png\
		knob21.png knob22.png knob23.png knob24.png knob25.png knob26.png\
		knob27.png knob28.png knob29.png knob30.png knob31.png knob32.png\
		knob33.png knob34.png knob35.png knob36.png knob37.png knob38.png\
		knob39.png knob40.png knob41.png knob42.png knob43.png knob44.png\
		knob45.png knob46.png knob47.png knob48.png knob49.png

tX_icons = audioengine.svg fx_close.svg fx_down.svg fx_up.svg\
					 grab.svg maximize.svg minimize_panel.svg minimize.svg\
					 play.svg power.svg record.svg sequencer.svg stop.svg\
					 samples.svg control.svg

tX_icons_resource_file = tX_icons.gresource.xml
tX_knob_resource_file = tX_knob.gresource.xml
tX_dialog_resource_file = tX_dialog.gresource.xml

BUILT_SOURCES = tX_knob_resources.c tX_icons_resources.c tX_dialog_resources.c
EXTRA_DIST = 	$(tX_knob_resource_file) $(tX_knob_images) $(tX_icons_resource_file) $(tX_icons) $(tX_dialog_resource_file)

tX_knob_resources.c: $(tX_knob_resource_file) $(tX_knob_images)
	$(GLIB_COMPILE_RESOURCES) --target=$@ --internal --generate-source $(tX_knob_resource_file)

tX_icons_resources.c: $(tX_icons_resource_file) $(tX_icons)
	$(GLIB_COMPILE_RESOURCES) --target=$@ --internal --generate-source $(tX_icons_resource_file)

tX_dialog_resources.c: $(tX_dialog_resource_file) ../../icons/terminatorX.png
	$(GLIB_COMPILE_RESOURCES) --target=$@ --internal --generate-source $(tX_dialog_resource_file)


