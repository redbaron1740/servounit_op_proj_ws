# Servounit Test Program

## 📋 프로젝트 개요

** Servounit Test Program**은 Linux 환경에서 PEAK PCAN USB 어댑터를 통해  EPS(Electric Power Steering) Servounit과 CAN 통신을 수행하는 콘솔 기반 테스트 프로그램입니다.

ncurses 기반의 인터랙티브 TUI(Text User Interface)를 제공하며, 사용자가 실시간으로 EPS 제어 명령을 전송하고 피드백 데이터를 모니터링할 수 있습니다.

### 주요 기능

| 기능 | 설명 |
|------------------------------|-----------------------------------------------------|
|   **CAN 디바이스 제어**        | PCAN USB 디바이스 초기화, 시작, 종료                   |
|   **조향각 제어**             | Steer Angle Control Mode (-850.0° ~ 850.0°)         |
| **Feedback-free 토크 제어**   | Feedback-free Torque Control Mode (-2.0 ~ 2.0 Nm)   |
| **모터 토크 제어**             | Motor Torque Control Mode (-2.0 ~ 2.0 Nm)           |
| **진동 모드 제어**             | Vibration Level 0~9 단계 설정                        |
| **실시간 EPS 데이터 모니터링**  | 조향각, 조향각 속도, 드라이버 토크, 모터 토크 등          |

### 프로젝트 구조

```
servounit_/
├── CMakeLists.txt          # CMake 빌드 설정 파일
├── README.md               # 프로젝트 문서
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

|   라이브러리     | 패키지명           | 용도              | 링크 플래그                 |
|----------------|-------------------|------------------|----------------------------|
| **ncurses**    | `libncurses5-dev` | TUI 메뉴/화면 표시 | `-lncurses`                |
| **PCANBasic**  | `libpcanbasic.so` | PCAN USB CAN 통신 | `/usr/lib/libpcanbasic.so` |
| **pthread**    | (시스템 기본)       | 멀티스레드 TX/RX  | `-lpthread`                |

---

## 🏗️ 빌드 방법

### 빌드 디렉토리 생성 및 CMake 구성

```bash
cd servounit_
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
cd servounit_ && mkdir -p build && cd build && cmake .. && make
```

### 빌드 결과물

빌드가 성공하면 `build/` 디렉토리에 실행 파일이 생성됩니다:

```
servounit_/build/servounit_test_program
```

### 클린 빌드

```bash
cd servounit_/build
make clean
make
```

### 실행

```bash
cd servounit_/build
./servounit_test_program
```

> ⚠️ **주의**: PCAN USB 디바이스가 연결되어 있어야 하며, `/dev/pcanusb*` 장치에 대한 접근 권한이 필요합니다.
> 필요 시 `sudo` 로 실행하거나 udev 규칙을 설정하세요.

### CMake 빌드 설정 상세

[`CMakeLists.txt`](CMakeLists.txt)에 정의된 빌드 구성:

| 항목              |                    값                     |
|------------------|-------------------------------------------|
| **프로젝트명**     | `Servounit_Test_Program`                  |
| **C++ 표준**      | C++17                                     |
| **컴파일러 플래그** | `-Wall -Wextra`                           |
| **실행 파일명**    | `servounit_test_program`                  |
| **Include 경로**  | `inc/`, `/usr/include`                    |
| **링크 라이브러리** | `libpcanbasic.so`, `ncurses`, `pthread`   |

---

## 📡 CAN Parsing 상세

### CAN 통신 개요

본 프로그램은 **J1939 Extended CAN (29-bit ID)** 프로토콜을 사용하며, PCAN USB 어댑터를 통해 **500 Kbps** 속도로 통신합니다. TX/RX는 별도의 스레드에서 비동기적으로 처리됩니다.

- **TX 주기**: 10ms (100Hz)
- **RX 폴링**: 1ms 간격으로 수신 큐 확인, 10ms 주기로 데이터 갱신
- **메시지 타입**: `PCAN_MESSAGE_EXTENDED` (29-bit Extended CAN ID)

### TX 메시지 (프로그램 → EPS Servounit)

프로그램에서 EPS로 전송하는 CAN 메시지 목록입니다.

| CAN ID (Hex) | PGN 이름   |                   설명                       | 데이터 길이 |
|--------------|-----------|----------------------------------------------|-----------|
| `0x18FEBFFE` | **EBC2**  | Electronic Brake Controller 2 — 차량 속도 정보 |   8 bytes |
| `0x0CFE6CFE` | **TCO1**  | Tachograph 1                                 |   8 bytes |
| `0x08FE6EFE` | **HRW**   | High Resolution Wheel Speed — 휠 속도 정보     |  8 bytes  |
| `0x18FEF100` | **CCVS1** | Cruise Control/Vehicle Speed 1 — 차량 속도     |  8 bytes  |
| `0x0CF004FE` | **EEC1**  | Electronic Engine Controller 1 — 엔진 RPM     |  8 bytes  |
| `0x18F009FE` | **VDC1**  | Vehicle Dynamic Control 1                    |   8 bytes |
| `0x18FF00FE` | **EPSI1** | EPS Input 1 — 조향각 명령 + 제어 모드 + CRC     |   8 bytes |
| `0x18FF01FE` | **EPSI2** | EPS Input 2 — 제어 가중치/토크 리밋 + CRC       |  8 bytes  |
| `0x18FF02FE` | **EPSI3** | EPS Input 3 — 진동 모드/레벨 + CRC             | 8 bytes   |

### RX 메시지 (EPS Servounit → 프로그램)

EPS에서 수신하는 CAN 메시지 목록입니다.

| CAN ID (Hex) | PGN 이름   |                       설명                                 | 데이터 길이 |
|--------------|-----------|------------------------------------------------------------|---------|
| `0x18FF0513` | **EPSO1** | EPS Output 1 — 시스템 상태, 조향각, 조향각 속도                | 8 bytes |
| `0x18FF0613` | **EPSO2** | EPS Output 2 — 드라이버 토크, 모터 토크, 제어 모드, 진동 상태    | 8 bytes |
| `0x18F00B13` | **ESC1**  | Electronic Stability Control 1 — 실제 내부 휠 각도            | 8 bytes |

### CAN 메시지 파싱 상세

#### EPSI1 (TX: `0x18FF00FE`) — 조향각 명령

```
Byte[2..3]: 조향각 명령값 (uint16, Little-Endian)
            변환: raw = (angle + 3212.7) × 10
            범위: -850.0° ~ 850.0°
            분해능: 0.1°

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
Byte[6]:    [7:4] ALV Counter (0x0~0xF)
Byte[7]:    CRC-8 WCDMA (Polynomial: 0x9B)
```

#### EPSI3 (TX: `0x18FF02FE`) — 진동 제어

```
Byte[0]:    [7:4] Vibration Level (0~9, 9=최대)
            [3:0] Vibration Enable (0: Off, 1: On)
Byte[6]:    [7:4] ALV Counter (0x0~0xF)
Byte[7]:    CRC-8 WCDMA (Polynomial: 0x9B)
```

#### EBC2 (TX: `0x18FEBFFE`) — 차량 속도

```
Byte[0..1]: 차량 속도 (uint16, Little-Endian)
            변환: raw = speed × 256
            분해능: 1/256 kph
```

#### EEC1 (TX: `0x0CF004FE`) — 엔진 속도

```
Byte[3..4]: 엔진 속도 (uint16, Little-Endian)
            변환: raw = rpm × 8
            분해능: 0.125 rpm
Byte[6]:    0x04 (고정값)
```

#### EPSO1 (RX: `0x18FF0513`) — EPS 상태 피드백

```
Byte[0..1]: 실제 조향각 (uint16, Little-Endian)
            변환: angle = raw × 0.1 - 3212.7
            단위: deg

Byte[2..3]: 조향각 속도 (uint16, Little-Endian)
            변환: speed = raw × 0.1 - 3212.7
            단위: deg/s

Byte[6]:    [3:0] System State (EPS_Sys_State enum)
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
```

#### EPSO2 (RX: `0x18FF0613`) — 토크 및 모드 피드백

```
Byte[0..1]: 드라이버 토크 (uint16, Little-Endian)
            변환: torque = raw × 0.000976562 - 31.374
            단위: Nm

Byte[2..3]: 모터 토크 (uint16, Little-Endian)
            변환: torque = raw × 0.000976562 - 31.374
            단위: Nm

Byte[5]:    [3:0] 현재 제어 모드 (EPS_Mode)
            [7:4] 진동 활성화 상태 (0: Off, 1: On)
```

#### ESC1 (RX: `0x18F00B13`) — 실제 휠 각도

```
Byte[0..1]: 실제 내부 휠 각도 (uint16, Little-Endian)
            변환: angle = raw × 0.00390625 - 125.0
            단위: deg
```

### CRC 알고리즘

#### CRC-8 WCDMA (EPSI1, EPSI2, EPSI3에 사용)

```
다항식(Polynomial): 0x9B
초기값: (CAN_ID의 하위 8비트) + 1
입력 데이터: Byte[0] ~ Byte[6] (7바이트)
최종 처리: 비트 반전 (~crc)
```

[`calc_crc_8bit_wcdma()`](inc/comm_pcan.hpp:368) 함수에서 구현되어 있으며, 각 EPSI 메시지의 Byte[7]에 CRC 값이 기록됩니다.

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

|          키          |                동작                   |
|----------------------|--------------------------------------|
|     `↑` / `↓`        |         메뉴 항목 이동                 |
|     `Enter`          |         선택 항목 실행                 |
|       `Q`            | 프로그램 종료 / 서브메뉴에서 메인으로 복귀 |
| `↑` / `↓` (서브메뉴)  | 차량 속도 증감 또는 진동 레벨 증감        |
| 숫자 입력 + `Enter`   |        조향각/토크 값 설정              |

---

## 📜 라이선스

이 프로그램은 **LGPL-2.1** 라이선스를 따릅니다.

PCAN Basic 관련 코드는 PEAK System-Technik GmbH의 라이선스를 따릅니다.

```
Copyright (C) 2026 ADUS Inc., All rights reserved.
Copyright (C) 2001-2020 PEAK System-Technik GmbH <www.peak-system.com>
```
