#!/usr/bin/bash
py -3.13 ./ttf2hdr.py --ttf ./ttf/pocket.ttf --w 8 --h 16 --pt 16 --threshold 128 --out ./pocket8x16.h --name pocket8x16
py -3.13 ./ttf2hdr.py --ttf ./ttf/roboto.ttf --w 8 --h 16 --pt 16 --threshold 128 --out ./roboto8x16.h --name roboto8x16
py -3.13 ./ttf2hdr.py --ttf ./ttf/corona.ttf --w 8 --h 16 --pt 16 --threshold 128 --out ./corona8x16.h --name corona8x16

echo "# include \"pocket8x16.h\"" > pocket8x16.cc
echo "# include \"roboto8x16.h\"" > roboto8x16.cc
echo "# include \"corona8x16.h\"" > corona8x16.cc
