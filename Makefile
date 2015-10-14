
all: geojson_to_s2

geojson_to_s2: geojson_to_s2.cc coveragebundle.pb.cc
	g++ -o geojson_to_s2 geojson_to_s2.cc coveragebundle.pb.cc -I/usr/include/s2 -lgdal -ls2 -ls2cellid -lprotobuf

geojson_to_s2_test: geojson_to_s2.cc coveragebundle.pb.cc
	g++ -o geojson_to_s2_test geojson_to_s2.cc coveragebundle.pb.cc -I/usr/include/s2 -lgdal -ls2 -ls2cellid -lprotobuf -DSINGLE_TZ=\"Australia/Sydney\"


coveragebundle.pb.cc: coveragebundle.proto
	protoc --cpp_out=. coveragebundle.proto
