a4summary.pdf: a4summary/summary.svg
	inkscape a4summary/summary.svg -A=a4summary.pdf --without-gui

priceflowchart.pdf: priceflowchart/priceflowchart.svg
	inkscape priceflowchart/priceflowchart.svg -A=priceflowchart.pdf --without-gui

croppedpricechart.pdf: priceflowchart.pdf
	pdfcrop priceflowchart.pdf croppedpricechart.pdf

a4details.pdf: a4details/top.tex croppedpricechart.pdf
	cd a4details && xelatex top.tex && mv top.pdf ../a4details.pdf

a4productinfo.pdf: a4summary.pdf a4details.pdf
	pdfjoin --paper a4paper --rotateoversize false a4summary.pdf a4details.pdf -o a4productinfo.pdf

all: a4productinfo.pdf priceflowchart.pdf

clean:
	rm a4productinfo.pdf a4details.pdf a4summary.pdf priceflowchart.pdf

# Display stuff
disp: a4productinfo.pdf
	open a4productinfo.pdf

disp-details: a4details.pdf
	open a4details.pdf
