#!/bin/bash

pdflatex main.tex
gs -sDEVICE=pdfwrite -dPDFSETTINGS=/prepress -q -o report.pdf main.pdf
rm main.pdf
