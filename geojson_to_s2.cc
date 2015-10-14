#include <gdal/ogrsf_frmts.h>
#include <s2/s2loop.h>
#include <s2/s2polygon.h>
#include <s2/s2regioncoverer.h>
#include <s2/s2regionunion.h>

#include <fstream>
#include <inttypes.h>
#include <map>
#include <string>
#include <vector>

#include "coveragebundle.pb.h"

using namespace std;

int main(int argc, char** argv) {
	GOOGLE_PROTOBUF_VERIFY_VERSION;
	if (argc != 3) {
		printf("Usage: %s [shapefile] [outfile]\n", argv[0]);
		return 1;
	}

	OGRRegisterAll();
	OGRDataSource *poDS;
	poDS = OGRSFDriverRegistrar::Open(argv[1], FALSE);
	if (poDS == NULL) {
		printf("Open failed.\n");
		return 1;
	}

	// Sanity check layers
	int layerCount = poDS->GetLayerCount();
	printf("Found %d layer(s)\n", layerCount);
	if (layerCount != 1) {
		printf("there is not exactly 1 layer, bailing out!\n");
		return 1;
	}

	OGRLayer *poLayer;
	poLayer = poDS->GetLayer(0);
	if (poLayer->GetGeomType() != wkbPolygon) {
		printf("Expected polygon layer type!\n");
		return 1;
	}

	// Begin iterating geometries and converting it to a S2Polygon
	OGRFeature *poFeature;
	poLayer->ResetReading();
	map<string, S2RegionUnion*> timezones;

	printf("Processing geometries...\n");
	//vector<tzgeom*> geometries;
	while ((poFeature = poLayer->GetNextFeature()) != NULL) {
		OGRGeometry *poGeometry;
		poGeometry = poFeature->GetGeometryRef();
		if (poGeometry == NULL || wkbFlatten(poGeometry->getGeometryType()) != wkbPolygon) {
			printf("unexpected geometry type\n");
			return 1;
		}
	
		OGRPolygon* poPolygon = (OGRPolygon*) poGeometry;
		OGRLinearRing* poRing;
		poRing = poPolygon->getExteriorRing();
	
		// Now access as a linestring
		vector<S2Point> points;
		for (int x=poRing->getNumPoints()-1; x; x--) {
			S2LatLng poLL = S2LatLng::FromDegrees(poRing->getY(x), poRing->getX(x));
			points.push_back(poLL.ToPoint());
		}
	
		S2Loop* poLoop = new S2Loop(points);
		vector<S2Loop*> loops;
		loops.push_back(poLoop);
		S2Polygon* poS2Polygon = new S2Polygon(&loops);
		
		// Get the name of the TZ
		string tzid (poFeature->GetFieldAsString(poFeature->GetFieldIndex("TZID")));

#ifdef SINGLE_TZ
		if (tzid != SINGLE_TZ) continue;
#endif

		// Check if it is already defined
		if (timezones.count(tzid) == 0) {
			timezones[tzid] = new S2RegionUnion();
		}

		// Add it to our map.
		timezones[tzid]->Add(poS2Polygon);

	}

	// Report on the amount of Timezones we have.
	printf("Found %d zones.\n", timezones.size());

	// Setup a coverage generator
	S2RegionCoverer coverer;
	coverer.set_min_level(1);
	coverer.set_max_level(10);
	coverer.set_max_cells(10000);

	// Build coverages
	printf("Building coverages...\n");
	maptz::CoverageBundleRaw timezone_coverages;
	//map<string, vector<S2CellId> > timezone_coverages;
	
	for (map<string, S2RegionUnion*>::iterator iter = timezones.begin(); iter != timezones.end(); ++iter) {
		vector<S2CellId> cells;
		maptz::CoverageBundleRaw::TzCoverage* tz = timezone_coverages.add_timezone();
		tz->set_tzid(iter->first);

		coverer.GetCovering(*(iter->second), &cells);
		for (vector<S2CellId>::iterator citer = cells.begin(); citer != cells.end(); ++citer) {
			tz->add_cell(citer->id());
		}
	}

	printf("%d coverages built.\n", timezone_coverages.timezone_size());

#ifdef SINGLE_TZ
	// Dump out the coverages for each zone
	printf(SINGLE_TZ ":\n");
	const maptz::CoverageBundleRaw::TzCoverage sydney = timezone_coverages.timezone(0);
	for (int x=0; x<sydney.cell_size(); x++) {
		printf("%" PRIu64 ",\n", sydney.cell(x));
	}
#endif

	// Dump to file
	fstream output(argv[2], ios::out | ios::trunc | ios::binary);
	if (!timezone_coverages.SerializeToOstream(&output)) {
		printf("Failed to write data to disk. :(\n");
		return 1;
	}
	google::protobuf::ShutdownProtobufLibrary();
	return 0;
}
