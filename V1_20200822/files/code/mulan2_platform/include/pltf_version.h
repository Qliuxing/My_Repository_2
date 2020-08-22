/*
 * Copyright (C) 2009-2015 Melexis N.V.
 *
 * Module: Software Platform
 *
 */

#ifndef MLX_PLTF_VERSION_H_
#define MLX_PLTF_VERSION_H_

/* SW platform version */
#define __MLX_PLTF_VERSION_MAJOR__              4
#define __MLX_PLTF_VERSION_MINOR__              1
#define __MLX_PLTF_VERSION_REVISION__           0
#define __MLX_PLTF_VERSION_CUSTOMER_BUILD__     0

#define __MLX_PLTF_VERSION_STRING__             "4.1.0.0"

/* Validate version range */
#if    (__MLX_PLTF_VERSION_MAJOR__ > 255)    \
    || (__MLX_PLTF_VERSION_MINOR__ > 255)    \
    || (__MLX_PLTF_VERSION_REVISION__       > 255) \
    || (__MLX_PLTF_VERSION_CUSTOMER_BUILD__ > 255)
#error "Maximum version number is 255"
#endif


#endif /* MLX_PLTF_VERSION_H_ */
