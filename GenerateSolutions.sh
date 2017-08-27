#!/bin/bash
./premake5.exe --file=TephraEngine.lua vs2017
./premake5.exe --file=TephraAssetCompiler.lua vs2017
(cd src/Core/components && ./UpdateComponents.sh)
