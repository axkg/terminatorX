section = main download faq screenshots scratches turntable aseqjoy docs

all: index.html aseqjoy.html download.html faq.html changelog.html scratches.html docs.html turntable.html screenshots.html links.html  bugs.html changelog.txt

XMLFILES=$(wildcard *.xml) $(wildcard *.xsl)

pretty:
	list='$(XMLFILES)'; for file in $$list; do tidy -i -w 80 -xml < $$file > tmp; if [ $$? -eq 0 ]; then mv tmp $$file; else echo error: $$?; fi; done

clean:	Makefile
	rm -rf *.html *.tmp *.txt

%.txt:	%.txt.html
	lynx -dump $*.txt.html > $*.txt

%.txt.html:	%.xml teXt.xsl tX.dtd sedscript linkdb.xml
	xsltproc teXt.xsl $*.xml > $*.tmp && sed -f sedscript < $*.tmp | (echo  '<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN" "http://www.w3.org/TR/REC-html401/loose.dtd">'; cat) > $*.txt.html ; rm $*.tmp

%.html:	 %.xml tX.xsl tX.dtd sedscript linkdb.xml
	xsltproc tX.xsl $*.xml > $*.tmp && sed -f sedscript < $*.tmp | (echo  '<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN" "http://www.w3.org/TR/REC-html401/loose.dtd">'; cat) > $*.html ; rm $*.tmp; htmlcr -O $*.html

dist: 	all
	(mkdir tX_website; mkdir tX_website/pix tX_website/pix/banner; cp Makefile createpreview sedscript script.js *.xml tX.dtd tX.xsl tX_website; cp pix/* tX_website/pix; cp pix/banner/* tX_website/pix/banner; tar cjf tX_website.tar.bz2 tX_website; rm -rf tX_website)
	
install: all
	cp -f script.js *.html ~/.public_html/tX
