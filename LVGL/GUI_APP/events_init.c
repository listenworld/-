/*
* Copyright 2025 NXP
* NXP Proprietary. This software is owned or controlled by NXP and may only be used strictly in
* accordance with the applicable license terms. By expressly accepting such terms or by downloading, installing,
* activating and/or otherwise using the software, you are agreeing that you have read, and that you agree to
* comply with and are bound by, such license terms.  If you do not agree to be bound by the applicable license
* terms, then you may not retain, install, activate or otherwise use the software.
*/

#include "events_init.h"
#include <stdio.h>
#include "lvgl.h"

#if LV_USE_GUIDER_SIMULATOR && LV_USE_FREEMASTER
#include "freemaster_client.h"
#endif


static void screen_tabview_1_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_CLICKED:
    {
        // 处理左右切换按键
        if(button_pressed_id == KEY_ID_LEFT) 
				{
					uint8_t id = lv_tabview_get_tab_act(guider_ui.screen_tabview_1);
					lv_tabview_set_act(guider_ui.screen_tabview_1,id-1,LV_ANIM_OFF);
        }
        else if(button_pressed_id == KEY_ID_RIGHT) 
				{
					uint8_t id = lv_tabview_get_tab_act(guider_ui.screen_tabview_1);
					lv_tabview_set_act(guider_ui.screen_tabview_1,id+1,LV_ANIM_OFF);
        }
        // 处理选择按键
        else if(button_pressed_id == KEY_ID_SELECT) 
				{
            // 这里可以添加选择操作
        }
        break;
    }
    default:
        break;
    }
}

void events_init_screen (lv_ui *ui)
{
    lv_obj_add_event_cb(ui->screen_tabview_1, screen_tabview_1_event_handler, LV_EVENT_ALL, ui);
}


void events_init(lv_ui *ui)
{

}
