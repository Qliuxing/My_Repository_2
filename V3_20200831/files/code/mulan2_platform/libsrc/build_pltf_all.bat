@echo off
rem Helper bat file to build the platform

@call "%MLXPATH_DATA_DIR%\mlx16-gcc-1.bat"
sh build_pltf_all.sh
