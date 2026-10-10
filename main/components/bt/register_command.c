#include "register_command.h"

void bt_cmds_register(bt_cmd_entry_t *cmds, size_t num_cmds) {
    for (size_t i = 0; i < num_cmds; i++) {
        bt_cmd_register(cmds[i].name, cmds[i].handler);
    }    
}