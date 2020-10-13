#ifndef MOD_DAP_H
#define MOD_DAP_H

extern bool dap_enabled;
/* called once during dap.init() */
extern void dap_setup(void);
/* called periodically from micropython event loop. */
extern void dap_loop(void);

#endif

