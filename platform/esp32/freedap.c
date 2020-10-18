#include "dap.h"
#include "dap_config.h"
#include "mod_dap.h"
#include "py/runtime.h"

/*- Definitions -------------------------------------------------------------*/
#define mp_raise_RuntimeError(msg) (mp_raise_msg(&mp_type_RuntimeError, (msg)))

/*- Variables ---------------------------------------------------------------*/
char dap_serial_number[48 + 1] = "000000000000000000000000";

//-----------------------------------------------------------------------------

static const char hexdigits[] = "0123456789ABCDEF";

static char *hexify(char *hex, const void *buf, size_t size) {
  char *tmp = hex;
  const uint8_t *b = buf;
  while (size--) {
    *tmp++ = hexdigits[*b >> 4];
    *tmp++ = hexdigits[*b++ & 0xF];
  }
  *tmp++ = 0;
  return hex;
}

static void serial_number_init(void) {
  memset(dap_serial_number, 0, sizeof(dap_serial_number));
  uint8_t  chipid[6];
  esp_efuse_mac_get_default(chipid);
  hexify(dap_serial_number, chipid, sizeof(chipid));
  return;
}

//-----------------------------------------------------------------------------

/* dap_setup(): called once */

void dap_setup() {

  serial_number_init();

}

//-----------------------------------------------------------------------------

/* dap_loop(): called periodically */

void dap_loop() {
/* empty */
}

// not truncated
