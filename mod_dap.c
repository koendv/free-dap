#include "py/nlr.h"
#include "py/obj.h"
#include "py/objarray.h"
#include "py/objstr.h"
#include "py/runtime.h"
#include "py/binary.h"
#include "py/mpconfig.h"
#include "mod_dap.h"
#include "dap_config.h"
#include "dap.h"

#define mp_raise_RuntimeError(msg) (mp_raise_msg(&mp_type_RuntimeError, (msg)))

STATIC const char dap_help_text[] =
    "dap.init()   -- start dap usb device\n"
    "dap.deinit() -- stop dap usb device\n"
    "dap.process(req, resp) -- process dap request\n"
    "dap.hid_info -- usb hid descriptor for usb_mode()\n"
    "dap.calibrate() -- calibrate swd clock frequency\n";

bool dap_enabled = false;
int dap_config_fast_clock = DAP_DEFAULT_FAST_CLOCK;
int dap_config_delay_constant = DAP_DEFAULT_DELAY_CONSTANT;

STATIC mp_obj_t mp_dap_help();
STATIC mp_obj_t mp_dap_init();
STATIC mp_obj_t mp_dap_deinit();
STATIC mp_obj_t mp_dap_process(mp_obj_t req, mp_obj_t resp);
STATIC mp_obj_t mp_dap_calibrate();
STATIC const mp_rom_obj_tuple_t mp_dap_hidinfo_obj;

STATIC MP_DEFINE_CONST_FUN_OBJ_0(mp_dap_help_obj, mp_dap_help);
STATIC MP_DEFINE_CONST_FUN_OBJ_0(mp_dap_init_obj, mp_dap_init);
STATIC MP_DEFINE_CONST_FUN_OBJ_0(mp_dap_deinit_obj, mp_dap_deinit);
STATIC MP_DEFINE_CONST_FUN_OBJ_2(mp_dap_process_obj, mp_dap_process);
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(mp_dap_calibrate_obj, 0, mp_dap_calibrate);

STATIC const mp_rom_map_elem_t dap_module_globals_table[] = {
    {MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_dap)},
    {MP_ROM_QSTR(MP_QSTR_help), MP_ROM_PTR(&mp_dap_help_obj)},
    {MP_ROM_QSTR(MP_QSTR_init), MP_ROM_PTR(&mp_dap_init_obj)},
    {MP_ROM_QSTR(MP_QSTR_deinit), MP_ROM_PTR(&mp_dap_deinit_obj)},
    {MP_ROM_QSTR(MP_QSTR_calibrate), MP_ROM_PTR(&mp_dap_calibrate_obj)},
    {MP_ROM_QSTR(MP_QSTR_process), MP_ROM_PTR(&mp_dap_process_obj)},
    {MP_ROM_QSTR(MP_QSTR_hid_info), MP_ROM_PTR(&mp_dap_hidinfo_obj)},
};

STATIC MP_DEFINE_CONST_DICT(dap_module_globals, dap_module_globals_table);

// dap module object
const mp_obj_module_t mp_module_dap = {
    .base = {&mp_type_module},
    .globals = (mp_obj_dict_t *)&dap_module_globals,
};

// Register the module to make it available in Python
MP_REGISTER_MODULE(MP_QSTR_dap, mp_module_dap, (MODULE_FREEDAP_ENABLED));

/* dap hid descriptor */

STATIC const mp_obj_str_t usb_hid_dap_desc_obj;

#define USBD_DAP_HID_REPORT_DESC_SIZE (33)

__ALIGN_BEGIN STATIC const uint8_t
    USBD_HID_DAP_ReportDesc[USBD_DAP_HID_REPORT_DESC_SIZE] __ALIGN_END = {
        0x06, 0x00, 0xff, // Usage Page (Vendor Defined 0xFF00)
        0x09, 0x01,       // Usage (0x01)
        0xa1, 0x01,       // Collection (Application)
        0x15, 0x00,       //   Logical Minimum (0)
        0x26, 0xff, 0x00, //   Logical Maximum (255)
        0x75, 0x08,       //   Report Size (8)
        0x95, 0x40,       //   Report Count (64)
        0x09, 0x01,       //   Usage (0x01)
        0x81, 0x02,       //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
        0x95, 0x40,       //   Report Count (64)
        0x09, 0x01,       //   Usage (0x01)
        0x91, 0x02,       //   Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
        0x95, 0x01,       //   Report Count (1)
        0x09, 0x01,       //   Usage (0x01)
        0xb1, 0x02,       //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
        0xc0,             // End Collection
};

STATIC const mp_rom_obj_tuple_t mp_dap_hidinfo_obj = {
    {&mp_type_tuple},
    5,
    {
        MP_ROM_INT(0),                     // subclass: none
        MP_ROM_INT(0),                     // protocol: none
        MP_ROM_INT(64),                    // full-speed usb. max packet length: 64
        MP_ROM_INT(8),                     // polling interval: 8ms
        MP_ROM_PTR(&usb_hid_dap_desc_obj), // report descriptor
    },
};

STATIC const mp_obj_str_t usb_hid_dap_desc_obj = {
    {&mp_type_bytes},
    0, // hash not valid
    USBD_DAP_HID_REPORT_DESC_SIZE,
    USBD_HID_DAP_ReportDesc,
};

/* code */
STATIC mp_obj_t mp_dap_help() {
  mp_print_str(MP_PYTHON_PRINTER, dap_help_text);
  return mp_const_none;
}

STATIC mp_obj_t mp_dap_init() {
  dap_setup();
  dap_enabled = true;
  return mp_const_none;
}

STATIC mp_obj_t mp_dap_deinit() {
  dap_enabled = false;
  return mp_const_none;
}

/* get, measure and set SWD clock frequency calibration data */

STATIC mp_obj_t mp_dap_calibrate(size_t n_args, const mp_obj_t *pos_args,
                                 mp_map_t *kw_args) {
  static const mp_arg_t allowed_args[] = {
      {MP_QSTR_cal, MP_ARG_KW_ONLY | MP_ARG_OBJ, {.u_obj = mp_const_none}},
      {MP_QSTR_delay, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = -1}},
  };
  mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
  mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args),
                   allowed_args, args);
  mp_obj_t cal_obj = args[0].u_obj;
  mp_int_t delay = args[1].u_int;

  // set calibration data
  if (cal_obj != mp_const_none) {
    if (!mp_obj_is_type(cal_obj, &mp_type_tuple))
      mp_raise_TypeError(MP_ERROR_TEXT("expected calibrate() tuple"));
    mp_obj_tuple_t *cal_tup = MP_OBJ_TO_PTR(cal_obj);
    dap_config_delay_constant = mp_obj_get_int(cal_tup->items[0]);
    dap_config_fast_clock = mp_obj_get_int(cal_tup->items[1]);
    return mp_const_none;
  }

  // output clock signal for measurement with external instrument
  if (delay != -1) {
    mp_print_str(MP_PYTHON_PRINTER, "type ctrl-c to cancel\n");
    dap_clock_test(delay); // does not return
    return mp_const_none;
  }

  // measure and output calibration tuple
  dap_calibrate();
  cal_obj = mp_obj_new_tuple(2, NULL);
  mp_obj_tuple_t *cal_tup = MP_OBJ_TO_PTR(cal_obj);
  cal_tup->items[0] = mp_obj_new_int(dap_config_delay_constant);
  cal_tup->items[1] = mp_obj_new_int(dap_config_fast_clock);
  return cal_obj;
}

/* access to bytes/bytearray contents and length */

/*
   o: micropython string, bytes or bytearray (input)
   len: length of string, bytes or bytearray (output)
   items: output, string, bytes or bytearray data (output)
 */

STATIC void mp_obj_get_data(mp_obj_t o, size_t *len, uint8_t **items) {
  if (mp_obj_is_type(MP_OBJ_FROM_PTR(o), &mp_type_bytearray)) {
    mp_obj_array_t *barray = MP_OBJ_FROM_PTR(o);
    *len = barray->len;
    *items = barray->items;
    return;
  }
  if (mp_obj_is_type(MP_OBJ_FROM_PTR(o), &mp_type_bytes) ||
      mp_obj_is_type(MP_OBJ_FROM_PTR(o), &mp_type_str)) {
    mp_obj_str_t *str = MP_OBJ_FROM_PTR(o);
    *len = str->len;
    *items = (void *)str->data;
    return;
  }
  mp_raise_TypeError(MP_ERROR_TEXT("expected string, bytes or bytearray"));
}

/* call dap from micropython.
   arguments:
     - request: string, bytes, or bytearray (64 bytes)
     - response: string, bytes, or bytearray (64 bytes)
   returns true if response needs to be sent; false if response does not need to
   be sent.
*/

STATIC mp_obj_t mp_dap_process(mp_obj_t req, mp_obj_t resp) {
  size_t app_request_len;
  uint8_t *app_request_buffer;
  size_t app_response_len;
  uint8_t *app_response_buffer;

  mp_obj_get_data(req, &app_request_len, &app_request_buffer);
  if (app_request_len < DAP_CONFIG_PACKET_SIZE)
    mp_raise_ValueError(MP_ERROR_TEXT("request too small"));

  mp_obj_get_data(resp, &app_response_len, &app_response_buffer);
  if (app_response_len < DAP_CONFIG_PACKET_SIZE)
    mp_raise_ValueError(MP_ERROR_TEXT("response too small"));

  if (dap_filter_request(app_request_buffer)) {
    dap_process_request(app_request_buffer, app_response_buffer);
    return mp_const_true;
  }

  memset(app_response_buffer, 0, DAP_CONFIG_PACKET_SIZE);
  return mp_const_false;
}

// not truncated
