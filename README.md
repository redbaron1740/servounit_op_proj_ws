# Servounit Test Program

## 📋 프로젝트 개요

Servounit Test Program은 Linux 환경에서 PEAK PCAN USB 어댑터를 통해 EPS(Electric Power Steering) Servounit과 CAN 통신을 수행하는 콘솔 기반 테스트 프로그램입니다.

ncurses 기반의 인터랙티브 TUI(Text User Interface)를 제공하며, 사용자가 실시간으로 EPS 제어 명령을 전송하고 피드백 데이터를 모니터링할 수 있습니다.

## ✨ 주요 기능

|              Function                |                           Descritions                                          |
|--------------------------------------|--------------------------------------------------------------------------------|
| **CAN device control**               | Initalize, start, stop of PCAN USB device                                      |
| **Steering control**                 | Steer Angle Control Mode (-850.0° ~ 850.0°)                                    |
| **Feedback-free torque control**     | Feedback-free Torque Control Mode (-2.0 ~ 2.0 Nm)                              |
| **Motor torque control**             | Motor Torque Control Mode (-2.0 ~ 2.0 Nm)                                      |
| **Vibration mode control**           | Select Vibration Level 0~9                                                     |
| **Setup the vehicle info**           | Setup a In-vehicle speed, lat/long acceleration, Yaw Rate, control param       |
| **Monirotring EPS data in realtime** | Steering angle, steering angle speed, driver wheel torq, motor torq, etc.      |

## 📁 프로젝트 구조

```
servounit_op_proj_ws/
├── CMakeLists.txt           # CMake 빌드 설정 파일
├── README.md                # 프로젝트 문서
├── GITHUB_GUIDE.md          # GitHub 사용 가이드
├── .gitignore               # Git 제외 파일 목록
├── inc/
│   ├── comm_pcan.hpp        # PCAN 통신 매니저 클래스 (TX/RX 스레드, CAN 파싱)
│   └── menu_display.hpp     # ncurses 기반 메뉴 UI 클래스
├── src/
│   └── main.cpp             # 메인 프로그램 (메뉴 로직, EPS 제어 로직)
└── build/                   # CMake 빌드 출력 디렉토리
```

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
# PCAN-Basic 다운로드 및 설치
wget https://www.peak-system.com/fileadmin/media/linux/files/PCAN-Basic.tar.gz
tar -xzf PCAN-Basic.tar.gz
cd PCAN-Basic_Linux-*
make
sudo make install
```

또는 PEAK Systems 공식 웹사이트에서 최신 버전 다운로드:
- https://www.peak-system.com/PCAN-USB.199.0.html?&L=1

## 🚀 빌드 및 실행

### 빌드 방법

```bash
# 프로젝트 디렉토리로 이동
cd servounit_op_proj_ws

# 빌드 디렉토리 생성
mkdir -p build
cd build

# CMake 설정 및 빌드
cmake ..
make
```

빌드가 성공하면 `build/` 디렉토리에 실행 파일이 생성됩니다:
```
servounit_op_proj_ws/build/servounit_test_program
```

### 클린 빌드

```bash
cd servounit_op_proj_ws
rm -rf build
mkdir build
cd build
cmake ..
make
```

### 실행 방법

```bash
# build 디렉토리에서 실행
cd build
sudo ./servounit_test_program

# 또는 프로젝트 루트에서 실행
sudo ./build/servounit_test_program
```

**주의**: PCAN 디바이스 접근을 위해 `sudo` 권한이 필요할 수 있습니다.

## 📊 CMake 빌드 설정

| 항목 | 값 |
|------|-----|
| **Project name** | 'Servounit Test Program' |
| **Standard C++** | C++17 (g++17) |
| **Compile flags** | '-Wall -Wextra' |
| **Execution file** | 'servounit_test_program' |
| **Include path** | 'inc/', '/usr/include' |
| **Link Library** | 'libpcanbasic.so', 'ncurses', 'pthread' |

## 🎮 사용 방법

### 메인 메뉴

프로그램을 실행하면 다음과 같은 메인 메뉴가 표시됩니다:

```
Servounit Test Program
========================================
  1. Open can device
  2. Set angle value for steering control
  3. Set Feedback-free torque value for steering control(-850~850)
  4. Set Motor torque value for steering control(-20~20)
  5. Steer Vibration mode on
  6. Set another Value (vehicle values, etc.)
  7. About Program
  Quit
========================================
Arrow Up/Down: Move | Enter: Select | Q: Quit
```

### 기본 조작법

|        key                |                      Function                         |
|---------------------------|-------------------------------------------------------|
| **↑ / ↓**                 | Scroll arrow up/down thorough main menu               |
| **Enter**                 | Run selected items                                    |
| **Q**                     | Quit program / Back to main menu                      |
| **↑ / ↓ (Submenu)**       | Press up/down arrow keys to adjust vehicle speed      |
| **Input number + Enter**  | Adjust the steering angle and torque value            |



### 1. CAN 디바이스 열기 (메뉴 1번)

EPS와 통신하기 위해 먼저 PCAN 디바이스를 초기화해야 합니다.

1. 메인 메뉴에서 `1. Open can device` 선택
2. 디바이스가 성공적으로 열리면 TX/RX 스레드가 자동으로 시작됩니다
3. 실시간 CAN 메시지 송수신이 시작됩니다

### 2. 조향각 제어 모드 (메뉴 2번)

조향각을 직접 제어하는 모드입니다.

- **제어 범위**: -850.0° ~ 850.0°
- **제어 모드**: Steer Angle Control Mode
- **입력 방법**: 숫자 입력 후 Enter
- **차량 속도 조절**: ↑/↓ 키로 실시간 조절

**동작 순서**:
1. EPS 모드: Off → Ready → Steer Angle Control Mode
2. 원하는 조향각 입력 (예: 45.5)
3. Enter 키로 명령 전송
4. 실시간 EPS 피드백 데이터 모니터링

### 3. Feedback-free 토크 제어 (메뉴 3번)

피드백 없는 토크 제어 모드입니다.

- **제어 범위**: -2.0 ~ 2.0 Nm
- **제어 모드**: Feedback-free Torque Control Mode
- **입력 방법**: 숫자 입력 후 Enter (음수 가능)
- **차량 속도 조절**: ↑/↓ 키로 실시간 조절

### 4. 모터 토크 제어 (메뉴 4번)

모터 토크를 직접 제어하는 모드입니다.

- **제어 범위**: -2.0 ~ 2.0 Nm
- **제어 모드**: Motor Torque Control Mode
- **입력 방법**: 숫자 입력 후 Enter (음수 가능)
- **차량 속도 조절**: ↑/↓ 키로 실시간 조절

### 5. 진동 모드 (메뉴 5번)

조향 핸들 진동 기능을 활성화하고 레벨을 조절합니다.

- **진동 레벨**: 0 ~ 9 (0: 최소, 9: 최대)
- **조절 방법**: ↑/↓ 키로 레벨 증감
- **종료**: Q 키로 진동 모드 비활성화 및 메인 메뉴 복귀

### 6. 차량 데이터 설정 메뉴 (메뉴 6번)

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

|           Item                    |     Scope    | Unit   |           Description                            |
|-----------------------------------|--------------|--------|--------------------------------------------------|
| **In-vehilce speed**              | 0 ~ 250      | kph    | Current speed of vehicle                         |
| **Relative wheel speed**          | -7 ~ 7       | kph    | Differential wheel speed of front left and right |
| **Yaw Rate**                      | -3.9 ~ 3.9   | rad/s² | Z-axis rotational accel of vehicle               |
| **Lateral accel (x axis)**        | -15 ~ 15     | m/s²   | Lateral acceleration                             |
| **Longitudinal accel (y axis)**   | -15 ~ 15     | m/s²   | Longitudianl acceleration                        |
| **Return Weight**                 | 0 ~ 100      | %      | Steering self-aligning torque weight value       |
| **Angle Weight**                  | 0 ~ 100      | %      | Steering angle control weight value              |
| **Torque Limit**                  | 0 ~ 10       | Nm     | Threshold value of steering torque               |

## 🏛️ 아키텍처

### 클래스 구조

#### 1. PCANManager 클래스 ([`comm_pcan.hpp`](inc/comm_pcan.hpp))

PCAN 디바이스 관리 및 CAN 통신을 담당하는 핵심 클래스입니다.

**주요 메서드**:
- `device_open_()`: PCAN 디바이스 초기화
- `device_close_()`: PCAN 디바이스 종료
- `device_start_()`: TX/RX 스레드 시작
- `device_stop_()`: TX/RX 스레드 종료
- `getRxData_eps()`: EPS 수신 데이터 조회
- `setCmdData_eps()`: EPS 제어 명령 설정

**스레드 구조**:
- **RX Thread**: CAN 메시지 수신 및 파싱 (10ms 주기)
- **TX Thread**: CAN 메시지 송신 (10ms 주기)

**송신 CAN 메시지**:
- `EPSI1 (0x18FF00FE)`: 조향각, 모터 토크, Feedback-free 토크, 제어 모드
- `EPSI2 (0x18FF01FE)`: Return weight, Angle weight, 토크 제한값
- `EPSI3 (0x18FF02FE)`: 진동 활성화, 진동 레벨
- `EBC2 (0x18FEBFFE)`: 차량 속도, 휠 속도
- `VDC2 (0x18F009FE)`: Yaw rate, 가속도
- `EEC1 (0xCF004FE)`: 엔진 속도
- `TCO1 (0xCFE6CFE)`: 차량 속도 (추가)
- `HRW (0x8FE6EFE)`: 휠 속도 정보
- `CCVS1 (0x18FEF100)`: 차량 속도 정보
- `TD (0x18FEE6FE)`: 시간 정보 (1초 주기)
- `VDHR (0x18FEC1FE)`: 차량 데이터

**수신 CAN 메시지**:
- `EPSO1 (0x18FF0513)`: 조향각, 조향각 속도, 시스템 상태
- `EPSO2 (0x18FF0613)`: 드라이버 토크, 모터 토크, 제어 모드, 진동 상태
- `ESC1 (0x18F00B13)`: 실제 내륜 조향각

**CRC 계산**:
- `calc_crc_8bit_wcdma()`: WCDMA 방식 8비트 CRC 계산 (Polynomial: 0x9B)

#### 2. Menu_select_instance 클래스 ([`menu_display.hpp`](inc/menu_display.hpp))

ncurses 기반 메뉴 UI를 제공하는 클래스입니다.

**주요 메서드**:
- `display()`: 메뉴 화면 렌더링
- `run()`: 메뉴 입력 처리 및 선택 반환

**UI 기능**:
- 화살표 키로 메뉴 항목 순환 이동
- 선택된 항목 강조 표시 (A_REVERSE)
- 도움말 및 저작권 정보 표시

#### 3. 메인 프로그램 ([`main.cpp`](src/main.cpp))

전체 프로그램 흐름을 제어하고 사용자 입력을 처리합니다.

**주요 함수**:
- `main()`: 프로그램 진입점, 메인 메뉴 루프
- `setMenu_ctrl_steer_angle()`: 조향각 제어 서브메뉴
- `setMenu_ctrl_free_torque()`: Feedback-free 토크 제어 서브메뉴
- `setMenu_ctrl_motor_torque()`: 모터 토크 제어 서브메뉴
- `setMenu_ctrl_vibration()`: 진동 모드 제어 서브메뉴
- `update_another_data()`: 차량 데이터 설정 서브메뉴
- `updateEPSData()`: EPS 데이터 화면 업데이트
- `showAbout()`: 프로그램 정보 표시

### EPS 제어 모드

| 모드 값 | 모드 이름 | 설명 |
|---------|-----------|------|
| 0 | Off | EPS 시스템 꺼짐 |
| 1 | Ready | EPS 시스템 준비 상태 |
| 2 | Feedback-free Torque Control | 피드백 없는 토크 제어 |
| 3 | Motor Torque Control | 모터 토크 제어 |
| 4 | Steer Angle Control | 조향각 제어 |
| 5 | Cancel | 제어 취소 |

### EPS 시스템 상태

| 상태 값 | 상태 이름 | 설명 |
|---------|-----------|------|
| 0 | Undefined | 정의되지 않음 |
| 1 | NMWait | 네트워크 관리 대기 |
| 2 | Terminal30Watt | 터미널 30 대기 |
| 3 | PreDrive | 드라이브 준비 |
| 4 | DriveDown | 드라이브 다운 |
| 5 | DriveUp | 드라이브 업 |
| 6 | PostRun | 드라이브 후 실행 |
| 7 | Off | 꺼짐 |
| 8 | Error | 오류 |
| 9 | Flash | 플래시 모드 |
| 10 | LowVoltage | 저전압 |

## 🔍 CAN 통신 프로토콜

### CAN 설정
- **디바이스**: PCAN_USBBUS1
- **Baudrate**: 500 kbps
- **메시지 타입**: Extended (29-bit ID)

### 데이터 변환 공식

#### 송신 데이터
- **조향각**: `value = (angle + 3212.7) * 10` (0.1° 단위)
- **토크**: `value = (torque + 31.374) * 1024` (0.000976562 Nm 단위)
- **차량 속도**: `value = speed * 256` (1/256 kph 단위)
- **Yaw Rate**: `value = (yawrate + 3.92) * 8192`
- **가속도 X**: `value = (accelX + 15.687) * 2048`
- **가속도 Y**: `value = (accelY + 12.5) * 10`

#### 수신 데이터
- **조향각**: `angle = value * 0.1 - 3212.7` (도)
- **토크**: `torque = value * 0.000976562 - 31.374` (Nm)
- **내륜 조향각**: `angle = value * 0.00390625 - 125.0` (도)

## 🛠️ 문제 해결

### PCAN 디바이스를 찾을 수 없음

```
No pcan-device connected!! Please, check pcan-device
```

**해결 방법**:
1. PCAN USB 어댑터가 올바르게 연결되어 있는지 확인
2. PCAN 드라이버가 설치되어 있는지 확인
   ```bash
   lsmod | grep pcan
   ```
3. 디바이스 권한 확인
   ```bash
   ls -l /dev/pcan*
   sudo chmod 666 /dev/pcanusb*
   ```

### ncurses 라이브러리 오류

```
fatal error: ncurses.h: No such file or directory
```

**해결 방법**:
```bash
sudo apt-get install libncurses5-dev libncursesw5-dev
```

### 빌드 오류: PCANBasic.h not found

**해결 방법**:
1. PCAN-Basic API 설치 확인
2. 헤더 파일 경로 확인
   ```bash
   find /usr -name "PCANBasic.h"
   ```
3. CMakeLists.txt의 include 경로 수정

## 📜 라이선스

이 프로그램은 **LGPL-2.1** 라이선스를 따릅니다.

**포함된 라이선스**:
- PCAN Basic 관련 코드: LGPL-2.1 (PEAK System-Technik GmbH)
- 프로젝트 코드: LGPL-2.1

자세한 내용은 소스 파일의 헤더를 참조하세요.

## 📞 문의

프로젝트 관련 문의사항이나 버그 리포트는 GitHub Issues를 통해 제출해 주세요.

- **개발자**: ADUS Inc.
- **라이선스**: LGPL-2.1
- **버전**: 1.0
- **Copyright**: © 2026 ADUS Inc., All rights reserved.

## 🔗 참고 자료

- [PEAK System PCAN-USB](https://www.peak-system.com/PCAN-USB.199.0.html)
- [PCAN-Basic API Documentation](https://www.peak-system.com/produktcd/Pdf/English/PCAN-Basic_API_en.pdf)
- [ncurses Programming Guide](https://tldp.org/HOWTO/NCURSES-Programming-HOWTO/)

---

**마지막 업데이트**: 2026-03-09
