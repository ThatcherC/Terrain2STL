default:
	gcc src/elevstl.c src/STLWriter.c src/elevation.c -o celevstl -lm

gelevstl:
	gcc gelevstl.c -o gelevstl -lgdal -I/usr/include/gdal
