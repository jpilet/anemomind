a4summary.pdf: a4summary/summary.svg
	"/usr/local/bin/inkscape" a4summary/summary.svg -A=a4summary.pdf --without-gui

a4details.pdf: a4details/top.tex
	cd a4details && xelatex top.tex && mv top.pdf ../a4details.pdf

all: a4summary.pdf a4details.pdf
