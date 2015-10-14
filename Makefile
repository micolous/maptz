
all: geojson_to_s2

geojson_to_s2: geojson_to_s2.cc
	g++ -o geojson_to_s2 geojson_to_s2.cc -I/usr/include/s2 -lgdal -ls2 -ls2cellid

coveragebundle.pb.cc: coveragebundle.proto
	protoc --cpp_out=. coveragebundle.proto
