#include "dap.h"
#include "dap_config.h"

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
#define mp_raise_RuntimeError(msg) (mp_raise_msg(&mp_type_RuntimeError, (msg)))

/*- Definitions -------------------------------------------------------------*/
#define STATUS_TIMEOUT 250 // ms

/*- Variables ---------------------------------------------------------------*/
static usbd_hid_itf_t *hid_itf = NULL;

static uint8_t app_request_buffer[DAP_CONFIG_PACKET_COUNT]
                                 [DAP_CONFIG_PACKET_SIZE];
static bool app_request_valid[DAP_CONFIG_PACKET_COUNT];
static int app_request_wr_ptr;
static int app_request_rd_ptr;

static uint8_t app_response_buffer[DAP_CONFIG_PACKET_COUNT]
                                  [DAP_CONFIG_PACKET_SIZE];
static bool app_response_valid[DAP_CONFIG_PACKET_COUNT];
static int app_response_wr_ptr;
static int app_response_rd_ptr;

char dap_serial_number[48 + 1];
uint32_t watchdog_timer = 0;

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

void dap_clear() {
  app_request_wr_ptr = 0;
  app_request_rd_ptr = 0;

  app_response_wr_ptr = 0;
  app_response_rd_ptr = 0;

  for (int i = 0; i < DAP_CONFIG_PACKET_COUNT; i++) {
    app_request_valid[i] = false;
    app_response_valid[i] = false;
  }

  watchdog_timer = 0;
}

//-----------------------------------------------------------------------------

void dap_setup() {

  dap_clear();

  serial_number_init();

  hid_itf = usbd_hid_interface();

  if (hid_itf == NULL)
    mp_raise_RuntimeError(MP_ERROR_TEXT("no usb hid device"));
}

//-----------------------------------------------------------------------------

static void receive_request(void) {
  if (app_request_valid[app_request_wr_ptr])
    return;

  if (hid_itf == NULL)
    return;

  if (usbd_hid_rx_num(hid_itf) == 0)
    return;

  if (usbd_hid_rx(hid_itf, DAP_CONFIG_PACKET_SIZE,
                  app_request_buffer[app_request_wr_ptr], 0) <= 0)
    return;

  if (!dap_filter_request(app_request_buffer[app_request_wr_ptr]))
    return;

  app_request_valid[app_request_wr_ptr] = true;
  app_request_wr_ptr = (app_request_wr_ptr + 1) % DAP_CONFIG_PACKET_COUNT;

  watchdog_timer = mp_hal_ticks_ms();

  return;
}

//-----------------------------------------------------------------------------

static void send_response(void) {
  if (!app_response_valid[app_response_rd_ptr])
    return;

  if (hid_itf == NULL)
    return;

  if (!USBD_HID_CanSendReport(&hid_itf->base))
    return;

  if (USBD_HID_SendReport(&hid_itf->base,
                          app_response_buffer[app_response_rd_ptr],
                          DAP_CONFIG_PACKET_SIZE) == USBD_FAIL)
    return;

  app_response_valid[app_response_rd_ptr] = false;
  app_response_rd_ptr = (app_response_rd_ptr + 1) % DAP_CONFIG_PACKET_COUNT;

  watchdog_timer = 0;

  return;
}

//-----------------------------------------------------------------------------

static void dap_process(void) {
  if (!app_request_valid[app_request_rd_ptr])
    return;

  if (app_response_valid[app_response_wr_ptr])
    return;

  dap_process_request(app_request_buffer[app_request_rd_ptr],
                      app_response_buffer[app_response_wr_ptr]);

  app_response_valid[app_response_wr_ptr] = true;
  app_response_wr_ptr = (app_response_wr_ptr + 1) % DAP_CONFIG_PACKET_COUNT;

  app_request_valid[app_request_rd_ptr] = false;
  app_request_rd_ptr = (app_request_rd_ptr + 1) % DAP_CONFIG_PACKET_COUNT;
}

//-----------------------------------------------------------------------------

static void dap_watchdog(void) {
  if (watchdog_timer == 0)
    return;
  uint32_t now = mp_hal_ticks_ms();
  if ((now > watchdog_timer) && (now - watchdog_timer > 5000)) {
    mp_print_str(MP_PYTHON_PRINTER, "dap reset\n");
    dap_clear();
  }
}

//-----------------------------------------------------------------------------

void dap_task() {

#ifdef MICROPY_HW_LED1
  MICROPY_HW_LED_ON(MICROPY_HW_LED1);
#endif

  receive_request();
  dap_process();
  send_response();
  dap_watchdog();

#ifdef MICROPY_HW_LED1
  MICROPY_HW_LED_OFF(MICROPY_HW_LED1);
#endif
}

// not truncated
