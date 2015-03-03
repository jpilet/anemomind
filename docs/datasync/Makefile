# inspired by http://robjhyndman.com/hyndsight/makefiles/
# http://stackoverflow.com/questions/2706064/compile-all-c-files-in-a-directory-into-separate-programs

DIAFILES = $(wildcard *.dia)
EPSFILES = $(patsubst %.dia,%.eps,$(DIAFILES))

all: top.pdf $(EPSFILES)

top.pdf: top.tex $(EPSFILES)
	pdflatex top.tex

%.eps: %.dia
	dia $< --export $@

clean:
	rm -f *.aux
	rm -f *.log
	rm -f *.pdf
	rm -f *.eps
	rm -f *~ # emacs autosave files.
