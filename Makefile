
all: geojson_to_s2 geojson_to_s2_test index_s2pb lookup_s2pb

geojson_to_s2: geojson_to_s2.cc coveragebundle.pb.cc
	g++ -o geojson_to_s2 geojson_to_s2.cc coveragebundle.pb.cc -I/usr/include/s2 -lgdal -ls2 -ls2cellid -lprotobuf

geojson_to_s2_test: geojson_to_s2.cc coveragebundle.pb.cc
	g++ -o geojson_to_s2_test geojson_to_s2.cc coveragebundle.pb.cc -I/usr/include/s2 -lgdal -ls2 -ls2cellid -lprotobuf -DSINGLE_TZ=\"Australia/Sydney\"

index_s2pb: index_s2pb.cc coveragebundle.pb.cc
	g++ -o index_s2pb -std=c++11 index_s2pb.cc coveragebundle.pb.cc -I/usr/include/s2 -ls2 -ls2cellid -lprotobuf

lookup_s2pb: lookup_s2pb.cc coveragebundle.pb.cc
	g++ -o lookup_s2pb -std=c++11 lookup_s2pb.cc coveragebundle.pb.cc -I/usr/include/s2 -ls2 -ls2cellid -lprotobuf

coveragebundle.pb.cc: coveragebundle.proto
	protoc --cpp_out=. coveragebundle.proto

