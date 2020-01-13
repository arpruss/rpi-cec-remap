#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <bcm_host.h>
#include "interface/vchiq_arm/vchiq_if.h"
#include "interface/vcos/vcos.h"
#include "interface/vchi/vchi.h"
#include "interface/vmcs_host/vc_cecservice_defs.h"
#include "interface/vmcs_host/vc_cec.h"
#include "cec.h"

static KeyCallback cb = NULL;
static int initialized = 0;
static uint16_t physical_address = 0xFFFF;

static void tv_callback(void *callback_data, uint32_t reason, uint32_t param1, uint32_t param2) {
    cec_update();
}

static void cec_callback(void *callback_data, uint32_t reason, uint32_t param1, uint32_t param2, uint32_t param3, uint32_t param4) {
#ifdef CECTEST
    printf("cec test %x %x\n", reason, param1);
#else
    switch(reason&0xFFFF) {
	case VC_CEC_BUTTON_PRESSED:
	case VC_CEC_REMOTE_PRESSED:
	    cb((param1>>16)&0xFF,1);
	    break;
	case VC_CEC_BUTTON_RELEASE:
	case VC_CEC_REMOTE_RELEASE:
	    cb((param1>>16)&0xFF,0);
	    break;
	default:
	    break;
    }
#endif
}

static void broadcast_physical_address(uint16_t address) {
    uint8_t message[4];
    message[0] = CEC_Opcode_ReportPhysicalAddress;
    message[1] = address >> 8;
    message[2] = address;
    message[3] = 1; // recording device
    vc_cec_send_message(CEC_BROADCAST_ADDR, message, 4, 1);
}


int init_cec(KeyCallback _cb) {
    bcm_host_init();
    
    vc_cec_set_passive(1);
    vc_cec_register_callback(cec_callback, NULL);
    vc_tv_register_callback(tv_callback, NULL);
    //vc_cec_set_logical_address(13/*FREEUSE*/, 2/*reserved*/, 0x18C086L/*BROADCOM*/);
    vc_cec_set_logical_address(1/*recorder 1*/, 1/*recording device*/, 0x18C086L/*BROADCOM*/);
    // TODO: set another address if this one is taken

    while (vc_cec_get_physical_address(&physical_address)) {
         sleep(1);
    }
    broadcast_physical_address(physical_address);

    //vc_cec_set_logical_address(0/*tv 1*/, 0/*tv*/, 0x18C086L/*BROADCOM*/);
    //vc_cec_register_all();
    
    vc_cec_register_command(CEC_Opcode_UserControlPressed);
    vc_cec_register_command(CEC_Opcode_UserControlReleased);

    cb = _cb;
    initialized = 1;
    return 0;
}

void end_cec(void) {
    vc_tv_unregister_callback(tv_callback);
    vc_cec_release_logical_address();
    vc_cec_set_passive(0);
    bcm_host_deinit();
    initialized = 0;
}

void cec_update(void) {
    if(initialized) {
    	end_cec();
	init_cec(cb);
    }
}

#ifdef CECTEST
int main() {
    if (init_cec(NULL)<0)
	return 1;
    while(1) sleep(6000);
    return 0;
}
#endif
