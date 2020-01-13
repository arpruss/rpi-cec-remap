#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "inject.h"
#include "cec.h"
#include "proc.h"

#undef MULTI_PRESS // TODO: with MULTI_PRESS, handle app switching
#define MAX_CHORD 4
#define END 0xFFFFFFFF
#define NUM_ORIGINAL 256
#define NUM_OUTPUT 256

typedef struct {
    unsigned original;
    unsigned output[MAX_CHORD];
} remap_entry;

typedef struct app_remap {
    const char* app_name;
    remap_entry* entries;
    struct app_remap* parent;
    remap_entry* map[NUM_ORIGINAL];
} app_remap;

remap_entry keys_default[] = {
    {3,{KEY_LEFT}},
    {4,{KEY_RIGHT}},
    {1,{KEY_UP}},
    {2,{KEY_DOWN}},
    {13,{KEY_ESC}},
    {0,{KEY_ENTER}},
    {11,{KEY_LEFTMETA}},
    {72,{KEY_REWIND}},
    {73,{KEY_FASTFORWARD}},
    {70,{KEY_PLAY}},
    {END}
};

/*remap_entry keys_kodi[] = {
    {11,{KEY_COMPOSE}},
    {END}
};*/

remap_entry keys_np[] = {
    {11,{KEY_E}},
    {72,{KEY_R}},
    {70,{KEY_SPACE}},
    {END}
};

remap_entry keys_ra[] = {
    {3,{KEY_Z}},
    {0,{KEY_X}},
    {72,{KEY_H}},
    {END}
};

remap_entry keys_es[] = {
    {13,{KEY_LEFTALT,KEY_F4}}
};

app_remap remap_default = { "", keys_default, NULL };
//app_remap remap_kodi = { "", keys_default, NULL },
app_remap remap_np = { "numptyphysics", keys_np, &remap_default };
app_remap remap_ra = { "retroarch", keys_ra, &remap_default };
app_remap remap_es = { "emulationstation", keys_es, &remap_ra };
app_remap* remaps[] = { &remap_es, &remap_ra, &remap_np, &remap_default };
#define num_remaps (sizeof remaps / sizeof *remaps)

char running[num_remaps-1];
const char* app_names[num_remaps-1];
char pressed_keys[NUM_OUTPUT];

static void add_to_map(app_remap* a, remap_entry** map) {
    while(a != NULL) {
        for(remap_entry* e = a->entries ; e->original != END ; e++)
	    if (e->original < NUM_ORIGINAL && NULL == map[e->original])
	        map[e->original] = e;
        a = a->parent;
    }
}

#ifdef MULTI_PRESS
#error TODO: Not yet supported.
#else
void handle_key(unsigned original, unsigned pressed) {
    if (original >= NUM_ORIGINAL)
	return;
    if (! pressed) {
	for(unsigned i = 0; i<NUM_OUTPUT; i++)
	    if(pressed_keys[i]) {
		 inject_key(i, 0);
		 pressed_keys[i] = 0;
	    }
	return;
    }
    proc_monitor(running);
    unsigned i;
    for (i = 0 ; i < num_remaps - 1 && !running[i]; i++) ;
    remap_entry* e = remaps[i]->map[original];
    if (e == NULL || e->output[0] == 0)
	return;
    if (e->output[1] == 0) {
	inject_key(e->output[0], 1);
	pressed_keys[e->output[0]] = 1;
    }
    else {
	int i;
        for(i = 0 ; i < MAX_CHORD && e->output[i] ; i++) {
	    inject_key(e->output[i], 1);	
	}
        for(i-- ; i >= 0 ; i--) {
	    inject_key(e->output[i], 0);	
	    pressed_keys[e->output[i]] = 0;
	}
    }
}
#endif

static void init_remaps(void) {
    memset(pressed_keys, 0, NUM_OUTPUT * sizeof(char));
    for(unsigned int i = 0 ; i < num_remaps ; i++) {
	memset(remaps[i]->map, 0, sizeof(remaps[i]->map));
        add_to_map(remaps[i], remaps[i]->map);
	if (i+1<num_remaps)
	    app_names[i] = remaps[i]->app_name;
    }
    if (init_proc_monitor(app_names, num_remaps-1) < 0) {
	fputs("Cannot init proc monitor", stderr);
	exit(2);
    }
    if (init_inject() < 0) {
	fputs("Cannot init inject", stderr);
	exit(3);
    }
    if (init_cec(handle_key) < 0) {
	fputs("Cannot init cec", stderr);
	exit(1);
    }
}

int main() {
    init_remaps();
    while(1) {
        sleep(8);
	puts("up");
	cec_update();
    }
    return 0;
}

