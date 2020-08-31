/*
 * Copyright (C) 2012-2013 Melexis N.V.
 *
 * Software Platform
 *
 */

#ifndef PLTF_LIB_MLX8110X_WDG_H_
#define PLTF_LIB_MLX8110X_WDG_H_

#include <awdg.h>

/* ----------------------------------------------------------------------------
 * Function is call in the idle loop of flash loader
 */
__MLX_TEXT__  static INLINE  void WDG_Manager (void)
{
    awdg_restart();
}

#endif /* PLTF_LIB_MLX8110X_WDG_H_ */
