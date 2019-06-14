#!/bin/bash
echo "test ok bg"
merge4tiff -c zip -n 255,255,255,0 -i1 inputs/01.jpg -i2 inputs/02.jpg -i3 inputs/03.jpg -m3 inputs/03m.tif -a uint -b 8 -s 4 -io outputs/test_ok_conversion.tif
if [ $? != 0 ] ; then 
    exit 1
else
    exit 0
fi