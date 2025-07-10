/*
 * menu.h
 *
 *  Created on: Mar 20, 2025
 *      Author: lilang
 */

#ifndef INC_MENU_H_
#define INC_MENU_H_

#include "main.h"
#include "LCD.h"


#define MAX_ITEMS 5  // 每页最大显示项数

#define KEY_UP 			KEY1_Pin  	// 按键宏定义
#define KEY_ENTER 		KEY2_Pin  	// 按键宏定义
#define KEY_DOWN 		KEY3_Pin  	// 按键宏定义

extern uint8_t key_record_time;
extern uint8_t key_mode;


// 刷新模式
typedef enum {
    CONTINOUS_REFRESH,			//连续刷新模式
		SINGLE_REFRESH					//单次刷新模式
} Refresh_mode_TypeDef;

// 菜单项结构体
typedef struct MenuItem {
    char *name;                 				// 菜单显示名称
    struct MenuItem *parent;    				// 父菜单项
    struct MenuItem *child;     				// 子菜单项链表头
    struct MenuItem *next;      				// 同级下一个菜单项
    void (*display)(void);      				// 自定义显示函数
    void (*key_handler)(uint16_t key); 			// 按键处理函数
		uint8_t refresh_mode;					//刷新模式
} MenuItem;

// 全局当前菜单指针
extern MenuItem *currentMenu;

// 函数声明
void Menu_DefaultDisplay(void);
void Menu_Refresh(void);
static void Menu_DefaultKeyHandler(uint16_t key);
void Menu_KeyProcess(uint16_t key);
void Menu_Init(void);

#endif /* INC_MENU_H_ */


