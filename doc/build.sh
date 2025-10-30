#!/bin/bash

pdflatex main.tex
gs -sDEVICE=pdfwrite -dPDFSETTINGS=/ebook -q -o report.pdf main.pdf
rm main.pdf
