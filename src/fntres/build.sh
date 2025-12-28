#!/usr/bin/bash
py -3.13 ./ttf2h8x16.py --ttf ./ttf/pocket.ttf --w 8 --h 16 --pt 16 --threshold 128 --out ./pocket8x16.h --name pocket8x16
py -3.13 ./ttf2h8x16.py --ttf ./ttf/roboto.ttf --w 8 --h 16 --pt 16 --threshold 128 --out ./roboto8x16.h --name roboto8x16
py -3.13 ./ttf2h8x16.py --ttf ./ttf/corona.ttf --w 8 --h 16 --pt 16 --threshold 128 --out ./corona8x16.h --name corona8x16

echo "# include \"pocket8x16.h\"" > pocket8x16.cc
echo "# include \"roboto8x16.h\"" > roboto8x16.cc
echo "# include \"corona8x16.h\"" > corona8x16.cc

py -3.13 ./ttf2h12x16.py --ttf ./ttf/pocket.ttf --w 12 --h 16 --pt 16 --threshold 128 --out ./pocket12x16.h --name pocket12x16
py -3.13 ./ttf2h12x16.py --ttf ./ttf/roboto.ttf --w 12 --h 16 --pt 16 --threshold 128 --out ./roboto12x16.h --name roboto12x16
py -3.13 ./ttf2h12x16.py --ttf ./ttf/corona.ttf --w 12 --h 16 --pt 16 --threshold 128 --out ./corona12x16.h --name corona12x16

echo "# include \"pocket12x16.h\"" > pocket12x16.cc
echo "# include \"roboto12x16.h\"" > roboto12x16.cc
echo "# include \"corona12x16.h\"" > corona12x16.cc
