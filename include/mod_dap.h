#ifndef MOD_DAP_H
#define MOD_DAP_H

extern bool dap_enabled;
/* call periodically from micropython event loop. */
extern void dap_loop(void);

#endif

