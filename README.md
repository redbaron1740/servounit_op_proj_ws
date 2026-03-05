# Servounit Test Program

## 📋 프로젝트 개요

**Servounit Test Program**은 Linux 환경에서 PEAK PCAN USB 어댑터를 통해 EPS(Electric Power Steering) Servounit과 CAN 통신을 수행하는 콘솔 기반 테스트 프로그램입니다.

ncurses 기반의 인터랙티브 TUI(Text User Interface)를 제공하며, 사용자가 실시간으로 EPS 제어 명령을 전송하고 피드백 데이터를 모니터링할 수 있습니다.

### 주요 기능

| 기능 | 설명 |
|------------------------------|-----------------------------------------------------|
| **CAN 디바이스 제어**        | PCAN USB 디바이스 초기화, 시작, 종료                   |
| **조향각 제어**             | Steer Angle Control Mode (-850.0° ~ 850.0°)         |
| **Feedback-free 토크 제어**   | Feedback-free Torque Control Mode (-2.0 ~ 2.0 Nm)   |
| **모터 토크 제어**             | Motor Torque Control Mode (-2.0 ~ 2.0 Nm)           |
| **진동 모드 제어**             | Vibration Level 0~9 단계 설정                        |
| **차량 데이터 설정**           | 차량 속도, 가속도, Yaw Rate, 제어 파라미터 등 설정      |
| **실시간 EPS 데이터 모니터링**  | 조향각, 조향각 속도, 드라이버 토크, 모터 토크 등          |

### 프로젝트 구조

```
servounit_op_proj_ws/
├── CMakeLists.txt          # CMake 빌드 설정 파일
├── README.md               # 프로젝트 문서
├── GITHUB_GUIDE.md         # GitHub 사용 가이드
├── .gitignore              # Git 제외 파일 목록
├── inc/
│   ├── comm_pcan.hpp       # PCAN 통신 매니저 클래스 (TX/RX 스레드, CAN 파싱)
│   └── menu_display.hpp    # ncurses 기반 메뉴 UI 클래스
├── src/
│   └── main.cpp            # 메인 프로그램 (메뉴 로직, EPS 제어 로직)
└── build/                  # CMake 빌드 출력 디렉토리
```

---

## 🔧 필수 설치 모듈 및 의존성

### 시스템 요구사항

- **OS**: Linux (Ubuntu 20.04+ / Debian 기반 권장)
- **컴파일러**: GCC/G++ (C++17 지원 필수)
- **CMake**: 3.10 이상
- **CAN 하드웨어**: PEAK PCAN-USB 어댑터

### 필수 라이브러리

#### 1. ncurses (TUI 라이브러리)

콘솔 기반 메뉴 인터페이스 및 실시간 데이터 표시에 사용됩니다.

```bash
sudo apt-get update
sudo apt-get install libncurses5-dev libncursesw5-dev
```

#### 2. PCAN Basic Linux API (PEAK CAN 드라이버)

PEAK Systems의 PCAN-USB 어댑터와 통신하기 위한 드라이버 및 API입니다.

```bash
# PEAK Linux 드라이버 다운로드 및 설치
# https://www.peak-system.com/linux/ 에서 최신 드라이버 다운로드

# peak-linux-driver 설치
tar -xzf peak-linux-driver-x.x.x.tar.gz
cd peak-linux-driver-x.x.x
sudo make clean
sudo make
sudo make install
```

#### 3. 빌드 도구

```bash
sudo apt-get install build-essential cmake
```

### 의존성 요약

| 라이브러리     | 패키지명           | 용도              | 링크 플래그                 |
|----------------|-------------------|------------------|----------------------------|
| **ncurses**    | `libncurses5-dev` | TUI 메뉴/화면 표시 | `-lncurses`                |
| **PCANBasic**  | `libpcanbasic.so` | PCAN USB CAN 통신 | `/usr/lib/libpcanbasic.so` |
| **pthread**    | (시스템 기본)       | 멀티스레드 TX/RX  | `-lpthread`                |

---

## 🏗️ 빌드 방법

### 빌드 디렉토리 생성 및 CMake 구성

```bash
cd servounit_op_proj_ws
mkdir -p build
cd build
cmake ..
```

### 컴파일

```bash
make
```

### 전체 과정 (한 번에 실행)

```bash
cd servounit_op_proj_ws && mkdir -p build && cd build && cmake .. && make
```

### 빌드 결과물

빌드가 성공하면 `build/` 디렉토리에 실행 파일이 생성됩니다:

```
servounit_op_proj_ws/build/servounit_test_program
```

### 클린 빌드

```bash
cd servounit_op_proj_ws/build
make clean
make
```

### 실행

```bash
cd servounit_op_proj_ws/build
./servounit_test_program
```

> ⚠️ **주의**: PCAN USB 디바이스가 연결되어 있어야 하며, `/dev/pcanusb*` 장치에 대한 접근 권한이 필요합니다.
> 필요 시 `sudo` 로 실행하거나 udev 규칙을 설정하세요.

### CMake 빌드 설정 상세

[`CMakeLists.txt`](CMakeLists.txt)에 정의된 빌드 구성:

| Field            | Value                                     |
|------------------|-------------------------------------------|
| *Project name*   | 'Servounit Test Program'                  |
| Standard C++     | C++17 (g++17)                             |
| Compile flags    | '-Wall -Wextra'                           |
| Execution file   | 'servounit_test_program'                  |
| Include path     | 'inc/', '/usr/include'                    | 
| Link Library     | 'libpcanbasic.so', 'ncurses', 'pthread'   |

---

## 📡 CAN Parsing 상세

### CAN 통신 개요

본 프로그램은 **J1939 Extended CAN (29-bit ID)** 프로토콜을 사용하며, PCAN USB 어댑터를 통해 **500 Kbps** 속도로 통신합니다. TX/RX는 별도의 스레드에서 비동기적으로 처리됩니다.

- **TX 주기**: 10ms (100Hz)
- **RX 폴링**: 1ms 간격으로 수신 큐 확인, 10ms 주기로 데이터 갱신
- **메시지 타입**: `PCAN_MESSAGE_EXTENDED` (29-bit Extended CAN ID)

### TX 메시지 (프로그램 → EPS Servounit)

프로그램에서 EPS로 전송하는 CAN 메시지 목록입니다.

| CAN ID (Hex) | PGN 이름   | 설명                                      | 데이터 길이 |
|--------------|-----------|-------------------------------------------|-----------|
| `0x18FEBFFE` | **EBC2**  | Electronic Brake Controller 2 — 차량 속도 정보 | 8 bytes |
| `0x0CFE6CFE` | **TCO1**  | Tachograph 1                              | 8 bytes |
| `0x08FE6EFE` | **HRW**   | High Resolution Wheel Speed — 휠 속도 정보  | 8 bytes  |
| `0x18FEF100` | **CCVS1** | Cruise Control/Vehicle Speed 1 — 차량 속도  | 8 bytes  |
| `0x0CF004FE` | **EEC1**  | Electronic Engine Controller 1 — 엔진 RPM  | 8 bytes  |
| `0x18F009FE` | **VDC1**  | Vehicle Dynamic Control 1 — Yaw Rate      | 8 bytes |
| `0x18FEE6FE` | **TD**    | Time/Date — 시간 정보                      | 8 bytes |
| `0x18FEC1FE` | **VDHR**  | Vehicle Direction/Speed — High Resolution | 8 bytes |
| `0x18FF00FE` | **EPSI1** | EPS Input 1 — 조향각 명령 + 제어 모드 + CRC  | 8 bytes |
| `0x18FF01FE` | **EPSI2** | EPS Input 2 — 제어 가중치/토크 리밋 + CRC    | 8 bytes  |
| `0x18FF02FE` | **EPSI3** | EPS Input 3 — 진동 모드/레벨 + CRC          | 8 bytes   |

### RX 메시지 (EPS Servounit → 프로그램)

EPS에서 수신하는 CAN 메시지 목록입니다.

| CAN ID (Hex) | PGN 이름   | 설명                                                  | 데이터 길이 |
|--------------|-----------|-------------------------------------------------------|---------|
| `0x18FF0513` | **EPSO1** | EPS Output 1 — 시스템 상태, 조향각, 조향각 속도           | 8 bytes |
| `0x18FF0613` | **EPSO2** | EPS Output 2 — 드라이버 토크, 모터 토크, 제어 모드, 진동 상태 | 8 bytes |
| `0x18F00B13` | **ESC1**  | Electronic Stability Control 1 — 실제 내부 휠 각도      | 8 bytes |

### CAN 메시지 파싱 상세

#### EPSI1 (TX: `0x18FF00FE`) — 조향각 명령

```
Byte[0..1]: 모터 토크 명령값 (uint16, Little-Endian)
            변환: raw = (torque + 31.374) × 1024
            범위: -31.374 ~ 31.374 Nm
            분해능: 0.000976562 Nm

Byte[2..3]: 조향각 명령값 (uint16, Little-Endian)
            변환: raw = (angle + 3212.7) × 10
            범위: -850.0° ~ 850.0°
            분해능: 0.1°

Byte[4..5]: Feedback-free 토크 명령값 (uint16, Little-Endian)
            변환: raw = (torque + 31.374) × 1024
            범위: -31.374 ~ 31.374 Nm
            분해능: 0.000976562 Nm

Byte[6]:    [7:4] ALV Counter (0x0~0xF, 순환 카운터)
            [3:0] 제어 모드 (EPS_Mode enum)
                  0: Off
                  1: Ready
                  2: Feedback-free Torque Control
                  3: Motor Torque Control
                  4: Steer Angle Control
                  5: Cancel

Byte[7]:    CRC-8 WCDMA (Polynomial: 0x9B)
```

#### EPSI2 (TX: `0x18FF01FE`) — 제어 파라미터

```
Byte[0]:    Active Return Weight Request (0~100%)
Byte[1]:    Angle Control Weight Request (0~100%)
Byte[2]:    Angle Control Torque Limit Request (0~10 Nm)
            변환: raw = limit × 20
            분해능: 0.05 Nm
Byte[3..5]: Reserved (0x00)
Byte[6]:    [7:4] ALV Counter (0x0~0xF)
            [3:0] Reserved (0x0)
Byte[7]:    CRC-8 WCDMA (Polynomial: 0x9B)
```

#### EPSI3 (TX: `0x18FF02FE`) — 진동 제어

```
Byte[0]:    [7:4] Vibration Level (0~9, 9=최대)
            [3:0] Vibration Enable (0: Off, 1: On)
Byte[1..5]: Reserved (0x00)
Byte[6]:    [7:4] ALV Counter (0x0~0xF)
            [3:0] Reserved (0x0)
Byte[7]:    CRC-8 WCDMA (Polynomial: 0x9B)
```

#### EBC2 (TX: `0x18FEBFFE`) — 차량 속도

```
Byte[0..1]: 차량 속도 (uint16, Little-Endian)
            변환: raw = speed × 256
            범위: 0 ~ 250 kph
            분해능: 1/256 kph

Byte[2..7]: 휠 속도 차이 (각 바이트)
            변환: raw = (speed + 7.8125) × 16
            범위: -7 ~ 7 kph
            분해능: 0.0625 kph
```

#### EEC1 (TX: `0x0CF004FE`) — 엔진 속도

```
Byte[0..2]: Reserved (0x00)
Byte[3..4]: 엔진 속도 (uint16, Little-Endian)
            변환: raw = rpm × 8
            분해능: 0.125 rpm
Byte[5]:    Reserved (0x00)
Byte[6]:    Engine Starter Status (0x04 = Running)
Byte[7]:    Reserved (0x00)
```

#### VDC1 (TX: `0x18F009FE`) — 차량 동역학 제어

```
Byte[0..1]: Yaw Rate (uint16, Little-Endian)
            변환: raw = (yawrate + 3.92) × 8192
            범위: -3.92 ~ 3.92 rad/s²
            분해능: 0.0001220703125 rad/s²
Byte[2..7]: Reserved (0x00)
```

#### EPSO1 (RX: `0x18FF0513`) — EPS 상태 피드백

```
Byte[0..1]: 실제 조향각 (uint16, Little-Endian)
            변환: angle = raw × 0.1 - 3212.7
            단위: deg

Byte[2..3]: 조향각 속도 (uint16, Little-Endian)
            변환: speed = raw × 0.1 - 3212.7
            단위: deg/s

Byte[4..5]: Reserved

Byte[6]:    [7:4] Reserved
            [3:0] System State (EPS_Sys_State enum)
                  0: Undefined
                  1: NMWait
                  2: Terminal30Watt
                  3: PreDrive
                  4: DriveDown
                  5: DriveUp
                  6: PostRun
                  7: Off
                  8: Error
                  9: Flash
                  10: LowVoltage

Byte[7]:    Reserved
```

#### EPSO2 (RX: `0x18FF0613`) — 토크 및 모드 피드백

```
Byte[0..1]: 드라이버 토크 (uint16, Little-Endian)
            변환: torque = raw × 0.000976562 - 31.374
            단위: Nm

Byte[2..3]: 모터 토크 (uint16, Little-Endian)
            변환: torque = raw × 0.000976562 - 31.374
            단위: Nm

Byte[4]:    Reserved

Byte[5]:    [7:4] 진동 활성화 상태 (0: Off, 1: On)
            [3:0] 현재 제어 모드 (EPS_Mode)

Byte[6..7]: Reserved
```

#### ESC1 (RX: `0x18F00B13`) — 실제 휠 각도

```
Byte[0..1]: 실제 내부 휠 각도 (uint16, Little-Endian)
            변환: angle = raw × 0.00390625 - 125.0
            단위: deg
            범위: -125 ~ 125 deg

Byte[2..7]: Reserved
```

### CRC 알고리즘

#### CRC-8 WCDMA (EPSI1, EPSI2, EPSI3에 사용)

```
다항식(Polynomial): 0x9B
초기값: (CAN_ID의 하위 8비트) + 1
입력 데이터: Byte[0] ~ Byte[6] (7바이트)
최종 처리: 비트 반전 (~crc)
```

[`calc_crc_8bit_wcdma()`](inc/comm_pcan.hpp:406) 함수에서 구현되어 있으며, 각 EPSI 메시지의 Byte[7]에 CRC 값이 기록됩니다.

### EPS 제어 모드 상태 전이

```
Off → Ready → [Mode Selection] → Cancel → Off
                  │
                  ├── Steer Angle Control Mode (4)
                  ├── Feedback-free Torque Control Mode (2)
                  └── Motor Torque Control Mode (3)
```

프로그램은 EPS의 현재 상태(`EPSO2`의 제어 모드 피드백)를 확인한 후, 다음 단계의 명령을 자동으로 전송합니다:
1. EPS가 `Off` 상태이면 → `Ready` 명령 전송
2. EPS가 `Ready` 상태이면 → 선택한 제어 모드 명령 전송
3. 해당 제어 모드에 진입하면 → 사용자 입력에 따라 제어값 전송

---

## 🎮 사용 방법

### 메인 메뉴

프로그램 실행 시 다음 메뉴가 표시됩니다:

```
Servounit Test Program
========================================
  1. Open can device
  2. Set angle value for steering control
  3. Set Feedback-free torque value for steering control
  4. Set Motor torque value for steering control
  5. Steer Vibration mode on
  6. Set another Value (vehicle values, etc.)
  7. About Program
  Quit
========================================
Arrow Up/Down: Move | Enter: Select | Q: Quit
```

### 조작 키

| 키                   | 동작                                    |
|----------------------|----------------------------------------|
| `↑` / `↓`            | 메뉴 항목 이동                          |
| `Enter`              | 선택 항목 실행                          |
| `Q`                  | 프로그램 종료 / 서브메뉴에서 메인으로 복귀 |
| `↑` / `↓` (서브메뉴)  | 차량 속도 증감 또는 진동 레벨 증감        |
| 숫자 입력 + `Enter`   | 조향각/토크 값 설정                     |

### 차량 데이터 설정 메뉴 (메뉴 6번)

차량 관련 파라미터를 설정할 수 있는 서브메뉴입니다:

```
Vehicle Data Information
========================================
  1. In-vehicle speed: 0.00 kph
  2. Front axle wheel relative speed(gap): 0.00 kph
  3. In-vehicle yawrate: 0.000000 rad/sec^2
  4. In-vehicle acceleration of x axis: 0.00 m/s^2
  5. In-vehicle acceleration of y axis: 0.00 m/s^2
  6. Required return weight of steering active : 50 %
  7. Required angle weight of steering angle: 50 %
  8. Required limitation of steering torque: 7.50 Nm
  Update info
```

#### 설정 가능한 파라미터

| 항목 | 범위 | 단위 | 설명 |
|------|------|------|------|
| **차량 속도** | 0 ~ 250 | kph | 차량의 현재 속도 |
| **휠 상대 속도** | -7 ~ 7 | kph | 전륜 액슬 휠 속도 차이 |
| **Yaw Rate** | -3.9 ~ 3.9 | rad/s² | 차량 회전 각속도 |
| **X축 가속도** | -15 ~ 15 | m/s² | 전후방 가속도 |
| **Y축 가속도** | -15 ~ 15 | m/s² | 좌우 가속도 |
| **Return Weight** | 0 ~ 100 | % | 조향 복원력 가중치 |
| **Angle Weight** | 0 ~ 100 | % | 조향각 제어 가중치 |
| **Torque Limit** | 0 ~ 10 | Nm | 조향 토크 제한값 |

---

## 🏛️ 아키텍처

### 클래스 구조

#### 1. [`PCANManager`](inc/comm_pcan.hpp:90) 클래스

PCAN 디바이스 관리 및 CAN 통신을 담당하는 핵심 클래스입니다.

**주요 메서드:**
- `device_open_()`: PCAN 디바이스 초기화
- `device_close_()`: PCAN 디바이스 종료
- `device_start_()`: TX/RX 스레드 시작
- `device_stop_()`: TX/RX 스레드 종료
- `getRxData_eps()`: 최신 EPS 수신 데이터 반환
- `setCmdData_eps()`: EPS 제어 명령 설정

**스레드 구조:**
- **RX 스레드** ([`can_rx_thread`](inc/comm_pcan.hpp:202)): 1ms 주기로 CAN 메시지 수신, 10ms 주기로 데이터 갱신
- **TX 스레드** ([`can_tx_thread`](inc/comm_pcan.hpp:233)): 10ms 주기로 CAN 메시지 전송

#### 2. [`Menu_select_instance`](inc/menu_display.hpp:9) 클래스

ncurses 기반 메뉴 UI를 제공하는 클래스입니다.

**주요 메서드:**
- `display()`: 메뉴 화면 렌더링
- `run()`: 메뉴 실행 및 사용자 입력 처리

### 데이터 구조

#### [`CAN_RX_INFO`](inc/comm_pcan.hpp:56) 구조체
EPS로부터 수신한 데이터를 저장합니다.

```cpp
typedef struct _tag_rx_vcu_info {
    uint8_t  m_nData_str_sys_state;          // 시스템 상태
    uint8_t  m_nData_str_ctrl_mode;          // 제어 모드
    uint8_t  m_nData_str_vibrate_enable;     // 진동 활성화
    double   m_fActual_inner_wheel_angle;    // 실제 내부 휠 각도
    double   m_fData_str_angle;              // 조향각
    double   m_fData_str_angle_speed;        // 조향각 속도
    double   m_fData_str_Driver_Tq;          // 드라이버 토크
    double   m_fData_str_Motor_Tq;           // 모터 토크
} CAN_RX_INFO;
```

#### [`CAN_CMD_INFO`](inc/comm_pcan.hpp:70) 구조체
EPS로 전송할 제어 명령을 저장합니다.

```cpp
typedef struct _tag_tx_upper_info {
    uint8_t  m_nCmd_str_ctrl_mode;                  // 제어 모드
    uint8_t  m_nCmd_str_vibrate_enable;             // 진동 활성화
    uint8_t  m_nCmd_str_active_ret_weight_req;      // 복원력 가중치
    uint8_t  m_nCmd_str_angle_ctrl_weight_req;      // 각도 제어 가중치
    uint8_t  m_nCmd_str_vibrate_level;              // 진동 레벨
    double   m_FCmd_str_angle;                      // 조향각 명령
    double   m_fCmd_str_mt_tq;                      // 모터 토크 명령
    double   m_fCmd_str_fbf_tq;                     // Feedback-free 토크 명령
    double   m_fCmd_in_vehicle_speed;               // 차량 속도
    double   m_fCmd_engine_speed;                   // 엔진 속도
    double   m_fCmd_relatve_veh_axle_speed;         // 휠 상대 속도
    double   m_fCmd_yawrate;                        // Yaw Rate
    double   m_fCmd_accelX;                         // X축 가속도
    double   m_fCmd_accelY;                         // Y축 가속도
    double   m_fCmd_str_angle_ctrl_tq_limit_req;    // 토크 제한
} CAN_CMD_INFO;
```

### 주요 함수

#### [`main()`](src/main.cpp:101)
프로그램의 진입점으로, 메인 메뉴를 표시하고 사용자 선택을 처리합니다.

#### [`updateEPSData()`](src/main.cpp:201)
EPS 데이터를 화면에 실시간으로 표시합니다.

#### [`submenu_display()`](src/main.cpp:251)
차량 데이터 설정 서브메뉴를 표시합니다.

#### [`update_another_data()`](src/main.cpp:290)
차량 관련 파라미터를 설정하는 메뉴를 처리합니다.

#### [`setMenu_ctrl_steer_angle()`](src/main.cpp:484)
조향각 제어 모드를 실행합니다.

#### [`setMenu_ctrl_free_torque()`](src/main.cpp:589)
Feedback-free 토크 제어 모드를 실행합니다.

#### [`setMenu_ctrl_motor_torque()`](src/main.cpp:539)
모터 토크 제어 모드를 실행합니다.

#### [`setMenu_ctrl_vibration()`](src/main.cpp:641)
진동 모드를 실행합니다.

---

## 🔍 트러블슈팅

### PCAN 디바이스를 찾을 수 없음

```
No pcan-device connected!! Please, check pcan-device
```

**해결 방법:**
1. PCAN USB 어댑터가 물리적으로 연결되어 있는지 확인
2. 드라이버가 올바르게 설치되었는지 확인:
   ```bash
   lsmod | grep peak
   ls -l /dev/pcan*
   ```
3. 디바이스 권한 확인:
   ```bash
   sudo chmod 666 /dev/pcanusb*
   ```

### ncurses 화면이 깨짐

**해결 방법:**
1. 터미널 크기를 충분히 크게 설정 (최소 80x24)
2. 프로그램 종료 후 터미널 초기화:
   ```bash
   reset
   ```

### CAN 메시지 전송 실패

```
CAN_Write Error: PCAN_ERROR_QXMTFULL
```

**해결 방법:**
- 정상적인 동작입니다. 프로그램이 자동으로 재시도합니다.
- 지속적으로 발생하면 CAN 버스 연결 상태를 확인하세요.

---

## 📜 라이선스

이 프로그램은 **LGPL-2.1** 라이선스를 따릅니다.

PCAN Basic 관련 코드는 PEAK System-Technik GmbH의 라이선스를 따릅니다.

```
Copyright (C) 2026 ADUS Inc., All rights reserved.
Copyright (C) 2001-2020 PEAK System-Technik GmbH <www.peak-system.com>
```

---

## 📞 문의

프로젝트 관련 문의사항이나 버그 리포트는 GitHub Issues를 통해 제출해 주세요.

**개발자**: ADUS Inc.
**라이선스**: LGPL-2.1
**버전**: 1.0
