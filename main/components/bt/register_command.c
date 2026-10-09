#include "register_command.h"


#if 0

static void cmd_brightness(const char *data) {
    int level = atoi(data);
    if (level < 0) level = 0;
    if (level > 100) level = 100;
    set_brightness_level((uint8_t)level);
    ESP_LOGI(TAG, "BT: brightness = %d", level);
}

static void cmd_suspend(const char *data) {
    (void)data;
    freeze_background_tasks();
    suspend_display();
    if (lvgl_port_lock(0)) {
        lv_indev_enable(touch_indev, false);
        lvgl_port_unlock();
    }
    xTaskCreatePinnedToCore(resume_display_task, "resume_display_task", 2048, NULL, 5, NULL, 1);
}

static void cmd_resume(const char *data) {
    (void)data;
    s_suspend_display = false;
    resume_display();
    resume_background_tasks();
    if (lvgl_port_lock(0)) {
        lv_indev_enable(touch_indev, true);
        lvgl_port_unlock();
    }
}

static void cmd_battery(const char *data) {
    (void)data;
    uint32_t mv = battery_get_voltage_mv();
    ESP_LOGI(TAG, "BT: battery = %.2f V", mv / 1000.0f);
}

static void cmd_log(const char *data) {
    ESP_LOGI(TAG, "BT log: %s", data);
}

static void cmd_reboot(const char *data) {
    (void)data;
    ESP_LOGW(TAG, "BT: reboot richiesto");
    vTaskDelay(pdMS_TO_TICKS(100));
    esp_restart();
}

#endif

void bt_cmds_register(bt_cmd_entry_t *cmds, size_t num_cmds) {
    //bt_cmd_register("brightness", cmd_brightness);
    //bt_cmd_register("suspend",    cmd_suspend);
    //bt_cmd_register("resume",     cmd_resume);
    //bt_cmd_register("battery",    cmd_battery);
    //bt_cmd_register("log",        cmd_log);
    //bt_cmd_register("reboot",     cmd_reboot);

    for (size_t i = 0; i < num_cmds; i++) {
        bt_cmd_register(cmds[i].name, cmds[i].handler);
    }    
}