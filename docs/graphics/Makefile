a4summary.pdf: a4summary/summary.svg
	inkscape a4summary/summary.svg -A=a4summary.pdf --without-gui

priceflow.eps: priceflowchart/priceflow.dia
	cd priceflowchart && ./dia-cmd priceflow.dia --export ../priceflow.eps

a4details.pdf: a4details/top.tex priceflow.eps
	cd a4details && xelatex top.tex && mv top.pdf ../a4details.pdf

a4productinfo.pdf: a4summary.pdf a4details.pdf
	pdfjoin --paper a4paper --rotateoversize false a4summary.pdf a4details.pdf -o a4productinfo.pdf

all: a4productinfo.pdf

clean:
	rm -f priceflow.eps a4productinfo.pdf a4details.pdf a4summary.pdf

# Display stuff
disp: a4productinfo.pdf
	open a4productinfo.pdf

disp-details: a4details.pdf
	open a4details.pdf
