/*
   Copyright (C) 2016 Orange

   This software is distributed under the terms and conditions of the 'BSD-3-Clause'
   license which can be found in the file 'LICENSE.txt' in this package distribution
   or at 'https://opensource.org/licenses/BSD-3-Clause'.
*/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>

#include <signal.h>
#include <limits.h>

#include <vmsys.h>
#include <vmsock.h>

#ifdef connect
#undef connect
#endif

#include <LTask.h>
#include <LGPRS.h>
#include <LGSM.h>


#include "liveobjects_iotsoftbox_api.h"
#include "liveobjects_sample_basic_mdk.h"

const char* appv_version   = "ARDUINO_MDK BASIC SAMPLE V04.14";

//#define SMS_TEL  "06xxxxxxx"

#define APP_FEATURE_LO_DATA                      1
#define APP_FEATURE_LO_PARAMS                    1
#define APP_FEATURE_LO_STATUS                    1
#define APP_FEATURE_LO_COMMANDS                  1
#define APP_FEATURE_LO_RESOURCES                 1

// set to :
//   LOTRACE_LEVEL_WARN
//   LOTRACE_LEVEL_NOTICE
//   LOTRACE_LEVEL_INF
//   LOTRACE_LEVEL_DBG1
//   LOTRACE_LEVEL_DBG2
//   LOTRACE_LEVEL_VERBOSE
#define DBG_DFT_TRACE_LEVEL      LOTRACE_LEVEL_INF

// set 0x0F to have all message dump (text+hexa)
#define DBG_DFT_MSG_DUMP         0

#define DBG_DFT_APP_LOG_LEVEL    1

#if 1
extern "C" void lo_trace_printf( const char  *format, ... );
#define MEOL             "\r\n"
#define PRINTF           lo_trace_printf
#else
#define PRINTF(...)      ((void)0)
#endif

#define PRINT_RAW(txt)   Serial.print(txt)

uint8_t     appv_log_level = DBG_DFT_APP_LOG_LEVEL;

// ==========================================================
//
// Live Objects IoT Client object (using iotsoftbox-mqtt library)
//
// - status information at connection
// - collected data to send
// - supported configuration parameters
// - supported commands
// - resources declaration (firmware, text file, etc.)

// ----------------------------------------------------------
// STATUS data
//
#if APP_FEATURE_LO_STATUS

int32_t appv_status_counter = 0;
char    appv_status_message[150] = "READY";

// Set of status
LiveObjectsD_Data_t appv_set_status[] = {
  { LOD_TYPE_STRING_C, "sample_version" ,  (void*)appv_version },
  { LOD_TYPE_INT32,    "sample_counter" ,  &appv_status_counter},
  { LOD_TYPE_STRING_C, "sample_message" ,  appv_status_message}
};
#define SET_STATUS_NB (sizeof(appv_set_status) / sizeof(LiveObjectsD_Data_t))

int     appv_hdl_status = -1;

#endif // APP_FEATURE_LO_STATUS

// ----------------------------------------------------------
// 'COLLECTED DATA'
//
#if APP_FEATURE_LO_DATA

#define STREAM_PREFIX   0

uint8_t  appv_measures_enabled = 1;

int32_t  appv_measures_temp_grad = -1;
float    appv_measures_volt_grad = -0.2;

// contains a counter incremented after each data sent
uint32_t appv_measures_counter = 0;

// contains the temperature level
int32_t  appv_measures_temp = 20;

// contains the battery level
float    appv_measures_volt = 5.0;

// Set of Collected data (published on a data stream)
LiveObjectsD_Data_t appv_set_measures[] = {
  { LOD_TYPE_UINT32, "counter" ,        &appv_measures_counter},
  { LOD_TYPE_INT32,  "temperature" ,    &appv_measures_temp},
  { LOD_TYPE_FLOAT,  "battery_level" ,  &appv_measures_volt }
};
#define SET_MEASURES_NB (sizeof(appv_set_measures) / sizeof(LiveObjectsD_Data_t))

int      appv_hdl_data = -1;

#endif // APP_FEATURE_LO_DATA

// ----------------------------------------------------------
// CONFIGURATION data
//
#if APP_FEATURE_LO_PARAMS

volatile uint32_t appv_cfg_timeout = 10;

// a structure containg various kind of parameters (char[], int and float)
struct conf_s {
  char     name[20];
  int32_t  threshold;
  float    gain;
} appv_conf = {
  "TICTAC",
  -3,
  1.05
};

// definition of identifer for each kind of parameters
#define PARM_IDX_NAME        1
#define PARM_IDX_TIMEOUT     2
#define PARM_IDX_THRESHOLD   3
#define PARM_IDX_GAIN        4

// Set of configuration parameters
LiveObjectsD_Param_t appv_set_param[] = {
  { PARM_IDX_NAME,      { LOD_TYPE_STRING_C, "name"    ,   appv_conf.name } },
  { PARM_IDX_TIMEOUT,   { LOD_TYPE_UINT32,   "timeout" ,   (void*)&appv_cfg_timeout } },
  { PARM_IDX_THRESHOLD, { LOD_TYPE_INT32,    "threshold" , &appv_conf.threshold } },
  { PARM_IDX_GAIN,      { LOD_TYPE_FLOAT,    "gain" ,      &appv_conf.gain } }
};
#define SET_PARAM_NB (sizeof(appv_set_param) / sizeof(LiveObjectsD_Param_t))

#endif // APP_FEATURE_LO_PARAMS

// ----------------------------------------------------------
// COMMANDS
//
#if APP_FEATURE_LO_COMMANDS

// Digital output to change the status of the RED LED
int  app_led_user = 0;

// counter used to postpone the LED command response
static int cmd_cnt = 0;

#define CMD_IDX_RESET       1
#define CMD_IDX_LED         2

// set of commands
LiveObjectsD_Command_t appv_set_commands[] = {
  { CMD_IDX_RESET, "RESET" , 0},
  { CMD_IDX_LED,   "LED"  , 0}
};
#define SET_COMMANDS_NB (sizeof(appv_set_commands) / sizeof(LiveObjectsD_Command_t))

#endif // APP_FEATURE_LO_COMMANDS

// ----------------------------------------------------------
// RESOURCE data
//
#if APP_FEATURE_LO_RESOURCES

#define RSC_IDX_MESSAGE     1
#define RSC_IDX_IMAGE       2
#define RSC_IDX_BINARY      3

#ifdef RSC_IDX_IMAGE
char appv_rv_image[10]    = "01.00";
char appv_rsc_image[3 * 1024] = "";
#endif
#ifdef RSC_IDX_BINARY
char appv_rv_binary[10]   = "01.00";
char appv_rsc_binary[1024];
#endif

char appv_rv_message[10]  = "01.00";

// Set of resources
LiveObjectsD_Resource_t appv_set_resources[] = {
#ifdef RSC_IDX_IMAGE
  { RSC_IDX_IMAGE,   "image",   appv_rv_image,   sizeof(appv_rv_image) - 1 },
#endif
#ifdef RSC_IDX_BINARY
  { RSC_IDX_BINARY,  "binary",  appv_rv_binary,  sizeof(appv_rv_binary) - 1 },
#endif
  { RSC_IDX_MESSAGE, "message", appv_rv_message, sizeof(appv_rv_message) - 1 } // resource used to update appv_status_message
};
#define SET_RESOURCES_NB (sizeof(appv_set_resources) / sizeof(LiveObjectsD_Resource_t))

// variables used to process the current resource transfer
uint32_t appv_rsc_size = 0;
uint32_t appv_rsc_offset = 0;

#endif // APP_FEATURE_LO_RESOURCES


// ==========================================================
// IotSoftbox-mqtt callback functions (in 'C' api)
//
extern "C" {

  // ----------------------------------------------------------
  // CONFIGURATION PARAMETERS Callback function
  // Check value in range, and copy string parameters

#if APP_FEATURE_LO_PARAMS

  // Called (by the LiveObjects thread) to update configuration parameters
  int main_cb_param_udp(const LiveObjectsD_Param_t* param_ptr, const void* value, int len)
  {
    if (param_ptr == NULL) {
      PRINTF("UPDATE  ERROR - invalid parameter x%p" MEOL, param_ptr);
      return -1;
    }
    PRINTF("UPDATE user_ref=%d %s ...." MEOL, param_ptr->parm_uref, param_ptr->parm_data.data_name);
    switch (param_ptr->parm_uref) {
      case PARM_IDX_NAME:
        {
          PRINTF("update name = %.*s" MEOL, len , (const char*) value);
          if ((len > 0) && (len < (sizeof(appv_conf.name) - 1))) {
            // Only c-string parameter must be updated by the user application (to check the string length)
            strncpy(appv_conf.name, (const char*) value, len);
            appv_conf.name[len] = 0;
            return 0; // OK.
          }
        }
        break;

      case PARM_IDX_TIMEOUT:
        {
          uint32_t timeout = *((const uint32_t*)value);
          PRINTF("update timeout = %"PRIu32"" MEOL, timeout);
          if ((timeout > 0) && (timeout <= 120) && (timeout != appv_cfg_timeout)) {
            return 0; // primitive parameter is updated by library
          }
        }
        break;
      case PARM_IDX_THRESHOLD:
        {
          int32_t threshold = *((const int32_t*)value);
          PRINTF("update threshold = %"PRIi32"" MEOL, threshold);
          if ((threshold >= -10) && (threshold <= 10) && (threshold != appv_conf.threshold)) {
            return 0; // primitive parameter is updated by library
          }
        }
        break;
      case PARM_IDX_GAIN:
        {
          float gain = *((const float*)value);
          PRINTF("update gain = %f" MEOL, gain);
          if ((gain > 0.0) && (gain < 10.0) && (gain != appv_conf.gain)) {
            return 0; // primitive parameter is updated by library
          }
        }
        break;
    }
    PRINTF("ERROR to update param[%d] %s !!" MEOL, param_ptr->parm_uref, param_ptr->parm_data.data_name);
    return -1;
  }
#endif // APP_FEATURE_LO_PARAMS

  // ----------------------------------------------------------
  // COMMAND Callback Functions
  //
#if APP_FEATURE_LO_COMMANDS

  int main_cmd_doSystemReset(LiveObjectsD_CommandRequestBlock_t* pCmdReqBlk);
  int main_cmd_doLED(LiveObjectsD_CommandRequestBlock_t* pCmdReqBlk);

  // Called (by the LiveObjects thread) to perform an 'attached/registered' command
  int main_cb_command(LiveObjectsD_CommandRequestBlock_t* pCmdReqBlk)
  {
    int ret;
    const LiveObjectsD_Command_t*  cmd_ptr;

    if ((pCmdReqBlk == NULL) || (pCmdReqBlk->hd.cmd_ptr == NULL) || (pCmdReqBlk->hd.cmd_cid == 0) ) {
      PRINTF("**** COMMAND : ERROR, Invalid parameter" MEOL);
      return -1;
    }

    cmd_ptr = pCmdReqBlk->hd.cmd_ptr;
    PRINTF("**** COMMAND %d %s - cid=%d" MEOL, cmd_ptr->cmd_uref, cmd_ptr->cmd_name, pCmdReqBlk->hd.cmd_cid);
    {
      int i;
      PRINTF("**** ARGS %d : " MEOL, pCmdReqBlk->hd.cmd_args_nb);
      for (i = 0; i < pCmdReqBlk->hd.cmd_args_nb; i++) {
        PRINTF("**** ARG [%d] (%d) :  %s %s" MEOL, i, pCmdReqBlk->args_array[i].arg_type,
               pCmdReqBlk->args_array[i].arg_name, pCmdReqBlk->args_array[i].arg_value);
      }
    }

    switch (cmd_ptr->cmd_uref) {
      case CMD_IDX_RESET: // RESET
        PRINTF("main_callbackCommand: command[%d] %s" MEOL, cmd_ptr->cmd_uref, cmd_ptr->cmd_name);
        ret = main_cmd_doSystemReset(pCmdReqBlk);
        break;

      case CMD_IDX_LED: // LED
        PRINTF("main_callbackCommand: command[%d] %s" MEOL, cmd_ptr->cmd_uref, cmd_ptr->cmd_name);
        ret = main_cmd_doLED(pCmdReqBlk);
        break;
      default :
        PRINTF("main_callbackCommand: ERROR, unknown command %d" MEOL, cmd_ptr->cmd_uref);
        ret = -4;
    }
    return ret;
  }

  // ----------------------------------------------------------
  // do a RESET command
  int main_cmd_doSystemReset(LiveObjectsD_CommandRequestBlock_t* pCmdReqBlk)
  {
    PRINTF(MEOL "*** RESET (but do nothing)." MEOL);
    return 1; // response = OK
  }


  // ----------------------------------------------------------
  // Delayed Response to the LED command
  void main_cmd_delayed_resp_LED ()
  {
    if (cmd_cnt > 0) {
      LiveObjectsD_Command_t* cmd_ptr = &appv_set_commands[CMD_IDX_LED - 1];
      if (appv_log_level > 1) PRINTF("main_cmd_delayed_resp_LED: cnt=%d cid=%d" MEOL, cmd_cnt, cmd_ptr->cmd_cid);
      if ((--cmd_cnt <= 0) && (cmd_ptr->cmd_cid)) {
        uint32_t code = 200;
        char msg [] = "USER LED TEST = OK";
        LiveObjectsD_Data_t cmd_resp[] = {
          { LOD_TYPE_UINT32,    "code" ,  &code},
          { LOD_TYPE_STRING_C,  "msg" ,   msg}
        };
        // switch off the Red LED
        app_led_user = 1;

        PRINTF(MEOL "*** main_cmd_resp_LED: RESPONSE ..." MEOL);
        LiveObjectsClient_CommandResponse(cmd_ptr->cmd_cid, cmd_resp, 2);

        cmd_ptr->cmd_cid = 0;
        cmd_cnt = 0;
      }
    }
  }

  // ----------------------------------------------------------
  // do a LED command
  int main_cmd_doLED(LiveObjectsD_CommandRequestBlock_t* pCmdReqBlk)
  {
    int ret;
    // switch on the Red LED
    app_led_user = 0;

    if (pCmdReqBlk->hd.cmd_args_nb == 0) {
      PRINTF("main_cmd_doLED: No ARG" MEOL);
      app_led_user = ! app_led_user;
      ret = 1; // Response OK
      cmd_cnt = 0;
    }
    else {
      int i;
      int cnt;
      for (i = 0; i < pCmdReqBlk->hd.cmd_args_nb; i++ ) {
        if ( !strncasecmp("ticks", pCmdReqBlk->args_array[i].arg_name, 5)
             && pCmdReqBlk->args_array[i].arg_type == 0 ) {
          cnt = atoi(pCmdReqBlk->args_array[i].arg_value);
          if ((cnt >= 0) && (cnt <= 3)) {
            cmd_cnt = cnt;
          }
          else cmd_cnt = 0;
          PRINTF("main_cmd_doLED: cmd_cnt = %d (%d)" MEOL, cmd_cnt, cnt);
        }
      }
    }

    if (cmd_cnt == 0) {
      app_led_user = ! app_led_user;
      ret = 1; // Response OK
    }
    else {
      LiveObjectsD_Command_t* cmd_ptr = (LiveObjectsD_Command_t*)(pCmdReqBlk->hd.cmd_ptr);
      app_led_user = 0;
      PRINTF("main_cmd_doLED: ccid=%d (%d)  DELAYED RESPONSE %u !!" MEOL, pCmdReqBlk->hd.cmd_cid, cmd_ptr->cmd_cid, cmd_cnt);
      cmd_ptr->cmd_cid = pCmdReqBlk->hd.cmd_cid;
      ret = 0; // pending
    }
    return ret; // response = OK
  }

#endif // APP_FEATURE_LO_COMMANDS

  // ----------------------------------------------------------
  // RESOURCE Callback Functions
  //
#if APP_FEATURE_LO_RESOURCES
  //  Called (by the LiveObjects thread) to notify either,
  //  - state = 0  : the begin of resource request
  //  - state = 1  : the end without error
  //  - state != 1 : the end with an error
  LiveObjectsD_ResourceRespCode_t main_cb_rsc_ntfy (
    uint8_t state, const LiveObjectsD_Resource_t* rsc_ptr,
    const char* version_old, const char* version_new, uint32_t size)
  {
    LiveObjectsD_ResourceRespCode_t ret = RSC_RSP_OK; // OK to update the resource

    PRINTF("*** rsc_ntfy: ..." MEOL);

    if ((rsc_ptr) && (rsc_ptr->rsc_uref > 0) && (rsc_ptr->rsc_uref <= SET_RESOURCES_NB)) {
      PRINTF("***   user ref     = %d" MEOL, rsc_ptr->rsc_uref);
      PRINTF("***   name         = %s" MEOL, rsc_ptr->rsc_name);
      PRINTF("***   version_old  = %s" MEOL, version_old);
      PRINTF("***   version_new  = %s" MEOL, version_new);
      PRINTF("***   size         = %u" MEOL, size);
      if (state) {
        if (state == 1) { // Completed without error
          PRINTF("***   state        = COMPLETED without error" MEOL);
          // Update version
          PRINTF(" ===> UPDATE - version %s to %s" MEOL, rsc_ptr->rsc_version_ptr, version_new);
          strncpy((char*)rsc_ptr->rsc_version_ptr, version_new, rsc_ptr->rsc_version_sz);
#ifdef RSC_IDX_IMAGE
          if (rsc_ptr->rsc_uref == RSC_IDX_IMAGE) {
            //trace_mutex_wait();
            PRINT_RAW(MEOL MEOL);
            PRINT_RAW(appv_rsc_image);
            PRINT_RAW(MEOL MEOL);
            //trace_mutex_release();
          }
#endif
        }
        else {
          PRINTF("***   state        = COMPLETED with error !!" MEOL);
          // Roll back ?
        }
        appv_rsc_offset = 0;
        appv_rsc_size = 0;

        // Push Status (message has been updated or not)
        appv_status_counter++;
        PRINTF("*** Push STATUS : counter=%u msg='%s'" MEOL, appv_status_counter, appv_status_message);
        LiveObjectsClient_PushStatus( appv_hdl_status );
      }
      else {
        appv_rsc_offset = 0;
        ret = RSC_RSP_ERR_NOT_AUTHORIZED;
        switch (rsc_ptr->rsc_uref ) {
          case RSC_IDX_MESSAGE:
            if (size < (sizeof(appv_status_message) - 1)) {
              ret = RSC_RSP_OK;
            }
            break;
#ifdef RSC_IDX_IMAGE
          case RSC_IDX_IMAGE:
            if (size < (sizeof(appv_rsc_image) - 1)) {
              ret = RSC_RSP_OK;
            }
            break;
#endif
#ifdef RSC_IDX_BINARY
          case RSC_IDX_BINARY:
            ret = RSC_RSP_OK;
            break;
#endif
        }
        if (ret == RSC_RSP_OK) {
          appv_rsc_size = size;
          PRINTF("***   state        = START - ACCEPTED" MEOL);;
        }
        else {
          appv_rsc_size = 0;
          PRINTF("***   state        = START - REFUSED" MEOL);
        }
      }
    }
    else {
      PRINTF("***  UNKNOWN USER REF (x%p %d)  in state=%d" MEOL, rsc_ptr, rsc_ptr->rsc_uref, state);
      ret = RSC_RSP_ERR_INVALID_RESOURCE;
    }
    return ret;
  }

  // Called (by the LiveObjects thread) to request the user
  // to read data from current resource transfer.
  int main_cb_rsc_data (const LiveObjectsD_Resource_t* rsc_ptr, uint32_t offset)
  {
    int ret;

    if (appv_log_level > 1)  PRINTF("*** rsc_data: rsc[%d]='%s' offset=%u - data ready ..." MEOL, rsc_ptr->rsc_uref, rsc_ptr->rsc_name, offset);

    if (rsc_ptr->rsc_uref == RSC_IDX_MESSAGE) {
      char buf[40];
      if (offset > (sizeof(appv_status_message) - 1)) {
        PRINTF("*** rsc_data: rsc[%d]='%s' offset=%u > %d - OUT OF ARRAY" MEOL,
               rsc_ptr->rsc_uref, rsc_ptr->rsc_name, offset, sizeof(appv_status_message) - 1);
        return -1;
      }
      ret = LiveObjectsClient_RscGetChunck(rsc_ptr, buf, sizeof(buf) - 1);
      if (ret > 0) {
        if ((offset + ret) > (sizeof(appv_status_message) - 1)) {
          PRINTF("*** rsc_data: rsc[%d]='%s' offset=%u - read=%d => %d > %d - OUT OF ARRAY" MEOL,
                 rsc_ptr->rsc_uref, rsc_ptr->rsc_name, offset, ret, offset + ret, sizeof(appv_status_message) - 1);
          return -1;
        }
        appv_rsc_offset += ret;
        memcpy(&appv_status_message[offset], buf, ret);
        appv_status_message[offset + ret] = 0;
        PRINTF("*** rsc_data: rsc[%d]='%s' offset=%u - read=%d/%d '%s'" MEOL,
               rsc_ptr->rsc_uref, rsc_ptr->rsc_name, offset, ret, sizeof(buf) - 1, appv_status_message);
      }
    }
#ifdef RSC_IDX_IMAGE
    else if (rsc_ptr->rsc_uref == RSC_IDX_IMAGE) {
      if (offset > (sizeof(appv_rsc_image) - 1)) {
        PRINTF("*** rsc_data: rsc[%d]='%s' offset=%u > %d - OUT OF ARRAY" MEOL,
               rsc_ptr->rsc_uref, rsc_ptr->rsc_name, offset, sizeof(appv_rsc_image) - 1);
        return -1;
      }
      int data_len = sizeof(appv_rsc_image) - offset - 1;
      ret = LiveObjectsClient_RscGetChunck(rsc_ptr, &appv_rsc_image[offset], data_len);
      if (ret > 0) {
        if ((offset + ret) > (sizeof(appv_rsc_image) - 1)) {
          PRINTF("*** rsc_data: rsc[%d]='%s' offset=%u - read=%d => %d > %d - OUT OF ARRAY" MEOL,
                 rsc_ptr->rsc_uref, rsc_ptr->rsc_name, offset, ret, offset + ret, sizeof(appv_rsc_image) - 1);
          return -1;
        }
        appv_rsc_offset += ret;
        if (appv_log_level > 0)
          PRINTF("*** rsc_data: rsc[%d]='%s' offset=%u - read=%d/%d - %u/%u" MEOL,
                 rsc_ptr->rsc_uref, rsc_ptr->rsc_name, offset, ret, data_len, appv_rsc_offset, appv_rsc_size);
      }
      else {
        PRINTF("*** rsc_data: rsc[%d]='%s' offset=%u - read error (%d) - %u/%u" MEOL,
               rsc_ptr->rsc_uref, rsc_ptr->rsc_name, offset, ret, appv_rsc_offset, appv_rsc_size);
      }
    }
#endif
#ifdef RSC_IDX_BINARY
    else if (rsc_ptr->rsc_uref == RSC_IDX_BINARY) {
      int data_len = sizeof(appv_rsc_binary) - 1;
      ret = LiveObjectsClient_RscGetChunck(rsc_ptr, appv_rsc_binary, data_len);
      if (ret > 0) {
        appv_rsc_offset += ret;
        if (appv_log_level > 0)
          PRINTF("*** rsc_data: rsc[%d]='%s' offset=%u - read=%d/%d - %u/%u" MEOL,
                 rsc_ptr->rsc_uref, rsc_ptr->rsc_name, offset, ret, data_len, appv_rsc_offset, appv_rsc_size);
      }
      else {
        PRINTF("*** rsc_data: rsc[%d]='%s' offset=%u - read error (%d) - %u/%u" MEOL,
               rsc_ptr->rsc_uref, rsc_ptr->rsc_name, offset, ret, appv_rsc_offset, appv_rsc_size);
      }
    }
#endif
    else {
      ret = -1;
    }
    return ret;
  }
#endif // APP_FEATURE_LO_RESOURCES


} // end of "C" code


// ----------------------------------------------------------
//
uint32_t loop_cnt = 0;

void appli_sched(void)
{
  ++loop_cnt;
  if (appv_log_level > 1) PRINTF("%lu:appli_sched: %u" MEOL, millis(), loop_cnt);

#if APP_FEATURE_LO_COMMANDS
  if (cmd_cnt > 0) {
    main_cmd_delayed_resp_LED();
  }
#endif // APP_FEATURE_LO_COMMANDS

#if APP_FEATURE_LO_DATA
  // Simulate measures : Voltage and Temperature ...
  if (appv_measures_volt <= 0.0)       appv_measures_volt_grad = 0.2;
  else if (appv_measures_volt >= 5.0)  appv_measures_volt_grad = -0.3;

  if (appv_measures_temp <= -3)        appv_measures_temp_grad = 1;
  else if (appv_measures_temp >= 20)  appv_measures_temp_grad = -1;

  appv_measures_volt += appv_measures_volt_grad;
  appv_measures_temp += appv_measures_temp_grad;

  if (appv_measures_enabled) {
    appv_measures_counter++;
  }

#if 1
  if (appv_log_level > 0) {
    PRINTF("%lu:appli_sched: %u -- cnt=%u temp=%d volt=%2.2f" MEOL, millis(), loop_cnt, appv_measures_counter, appv_measures_temp, appv_measures_volt);
    PRINTF(" -- cnt=%u" MEOL, appv_measures_counter);
    PRINTF(" -- temp=%d" MEOL, appv_measures_temp);
    PRINTF(" -- volt=%f %g" MEOL, appv_measures_volt, appv_measures_volt);
    Serial.print(" ** volt= ");
    Serial.print(appv_measures_volt);
    Serial.println("");
  }
#endif

  if (appv_measures_enabled) {
    int ret;
    if (appv_log_level > 0) {
      PRINTF("%lu:appli_sched: %u - PUBLISH ..." MEOL, millis(), loop_cnt);
    }

    ret = LiveObjectsClient_PushData( appv_hdl_data );
    if (ret) {
      PRINTF("%lu:thread_appli: %u - PUBLISH ERROR %d" MEOL, millis(), loop_cnt, ret);
    }
  }
#endif // APP_FEATURE_LO_DATA
}

// ==========================================================
//
static VMINT    appv_bearer_handle;
static VMINT    appv_bearer_event = 0;

static short    appv_con_liveobjects = 0;
static enum {
  DNS_RESOLVE_ERROR = - 1,
  DNS_RESOLVE_IDLE = 0,
  DNS_RESOLVE_PENDING,
  DNS_RESOLVE_OK
} appv_con_dnsresolve = DNS_RESOLVE_IDLE;


void app_dns_resolve_awakeup(char state)
{
#if 1
  Serial.print(millis());
  Serial.print(":app_dns_resolve_awakeup: DNS ");
  if (state == 1) Serial.print("RESOLVED ");
  else if (state == 2) Serial.print("BAD ");
  else Serial.print("ERROR ");
#if DNS_BLOCKING
  Serial.print("=> post signal");
#endif
  Serial.println("");
#endif

  appv_con_dnsresolve = (state == 1) ? DNS_RESOLVE_OK : DNS_RESOLVE_ERROR;

#if DNS_BLOCKING
  LTask.post_signal();
#endif
}

boolean  liveobjects_do_init(void* ctx)
{
  int ret;

  appv_con_liveobjects = 0;
  appv_con_dnsresolve = DNS_RESOLVE_IDLE;

  LiveObjectsClient_InitDbgTrace(DBG_DFT_TRACE_LEVEL);
  LiveObjectsClient_SetDbgMsgDump(DBG_DFT_MSG_DUMP);

  PRINTF("liveobjects_do_init: LiveObjectsClient_Init ..." MEOL);
  ret = LiveObjectsClient_Init(NULL);
  if (ret) {
    PRINTF("liveobjects_do_init: LiveObjectsClient_Init -> ERROR (%d)" MEOL, ret);
    return true;
  }

  // Attach my local Configuration Parameters to the LiveObjects Client instance
  // ----------------------------------------------------------------------------
#if APP_FEATURE_LO_PARAMS
  ret = LiveObjectsClient_AttachCfgParams(appv_set_param, SET_PARAM_NB, main_cb_param_udp);
  if (ret) PRINTF("liveobjects_do_init: LiveObjectsClient_AttachCfgParams -> ERROR (%d)" MEOL, ret);
  else PRINTF("liveobjects_do_init: LiveObjectsClient_AttachCfgParams -> OK - ret=%d" MEOL, ret);
#endif

  // Attach my local RESOURCES to the LiveObjects Client instance
  // ------------------------------------------------------------
#if APP_FEATURE_LO_RESOURCES
  ret = LiveObjectsClient_AttachResources(appv_set_resources, SET_RESOURCES_NB, main_cb_rsc_ntfy, main_cb_rsc_data);
  if (ret < 0) PRINTF("liveobjects_do_init: LiveObjectsClient_AttachResources -> ERROR (%d)" MEOL, ret);
  else PRINTF("liveobjects_do_init: LiveObjectsClient_AttachResources -> OK - ret=%d" MEOL, ret);
#endif

  // Attach my local STATUS data to the LiveObjects Client instance
  // --------------------------------------------------------------
#if APP_FEATURE_LO_STATUS
  appv_hdl_status = LiveObjectsClient_AttachStatus(appv_set_status, SET_STATUS_NB);
  if (appv_hdl_status) PRINTF("liveobjects_do_init: LiveObjectsClient_AttachStatus -> ERROR (%d)" MEOL, appv_hdl_status);
  else PRINTF("liveobjects_do_init: LiveObjectsClient_AttachStatus -> OK - ret=%d" MEOL, appv_hdl_status);
#endif

  // Attach one set of collected data to the LiveObjects Client instance
  // --------------------------------------------------------------------
#if APP_FEATURE_LO_DATA
  appv_hdl_data = LiveObjectsClient_AttachData(STREAM_PREFIX, "LO_sample_measures", "mV1", "\"Test\"", NULL, appv_set_measures, SET_MEASURES_NB);
  if (appv_hdl_data) PRINTF("liveobjects_do_init: LiveObjectsClient_AttachData -> ERROR (%d)" MEOL, appv_hdl_data);
  else PRINTF("liveobjects_do_init: LiveObjectsClient_AttachData -> OK - ret=%d" MEOL, appv_hdl_data);
#endif

  // Attach a set of commands to the LiveObjects Client instance
  // -----------------------------------------------------------
#if APP_FEATURE_LO_COMMANDS
  ret = LiveObjectsClient_AttachCommands(appv_set_commands, SET_COMMANDS_NB, main_cb_command);
  if (ret) PRINTF("liveobjects_do_init: LiveObjectsClient_AttachCommands -> ERROR (%d)" MEOL, ret);
  else PRINTF("liveobjects_do_init: LiveObjectsClient_AttachCommands -> OK - ret=%d" MEOL, ret);
#endif

  // Enable the receipt of commands
  // ------------------------------
#if APP_FEATURE_LO_COMMANDS
  ret = LiveObjectsClient_ControlCommands(true);
  if (ret) PRINTF("liveobjects_do_init: LiveObjectsClient_ControlCommands -> ERROR (%d)" MEOL, ret);
  else PRINTF("liveobjects_do_init: LiveObjectsClient_ControlCommands -> OK - ret=%d" MEOL, ret);
#endif

  // Enable the receipt of resource update requests
  // ----------------------------------------------
#if APP_FEATURE_LO_RESOURCES
  ret = LiveObjectsClient_ControlResources(true);
  if (ret) PRINTF("liveobjects_do_init: LiveObjectsClient_ControlResources -> ERROR (%d)" MEOL, ret);
  else PRINTF("liveobjects_do_init: LiveObjectsClient_ControlResources -> OK - ret=%d" MEOL, ret);
#endif

  appv_con_liveobjects = 1;
  appv_con_dnsresolve = DNS_RESOLVE_IDLE;

  PRINTF("liveobjects_do_init: OK" MEOL);
  return true;
}

// ==========================================================
//
boolean liveobjects_do_resolve(void* ctx)
{
  int ret;
  PRINTF("liveobjects_do_resolve: call LiveObjectsClient_DnsResolve ..." MEOL);
  appv_con_dnsresolve = DNS_RESOLVE_IDLE;
  ret = LiveObjectsClient_DnsResolve();
  if (ret > 0) {
    appv_con_dnsresolve = DNS_RESOLVE_PENDING;
#if DNS_BLOCKING
    PRINTF("liveobjects_do_resolve: LiveObjectsClient_DnsResolve -> ret (%d) => wait signal" MEOL, ret);
    return false;
#else
    PRINTF("liveobjects_do_resolve: LiveObjectsClient_DnsResolve -> ret (%d) => don't wait !!!" MEOL, ret);
    return true;
#endif
  }
  else if (ret == 0) {
    PRINTF("liveobjects_do_resolve: LiveObjectsClient_DnsResolve -> ret (%d) => OK" MEOL, ret);
    appv_con_dnsresolve = DNS_RESOLVE_OK;
  }
  else {
    appv_con_dnsresolve = DNS_RESOLVE_ERROR;
    PRINTF("liveobjects_do_resolve: LiveObjectsClient_DnsResolve -> ERROR (%d)", ret);
  }
  return true;
}
// ==========================================================
//
boolean liveobjects_do_connect(void* ctx)
{
  int ret;

  if (appv_con_liveobjects != 1) {
    PRINTF("liveobjects_do_connect: ERROR - unexpected state - appv_con_liveobjects=%d" MEOL, appv_con_liveobjects);
    return true;
  }
  if (appv_bearer_event != VM_BEARER_ACTIVATED) {
    PRINTF("liveobjects_do_connect: ERROR - unexpected state - appv_bearer_event=%d" MEOL, appv_bearer_event);
    return true;
  }
  if (appv_con_dnsresolve != DNS_RESOLVE_OK) {
    PRINTF("liveobjects_do_connect: ERROR - unexpected state - appv_con_dnsresolve=%d" MEOL, appv_con_dnsresolve);
    return true;
  }

  // Connect to the LiveObjects Platform
  // -----------------------------------
  Serial.println("liveobjects_do_connect: LiveObjectsClient_Connect ...");
  ret = LiveObjectsClient_Connect();
  if (ret) {
    PRINTF("liveobjects_do_connect: LiveObjectsClient_Connect -> ERROR (%d)" MEOL, ret);
    return true;
  }

#if APP_FEATURE_LO_STATUS
  ret = LiveObjectsClient_PushStatus(appv_hdl_status);
  if (ret) PRINTF("liveobjects_do_connect: LiveObjectsClient_PushStatus -> ERROR (%d)" MEOL, ret);
#endif

  appv_con_liveobjects = 2;

  PRINTF("liveobjects_do_connect: OK" MEOL);
  return true;
}

// ==========================================================
// Network Setup
//

// invoked in main thread context
void bearer_callback(VMINT handle, VMINT event, VMUINT data_account_id, void *user_data)
{
  Serial.print("bearer_callback: ");
  if (VM_BEARER_WOULDBLOCK == appv_bearer_handle) {
    Serial.print("VM_BEARER_WOULDBLOCK");
  }
  else {
    Serial.print(appv_bearer_handle);
  }
  Serial.print(" handle=");
  Serial.print(handle);

  Serial.print(" event=");
  Serial.print(event);

  switch (event)
  {
    case VM_BEARER_DEACTIVATED:
      Serial.println(" VM_BEARER_DEACTIVATED");
      if ((VM_BEARER_WOULDBLOCK == appv_bearer_handle))
        LTask.post_signal();
      break;
    case VM_BEARER_ACTIVATING:
      Serial.println(" VM_BEARER_ACTIVATING");
      break;
    case VM_BEARER_ACTIVATED:
      Serial.println(" VM_BEARER_ACTIVATED -> Post Signal");
      if ((VM_BEARER_WOULDBLOCK == appv_bearer_handle))
        LTask.post_signal();
      break;
    case VM_BEARER_DEACTIVATING:
      Serial.println(" VM_BEARER_DEACTIVATING");
      break;
    default:
      Serial.println(" (unknown)");
      break;
  }
  appv_bearer_handle = handle;
  appv_bearer_event = event;
}

boolean bearer_open(void* ctx) {
  appv_bearer_event = VM_BEARER_DEACTIVATED;

  Serial.println("bearer_open: GPRS ...");
  appv_bearer_handle = vm_bearer_open(VM_APN_USER_DEFINE ,  NULL, bearer_callback);

  if (VM_BEARER_WOULDBLOCK == appv_bearer_handle) {
    Serial.println("bearer_open: VM_BEARER_WOULDBLOCK -> wait signal ...");
    return false;
  }
  if (appv_bearer_handle >= 0) {
    Serial.println("bearer_open: OK");
    appv_bearer_event = VM_BEARER_ACTIVATED;
  }
  else {
    Serial.print("bearer_open: ERROR ");
    Serial.print(appv_bearer_handle);
    Serial.println (" ");
  }
  return true;
}

// ==========================================================

// Main setup function
//
void setup()
{
  LTask.begin();

  Serial.begin(9600);
  while (!Serial)
    delay(100);

  delay(100);

  Serial.println(appv_version);
  Serial.println("Start after 5 secondes ..");
  Serial.flush();
  delay(5000);

  Serial.println(appv_version);
  Serial.flush();

  Serial.print(millis());
  Serial.println(":SIM/GSM Ready ...");
  while (!LSMS.ready())
    delay(1000);
  Serial.print(millis());
  Serial.println(":OK");

#ifdef SMS_TEL
  LSMS.beginSMS(SMS_TEL);
  LSMS.print("Hello from LinkIt ");
  if (LSMS.endSMS()) {
    Serial.println("SMS is sent");
  }
  else {
    Serial.println("SMS is not sent");
  }
#endif

  // keep retrying until connected to network
  Serial.print(millis());
  Serial.println(":Connecting to GPRS...");
  Serial.flush();
  while (!LGPRS.attachGPRS(GPRS_APN, GPRS_USERNAME, GPRS_PASSWORD))
  {
    delay(1000);
    Serial.println(" -> Retry ...");
  }

  Serial.print(millis());
  Serial.print(":OK apn= ");
  Serial.print(LGPRS.getAPN());
  Serial.println("");

#if 0
  Serial.print(millis());
  Serial.println(":pause 1000 ms...");
  delay(1000);
#endif
  Serial.print(millis());
  Serial.println(":setup: call liveobjects_do_init ...");
  LTask.remoteCall(&liveobjects_do_init, NULL);

#if 1
  if (appv_con_liveobjects == 1) {
#if 0
    Serial.print(millis());
    Serial.println(":pause 5000 ms...");
    delay(5000);
#endif
    Serial.print(millis());
    Serial.println(":setup: call liveobjects_do_resolve ...");
    LTask.remoteCall(&liveobjects_do_resolve, NULL);
    Serial.print(millis());
    Serial.print(":setup: resolve result = ");
    Serial.print(appv_con_dnsresolve);
    Serial.println("");
  }
#endif

  Serial.print(millis());
  Serial.println(":setup: call bearer_open ...");
  LTask.remoteCall(&bearer_open, NULL);

#if 0
  Serial.print(millis());
  Serial.println(":pause 2000 ms...");
  delay(2000);
#endif
}

// nativeLoop which will use to do your main job in the loop
boolean nativeLoop(void* ctx)
{
  int ret;
  unsigned long tf = millis() + 10000;

  if (appv_log_level > 2)  {
    Serial.print(millis());
    Serial.println(":nativeLoop ...");
    Serial.flush();
  }

  appli_sched();

  if (appv_con_liveobjects == 2) {
    if (appv_log_level > 1) {
      Serial.print(millis());
      Serial.println(":nativeLoop: Call LiveObjectsClient_Cycle(5000) ...");
    }
    ret = LiveObjectsClient_Cycle(10000);
    if (ret) {
      Serial.print(millis());
      Serial.println(":nativeLoop: ERROR returned by LiveObjectsClient_Cycle");
      appv_con_liveobjects = 1;
    }
  }
  else {
    Serial.print(millis());
    Serial.print(":nativeLoop: unexpected connection state ");
    Serial.print(appv_con_liveobjects);
    Serial.println(" !!");
  }

  if (tf) {
    unsigned long now = millis();
    if (tf  > now) {
      tf = tf - now;
      if (appv_log_level > 0)  {
        Serial.print(now);
        Serial.print(":nativeLoop: pause ");
        Serial.print(tf);
        Serial.println(" ms");
      }
      delay(tf);
    }
  }

#if 0
  Serial.print(millis());
  Serial.println(":nativeLoop: Done");
#endif
  Serial.flush();
  return true;
}

// ARDUINO LOOP
void loop()
{
#if 1
  if (appv_log_level > 2)  {
    Serial.print(millis());
    Serial.println(":loop ...");
    Serial.flush();
  }
#endif
  if (appv_con_liveobjects == 2) {
    LTask.remoteCall(nativeLoop, NULL);
  }
  else if ((appv_bearer_event == VM_BEARER_ACTIVATED) &&  (appv_con_liveobjects == 1)) {
    if (appv_con_dnsresolve == DNS_RESOLVE_OK) {
      Serial.print(millis());
      Serial.println(":loop: call liveobjects_do_connect ...");
      LTask.remoteCall(&liveobjects_do_connect, NULL);
      if (appv_con_liveobjects != 2) {
        delay(5000);
      }
    }
    else if (appv_con_dnsresolve == DNS_RESOLVE_IDLE) {
      Serial.print(millis());
      Serial.println(":loop: call liveobjects_do_resolve ...");
      LTask.remoteCall(&liveobjects_do_resolve, NULL);
    }
    else {
      Serial.print(millis());
      Serial.println(":loop: pause 1 s ...");
      delay(1000);
    }
  }
  else {
    Serial.print(millis());
    Serial.println(":loop: pause 5 s ...");
    delay(5000);
  }

#if 0
  Serial.print(millis());
  Serial.println(":loop: Done");
  Serial.flush();
#endif
}

