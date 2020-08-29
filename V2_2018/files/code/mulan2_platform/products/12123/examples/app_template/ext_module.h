#ifndef _EXT_MODULE_H_
#define _EXT_MODULE_H_

/*
 *  Test dp variables
 */
extern volatile uint16_t var_default;
extern volatile uint16_t var_dp   __attribute__((dp));
extern volatile uint16_t var_nodp __attribute__((nodp));

#endif /* _EXT_MODULE_H_ */
