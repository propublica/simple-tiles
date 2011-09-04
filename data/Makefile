all: 10m_admin_0_countries.shp tl_2010_55_cd108.shp tl_2010_36047_roads.shp \
	 ne_10m_populated_places.shp

tl_2010_36047_roads.shp:
	curl -O ftp://ftp2.census.gov/geo/tiger/TIGER2010/ROADS/tl_2010_36047_roads.zip
	unzip tl_2010_36047_roads.zip

10m_admin_0_countries.shp:
	curl -O http://www.nacis.org/naturalearth/10m/cultural/10m-admin-0-countries.zip
	unzip 10m-admin-0-countries.zip

ne_10m_populated_places.shp:
	curl -O http://www.nacis.org/naturalearth/10m/cultural/10m-populated-places.zip
	unzip 10m-populated-places.zip

tl_2010_55_cd108.shp:
	curl -O ftp://ftp2.census.gov/geo/tiger/TIGER2010/CD/108/tl_2010_55_cd108.zip
	unzip tl_2010_55_cd108.zip

.PHONY: all
