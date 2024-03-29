# WiFiBridge STM32 Example
STM32 NUCLEO-F103RB 보드와 [WiFiBridge](https://www.devicemart.co.kr/goods/view?no=14823642) 모듈을 사용한 예제입니다.
![nucleo_wifibridge](https://user-images.githubusercontent.com/67400790/201620603-8b83b9f2-3995-438e-9ac2-6e5421b81964.png)
<br>
<br>

## 개요
WiFiBridge 모듈의 **자동** 또는 **수동**모드 사용에 대한 간단한 예제이다.
<br>
<br>

### 자동모드(기본 동작)
WiFiBridge 모듈의 **자동/AP 모드**를 사용해 시리얼 장치(STM32)를 무선 Station 장치 연결을 대기한다.

연결이 완료된 후 Socket 데이터를 시리얼 장치(STM32)에 별도의 명령없이 송수신시켜준다(Brdige 동작).

연결된 이후에는 무선 장치와 시리얼 장치간 데이터를 손쉽게 전송할 수 있다.
<br>
<br>

### 수동모드
WiFiBridge와 연결되는 UART 데이터, PC와 연결되는 UART 데이터를 bypass 시켜준다.

별도의 USB 컨버터 없이 WiFiBridge 모듈의 Command를 사용할 수 있다.

B1 Switch를 누를 경우, WiFiBridge 모듈의 CTS핀에 출력을 인가해 모듈을 리셋한다.
<br>
<br>


## 결선도
<!-- 결선도 이미지 및 표 추가 -->
![nucleo_wifibridge_conn](https://user-images.githubusercontent.com/67400790/201606356-1285fb1a-d4c6-4aea-8b1f-187a34ef08f8.png)

|NUCLEO-F103RB<br>(Arduino Pin)|WiFiBridge|
|:----------------------------:|:--------:|
|+5V                           |VCC       |
|D2                            |TX        |
|D8                            |RX        |
|D4                            |CTS       |
|D3                            |RTS       |
|GND                           |GND       |

<br>

### Quick Start
보다 상세한 내용은 제품 User Manual을 참고해주세요.
1. WiFiBridge 모듈의 스위치를 **자동/AP 모드**로 구성
2. NUCLEO 보드와 WiFiBridge 연결
3. NUCLEO 보드 PC 연결 및 Terminal 프로그램 연결<br>
(Baud: 115200bps, Data: 8bit, Parity: none, Stop: 1bit, FlowControl: none)
4. 예제 폴더의 Prebuilt_binary 폴더의 bin 파일을 NUCLEO 보드의 가상 드라이브에 복사해 FW를 업데이트 진행.
5. PC에서 'wifibridge_5G' WiFi 검색 후 연결
6. PC에서 Socket Terminal 프로그램 실행 후 TCP Cleint로 실행<br>Server IP: 10.8.6.1, Port: 5001 연결
7. 양방향 데이터 전송
<br>
<br>

### Usage
WiFiBridge의 사용 모드에 따라 소스코드에 선언된 `#define USE_ATUO`의 값을 변경한다.

|USE_AUTO   |사용 모드  |
|:---------:|:---------:|
|0          |Manual 모드|
|1 (default)|Auto 모드  |

- Auto 모드의 경우, RTS 상태를 읽어 WiFiBridge 모듈이 사용 가능할 때 까지 대기한다.

- Manual 모드의 경우, RTS 상태에 관계없이 WiFiBridge UART 데이터를 PC와 연결되는 UART로 bypass 시켜준다.
