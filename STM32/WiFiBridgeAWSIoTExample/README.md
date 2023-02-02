# WiFiBridge AWSIoT Example
NUCLEO-F103RB + [NUCLEOEVB](https://www.devicemart.co.kr/goods/view?no=12545343) 보드와 [WiFiBridge](https://www.devicemart.co.kr/goods/view?no=14823642) 모듈을 사용해 AWS IoT에 데이터를 전송하고 제어하는 예제입니다.
![nucleo_wifibridge](https://user-images.githubusercontent.com/67400790/214981020-27a1f8ea-fdce-4403-820d-25440a8c046e.png)
<br>
> WiFiBridge V2.0부터 AWS IoT 기능이 지원됩니다.

<br>

## 개요
NUCLEO + NUCLEOEVB 보드 및 WiFiBridge 모듈을 사용해 다양한 센서 데이터를 클라우드(AWS IoT)로 전송하고, 원격으로 모니터링 및 제어가 가능하다. 

AWS IoT 기능을 사용하기 위해선 Command 사용이 필요하며, Bridge 동작과 동시에 사용이 불가능하다.

따라서 AWS IoT를 사용하기 위해선 WiFiBridge의 스위치를 **수동모드**로 설정해 사용해야 한다.
<br>
<br>

## 이미지
<!-- 결합 이미지 추가 -->
![NUCLEOEVB_FullAccessory_V2](https://user-images.githubusercontent.com/67400790/216243724-ae03a490-43a2-466e-a148-832d5c60e436.png)


## Topic / Message
NUCLEOEVB 보드에서 수집한 데이터 목록, 각 주제별 게시(Publish)

|    Topic      |        Message       |                     Message 설명                |
|:-------------:|:--------------------:|:----------------------------------------------:|
|EVBDemo/var	|가변저항 ADC 값        |0 ~ 4095                                        |
|EVBDemo/photo	|Photo Sensor ADC 값   |0 ~ 4095                                         |
|EVBDemo/temp	|TC1047 온도	       |[℃], 소수점 1자리                                |
|EVBDemo/3axis	|3축(x,y,z) 중력 가속도	|[g], 소수점 2자리<br>각 항목은 ‘,’로 구분          |
|EVBDemo/encoder|Encoder Counter 값	   |                                                 |
|EVBDemo/bh002	|BH-002-00 온도 및 습도 |[℃] [%], 온도 소수점 1자리<br>각 항목은 ‘,’로 구분|
|EVBDemo/switch	|Switch 1~4 상태	   |0:None, 1: Push<br>각 항목은 ‘,’로 구분           |


NUCLEOEVB 보드 제어 목록, 각 주제별 구독(Subscribe)

|      Topic       |        Message        |                     Message 설명                |
|:----------------:|:---------------------:|:-----------------------------------------------:|
|EVBDemo/ServoMotor|서보모터 회전 각도       |-90 ~ 90 [degree]                                 |
|EVBDemo/LED	   |LED 1~4 상태	        |0: OFF, 1: ON                                   |
|EVBDemo/RGB	   |RGB LED PWM 값	        |0 ~ 255 <br>각 항목은 ‘,’로 구분                 |
|EVBDemo/DAC       |DAC 출력값              |0 ~ 4095                                        |
|EVBDemo/CLCD	   |1602 CLCD에 출력할 텍스트|2개 라인에 출력할 텍스트는 쌍따옴표(“)로 묶음. <br>각 항목은 ‘,’로 구분|

<br>

## Quick Start
보다 상세한 내용은 제품 User Manual을 참고해주세요.
1. WiFiBridge 모듈의 스위치를 **수동 모드**로 구성
2. NUCLEO, NUCLEOEVB 보드와 WiFiBridge 연결 (커넥터 연결 주의)
3. NUCLEO 보드 PC 연결 및 Terminal 프로그램 연결<br>
(Baud: 115200bps, Data: 8bit, Parity: none, Stop: 1bit, FlowControl: none)
4. 예제 프로젝트에서 AP, AWS IoT 연결 정보, 인증서 및 키 변경
5. 프로젝트 빌드 및 실행
6. AWSIoT 콘솔의 MQTT 테스트 클라이언트로 데이터 확인 및 제어

**모니터링 및 제어 용 MQTT Client 앱 추후 업데이트 예정**
<br>

## Usage
소스코드에 선언된 `#define WIFIBRIDGE_DEBUG` 및 `#define WIFIBRIDGE_BYPASS`를 변경해 Debug Message를 활성화 시키거나 WiFiBridge UART 데이터를 PC와 연결되는 UART로 bypass 시킬 수 있다.

|WIFIBRIDGE_DEBUG|      설명          |
|:--------------:|:------------------:|
|0 (default)     |Debug 메시지 비활성화|
|1               |Debug 메시지 활성화  |

|WIFIBRIDGE_BYPASS|           설명                  |
|:---------------:|:-------------------------------:|
|0 (default)      |기본 예제 동작(센서 데이터 게시)   |
|1                |AWS IoT 연결 후 UART 데이터 Bypass|

- Bypass 사용 시 B1 Switch를 누를 경우, WiFiBridge 모듈의 CTS핀에 출력을 인가해 모듈을 리셋한다.
