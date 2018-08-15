name   := paper
viewer := mupdf
attach := attachments

sections := $(wildcard sections/*.tex)

all: $(name).pdf
view: $(name).pdf
	$(viewer) $(name).pdf

$(name).pdf: $(name).tex $(sections) $(name).bib
	mkdir -p build
	pdflatex -output-directory build/ $(name)
	bibtex build/$(name)
	pdflatex -output-directory build/ $(name)
	pdflatex -output-directory build/ $(name)
	mv build/$(name).pdf .

polyglot: polyglot/$(name).pdf
polyglot/$(attach).zip: FORCE
	mkdir -p polyglot
	zip -r polyglot/$(attach).zip $(attach)
polyglot/$(name).pdf: $(name).pdf polyglot/$(attach).zip
	cat $(name).pdf polyglot/$(attach).zip > polyglot/$(name).pdf
	zip -A polyglot/$(name).pdf
distribute: distclean all polyglot
	cp polyglot/$(name).pdf .

clean:
	rm -rf build
distclean: clean
	rm -rf polyglot
	rm -f $(name).pdf
.PHONY: all view polyglot clean distclean distribute
FORCE:
