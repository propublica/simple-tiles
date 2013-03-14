all: ne_10m_admin_0_countries.shp tl_2010_55_cd108.shp tl_2010_36047_roads.shp \
	 ne_10m_populated_places.shp

tl_2010_36047_roads.shp:
	curl -O http://www2.census.gov/geo/tiger/TIGER2010/ROADS/tl_2010_36047_roads.zip
	unzip tl_2010_36047_roads.zip

ne_10m_admin_0_countries.shp:
	curl -LO http://www.naturalearthdata.com/http//www.naturalearthdata.com/download/10m/cultural/ne_10m_admin_0_countries.zip
	unzip ne_10m_admin_0_countries.zip

ne_10m_populated_places.shp:
	curl -LO http://www.naturalearthdata.com/http//www.naturalearthdata.com/download/10m/cultural/ne_10m_populated_places.zip
	unzip ne_10m_populated_places.zip

tl_2010_55_cd108.shp:
	curl -O ftp://ftp2.census.gov/geo/tiger/TIGER2010/CD/108/tl_2010_55_cd108.zip
	unzip tl_2010_55_cd108.zip

.PHONY: all
