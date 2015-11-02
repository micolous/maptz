CXXFLAGS = -std=c++11 `pkg-config s2 --cflags --libs` -lprotobuf
CXXFLAGS_GDAL = ${CXXFLAGS} `gdal-config --libs --cflags`

all: geojson_to_s2 geojson_to_s2_test index_s2pb lookup_s2pb

geojson_to_s2: geojson_to_s2.cc coveragebundle.pb.cc
	g++ -o geojson_to_s2 geojson_to_s2.cc coveragebundle.pb.cc ${CXXFLAGS_GDAL}

geojson_to_s2_test: geojson_to_s2.cc coveragebundle.pb.cc
	g++ -o geojson_to_s2_test geojson_to_s2.cc coveragebundle.pb.cc ${CXXFLAGS_GDAL} -DSINGLE_TZ=\"Australia/Sydney\"

index_s2pb: index_s2pb.cc coveragebundle.pb.cc
	g++ -o index_s2pb index_s2pb.cc coveragebundle.pb.cc ${CXXFLAGS}

lookup_s2pb: lookup_s2pb.cc coveragebundle.pb.cc
	g++ -o lookup_s2pb lookup_s2pb.cc coveragebundle.pb.cc ${CXXFLAGS}

coveragebundle.pb.cc: coveragebundle.proto
	protoc --cpp_out=. coveragebundle.proto

clean:
	rm -f geojson_to_s2 geojson_to_s2_test index_s2pb lookup_s2pb coveragebundle.pb.cc coveragebundle.pb.h

