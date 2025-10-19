#!/bin/bash

gs -sDEVICE=pdfwrite -dPDFSETTINGS=/ebook -q -o report.pdf main.pdf
