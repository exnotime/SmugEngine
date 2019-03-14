glslangValidator.exe -S rgen -V -e main -o spv/rg.spv RayGen.glsl
glslangValidator.exe -S rchit -V -e main -o spv/ah.spv RayAnyHit.glsl
pause