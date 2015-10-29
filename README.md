# maptz

WIP geojson -> S2 coverage generator.

This converts a series of geojson polygon geometries into an index of S2 coverages, with customisable accuracy levels.  This uses the GDAL library for geometry file reading, so can also read other supported file formats (like ESRI Shapefile).

This is stored as a tree, then converted to protobuf format for small file size.  Currently this only has a C++ implementation.

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
