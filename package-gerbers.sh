#!/bin/bash

set -xe

STAMP=`date +"%Y%m%d_%H%M%S"`
rm -f hardware/main-board/gerbers/*.zip
rm -f hardware/sensor-board/gerbers/*.zip
rm -f *.zip

cd hardware/main-board/gerbers/
zip -D ../../../fk-naturalist-0.9-$STAMP.zip *
cd ../../../

cd hardware/sensor-board/gerbers/
zip -D ../../../fk-naturalist-sensors-0.9-$STAMP.zip *
cd ../../../
