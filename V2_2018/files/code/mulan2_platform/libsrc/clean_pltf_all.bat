@echo off
rem Helper bat file to clean the platform

@call "%MLXPATH_DATA_DIR%\mlx16-gcc-1.bat"
sh clean_pltf_all.sh
