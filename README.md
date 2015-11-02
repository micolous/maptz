# maptz

WIP geojson -> S2 coverage generator.

This converts a series of geojson polygon geometries into an index of S2 coverages, with customisable accuracy levels.  This uses the GDAL library for geometry file reading, so can also read other supported file formats (like ESRI Shapefile).

This is stored as a tree, then converted to protobuf format for small file size.  Currently this only has a C++ implementation.

This requires libgdal-dev, libprotobuf-dev, and [libs2-dev](https://github.com/micolous/s2-geometry-library).

## Creating the database

At present, this is built around a [timezone map of the world](http://efele.net/maps/tz/world/).

Create the cell list file with the following command:

```sh
./geojson_to_s2 world_tz.shp world_tz.pb 11
```

This will generate a coverage file of detail level 11.  Valid detail levels are 1 - 30, which correspond to the size of S2 cells.  Each increase in detail level will at least double the filesize.

This takes a while to build, and creates a protobuf of the message type `CoverageBundleRaw`.

## Indexing the database

Once a list of coverages is created, they must be indexed.

```sh
./index_s2pb world_tz.pb world_tz_idx.pb
```

This index file can be used on it's own, and is simply a rearrangement of the data that is in the `CoverageBundleRaw`.

The protobuf message type for this file is `S2Index`, and forms six 4-ary trees.

## Performing lookups

Lookups convert the input lat/long (WGS84 datum) into an S2 cell ID, then the address of this cell is looked up in the tree.  The program used here takes the input as degrees of latitude and longitude, multiplied by 10^6.  South and West are negative values.

The tree must be walked in order to properly capture the results.

Broken Hill, NSW, Australia:

```console
$ ./lookup_s2pb world_index.pb -31956700 141467800
Result for cellid 3/111313111201110103320001003010:
 - Australia/Broken_Hill
```

Kyoto, Japan:

```console
$ ./lookup_s2pb world_index.pb 35011667 135768333
Result for cellid 3/000000201010123002102101101110:
 - Asia/Tokyo
```

Because the coverage is generated to "overlap" a certain polygon (rather than be contained within it), some cells near borders may be return multiple results, which could be wrong.  The error margin is a cell of up to the size of the smallest cell generated.

Each region will be represented by up to 10,000 cells (hard coded limit).  While L30 cells could be used, in reality the maximum cell count will be hit for most large regions.

A coverage to an accuracy of level 10 will have an error of up to about 53-102 km² (depending on distance from the equator).  At level 11, this reduces to 13-26 km², and at level 12 this reduces to 3.3-6.4 km².  Each increase typically doubles file size.

