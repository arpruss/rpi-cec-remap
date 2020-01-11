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

static KeyCallback cb;

void callback(void *callback_data, uint32_t reason, uint32_t param1, uint32_t param2, uint32_t param3, uint32_t param4) {
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
    //printf("# called back for reason %x with data %x %x %x %x\n", reason, param1, param2, param3, param4);
}


int init_cec(KeyCallback _cb) {
    VCHI_INSTANCE_T vchi_instance;
    VCHI_CONNECTION_T *vchi_connections;
    
    // initialise bcm_host
    bcm_host_init();
    
    // initialise vcos/vchi
    vcos_init();
    if (vchi_initialise(&vchi_instance) != VCHIQ_SUCCESS) {
        fprintf(stderr, "failed to open vchiq instance\n");
        return -2;
    }
    
    // create a vchi connection
    if ( vchi_connect( NULL, 0, vchi_instance ) != 0) {
        fprintf(stderr, "failed to connect to VCHI\n");
        return -3;
    }
    
    // connect to cec
    vc_vchi_cec_init( vchi_instance, &vchi_connections, 1);

    vc_cec_set_passive(1);
    vc_cec_register_callback(callback, NULL);
    //vc_cec_register_all();
    vc_cec_register_command(CEC_Opcode_UserControlPressed);
    vc_cec_register_command(CEC_Opcode_UserControlReleased);
    cb = _cb;
    return 0;
}

void end_cec(void) {
}

#if 0
int main() {
    if (cec_init()<0)
	return 1;
    while(1) sleep(6000);
    return 0;
}
#endif
