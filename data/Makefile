all: ne_10m_admin_0_countries.shp tl_2010_55_cd108.shp tl_2010_36047_roads.shp \
	 ne_10m_populated_places.shp

tl_2010_36047_roads.shp:
	wget ftp://ftp2.census.gov/geo/tiger/TIGER2010/ROADS/tl_2010_36047_roads.zip
	unzip tl_2010_36047_roads.zip

ne_10m_admin_0_countries.shp:
	wget http://www.naturalearthdata.com/http//www.naturalearthdata.com/download/10m/cultural/ne_10m_admin_0_countries.zip
	unzip ne_10m_admin_0_countries.zip

ne_10m_populated_places.shp:
	wget http://www.naturalearthdata.com/http//www.naturalearthdata.com/download/10m/cultural/ne_10m_populated_places.zip
	unzip ne_10m_populated_places.zip

tl_2010_55_cd108.shp:
	wget ftp://ftp2.census.gov/geo/tiger/TIGER2010/CD/108/tl_2010_55_cd108.zip
	unzip tl_2010_55_cd108.zip

.PHONY: all