#include <gdal/ogrsf_frmts.h>
#include <s2/s2loop.h>
#include <s2/s2polygon.h>
#include <s2/s2regioncoverer.h>
#include <s2/s2regionunion.h>

#include <inttypes.h>
#include <map>
#include <string>
#include <vector>

using std::map;
using std::string;
using std::vector;

/*
struct tzgeom {
	S2Polygon* poly;
	string* tzid;
};
*/
int main(int argc, char** argv) {
	if (argc != 2) {
		printf("Usage: %s [shapefile]\n", argv[0]);
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
	map<string, vector<S2CellId> > timezone_coverages;
	
	for (map<string, S2RegionUnion*>::iterator iter = timezones.begin(); iter != timezones.end(); ++iter) {
		vector<S2CellId> coverages;
		coverer.GetCovering(*(iter->second), &coverages);
		timezone_coverages[iter->first] = coverages;
	}

	printf("%d coverages built.\n", timezone_coverages.size());

#ifdef SINGLE_TZ
	// Dump out the coverages for each zone
	printf(SINGLE_TZ ":\n");
	vector<S2CellId> sydney = timezone_coverages[SINGLE_TZ];
	for (int x=0; x<sydney.size(); x++) {
		printf("%" PRIu64 ",\n", sydney[x].id());
	}
#endif
/*
	printf("%d geometries.\n", geometries.size());

	uint64 face = 3, max_level = 6;
	// Start at level 1 cells on that face
	const S2Cell start_cellid ((face << 61) + ((uint64)(1) << 60));

	// Find all the tzgeom that interset with this cell
	S2Polygon* cell_poly = new S2Polygon(start_cellid);
	for (int x=0; x<geometries.size(); x++) {
		geometries[x]->poly->Intersects(cell_poly);
	}
*/
	return 0;
}
