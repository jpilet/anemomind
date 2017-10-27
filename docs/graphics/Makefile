a4summary.pdf: a4summary/summary.svg
	"/usr/local/bin/inkscape" a4summary/summary.svg -A=a4summary.pdf --without-gui

a4details.pdf: a4details/top.tex
	cd a4details && xelatex top.tex && mv top.pdf ../a4details.pdf

a4productinfo.pdf: a4summary.pdf a4details.pdf
	pdfjoin --paper a4paper --rotateoversize false a4summary.pdf a4details.pdf -o a4productinfo.pdf

all: a4productinfo.pdf

disp: a4productinfo.pdf
	open a4productinfo.pdf
