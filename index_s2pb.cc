#include <s2/s2cellid.h>

#include <fstream>
#include <inttypes.h>
#include <unordered_set>
#include <string>

#include "coveragebundle.pb.h"

using namespace std;

int main(int argc, char** argv) {
	GOOGLE_PROTOBUF_VERIFY_VERSION;
	if (argc != 3) {
		printf("Usage: %s [inpb] [outpb]\n", argv[0]);
		return 1;
	}
	
	// Lets open up our source file
	maptz::CoverageBundleRaw timezone_coverages;
	fstream input(argv[1], ios::in | ios::binary);
	if (!timezone_coverages.ParseFromIstream(&input)) {
		printf("Failed to parse input CoverageBundleRaw.\n");
		return 1;
	}
	
	maptz::S2Index index;
	// Load out the available timezone names, creating a mapping
	unordered_set<string> zones;
	for (int i=0; i < timezone_coverages.timezone_size(); i++) {
		const maptz::CoverageBundleRaw::TzCoverage& tz = timezone_coverages.timezone(i);
		zones.insert(tz.tzid());
	}
	
	// Now we have a set with all the zones in it, which are guaranteed to be
	// unique. Lets dump that list into a mapping that we can use later on.
	map<string, uint16_t> zones_map;
	int i = 0;
	for (unordered_set<string>::iterator it=zones.begin(); it!=zones.end(); ++it, i++) {
		zones_map[*it] = i;
		index.add_polylist(*it);
		//printf("%s = %d\n", it->c_str(), i);
	}
	
	// Now start iterating through all the zones and build the structures around
	// it.
	for (int i=0; i < timezone_coverages.timezone_size(); i++) {
		const maptz::CoverageBundleRaw::TzCoverage& tz = timezone_coverages.timezone(i);
		uint16_t tz_numeric = zones_map[tz.tzid()];
		
		for (int j=0; j<tz.cell_size(); j++) {
			S2CellId cellid(tz.cell(j));
			
			// What is the level we need to iterate to?
			int level = cellid.level();
			
			// Find the face that this should belong to
			maptz::S2IndexNode* face;
			switch (cellid.face()) {
				case 0:
					face = index.mutable_face0();
					break;
				case 1:
					face = index.mutable_face1();
					break;
				case 2:
					face = index.mutable_face2();
					break;
				case 3:
					face = index.mutable_face3();
					break;
				case 4:
					face = index.mutable_face4();
					break;
				case 5:
					face = index.mutable_face5();
					break;
				default:
					printf("invalid face\n");
					return 1;
			}
			
			// Now we know the face, populate the tree with the bits we want...
			for (int l=1; l<=level; l++) {
				switch (cellid.child_position(l)) {
					case 0:
						face = face->mutable_child00();
						break;
					case 1:
						face = face->mutable_child01();
						break;
					case 2:
						face = face->mutable_child10();
						break;
					case 3:
						face = face->mutable_child11();
						break;
				}
				
				if (l == level) {
					// register ourselves here
					face->add_polyid(tz_numeric);
				}
			}
		}
	}
	
	// Dump to file
	fstream output(argv[2], ios::out | ios::trunc | ios::binary);
	if (!index.SerializeToOstream(&output)) {
		printf("Failed to write data to disk. :(\n");
		return 1;
	}
	google::protobuf::ShutdownProtobufLibrary();
	
	return 0;
}

