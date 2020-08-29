@echo off

gmake -s clean PROFILE=80252BA-flash SUBMODULE="IO_EX1 RAM_application_naked"
gmake -s all   PROFILE=80252BA-flash SUBMODULE="IO_EX1 RAM_application_naked"

