#include <s2/s2cellid.h>
#include <s2/s2latlng.h>

#include <fstream>
#include <inttypes.h>
#include <string>
#include <unordered_set>

#include "coveragebundle.pb.h"

using namespace std;

int main(int argc, char** argv) {
	GOOGLE_PROTOBUF_VERIFY_VERSION;
	if (argc != 4) {
		printf("Usage: %s [inpb] [latE6] [lngE6]\n", argv[0]);
		return 1;
	}
	
	// Lets open up our source file
	maptz::S2Index index;
	fstream input(argv[1], ios::in | ios::binary);
	if (!index.ParseFromIstream(&input)) {
		printf("Failed to parse input S2Index.\n");
		return 1;
	}

	// Convert to cellid from lat,lng
	S2LatLng ll = S2LatLng::FromE6(atol(argv[2]), atol(argv[3]));
	S2CellId cellid = S2CellId::FromLatLng(ll);
	
	// Now start lookup!
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

	// Start iterating and collecting results
	unordered_set<string> results;

	for (int l=1; l<=level; l++) {
		for (int i=0; i<(face->polyid_size()); i++) {
			results.insert(index.polylist(face->polyid(i)));
		}

		switch (cellid.child_position(l)) {
			case 0:
				if (face->has_child00()) {
					face = face->mutable_child00();
				} else {
					face = NULL;
				}
				break;
			case 1:
				if (face->has_child01()) {
					face = face->mutable_child01();
				} else {
					face = NULL;
				}
				break;
			case 2:
				if (face->has_child10()) {
					face = face->mutable_child10();
				} else {
					face = NULL;
				}
				break;
			case 3:
				if (face->has_child11()) {
					face = face->mutable_child11();
				} else {
					face = NULL;
				}
				break;
		}
		
		if (face == NULL) {
			break;
		}
	}

	printf("Result for cellid %s:\n", cellid.ToString().c_str());
	for (unordered_set<string>::iterator it=results.begin(); it!=results.end(); ++it) {
		printf(" - %s\n", (*it).c_str());
	}
	google::protobuf::ShutdownProtobufLibrary();
	
	return 0;
}

