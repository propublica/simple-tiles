all: 10m_admin_0_countries.shp tl_2010_55_cd108.shp

10m_admin_0_countries.shp:
	curl -O http://www.nacis.org/naturalearth/10m/cultural/10m-admin-0-countries.zip
	unzip 10m-admin-0-countries.zip

tl_2010_55_cd108.shp:
	curl -O ftp://ftp2.census.gov/geo/tiger/TIGER2010/CD/108/tl_2010_55_cd108.zip
	unzip tl_2010_55_cd108.zip

.PHONY: all