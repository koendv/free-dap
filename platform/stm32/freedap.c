#include "dap.h"
#include "dap_config.h"
#include "mod_dap.h"

#ifdef MIN
#undef MIN
#endif

#ifdef MAX
#undef MAX
#endif

#include "usb.h"
#include "usbd_cdc_msc_hid.h"
#include "usbd_hid_interface.h"
#include "usbdev/class/inc/usbd_cdc_msc_hid.h"

#include "py/runtime.h"

/*- Definitions -------------------------------------------------------------*/
#define mp_raise_RuntimeError(msg) (mp_raise_msg(&mp_type_RuntimeError, (msg)))

/*- Variables ---------------------------------------------------------------*/
static usbd_hid_itf_t *hid_itf = NULL;
char dap_serial_number[48 + 1];

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
  uint8_t *id = (uint8_t *)MP_HAL_UNIQUE_ID_ADDRESS;
  uint32_t id_len = 12;
  hexify(dap_serial_number, id, id_len);
  return;
}

//-----------------------------------------------------------------------------

/* dap_setup(): called once */

void dap_setup() {

  serial_number_init();

  hid_itf = usbd_hid_interface();
  if (hid_itf == NULL)
    mp_raise_RuntimeError(MP_ERROR_TEXT("no usb hid device"));
}

//-----------------------------------------------------------------------------

/* dap_loop(): called periodically */

void dap_loop() {
  uint8_t app_request_buffer[DAP_CONFIG_PACKET_SIZE];
  uint8_t app_response_buffer[DAP_CONFIG_PACKET_SIZE];

  if (!dap_enabled) return;

#ifdef MICROPY_HW_LED1
  MICROPY_HW_LED_ON(MICROPY_HW_LED1);
#endif

  while ((hid_itf != NULL) && 
         USBD_HID_CanSendReport(&hid_itf->base) &&
         (usbd_hid_rx_num(hid_itf) != 0) &&
         (usbd_hid_rx(hid_itf, DAP_CONFIG_PACKET_SIZE, app_request_buffer, 0) > 0) &&
         dap_filter_request(app_request_buffer)) {
    dap_process_request(app_request_buffer, app_response_buffer);
    USBD_HID_SendReport(&hid_itf->base, app_response_buffer, DAP_CONFIG_PACKET_SIZE);
  }

#ifdef MICROPY_HW_LED1
  MICROPY_HW_LED_OFF(MICROPY_HW_LED1);
#endif
}

// not truncated
