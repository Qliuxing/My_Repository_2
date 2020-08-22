# ------------------------
# BUILD CONFIGURATION FILE
# ------------------------

#
# MCU Type Nr
#
#PRODUCT ?= 81300
#PRODUCT ?= 81310
PRODUCT ?= 81315

#
# MCU Rev Nr
#
ifeq ($(PRODUCT), 81300)
	#CHIPREV ?= D
	CHIPREV ?= E
endif

ifeq ($(PRODUCT), 81310)
	CHIPREV ?= C
endif

ifeq ($(PRODUCT), 81315)
	CHIPREV ?= A
endif

#
# MLX4 Firmware Type
#
#MLX4_TYPE ?= nolin
#MLX4_TYPE ?= loop
#MLX4_TYPE ?= lin13-9600-loader
MLX4_TYPE ?= lin20-9600-loader
#MLX4_TYPE ?= lin21-9600-loader
#MLX4_TYPE ?= sae-loader
