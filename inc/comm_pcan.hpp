#ifndef COMM_PCAN_HPP_
#define COMM_PCAN_HPP_

#include <iostream>
#include <mutex>
#include <thread>
#include <map>
#include <atomic>
#include <vector>
#include <unistd.h>
#include <cstring>
#include <string>
#include <chrono>

#include "PCANBasic.h"

// PCAN-Basic device used to read on
constexpr uint16_t PCAN_DEVICE  =  PCAN_USBBUS1;
constexpr uint16_t PCAN_BAUD    =  PCAN_BAUD_500K;	
constexpr uint32_t MAX_QUEUE_SIZE   = 10;

constexpr DWORD EBC2 		= 0x18FEBFFE; 		//EBC2  period: 20m
constexpr DWORD TCO1 		= 0xCFE6CFE; 		//TCO1    20ms
constexpr DWORD HRW 		= 0x8FE6EFE; 		//HRW     20ms
constexpr DWORD CCVS1 		= 0x18FEF100; 		//CCVS1	  20ms
constexpr DWORD EEC1 		= 0xCF004FE; 		//EEC1    10ms
constexpr DWORD VDC2 		= 0x18F009FE; 		//VDC2    10ms
//constexpr DWORD nID_SAS1 		= 0x18F01DEF; 		//EPS1
constexpr DWORD EPSI1 		= 0x18FF00FE; 		//EPS2   10ms
constexpr DWORD EPSI2 		= 0x18FF01FE; 		//EPS3    10ms
constexpr DWORD EPSI3 		= 0x18FF02FE; 		//EPS4    10ms
constexpr DWORD TD	 		= 0x18FEE6FE; 		//EPS5   1000
constexpr DWORD VDHR 		= 0x18FEC1FE; 		//EPS6    1000

constexpr DWORD EPSO1 		= 0x18FF0513; 		//EPSO1
constexpr DWORD EPSO2 		= 0x18FF0613; 		//EPSO2
constexpr DWORD ESC1 		= 0x18F00B13; 		//ESC1


////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>	Main entry-point for this application. </summary>
///
/// <remarks>	 </remarks>
///
/// <param name="argc">	The argc. </param>
/// <param name="argv">	[in,out] If non-null, the argv. </param>
///
/// <returns>	. </returns>
////////////////////////////////////////////////////////////////////////////////////////////////////

enum {thread_rx = 0, thread_tx = 1};
enum class VIBRATE_MODE : uint8_t { vibrate_disable = 0, vibrate_enable = 1 };
enum class EPS_Mode: uint8_t { Off = 0, Ready = 1, Mode_Free_tq_ctrl, Mode_Tq_ctrl, Mode_Str_angle_ctrl, Cancel, Error_eps};
enum class EPS_Sys_State: uint8_t {Undefined = 0, NMWait = 1, Terminal30Watt, PreDrive, DriveDown, DriveUp, PostRun, stat_Off, Error_sys, Flash, LowVoltage };

typedef struct _tag_rx_vcu_info 
{
	uint8_t		m_nData_str_sys_state;			// 0: undefined, 1: NMWait, 2: Terminal30Watt, 3: PreDrive, 4: DriveDown, 5: DriveUp, 6: PostRun, 7: Off, 8: Error, 9: flash, 10: LowVoltage
    uint8_t    	m_nData_str_ctrl_mode;          // 0:off, 1:ready, 2:feed_tq_mode, 3:tq_mode, 4: steer_angle_mode, 5: cancle 
	uint8_t     m_nData_str_vibrate_enable; 	// 0: disable, 1: enable
	uint8_t		m_nData_str_hands_off_value;	// 0 ~ 100%
	uint8_t     m_nData_reserved[4];			// reserved
	double	   	m_fActual_inner_wheel_angle;  		//  0 ~ 300.0 kph
	double	   	m_fData_str_angle;       		//  -1,200 ~ 1,200 deg
	double	   	m_fData_str_angle_speed;   		//  -1,200 ~ 1,200 deg/s
	double      m_fData_str_Driver_Tq;       	//Nm
	double      m_fData_str_Motor_Tq;        	//Nm
}CAN_RX_INFO;

typedef struct _tag_tx_upper_info
{
    uint8_t    	m_nCmd_str_ctrl_mode;           	// 0:off, 1:ready, 2:feed_tq_mode, 3:tq_mode, 4: steer_angle_mode, 5: cancle 
	uint8_t     m_nCmd_str_vibrate_enable; 			// 0: disable, 1: enable
	uint8_t		m_nCmd_str_active_ret_weight_req;	// 0 ~ 100%  
	uint8_t		m_nCmd_str_angle_ctrl_weight_req;	// 0 ~ 100%  
	uint8_t		m_nCmd_str_angle_ctrl_tq_limit_req;	// 0 ~ 10Nm  
	uint8_t     m_nCmd_str_vibrate_level;  			// 0 ~ 9: max vibration level
	uint8_t		m_reserved[2];						//reserved
	double	   	m_FCmd_str_angle;       			//  -1,200 ~ 1,200 deg
	double		m_fCmd_str_mt_tq;					// -32.374 ~ 32.374 Nm
	double		m_fCmd_str_fbf_tq;					// -32.374 ~ 32.374 Nm
	double 		m_fCmd_in_vehicle_speed;          	//kph
	double      m_fCmd_engine_speed;				//rpm
	double		m_fCmd_relatve_veh_axle_speed;		//kpk
	double 		m_fCmd_yawrate;
	double		m_fCmd_accelX;
	double		m_fCmd_accelY;
}CAN_CMD_INFO;

class PCANManager
{
public:
	PCANManager(uint16_t can_device = 0, uint16_t baudrate = PCAN_BAUD_500K	)
		: pcan_device_(can_device), pcan_baud_(baudrate), bRunning_(false)
		{
			pCmd_info.reserve(MAX_QUEUE_SIZE);
			pRx_info.reserve(MAX_QUEUE_SIZE);
		}


	~PCANManager() 
	{
		device_stop_();
		device_close_();
	}


public: //methods
	int32_t device_open_()
	{
		TPCANStatus Status ;
		int32_t ret = 0;

		if ( (Status = CAN_Initialize(pcan_device_, pcan_baud_)) != PCAN_ERROR_OK )
		{
			std::cerr << "No pcan-device connected!! Please, check pcan-device\n\n";
			std::cerr << "CAN_Initialize Error: 0x" << std::hex << (int)Status << std::endl;
			ret = -1;
		}
		else
		{
			std::cout << "CAN_Initialize(" << std::hex << pcan_device_ << "h): Status=0x" << std::hex << (int)Status << std::endl;
		}
		return ret;
	}

	void device_close_() 
	{
		if (pcan_device_ != 0) 
		{
			CAN_Uninitialize(pcan_device_);
			pcan_device_ = 0;  // 이중 호출 방지
		}
	}

	void device_start_()
	{
		bRunning_ = true;

		pThread_[thread_rx] = std::thread(&PCANManager::can_rx_thread, this);
		pThread_[thread_tx] = std::thread(&PCANManager::can_tx_thread, this);	
	}

	void device_stop_()
	{
		bRunning_ = false;

		if( pThread_[thread_rx].joinable() ) pThread_[thread_rx].join();
		if( pThread_[thread_tx].joinable() ) pThread_[thread_tx].join();
	}

	void device_set_rx_filter()
	{
		// if(PCAN_ERROR_OK == (Status = CAN_SetValue(pcan_device,PCAN_MESSAGE_FILTER,&nFilterCode_Mask_,sizeof(nFilterCode_Mask_))))
		// {
		// 	if(PCAN_ERROR_OK == CAN_FilterMessages(pcan_device,0x1A8400,0x1A8420,PCAN_MESSAGE_EXTENDED))
		// 		printf("Ther filter was configured to accept the 3 ext. IDs : 0x2B0 0x1A4800 0x1A4820 \n");
		// 	/*						How to write the acceptance filter code & filter mask								*/
		// 	/*						Code value																			*/
		// 	/*	 2B0h:	0b	0000	0000	0000	0000	0000	0010	1011	0000								*/
		// 	/*1A4800h:	0b	0000	0000	0001	1010	0100	1000	0000	0000								*/
		// 	/*1A4820h:	0b	0000	0000	0001	1010	0100	1010	0010	0000								*/
		// 	/*--------------------------------------------------------------------------------------------  AND			*/
		// 	/*000000h:	0b	0000	0000	0000	0000	0000	0000	0000	0000				 result			*/
		// 	/*	  Mask value															*/
		// 	/*	 2B0h:	0b	0000	0000	0000	0000	0000	0010	1011	0000								*/
		// 	/*1A4800h:	0b	0000	0000	0001	1010	0100	1000	0000	0000								*/
		// 	/*1A4820h:	0b	0000	0000	0001	1010	0100	1010	0010	0000								*/
		// 	/*--------------------------------------------------------------------------------------------  XOR			*/
		// 	/*000090h:	0b	0000	0000	0000	0000	0000	0000	1001	0000				 result			*/

		// }	
		return;		
	}

	//최신 EPS 정보 메시지 외부로 전달하는 함수
	CAN_RX_INFO& getRxData_eps()
	{
		static CAN_RX_INFO dummy{};
		{
			std::lock_guard<std::mutex> lock(pMux_lock_[thread_rx]);
			if (false == pRx_info.empty()) 
			{
				dummy = pRx_info.back();
				pRx_info.clear();
			}
		}
		return dummy;
	}

	//최신 EPS 제어 메시지 외부에서 설정, 전달하는 함수
	void setCmdData_eps(const CAN_CMD_INFO& cmd_info)
	{
		{
			std::lock_guard<std::mutex> lock(pMux_lock_[thread_tx]);
			if(pCmd_info.size() >= MAX_QUEUE_SIZE) pCmd_info.erase(pCmd_info.begin());
			pCmd_info.push_back(cmd_info);
		}		
	}

private: //methods
	void can_rx_thread()
	{
		TPCANMsg rx_msg{};
		TPCANStatus status;
		TPCANTimestamp timestamp{};
		auto last = std::chrono::steady_clock::now();


		while(bRunning_)
		{
			while (PCAN_ERROR_QRCVEMPTY != (status = CAN_Read(pcan_device_, &rx_msg, &timestamp)))
			{
				can_rx_message_handler(rx_msg);
			}

			auto now = std::chrono::steady_clock::now();
			if (std::chrono::duration_cast<std::chrono::milliseconds>(now - last) >= std::chrono::milliseconds(10))
			{
				// 10ms마다 EPS 데이터 업데이트
				{
					std::lock_guard<std::mutex> lock(pMux_lock_[thread_rx]);
					if(pRx_info.size() >= MAX_QUEUE_SIZE )pRx_info.erase(pRx_info.begin());
					pRx_info.push_back(rx_dummy_);
				}
				last = now;
			}

			std::this_thread::sleep_for(std::chrono::microseconds(1000));
		}		
	}

	void can_tx_thread()
	{
		uint8_t 	nCnt = 0;
		CAN_CMD_INFO dummy{};
		uint16_t	dummy_data_u16;
		uint8_t		dummy_data_u8;
		uint32_t	elapsed = 0;  

		while(bRunning_)
		{
			// 송신 스레드 시작
			{
				std::lock_guard<std::mutex> lock(pMux_lock_[thread_tx]);

				if (false == pCmd_info.empty())
				{
					dummy = pCmd_info.back();
					pCmd_info.clear();
				}
			}
			//m_tx_msg_18F01DEFh.DATA[7] = nCnt & 0x0f;

			if ( 1) //elapsed % 2 == 1 )  //20ms
			{
				dummy_data_u16 = static_cast<uint16_t>((dummy.m_fCmd_in_vehicle_speed) * 256.0f); // 예시로 차량 속도를 0.1kph 단위로 변환하여 전송
				m_tx_msg_18FEBFFEh.DATA[0] = static_cast<uint8_t>(dummy_data_u16 & 0xFF); // 하위 8비트
				m_tx_msg_18FEBFFEh.DATA[1] = static_cast<uint8_t>((dummy_data_u16 >> 8) & 0xFF); // 상위 8비
				
				dummy_data_u8 = static_cast<uint8_t>((dummy.m_fCmd_relatve_veh_axle_speed + 7.8125f)*16);// 예시로 실속과 휠 axle 상대 속도를 kph 단위를 8비트로 변환하여 전송
				m_tx_msg_18FEBFFEh.DATA[2] = m_tx_msg_18FEBFFEh.DATA[3] = m_tx_msg_18FEBFFEh.DATA[4] = dummy_data_u8; 
				m_tx_msg_18FEBFFEh.DATA[5] = m_tx_msg_18FEBFFEh.DATA[6] = m_tx_msg_18FEBFFEh.DATA[7] = dummy_data_u8;
				can_tx_message_handler(m_tx_msg_18FEBFFEh);  //EBC2

				m_tx_msg_CFE6CFEh.DATA[6] = static_cast<uint8_t>(dummy_data_u16 & 0xFF); // 하위 8비트
				m_tx_msg_CFE6CFEh.DATA[7] = static_cast<uint8_t>((dummy_data_u16 >> 8) & 0xFF); // 상위 8비
				can_tx_message_handler(m_tx_msg_CFE6CFEh);   //TCO1

				m_tx_msg_8FE6EFEh.DATA[0] = static_cast<uint8_t>(dummy_data_u16 & 0xff); // 예시로 차량 속도 kph 단위를 16비트로 변환하여 전송
				m_tx_msg_8FE6EFEh.DATA[1] = static_cast<uint8_t>((dummy_data_u16 >> 8) & 0xFF); // 상위 8비
				m_tx_msg_8FE6EFEh.DATA[2] = static_cast<uint8_t>(dummy_data_u16 & 0xff); // 예시로 차량 속도 kph 단위를 16비트로 변환하여 전송
				m_tx_msg_8FE6EFEh.DATA[3] = static_cast<uint8_t>((dummy_data_u16 >> 8) & 0xFF); // 상위 8비
				m_tx_msg_8FE6EFEh.DATA[4] = static_cast<uint8_t>(dummy_data_u16 & 0xff); // 예시로 차량 속도 kph 단위를 16비트로 변환하여 전송
				m_tx_msg_8FE6EFEh.DATA[5] = static_cast<uint8_t>((dummy_data_u16 >> 8) & 0xFF); // 상위 8비
				m_tx_msg_8FE6EFEh.DATA[6] = static_cast<uint8_t>(dummy_data_u16 & 0xff); // 예시로 차량 속도 kph 단위를 16비트로 변환하여 전송
				m_tx_msg_8FE6EFEh.DATA[7] = static_cast<uint8_t>((dummy_data_u16 >> 8) & 0xFF); // 상위 8비
				can_tx_message_handler(m_tx_msg_8FE6EFEh);   // HRW

				m_tx_msg_18FEF100h.DATA[1] = static_cast<uint8_t>(dummy_data_u16 & 0xff); // 예시로 차량 속도 kph 단위를 16비트로 변환하여 전송
				m_tx_msg_18FEF100h.DATA[2] = static_cast<uint8_t>((dummy_data_u16 >> 8) & 0xFF); // 상위 8비
				can_tx_message_handler(m_tx_msg_18FEF100h);  //CCVS1
			}

			if ( elapsed % 100 == 50 )  //1000ms
			{
				auto now = std::chrono::system_clock::now();
				std::time_t time_now = std::chrono::system_clock::to_time_t(now);
				std::tm local_tm ;
				localtime_r(&time_now, &local_tm);

				m_tx_msg_18FEE6FEh.DATA[0] = static_cast<uint8_t>(local_tm.tm_sec * 4);
				m_tx_msg_18FEE6FEh.DATA[1] = static_cast<uint8_t>(local_tm.tm_min);
				m_tx_msg_18FEE6FEh.DATA[2] = static_cast<uint8_t>(local_tm.tm_hour);
				m_tx_msg_18FEE6FEh.DATA[3] = static_cast<uint8_t>(local_tm.tm_mon+1);
				m_tx_msg_18FEE6FEh.DATA[4] = static_cast<uint8_t>(local_tm.tm_mday * 4);
				m_tx_msg_18FEE6FEh.DATA[5] = static_cast<uint8_t>(local_tm.tm_year+1900 - 1985);
				can_tx_message_handler(m_tx_msg_18FEE6FEh);    //TD (Time)
				can_tx_message_handler(m_tx_msg_18FEC1FEh);
			}
			

			dummy_data_u16 = static_cast<uint16_t>((dummy.m_fCmd_engine_speed) * 8); // 예시로 엔진 속도를 0.25rpm 단위로 변환하여 전송
			m_tx_msg_CF004FEh.DATA[3] = static_cast<uint8_t>(dummy_data_u16 & 0xFF); // 하위 8비트
			m_tx_msg_CF004FEh.DATA[4] = static_cast<uint8_t>((dummy_data_u16 >> 8) & 0xFF); // 상위 8비
			m_tx_msg_CF004FEh.DATA[6] = 0x04;   //engine starter status
			can_tx_message_handler(m_tx_msg_CF004FEh);	  //EEC1
			
			dummy_data_u16 = static_cast<uint16_t>( (dummy.m_fCmd_yawrate + 3.92f) * 8192 );  //yawrate   
			m_tx_msg_18F009FEh.DATA[0] = static_cast<uint8_t>(dummy_data_u16 & 0x00FF); // 하위 8비트
			m_tx_msg_18F009FEh.DATA[1] = static_cast<uint8_t>((dummy_data_u16 >> 8) & 0x00FF); // 상위 8비트
			can_tx_message_handler(m_tx_msg_18F009FEh);   //vdc1
			//can_tx_message_handler(m_tx_msg_18F01DEFh);

			dummy_data_u16 = static_cast<uint16_t>( (dummy.m_fCmd_str_mt_tq + 31.374f) * 1024);
			m_tx_msg_18FF00FEh.DATA[0] = static_cast<uint8_t>(dummy_data_u16 & 0x00FF); // 하위 8비트
			m_tx_msg_18FF00FEh.DATA[1] = static_cast<uint8_t>((dummy_data_u16 >> 8) & 0x00FF); // 상위 8비트
			dummy_data_u16 = static_cast<uint16_t>((dummy.m_FCmd_str_angle + 3212.7f) * 10); // 예시로 조향각을 0.1도 단위로 변환하여 전송
			m_tx_msg_18FF00FEh.DATA[2] = static_cast<uint8_t>(dummy_data_u16 & 0xFF); // 하위 8비트
			m_tx_msg_18FF00FEh.DATA[3] = static_cast<uint8_t>((dummy_data_u16 >> 8) & 0x00FF); // 상위 8비트
			dummy_data_u16 = static_cast<uint16_t>( (dummy.m_fCmd_str_fbf_tq + 31.374f) * 1024);
			m_tx_msg_18FF00FEh.DATA[4] = static_cast<uint8_t>(dummy_data_u16 & 0x00FF); // 하위 8비트
			m_tx_msg_18FF00FEh.DATA[5] = static_cast<uint8_t>((dummy_data_u16 >> 8) & 0x00FF); // 상위 8비트
			m_tx_msg_18FF00FEh.DATA[6] = (nCnt << 4) | (dummy.m_nCmd_str_ctrl_mode & 0x0F); // ALV 카운트(상위 4비트)와 제어 모드(하위 4비트)를 하나의 바이트에 결합
			m_tx_msg_18FF00FEh.DATA[7] = calc_crc_8bit_wcdma(m_tx_msg_18FF00FEh.ID, m_tx_msg_18FF00FEh.DATA, m_tx_msg_18FF00FEh.LEN);
			can_tx_message_handler(m_tx_msg_18FF00FEh);

			m_tx_msg_18FF01FEh.DATA[0] = dummy.m_nCmd_str_active_ret_weight_req;
			m_tx_msg_18FF01FEh.DATA[1] = dummy.m_nCmd_str_angle_ctrl_weight_req;
			m_tx_msg_18FF01FEh.DATA[2] = dummy.m_nCmd_str_angle_ctrl_tq_limit_req;
			m_tx_msg_18FF01FEh.DATA[6] = (nCnt << 4);
			m_tx_msg_18FF01FEh.DATA[7] = calc_crc_8bit_wcdma(m_tx_msg_18FF01FEh.ID, m_tx_msg_18FF01FEh.DATA, m_tx_msg_18FF01FEh.LEN);
			can_tx_message_handler(m_tx_msg_18FF01FEh);

			m_tx_msg_18FF02FEh.DATA[0] = dummy.m_nCmd_str_vibrate_enable | ((dummy.m_nCmd_str_vibrate_level << 4) & 0xF0); // 진동 활성화 여부(하위 4비트)와 진동 레벨(상위 4비트)을 하나의 바이트에 결합
			m_tx_msg_18FF02FEh.DATA[6] = (nCnt << 4);
			m_tx_msg_18FF02FEh.DATA[7] = calc_crc_8bit_wcdma(m_tx_msg_18FF02FEh.ID, m_tx_msg_18FF02FEh.DATA, m_tx_msg_18FF02FEh.LEN);
			can_tx_message_handler(m_tx_msg_18FF02FEh);


			nCnt = (nCnt+1) & 0x0F;
			elapsed++;

			std::this_thread::sleep_for(std::chrono::microseconds(10000));
		}
	}


private: //functions
	void can_tx_message_handler(TPCANMsg& tx_msg)
	{
		TPCANStatus Status;

		Status = CAN_Write(pcan_device_, &tx_msg);

		if (Status == PCAN_ERROR_QXMTFULL || Status == PCAN_ERROR_XMTFULL) 	
		{ 
			std::this_thread::sleep_for(std::chrono::microseconds(200));
			CAN_Write(pcan_device_, &tx_msg);
		}
	}

	void can_rx_message_handler(const TPCANMsg& rx_msg)
	{
		switch(rx_msg.ID)
		{
			case EPSO1:  //EPS01
				rx_dummy_.m_nData_str_sys_state = rx_msg.DATA[6] & 0x0F; // 하위 4비트
				rx_dummy_.m_fData_str_angle = static_cast<double>((static_cast<uint16_t>(rx_msg.DATA[1]) << 8) | static_cast<uint16_t>(rx_msg.DATA[0])) * 0.1f - 3212.7f;
				rx_dummy_.m_fData_str_angle_speed = static_cast<double>((static_cast<uint16_t>(rx_msg.DATA[3]) << 8) | static_cast<uint16_t>(rx_msg.DATA[2])) * 0.1f - 3212.7f;
				break;  
			case EPSO2:  //EPS02
				rx_dummy_.m_fData_str_Driver_Tq = static_cast<double>((static_cast<uint16_t>(rx_msg.DATA[1]) << 8) | static_cast<uint16_t>(rx_msg.DATA[0])) * 0.000976562f - 31.374f;
				rx_dummy_.m_fData_str_Motor_Tq = static_cast<double>((static_cast<uint16_t>(rx_msg.DATA[3]) << 8) | static_cast<uint16_t>(rx_msg.DATA[2])) * 0.000976562f - 31.374f;
				rx_dummy_.m_nData_str_ctrl_mode = rx_msg.DATA[5] & 0x0F; // 하위 4비트
				rx_dummy_.m_nData_str_vibrate_enable = (rx_msg.DATA[5] & 0xF0) >> 4; // 상위 4비트
				break;   
			case ESC1:  //ESC1 
				rx_dummy_.m_fActual_inner_wheel_angle = static_cast<double>((static_cast<uint16_t>(rx_msg.DATA[1]) << 8) | static_cast<uint16_t>(rx_msg.DATA[0])) * 0.00390625f -125.0f;
				break;
			default:
				break;
		}
	}

	uint8_t calc_crc_4bit(const uint32_t nExtID, const uint8_t* data, const uint8_t nAlvCnt)
	{
		uint8_t crc = static_cast<uint8_t>(nExtID & 0xFF); // Start with the lower 4 bits of the extended ID
		uint8_t temp = 0;

		crc += static_cast<uint8_t>((nExtID>>8) & 0xFF);
		crc += static_cast<uint8_t>((nExtID>>16) & 0xFF);
		crc += static_cast<uint8_t>((nExtID>>24) & 0xFF);

		for(uint8_t i = 0; i < 7; ++i)
		{
			temp += data[i];
		}

		crc += temp + (nAlvCnt & 0x0F); // Add the ALV count (only the lower 4 bits)


		return ((crc + (crc>>4)) & 0x0F); // Return only the lower 4 bits of the result
	}

	uint8_t calc_crc_8bit_wcdma(const uint32_t nExtID, const uint8_t* data, const uint8_t nLen)
	{
		uint8_t crc = static_cast<uint8_t>((nExtID & 0xFF)+1); // Start with the lower 4 bits of the extended ID

		for(uint8_t i = 0; i < nLen - 1; ++i)
		{
			crc = crc ^ data[i];
			for( uint8_t j = 0; j < 8; ++j)
			{
				if((crc & 0x80) == 0x80)
					crc = (crc << 1) ^ 0x9B; // Polynomial used in WCDMA
				else
					crc <<= 1;
			}
		}

		return ~crc; // Return the bitwise NOT of the result
	}

private: //members
	uint16_t 			pcan_device_;
	uint16_t			pcan_baud_;

	TPCANMsg	        m_tx_msg_18FEBFFEh 	={.ID = EBC2, .MSGTYPE = PCAN_MESSAGE_EXTENDED, .LEN = 8, .DATA = {0}};
	TPCANMsg	        m_tx_msg_CFE6CFEh 	={.ID = TCO1, .MSGTYPE = PCAN_MESSAGE_EXTENDED, .LEN = 8, .DATA = {0}};
	TPCANMsg	        m_tx_msg_8FE6EFEh 	={.ID = HRW, .MSGTYPE = PCAN_MESSAGE_EXTENDED, .LEN = 8, .DATA = {0}};
	TPCANMsg	        m_tx_msg_18FEF100h 	={.ID = CCVS1, .MSGTYPE = PCAN_MESSAGE_EXTENDED, .LEN = 8, .DATA = {0}};
	TPCANMsg	        m_tx_msg_CF004FEh 	={.ID = EEC1, .MSGTYPE = PCAN_MESSAGE_EXTENDED, .LEN = 8, .DATA = {0}};
	TPCANMsg	        m_tx_msg_18F009FEh 	={.ID = VDC2, .MSGTYPE = PCAN_MESSAGE_EXTENDED, .LEN = 8, .DATA = {0}};
	TPCANMsg	        m_tx_msg_18FF00FEh 	={.ID = EPSI1, .MSGTYPE = PCAN_MESSAGE_EXTENDED, .LEN = 8, .DATA = {0}};
	TPCANMsg	        m_tx_msg_18FF01FEh 	={.ID = EPSI2, .MSGTYPE = PCAN_MESSAGE_EXTENDED, .LEN = 8, .DATA = {0}};
	TPCANMsg	        m_tx_msg_18FF02FEh 	={.ID = EPSI3, .MSGTYPE = PCAN_MESSAGE_EXTENDED, .LEN = 8, .DATA = {0}};
	TPCANMsg	        m_tx_msg_18FEE6FEh 	={.ID = TD, .MSGTYPE = PCAN_MESSAGE_EXTENDED, .LEN = 8, .DATA = {0}};
	TPCANMsg	        m_tx_msg_18FEC1FEh 	={.ID = VDHR, .MSGTYPE = PCAN_MESSAGE_EXTENDED, .LEN = 8, .DATA = {0}};
	

	std::atomic<bool>   		bRunning_;
	std::thread					pThread_[2]{};
	std::mutex					pMux_lock_[2]{};
	std::vector<CAN_CMD_INFO> 	pCmd_info;
	std::vector<CAN_RX_INFO> 	pRx_info;

	CAN_RX_INFO					rx_dummy_{};
};

#endif // COMM_PCAN_HPP_

