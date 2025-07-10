/*
 * menu.c
 *
 *  Created on: Mar 20, 2025
 *      Author: lilang
 */

/**
  * @file    menu.c
  * @brief   OLED多级菜单系统使用说明
  * @version V1.1
  * @date    2025-03-20
  *
  * @note    功能特性：
  *          - 支持无限级菜单嵌套
  *          - 提供默认显示模板和按键处理逻辑
  *          - 支持单次/连续两种刷新模式
  *          - 自动处理菜单项高亮和导航
  *          - 最大支持4项/页的显示（通过MAX_ITEMS配置）
  *
  * @section 使用步骤
  * 1. 硬件准备
  *    - 确认OLED模块正确连接
  *    - 配置KEY_UP/KEY_DOWN/KEY_ENTER/KEY_BACK按键GPIO
  * 
  * 2. 系统集成
  *    - 将menu.h/menu.c加入工程
  *    - 在main.h中包含所需头文件
  *    - 调用Menu_Init()初始化菜单结构
  * 
  * 3. 菜单项定义（参考示例）
  *    - 在menu.c中定义新的MenuItem结构体
  *    - 配置name/display/key_handler/refresh_mode等参数
  *    - 修改Menu_Init()中的链表关系
  * 
  * 4. 主循环处理
  *    - 周期调用Menu_KeyProcess(Key_Scan())
  *    - 调用Menu_Refresh()执行显示刷新
  *
  * @section API说明
  * | 函数名称               | 功能说明                         | 调用时机                |
  * |------------------------|---------------------------------|-------------------------|
  * | Menu_Init()            | 初始化菜单层级结构               | 系统启动时调用一次       |
  * | Menu_KeyProcess()      | 处理按键输入                     | 检测到按键事件后立即调用 |
  * | Menu_Refresh()         | 执行显示刷新                     | 主循环中持续调用         |
  * | Menu_DefaultDisplay()  | 默认菜单显示模板                 | 菜单项display=NULL时调用|
  *
  * @section 自定义显示函数
  * 1. 在menu.c中定义显示函数：
  * void CustomDisplay(void) {
  *     OLED_Clear();
  *     OLED_Chinese(0,0,"自定义菜单");
  *     // 添加显示内容
  *     OLED_Update();
  * }
	*/
/***********************************
	本驱动文件仅适配HAL库版本
***********************************/
#include "stm32g4xx_hal.h"
#include "main.h"
#include "LCD.h"
#include "menu.h"
#include <string.h>
#include "EEPROM_M24C02.h"
#include "Mymain.h"

uint8_t M24C02_Vmax[2];
uint8_t M24C02_vset[2];

MenuItem *currentMenu = NULL;							
static uint8_t selectedIndex = 1;	//光标位置
static uint8_t cursorIndex = 1;		//光标模式
static MenuItem *displayItems[MAX_ITEMS]; 

static uint8_t single_refresh_request=1;


/**
  * @brief  用户编写菜单需要编写三个部分，一.菜单内容显示函数，二.系统菜单定义
	* 三.添加进入菜单初始化的链表中
	* @note  
  */



/* 菜单内容显示函数 ---用户可自定义部分*/
/*Mode---------------------------------------------------------------------*/
void Display_Mode(void)
{
	//构建菜单
	POINT_COLOR=BLACK;
	LCD_ShowString(0,32 ,12,"|<--------------------------------->",1);
	for(uint8_t i=0; i<15; i++)LCD_ShowString(218,i*16,16,"|",1);
	LCD_ShowString(0,160,12,"|<--------------------------------->",1);
	
    if(currentMenu->parent)	POINT_COLOR=RED,LCD_ShowString(0,0,32,currentMenu->name,1);

		// 显示菜单项
		MenuItem *item = currentMenu->child;
		for(uint8_t i=1; i<=MAX_ITEMS && item; i++) {
			// 高亮显示选中项LCD_ShowChinese(0,i*40-4,WHITE,BLACK,item->name, 32 , 1) ;
			if(i == selectedIndex)  POINT_COLOR=RED,LCD_ShowString(0,i*27+21,24,item->name,1);
				else				POINT_COLOR=BLACK,LCD_ShowString(0,i*27+21,24,item->name,1);
				item = item->next;
			
		}
		POINT_COLOR=BLACK;LCD_ShowString(0,140,24,"Please select a mode.",1);
		
}
void Display_Modeset(void)
{
    if(currentMenu->parent)	POINT_COLOR=RED,LCD_ShowString(126,0,32,currentMenu->name,0);

		// 显示菜单项
		MenuItem *item = currentMenu->child;
		for(uint8_t i=1; i<=MAX_ITEMS && item; i++) {
			// 高亮显示选中项LCD_ShowChinese(0,i*40-4,WHITE,BLACK,item->name, 32 , 1) ;
			if(i == selectedIndex)  POINT_COLOR=RED,LCD_ShowString(96,i*27+21,24,item->name,1);
				else				POINT_COLOR=BLACK,LCD_ShowString(96,i*27+21,24,item->name,1);
				item = item->next;
		}
}

static void Key_Mode(uint16_t key)
{
		switch(key) {
			
			case KEY_UP:{
				if			(selectedIndex >  1)			selectedIndex--;
				else if		(selectedIndex == 1)			selectedIndex=3;
				break;}
			case KEY_DOWN:
				if			(selectedIndex < 3)			selectedIndex++;
				else if		(selectedIndex == 3) 			selectedIndex=1;
				break;
			case KEY_ENTER:
				if(displayItems[selectedIndex]->child||displayItems[selectedIndex]->display!=Menu_DefaultDisplay) 
					{
						currentMenu = displayItems[selectedIndex];
						cursorIndex = 1;
					}
				break;
			default:
				break;
		}	
}

void Key_PickMode(uint16_t key)
{
	
	LCD_ShowString(0,160 ,24,"Confirm with the middle key;cancel with others.",1);
	switch(key) {
		
		case KEY_UP:
		case KEY_DOWN:
			currentMenu = currentMenu->parent;
			selectedIndex = 1;
			LCD_Clear(WHITE);
			break;
		case KEY_ENTER:
			
					 if(strcmp(currentMenu->name,"Grid")== 0)Program.Mode = Mode_PV_Grid;
				else if(strcmp(currentMenu->name,"Stro")== 0)Program.Mode = Mode_PV_Grid_Storage;
				else if(strcmp(currentMenu->name,"Dc_S")== 0)Program.Mode = Mode_PV_DC_Storage;
				else if(strcmp(currentMenu->name,"Dc_V")== 0)Program.Mode = Mode_PV_DC_V;
				else if(strcmp(currentMenu->name,"Dc_I")== 0)Program.Mode = Mode_PV_DC_I;
				else if(strcmp(currentMenu->name,"Ac_S")== 0)Program.Mode = Mode_PV_AC_Storage;
				else if(strcmp(currentMenu->name,"Ac_V")== 0)Program.Mode = Mode_PV_AC_V;
				else if(strcmp(currentMenu->name,"Ac_I")== 0)Program.Mode = Mode_PV_AC_I;
		
				currentMenu = currentMenu->parent,currentMenu = currentMenu->parent,selectedIndex = 1;
				LCD_Clear(WHITE);

			break;

	}
}
/*Set---------------------------------------------------------------------*/
void Display_Set(void)
{
	//构建菜单
	POINT_COLOR=BLACK;
	LCD_ShowString(0,32 ,12,"|<--------------------------------->",1);
	for(uint8_t i=0; i<15; i++)LCD_ShowString(218,i*16,16,"|",1);
	LCD_ShowString(0,160,12,"|<--------------------------------->",1);
	
	if(currentMenu->parent)	POINT_COLOR=RED,LCD_ShowString(0,0,32,currentMenu->name,1);
	//
	if(Program.Mode==Mode_Kong)
	{
		POINT_COLOR=BLACK;
		LCD_ShowString(0,140,20,"Please return to the selection mode.",1);
	}else
		{
			
			
		}
	
	
	
}
static void Key_Set(uint16_t key)
{
		if(Program.Mode==Mode_Kong)
	{
				switch(key) {
					
					case KEY_UP:
					case KEY_DOWN:
					case KEY_ENTER:
						currentMenu = currentMenu->parent;
						selectedIndex = 1;
						LCD_Clear(WHITE);
						break;
				}	
	}	else{//模式选择完毕才允许调值
			if(cursorIndex == 1){
				switch(key) {
					
					case KEY_UP:{
						if			(selectedIndex >  1)			selectedIndex--;
						else if		(selectedIndex == 1)			selectedIndex=3;
						break;}
					case KEY_DOWN:
						if			(selectedIndex < 3)			selectedIndex++;
						else if		(selectedIndex == 3) 			selectedIndex=1;
						break;
					case KEY_ENTER:
						cursorIndex=0;
						break;
					default:
						break;
				}	
			}	else{
				switch(key) {
					
					case KEY_UP:{
						if			(selectedIndex >  1)			selectedIndex--;
						else if		(selectedIndex == 1)			selectedIndex=3;
						break;}
					case KEY_DOWN:
						if			(selectedIndex < 3)			selectedIndex++;
						else if		(selectedIndex == 3) 			selectedIndex=1;
						break;
					case KEY_ENTER:
						cursorIndex=0;
						break;
					default:
						break;
				}	
					
				}

	}}
/*Show---------------------------------------------------------------------*/
void Display_Show(void)
{
	//构建菜单
	POINT_COLOR=BLACK;
	LCD_ShowString(0,32 ,12,"|<--------------------------------->",1);
	for(uint8_t i=0; i<15; i++)LCD_ShowString(218,i*16,16,"|",1);
	LCD_ShowString(0,160,12,"|<--------------------------------->",1);
	
	if(currentMenu->parent)	POINT_COLOR=RED,LCD_ShowString(0,0,32,currentMenu->name,1);
	//
	if(Program.Mode==Mode_Kong)
	{
		LCD_ShowString(0,140,24,"Please return to the selection mode.",1);
	}else
		{
			
			
		}
	
	
	
}

uint8_t aim=1;
static void Key_Show(uint16_t key)
{
		if(Program.Mode==Mode_Kong)
	{
				switch(key) {
					
					case KEY_UP:
					case KEY_DOWN:
					case KEY_ENTER:
						currentMenu = currentMenu->parent;
						selectedIndex = 1;
						LCD_Clear(WHITE);
						break;
				}	
	}	else{//模式选择完毕才允许调值
			if(cursorIndex == 1){
				switch(key) {
					
					case KEY_UP:{
						if			(selectedIndex >  1)			selectedIndex--;
						else if		(selectedIndex == 1)			selectedIndex=3;
						break;}
					case KEY_DOWN:
						if			(selectedIndex < 3)			selectedIndex++;
						else if		(selectedIndex == 3) 			selectedIndex=1;
						break;
					case KEY_ENTER:
						cursorIndex=0;
						break;
					default:
						break;
				}	
			}	else{
				switch(key) {
					
					case KEY_UP:{
						if			(selectedIndex >  1)			selectedIndex--;
						else if		(selectedIndex == 1)			selectedIndex=3;
						break;}
					case KEY_DOWN:
						if			(selectedIndex < 3)			selectedIndex++;
						else if		(selectedIndex == 3) 			selectedIndex=1;
						break;
					case KEY_ENTER:
						cursorIndex=0;
						break;
					default:
						break;
				}	
					
				}
			
	}}
/*Start---------------------------------------------------------------------*/
void Display_Start(void)
{
	//构建菜单
	POINT_COLOR=BLACK;
	LCD_ShowString(0,32 ,12,"|<--------------------------------->",1);
	for(uint8_t i=0; i<15; i++)LCD_ShowString(218,i*16,16,"|",1);
	LCD_ShowString(0,160,12,"|<--------------------------------->",1);
	
    if(currentMenu->parent)	POINT_COLOR=RED,LCD_ShowString(0,0,32,currentMenu->name,1);
	
	
	
}
static void Key_Start(uint16_t key)
{
	if(cursorIndex == 1){
		switch(key) {
			
			case KEY_UP:{
				if			(selectedIndex >  1)			selectedIndex--;
				else if		(selectedIndex == 1)			selectedIndex=3;
				break;}
			case KEY_DOWN:
				if			(selectedIndex < 3)			selectedIndex++;
				else if		(selectedIndex == 3) 			selectedIndex=1;
				break;
			case KEY_ENTER:
				cursorIndex=0;
				break;
			default:
				break;
		}	
	}
	else{
		switch(key) {
			
			case KEY_UP:{
				if			(selectedIndex >  1)			selectedIndex--;
				else if		(selectedIndex == 1)			selectedIndex=3;
				break;}
			case KEY_DOWN:
				if			(selectedIndex < 3)			selectedIndex++;
				else if		(selectedIndex == 3) 			selectedIndex=1;
				break;
			case KEY_ENTER:
				cursorIndex=0;
				break;
			default:
				break;
		}	
	}}
/******************************子菜单处理*************************************///end

/* 系统菜单定义 */
MenuItem menuMain = {
    .name = "Main",
    .display = Menu_DefaultDisplay,
    .key_handler = Menu_DefaultKeyHandler,
		.refresh_mode = SINGLE_REFRESH
};
/*-----------Mode-----------*/
MenuItem menuMode = {
    .name = "Mode",
    .display = Display_Mode,
    .key_handler = Key_Mode,
		.refresh_mode = SINGLE_REFRESH
};
		//PvGd
		MenuItem menuPvGd = {
			.name = "PvGd",
			.display = Display_Modeset,
			.key_handler = Key_Mode,
				.refresh_mode = SINGLE_REFRESH
		};
				MenuItem menuGrid = {
					.name = "Grid",
					.display = Display_Mode,
					.key_handler = Key_PickMode,
						.refresh_mode = SINGLE_REFRESH
				};
				MenuItem menuStro = {
					.name = "Stro",
					.display = Display_Mode,
					.key_handler = Key_PickMode,
						.refresh_mode = SINGLE_REFRESH
				};
		//PvAc
		MenuItem menuPvAc = {
			.name = "PvAc",
			.display = Display_Modeset,
			.key_handler = Key_Mode,
				.refresh_mode = SINGLE_REFRESH
		};
				MenuItem menuAc_S = {
					.name = "Ac_S",
					.display = Display_Mode,
					.key_handler = Key_PickMode,
						.refresh_mode = SINGLE_REFRESH
				};
				MenuItem menuAc_V = {
					.name = "Ac_V",
					.display = Display_Mode,
					.key_handler = Key_PickMode,
						.refresh_mode = SINGLE_REFRESH
				};
				MenuItem menuAc_I = {
					.name = "Ac_I",
					.display = Display_Mode,
					.key_handler = Key_PickMode,
						.refresh_mode = SINGLE_REFRESH
				};
		//PvDc
		MenuItem menuPvDc = {
			.name = "PvDc",
			.display = Display_Modeset,
			.key_handler = Key_Mode,
				.refresh_mode = SINGLE_REFRESH
		};
				MenuItem menuDc_S = {
					.name = "DcDc_S",
					.display = Display_Mode,
					.key_handler = Key_PickMode,
						.refresh_mode = SINGLE_REFRESH
				};
				MenuItem menuDc_V = {
					.name = "Dc_V",
					.display = Display_Mode,
					.key_handler = Key_PickMode,
						.refresh_mode = SINGLE_REFRESH
				};
				MenuItem menuDc_I = {
					.name = "Dc_I",
					.display = Display_Mode,
					.key_handler = Key_PickMode,
						.refresh_mode = SINGLE_REFRESH
				};
/*-----------Set-----------*/
MenuItem menuSet = {
    .name = "Set",
    .display = Display_Set,
    .key_handler = Key_Set,
		.refresh_mode = SINGLE_REFRESH
};
		MenuItem menuDc_value = {
			.name = "Dc",
			.display = Display_Set,
			.key_handler = Key_Set,
				.refresh_mode = SINGLE_REFRESH
		};
		MenuItem menuAc_value = {
			.name = "Ac",
			.display = Display_Set,
			.key_handler = Key_Set,
				.refresh_mode = SINGLE_REFRESH
		};
		MenuItem menuBack = {
			.name = "Back",
			.display = Display_Set,
			.key_handler = Key_Set,
				.refresh_mode = SINGLE_REFRESH
		};

/*----------Show----------*/
MenuItem menuShow = {
    .name = "Show",
    .display = Display_Show,
    .key_handler = Key_Show,
		.refresh_mode = SINGLE_REFRESH
};
/*----------Start----------*/
MenuItem menuStart = {
    .name = "Start",
    .display = Display_Start,
    .key_handler = Key_Start,
		.refresh_mode = SINGLE_REFRESH
};


/**
  * @brief  菜单系统初始化
  * @note   构建菜单层级结构，设置父子关系和同级关系
  *         初始化当前菜单指针和选中项索引
  *         示例结构：
  *         Main
  *         ├─ Set
  *         ├─ Mode
  *         └─ Show
  *             └─ DateTime
  */
void Menu_Init(void) {
    // 构建菜单结构
    menuMain.child = &menuMode;
    menuMode.parent = &menuMain;
		//mode
		menuMode.next = &menuSet;
    menuSet.parent = &menuMain;
				//PvGd
				menuMode.child = &menuPvGd;
			menuPvGd.parent = &menuMode;
						menuPvGd.child = &menuGrid;
					menuGrid.parent = &menuPvGd;
						menuGrid.next = &menuStro;
					menuStro.parent = &menuPvGd;
				//PvDc
				menuPvGd.next = &menuPvDc;
			menuPvDc.parent = &menuMode;
						menuPvDc.child = &menuDc_S;
					menuDc_S.parent = &menuPvDc;
						menuDc_S.next = &menuDc_V;
					menuDc_V.parent = &menuPvDc;
						menuDc_V.next = &menuDc_I;
					menuDc_I.parent = &menuPvDc;
				//PvAc
				menuPvDc.next = &menuPvAc;
			menuPvAc.parent = &menuMode;
						menuPvAc.child = &menuAc_S;
					menuAc_S.parent = &menuPvAc;
						menuAc_S.next = &menuAc_V;
					menuAc_V.parent = &menuPvAc;
						menuAc_V.next = &menuAc_I;
					menuDc_I.parent = &menuPvAc;
			
	
		//set
		menuSet.next = &menuShow;
    menuShow.parent = &menuMain;
				menuSet.child = &menuDc_value;
			menuDc_value.parent = &menuSet;
				menuDc_value.next = &menuAc_value;
			menuAc_value.parent = &menuSet;
				menuAc_value.next = &menuBack;
			menuBack.parent = &menuSet;
	
		//start
		menuShow.next = &menuStart;
    menuStart.parent = &menuMain;
	
	
    // 初始化显示地址
    currentMenu = &menuMain;
    selectedIndex = 1;
	cursorIndex = 1;
}

/**
  * @brief  默认菜单显示函数
  * @note   显示当前菜单层级，包含以下元素：
  *         - 顶部状态栏显示当前菜单名称（右对齐）
  *         - 列表式显示子菜单项（每项占16像素高度）
  *         - 高亮显示选中项（反色矩形框）
  * @note   标题显示规则：
  *         - 存在父菜单时显示当前菜单名称（右对齐）
  *         - 根菜单显示"main"标识
  */
void Menu_DefaultDisplay(void) 
{
	//构建菜单
	POINT_COLOR=BLACK;
	LCD_ShowString(0,32 ,12,"|<--------------------------------->",1);
	for(uint8_t i=0; i<15; i++)LCD_ShowString(218,i*16,16,"|",1);
	LCD_ShowString(0,160,12,"|<--------------------------------->",1);
	
	
    // 显示菜单标题
//		LCD_ShowChinese(0,0,WHITE,BLACK,currentMenu->name, 32 , 0) ;
			POINT_COLOR=RED,LCD_ShowString(0, 0, 32, "Main", 1);

    // 显示菜单项
    MenuItem *item = currentMenu->child;
    for(uint8_t i=1; i<=MAX_ITEMS && item; i++) 
		{
			// 高亮显示选中项LCD_ShowChinese(0,i*40-4,WHITE,BLACK,item->name, 32 , 1) ;
        if(i == selectedIndex)  POINT_COLOR=RED,LCD_ShowString(0,i*27+21,24,item->name,1);
			else				POINT_COLOR=BLACK,LCD_ShowString(0,i*27+21,24,item->name,1);
        item = item->next;
		}
		
	POINT_COLOR=DARKBLUE;
    switch(Program.Mode) {
        case Mode_Kong:				LCD_ShowString(230,0 ,32,"Kong",1);break;
        case Mode_PV_Grid:			LCD_ShowString(230,0 ,32,"Gd_G",1);break;
        case Mode_PV_Grid_Storage:	LCD_ShowString(230,0 ,32,"Gd_S",1);break;
        case Mode_PV_DC_Storage:	LCD_ShowString(230,0 ,32,"DC_S",1);break;
        case Mode_PV_DC_V:			LCD_ShowString(230,0 ,32,"DC_V",1);break;
        case Mode_PV_DC_I:			LCD_ShowString(230,0 ,32,"DC_I",1);break;
        case Mode_PV_AC_Storage:	LCD_ShowString(230,0 ,32,"AC_S",1);break;
        case Mode_PV_AC_V:			LCD_ShowString(230,0 ,32,"AC_V",1);break;
        case Mode_PV_AC_I:			LCD_ShowString(230,0 ,32,"AC_I",1);break;
    }
}

/**
  * @brief  菜单刷新控制函数
  * @param  refresh_mode 刷新模式
  *         @arg CONTINOUS_REFRESH 连续刷新模式（立即执行显示）
  *         @arg SINGLE_REFRESH   单次刷新模式（需配合refresh_request标志）
  * @note   单次刷新模式下需要设置single_refresh_request=1触发刷新
  *         刷新完成后自动清除单次刷新请求标志
  */
void Menu_Refresh(void) 
{
	  switch(currentMenu->refresh_mode) 
		{
    case CONTINOUS_REFRESH:  // 连续刷新
				if(currentMenu && currentMenu->display) 
				{
						currentMenu->display();
				}
				break;
    case SINGLE_REFRESH:  	// 单次刷新
				if(currentMenu && currentMenu->display && single_refresh_request) 
				{
						currentMenu->display();
						single_refresh_request=0;
				}
      break;
		default:
				break;
    }
}

/**
  * @brief  默认菜单按键处理函数
  * @param  key 按键输入值
  *         @arg KEY_UP    上移选择项
  *         @arg KEY_DOWN  下移选择项
  *         @arg KEY_ENTER 进入子菜单
  *         @arg KEY_BACK  返回上级菜单
  * @note   功能特性：
  *         - 自动计算当前菜单有效子项数量（最大支持MAX_ITEMS项）
  *         - 支持菜单层级无限回溯（通过parent指针）
  *         - 选中项索引自动复位保护（防止越界）
  */
static void Menu_DefaultKeyHandler(uint16_t key) 
{
    uint8_t itemCount = 1;
    MenuItem *item = currentMenu->child;
    
    // 计算子项数量
    while(item && itemCount < MAX_ITEMS) {
        displayItems[itemCount++] = item;
        item = item->next;
    }
		
    switch(key) {
    case KEY_UP:{
        if			(selectedIndex >  1)	selectedIndex--;
		else if		(selectedIndex == 1)	selectedIndex=itemCount-1;
        break;}
    case KEY_DOWN:{
        if			(selectedIndex < itemCount-1)	selectedIndex++;
		else if		(selectedIndex == itemCount-1) 		selectedIndex=1;
        break;}
    case KEY_ENTER:
        if(displayItems[selectedIndex]->child||displayItems[selectedIndex]->display!=Menu_DefaultDisplay) 
			{
				currentMenu = displayItems[selectedIndex];
				selectedIndex = 1;
			}
				LCD_Clear(WHITE);
        break;
    default:
        break;
    }
}

/**
  * @brief  菜单按键事件处理入口
  * @param  key 按键输入值
  * @note   核心处理流程：
  *         1. 设置单次刷新请求标志
  *         2. 调用当前菜单绑定的按键处理函数
  *         3. 通过Menu_Refresh()执行实际显示更新
  * @warning 需确保在按键扫描后调用本函数
  */
void Menu_KeyProcess(uint16_t key) 
{
    if(currentMenu && currentMenu->key_handler) 
		{
				single_refresh_request=1;//单次刷新重新赋值为1
        currentMenu->key_handler(key);
    }
}

