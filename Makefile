.PHONY: format

default:
	gcc src/elevstl.c src/STLWriter.c src/elevation.c -o celevstl -lm

gelevstl: gelevstl.c
	g++ gelevstl.c -o gelevstl -lgdal -I/usr/include/gdal

g2elevstl: gelevstl2.c
	gcc gelevstl2.c -o gelevstl2 -lgdal -I/usr/include/gdal

format:
	clang-format-11 -i gelevstl.c
