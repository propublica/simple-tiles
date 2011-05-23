#! /usr/bin/bash

if [ ! -e tl_2010_us_cd108.shp ]
then
  curl -O ftp://ftp2.census.gov/geo/tiger/TIGER2010/CD/108/tl_2010_us_cd108.zip
  unzip *.zip
fi