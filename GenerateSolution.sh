#!/bin/bash
./premake5.exe vs2015
(cd src/Core/components && ./UpdateComponents.sh)
