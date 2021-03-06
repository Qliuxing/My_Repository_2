set SUBMODULE=BASIC
rem or
rem gmake PROFILE=<profile> SUBMODULE=<ex1 ex2> all

REM No LIN
REM set PROFILE=81300-nolin
REM gmake PROFILE=81300-nolin SUBMODULE=BASIC clean
REM gmake PROFILE=81300-nolin SUBMODULE=BASIC all

REM LIN 1.3
REM set PROFILE=81300-lin13-9600
REM gmake PROFILE=81300-lin13-9600-loader SUBMODULE=BASIC clean
REM gmake PROFILE=81300-lin13-9600-loader SUBMODULE=BASIC all

REM LIN 2.0
REM set PROFILE=81300-lin20-9600
REM gmake PROFILE=81300-lin20-9600-loader SUBMODULE=BASIC clean
REM gmake PROFILE=81300-lin20-9600-loader SUBMODULE=BASIC all

REM LIN 2.1
set PROFILE=81300-lin21-9600
gmake PROFILE=81300-lin21-9600-loader SUBMODULE=BASIC clean
gmake PROFILE=81300-lin21-9600-loader SUBMODULE=BASIC all

REM LIN 2.2 (not supported)
