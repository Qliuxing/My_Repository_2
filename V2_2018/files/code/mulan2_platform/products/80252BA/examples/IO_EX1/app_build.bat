@echo off

gmake clean PRODUCT=80252BA PROFILE=80252BA-flash TARGET=app-flash
gmake all   PRODUCT=80252BA PROFILE=80252BA-flash TARGET=app-flash

gmake clean PRODUCT=80252BA PROFILE=80252BA-otp TARGET=app-otp
gmake all   PRODUCT=80252BA PROFILE=80252BA-otp TARGET=app-otp
