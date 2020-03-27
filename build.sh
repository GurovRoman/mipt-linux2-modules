#!/bin/sh

cd linux-source-5.4
make bindeb-pkg

cd ../custom_module
make src

cd ..