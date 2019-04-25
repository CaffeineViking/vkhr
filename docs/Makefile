name   := vkhr
viewer := mupdf

all: $(name).pdf
view: $(name).pdf
	$(viewer) $(name).pdf

$(name).pdf: $(name).tex $(name).bib
	mkdir -p build
	pdflatex -shell-escape -interaction=nonstopmode -output-directory build/ $(name)
	biber build/$(name)
	pdflatex -shell-escape -interaction=nonstopmode -output-directory build/ $(name)
	pdflatex -shell-escape -interaction=nonstopmode -output-directory build/ $(name)
	mv build/$(name).pdf .

clean:
	rm -rf build
distclean: clean
	rm -f $(name).pdf
.PHONY: all view clean distclean
FORCE:
