/* SPDX-License-Identifier: LGPL-2.1-only */
/*
 * pcaneventread.cpp - PCANBasic Example: Event Read
 *
 * Copyright (C) 2001-2020  PEAK System-Technik GmbH <www.peak-system.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 *
 * Contact:    <linux@peak-system.com>
 * Maintainer: Stephane Grosjean <s.grosjean@peak-system.com>
 * Author:     Thomas Haber <thomas@toem.de>
 */
#include <ncurses.h>
#include <string>
#include <vector>
#include <unistd.h>
#include <algorithm>
#include <ctime>
#include <stdexcept>

#include "comm_pcan.hpp"
#include "menu_display.hpp"

void updateEPSData(CAN_RX_INFO& data_info,double fAngle,uint8_t temp, bool bIsCtrlAngle, bool bIsVibMode);
void showAbout() ;
void setMenu_ctrl_steer_angle(CAN_CMD_INFO& cmd_info);
void setMenu_ctrl_free_torque(CAN_CMD_INFO& cmd_info);
void setMenu_ctrl_motor_torque(CAN_CMD_INFO& cmd_info);
void setMenu_ctrl_vibration(CAN_CMD_INFO& cmd_info);
void update_another_data(CAN_CMD_INFO& cmd_info);

enum class MENU_TYPE : uint8_t {CanDeviceOperate = 0, SteerCtrlMenu, FeedbackFreeTqCtrl, MotorTqCtrl, Vibrated_on, Input_anotherMenu, About, Quit};

template<typename E>
constexpr auto CASE_CAST(E e) -> typename std::underlying_type<E>::type 
{
	return static_cast<typename std::underlying_type<E>::type>(e);
}

template<typename E>
constexpr auto VALUE_CAST(E e) -> typename std::underlying_type<E>::type 
{
	return static_cast<typename std::underlying_type<E>::type>(e);
}

std::vector<std::string> menu_items = {
	"1. Open can device ", 
	"2. Set angle value for steering control", 
	"3. Set Feedback-free torque value for steering control", 
	"4. Set Motor torque value for steering control", 
	"5. Steer Vibration mode on", 
	"6. Set another Value (vehicle values, etc.)", 
	"7. About Program",
	"Quit"
};
std::vector<std::string> streer_ctrl_mode_items = {
	"Off",
	"Ready",
	"Feedback-free Torque Control Mode",
	"Motor Torque Control Mode",
	"Steer Angle Control Mode",
	"Cancel",
	"Invalid"
};
std::vector<std::string> streer_sys_state_items = {
	"Undefined",
	"NMWait",
	"Terminal30Watt",
	"PreDrive",
	"DriveDown",
	"DriveUp",
	"PostRun",
	"Off",
	"Error",
	"Flash",
	"LowVoltage",
	"Invalid"
};
std::vector<std::string> steer_vibration_mode_items = {
	"Vibration Off",
	"vibration On",
	"Invalid"
};

PCANManager pcan_manager(PCAN_USBBUS1, PCAN_BAUD);
bool IsWorkingDevice = false;
double fAngle;

int main(void)//int argc, char* argv[]) 
{
	setlocale(LC_ALL, "");
	initscr();
	cbreak();
	noecho();
	keypad(stdscr, TRUE);
	curs_set(0); // 커서 숨기기

	std::string str_title = "Servounit Test Program";
	CAN_CMD_INFO cmd_info{};

	bool bRun = true;

	cmd_info.m_nCmd_str_ctrl_mode = VALUE_CAST(EPS_Mode::Off)	;
	cmd_info.m_nCmd_str_vibrate_enable = VALUE_CAST(VIBRATE_MODE::vibrate_disable);  
	cmd_info.m_nCmd_str_active_ret_weight_req = 50;   	//%
	cmd_info.m_nCmd_str_angle_ctrl_weight_req = 50;   	//%
	cmd_info.m_nCmd_str_angle_ctrl_tq_limit_req = 7; 	//Nm
	cmd_info.m_FCmd_str_angle = 0.0f;					//deg
	cmd_info.m_fCmd_str_mt_tq = 0.0f;					//Nm
	cmd_info.m_fCmd_str_fbf_tq = 0.0f;					//Nm
	cmd_info.m_nCmd_str_vibrate_level = 5; 				// 0 ~ 9(max)
	cmd_info.m_fCmd_in_vehicle_speed = 0.0f;			//kph
	cmd_info.m_fCmd_engine_speed = 320.0f;				//rpm
	
	while (bRun)
	{
		Servounit_Operating_Prog main_menu(str_title, menu_items);
		int selection = main_menu.run();

		usleep(10000);

		switch(selection) 
		{
			case -1:
			case CASE_CAST(MENU_TYPE::Quit): // 종료
				clear();
				mvprintw(2, 10, "can device's been terminated safely!!!");
				mvprintw( 3, 10, "Closed bosch servounit test program...");
				refresh();

				if( IsWorkingDevice == true )
				{
					pcan_manager.device_stop_();
					sleep(1);
					pcan_manager.device_close_();
					sleep(1);
				}
			    bRun = false;  
				break;

			case CASE_CAST(MENU_TYPE::CanDeviceOperate):  // CAN 디바이스 열기
			    clear();
				mvprintw( 2, 10, "Opening CAN device...");

				if(0 > pcan_manager.device_open_())
				{
					mvprintw( 4, 10, "Failed to open CAN device.");
					IsWorkingDevice = false;
				}
				else  //can device opened successfully
				{
					IsWorkingDevice = true;
					mvprintw( 4, 10, "CAN device opened successfully.");
					pcan_manager.device_start_();
					mvprintw( 5, 10, "CAN device started successfully.");

					pcan_manager.setCmdData_eps(cmd_info);

				}
				refresh();
				usleep(2000000);

				break;
				case CASE_CAST(MENU_TYPE::Input_anotherMenu):  update_another_data(cmd_info);
					break;
			
			case CASE_CAST(MENU_TYPE::Vibrated_on): setMenu_ctrl_vibration(cmd_info);	break; 			// 조향 진동 모드 설정
			case CASE_CAST(MENU_TYPE::FeedbackFreeTqCtrl): setMenu_ctrl_free_torque(cmd_info);break; 	// Feedback-free torque control 설정
			case CASE_CAST(MENU_TYPE::MotorTqCtrl): setMenu_ctrl_motor_torque(cmd_info); break; 		// Motor torque control 설정
			case CASE_CAST(MENU_TYPE::SteerCtrlMenu): setMenu_ctrl_steer_angle(cmd_info); break;   	// streeing Value 설정
			default: showAbout();	break;
		}
	}

	clear();
	mvprintw(3,10,"Thank you for using this service");
	refresh();

	sleep(2);

    // NCurses 종료
    endwin();

	return 0;
}

void updateEPSData(CAN_RX_INFO& data_info, double fAngle,uint8_t temp, bool bIsCtrlAngle, bool bIsVibrated)
{
	// EPS 데이터 업데이트 로직 구현
	// 예: CAN 메시지 수신 및 데이터 처리

	int lines = 0;
	int select_temp = 0;

	// 현재 날짜와 시간 가져오기
	time_t now = time(nullptr);
	struct tm* local_time = localtime(&now);
	char time_str[64];
	strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", local_time);

    clear();

	mvprintw(lines++, 3, "########## EPS Data Information ##########");

	if( VALUE_CAST(EPS_Mode::Off) > data_info.m_nData_str_ctrl_mode || data_info.m_nData_str_ctrl_mode > VALUE_CAST(EPS_Mode::Cancel) ) select_temp = VALUE_CAST(EPS_Mode::Error_eps); // Invalid
	else select_temp = data_info.m_nData_str_ctrl_mode;
	mvprintw(lines++, 3, "Steer control mode: %s", streer_ctrl_mode_items[select_temp].c_str());

	if( VALUE_CAST(EPS_Sys_State::Undefined) > data_info.m_nData_str_sys_state || data_info.m_nData_str_sys_state > VALUE_CAST(EPS_Sys_State::LowVoltage) ) select_temp = VALUE_CAST(EPS_Sys_State::Undefined); // Invalid
	else select_temp = data_info.m_nData_str_sys_state;
	mvprintw(lines++, 3, "Steer System State: %s", streer_sys_state_items[select_temp].c_str());

	
	if( data_info.m_nData_str_vibrate_enable > VALUE_CAST(VIBRATE_MODE::vibrate_enable) ) select_temp = VALUE_CAST(VIBRATE_MODE::vibrate_disable); // Invalid
	else select_temp = data_info.m_nData_str_vibrate_enable;
	mvprintw(lines++, 3, "Vibration Mode: %s", steer_vibration_mode_items[select_temp].c_str());

    mvprintw(lines++, 3, "Actual Inner Wheel angle: %.2f deg", data_info.m_fActual_inner_wheel_angle);
    mvprintw(lines++, 3, "Steer angle: %.2f deg", data_info.m_fData_str_angle);
    mvprintw(lines++, 3, "Steer angle speed: %.2f deg/s", data_info.m_fData_str_angle_speed);
	mvprintw(lines++, 3, "Driver torque: %.2f Nm", data_info.m_fData_str_Driver_Tq);
	mvprintw(lines++, 3, "Motor torque: %.2f Nm", data_info.m_fData_str_Motor_Tq);
	lines++;
	mvprintw(lines++, 3, "#################################################");
	lines++;
	mvprintw(lines++, 3, bIsCtrlAngle ? "Input desired steer angle (-850.0 ~ 850.0): %.1lf":"Input desired torque (-2.0 ~ 2.0): %.1lf", fAngle);
	mvprintw(lines++, 3, bIsVibrated ? "Press 'Arrow Up/Down' to in/decrease vibrated level: %d":"Press 'Arrow Up/Down' to in/decrease Vehicle speed: %dkph", temp);
	mvprintw(lines++, 3, "Press 'q' to return to main menu");
	lines++;
	mvprintw(lines++, 3, "Current Time: %s", time_str);

	refresh();
}

void update_another_data(CAN_CMD_INFO& cmd_info)
{
	// 다른 데이터 업데이트 로직 구현
	// 예: CAN 메시지 수신 및 데이터 처리
	clear();
	
	refresh();
	usleep(200000);
}

void setMenu_ctrl_steer_angle(CAN_CMD_INFO& cmd_info)
{
	clear();

	if( IsWorkingDevice == false ) 
	{
		mvprintw( 2, 10, "CAN device is not working.");
		refresh();
		usleep(2000000);

		return;
	}
	

	// 조향 제어 각도 설정 메뉴 로직 구현
	// 예: 사용자 입력을 받아 조향 각도 설정
	nodelay(stdscr, TRUE); // Non-blocking input
	bool bSubMenuRun = true;
	std::string strbuff;
	int ch;
	int nVehSpeed = 0;

	fAngle = 0.0f;

	while(bSubMenuRun)
	{
		CAN_RX_INFO data_info = pcan_manager.getRxData_eps();
		updateEPSData(data_info,fAngle, nVehSpeed,true, false);

		ch = getch();

		if(ch == 'q' || ch == 'Q') 
		{
			bSubMenuRun = false;
			cmd_info.m_FCmd_str_angle = 0.0f;
			cmd_info.m_nCmd_str_ctrl_mode = CASE_CAST(EPS_Mode::Off);
		}			
		else if( ch == KEY_UP)
		{
			nVehSpeed += 1;
			cmd_info.m_fCmd_in_vehicle_speed = static_cast<double>(nVehSpeed);
		}
		else if (ch == KEY_DOWN)
		{
			nVehSpeed -= 1;
			if(nVehSpeed < 0) nVehSpeed = 0;     //kph
			cmd_info.m_fCmd_in_vehicle_speed = static_cast<double>(nVehSpeed);
		}	

		switch(data_info.m_nData_str_ctrl_mode)
		{
			case CASE_CAST(EPS_Mode::Off):
				cmd_info.m_nCmd_str_ctrl_mode = CASE_CAST(EPS_Mode::Ready);
				break;
			case CASE_CAST(EPS_Mode::Ready):
				cmd_info.m_nCmd_str_ctrl_mode = CASE_CAST(EPS_Mode::Mode_Str_angle_ctrl);
				break;
			case CASE_CAST(EPS_Mode::Mode_Str_angle_ctrl):
				
				if(ch == '-')
				{
					if( true == strbuff.empty()) strbuff.push_back(ch);
				}
				else if( ch >= '0' && ch <= '9')
				{
					strbuff.push_back(ch);
				}
				else if( ch == '.' )
				{
					if( false == strbuff.empty() && strbuff.find('.') == std::string::npos)
						strbuff.push_back(ch);
				}
				else if(ch == KEY_BACKSPACE || ch == 127)
				{
					if (false == strbuff.empty()) strbuff.pop_back();
				}
				else if( ch == '\n' || ch == KEY_ENTER)
				{
					if( false == strbuff.empty())
					{
						try {
							double val = std::stod(strbuff);
							if( val < -850.0) val = -850.0;
							else if( val > 850.0) val = 850.0;
							fAngle = val;
							cmd_info.m_FCmd_str_angle = fAngle;
						} catch (const std::exception&) {
							// 잘못된 입력 무시
						}
						strbuff.clear();
					}
				}
			
				break;
			default:
				cmd_info.m_nCmd_str_ctrl_mode = CASE_CAST(EPS_Mode::Off);
				break;
		}
		pcan_manager.setCmdData_eps(cmd_info);
		usleep(100000); // CPU 사용량 감소를 위해 잠시 대기
	}
	nodelay(stdscr, FALSE); // Blocking input
	refresh();
}

void setMenu_ctrl_free_torque(CAN_CMD_INFO& cmd_info)
{
	clear();

	if( IsWorkingDevice == false ) 
	{
		mvprintw( 2, 10, "CAN device is not working.");
		refresh();
		usleep(2000000);
		return;
	}
	

	// 조향 제어 각도 설정 메뉴 로직 구현
	// 예: 사용자 입력을 받아 조향 각도 설정
	nodelay(stdscr, TRUE); // Non-blocking input
	bool bSubMenuRun = true;
	std::string strbuff;
	int ch;
	int nVehSpeed = 0;

	fAngle = 0.0f;

	while(bSubMenuRun)
	{
		CAN_RX_INFO data_info = pcan_manager.getRxData_eps();
		updateEPSData(data_info,fAngle, nVehSpeed, false, false);

		ch = getch();

		if(ch == 'q' || ch == 'Q') 
		{
			bSubMenuRun = false;
			cmd_info.m_FCmd_str_angle = 0.0f;
			cmd_info.m_nCmd_str_ctrl_mode = CASE_CAST(EPS_Mode::Off);
		}
		else if( ch == KEY_UP)
		{
			nVehSpeed += 1;
			cmd_info.m_fCmd_in_vehicle_speed = static_cast<double>(nVehSpeed);
		}
		else if (ch == KEY_DOWN)
		{
			nVehSpeed -= 1;
			if(nVehSpeed < 0) nVehSpeed = 0;     //kph
			cmd_info.m_fCmd_in_vehicle_speed = static_cast<double>(nVehSpeed);
		}	


		switch(data_info.m_nData_str_ctrl_mode)
		{
			case CASE_CAST(EPS_Mode::Off):
				cmd_info.m_nCmd_str_ctrl_mode = CASE_CAST(EPS_Mode::Ready);
				break;
			case CASE_CAST(EPS_Mode::Ready):
				cmd_info.m_nCmd_str_ctrl_mode = CASE_CAST(EPS_Mode::Mode_Free_tq_ctrl);
				break;
			case CASE_CAST(EPS_Mode::Mode_Free_tq_ctrl):
				
				if(ch == '-')
				{
					if( true == strbuff.empty()) strbuff.push_back(ch);
				}
				else if( ch >= '0' && ch <= '9')
				{
					strbuff.push_back(ch);
				}
				else if( ch == '.' )
				{
					if( false == strbuff.empty() && strbuff.find('.') == std::string::npos)
						strbuff.push_back(ch);
				}
				else if(ch == KEY_BACKSPACE || ch == 127)
				{
					if (false == strbuff.empty()) strbuff.pop_back();
				}
				else if( ch == '\n' || ch == KEY_ENTER)
				{
					if( false == strbuff.empty())
					{
						try {
							double val = std::stod(strbuff);
							if( val < -2.0) val = -2.0;
							else if( val > 2.0) val = 2.0;
							fAngle = val;
							cmd_info.m_fCmd_str_fbf_tq = fAngle;
						} catch (const std::exception&) {
							// 잘못된 입력 무시
						}
						strbuff.clear();
					}
				}
				break;
			default:
				cmd_info.m_nCmd_str_ctrl_mode = CASE_CAST(EPS_Mode::Off);
				break;
		}
		pcan_manager.setCmdData_eps(cmd_info);
		usleep(100000); // CPU 사용량 감소를 위해 잠시 대기
	}
	nodelay(stdscr, FALSE); // Blocking input
	refresh();
}

void setMenu_ctrl_motor_torque(CAN_CMD_INFO& cmd_info)
{
	clear();

	if( IsWorkingDevice == false ) 
	{
		mvprintw( 2, 10, "CAN device is not working.");
		refresh();
		usleep(2000000);
		return;
	}
	

	// 조향 제어 각도 설정 메뉴 로직 구현
	// 예: 사용자 입력을 받아 조향 각도 설정
	nodelay(stdscr, TRUE); // Non-blocking input
	bool bSubMenuRun = true;
	std::string strbuff;
	int ch;
	int nVehSpeed = 0;
	fAngle = 0.0f;

	while(bSubMenuRun)
	{
		CAN_RX_INFO data_info = pcan_manager.getRxData_eps();
		updateEPSData(data_info,fAngle, nVehSpeed, false, false);

		ch = getch();

		if(ch == 'q' || ch == 'Q') 
		{
			bSubMenuRun = false;
			cmd_info.m_FCmd_str_angle = 0.0f;
			cmd_info.m_nCmd_str_ctrl_mode = CASE_CAST(EPS_Mode::Off);
		}
		else if( ch == KEY_UP)
		{
			nVehSpeed += 1;
			cmd_info.m_fCmd_in_vehicle_speed = static_cast<double>(nVehSpeed);
		}
		else if (ch == KEY_DOWN)
		{
			nVehSpeed -= 1;
			if(nVehSpeed < 0) nVehSpeed = 0;     //kph
			cmd_info.m_fCmd_in_vehicle_speed = static_cast<double>(nVehSpeed);
		}	

		switch(data_info.m_nData_str_ctrl_mode)
		{
			case CASE_CAST(EPS_Mode::Off):
				cmd_info.m_nCmd_str_ctrl_mode = CASE_CAST(EPS_Mode::Ready);
				break;
			case CASE_CAST(EPS_Mode::Ready):
				cmd_info.m_nCmd_str_ctrl_mode = CASE_CAST(EPS_Mode::Mode_Tq_ctrl);
				break;
			case CASE_CAST(EPS_Mode::Mode_Tq_ctrl):
				if(ch == '-')
				{
					if( true == strbuff.empty()) strbuff.push_back(ch);
				}
				else if( ch >= '0' && ch <= '9')
				{
					strbuff.push_back(ch);
				}
				else if( ch == '.' )
				{
					if( false == strbuff.empty() && strbuff.find('.') == std::string::npos)
						strbuff.push_back(ch);
				}
				else if(ch == KEY_BACKSPACE || ch == 127)
				{
					if (false == strbuff.empty()) strbuff.pop_back();
				}
				else if( ch == '\n' || ch == KEY_ENTER)
				{
					if( false == strbuff.empty())
					{
						try {
							double val = std::stod(strbuff);
							if( val < -2.0) val = -2.0;
							else if( val > 2.0) val = 2.0;
							fAngle = val;
							cmd_info.m_fCmd_str_mt_tq = fAngle;
						} catch (const std::exception&) {
							// 잘못된 입력 무시
						}
						strbuff.clear();
					}
				}
				break;
			default:
				cmd_info.m_nCmd_str_ctrl_mode = CASE_CAST(EPS_Mode::Off);
				break;
		}
		pcan_manager.setCmdData_eps(cmd_info);
		usleep(100000); // CPU 사용량 감소를 위해 잠시 대기
	}
	nodelay(stdscr, FALSE); // Blocking input
	refresh();

}

void setMenu_ctrl_vibration(CAN_CMD_INFO& cmd_info)
{
	clear();

	if( IsWorkingDevice == false ) 
	{
		mvprintw( 2, 10, "CAN device is not working.");
		refresh();
		usleep(2000000);
		return;
	}
	

	// 조향 제어 각도 설정 메뉴 로직 구현
	// 예: 사용자 입력을 받아 조향 각도 설정
	nodelay(stdscr, TRUE); // Non-blocking input
	bool bSubMenuRun = true;
	std::string strbuff;
	int ch;
	int vib_level = 5;

	cmd_info.m_nCmd_str_vibrate_enable = VALUE_CAST(VIBRATE_MODE::vibrate_enable);

	while(bSubMenuRun)
	{
		CAN_RX_INFO data_info = pcan_manager.getRxData_eps();
		updateEPSData(data_info,fAngle, vib_level , false, true);

		ch = getch();
		
		if(ch == 'q' || ch == 'Q')
		{
			bSubMenuRun = false;
			cmd_info.m_nCmd_str_vibrate_level = vib_level = 5;
			cmd_info.m_nCmd_str_vibrate_enable = VALUE_CAST(VIBRATE_MODE::vibrate_disable);
			cmd_info.m_nCmd_str_ctrl_mode = CASE_CAST(EPS_Mode::Off);
		}
		else if(ch == KEY_UP )
		{
			 vib_level = (vib_level + 1) % 10 ;
			cmd_info.m_nCmd_str_vibrate_level = vib_level;
		}
		else if(ch == KEY_DOWN)
		{
			if ( vib_level > 0) vib_level--;
			else
				vib_level = 9;

			cmd_info.m_nCmd_str_vibrate_level = vib_level;

		}

		pcan_manager.setCmdData_eps(cmd_info);
		usleep(100000); // CPU 사용량 감소를 위해 잠시 대기
	}
	nodelay(stdscr, FALSE); // Blocking input
	refresh();

}

void showAbout() 
{
    clear();
    box(stdscr, 0, 0);
    
    mvprintw(2, 4, "Program Information");
    mvprintw(3, 4, "========================================");
    mvprintw(5, 4, "Name: Servounit Test Program ");
    mvprintw(6, 4, "Version: 1.0");
    mvprintw(7, 4, "Description: Interactive command values using console line with servounit on Linux");
    mvprintw(9, 4, "Press any key to return...");
    
    refresh();
    getch();
}
