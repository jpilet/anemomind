a4details/lang.tex:
	cd a4details && ./configure_language.sh

a4summary.pdf: a4summary/summary.svg
	inkscape a4summary/summary.svg -A=a4summary.pdf --without-gui

priceflowen.eps: priceflowchart/priceflowen.dia
	cd priceflowchart && ./dia-cmd priceflowen.dia --export ../priceflowen.eps

priceflowfr.eps: priceflowchart/priceflowfr.dia
	cd priceflowchart && ./dia-cmd priceflowfr.dia --export ../priceflowfr.eps

A4DET = "a4details$(ANEMOLANG).pdf"

A4DET: a4details/top.tex priceflowen.eps priceflowfr.eps a4details/lang.tex
	cd a4details && xelatex top.tex && mv top.pdf "../$(A4DET)" && rm lang.tex

A4INFO = "a4productinfo$(ANEMOLANG).pdf"
A4INFO: a4summary.pdf A4DET
	pdfjoin --paper a4paper --rotateoversize false a4summary.pdf "$(A4DET)" -o "$(A4INFO)"

all: "$(A4INFO)"

clean:
	rm -f priceflow.eps "$(A4INFO)" a4details.pdf a4summary.pdf

# Display stuff
disp: A4INFO
	open "$(A4INFO)"

disp-details: A4DET
	open "$(A4DET)"
