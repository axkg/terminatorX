all:
	./render_all && ./merge && ./pixbuf

clean:
	rm -f knob*png merge*png knobs.pixbuf

tX_knob.png: tX_knob.pov
	/usr/local/share/povray-3.6/scripts/render_scene.sh tX_knob.pov
