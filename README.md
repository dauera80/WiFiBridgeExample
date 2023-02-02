# WiFiBridge Example
이 깃허브는 [WiFiBridge](https://www.devicemart.co.kr/goods/view?no=14823642) 모듈 사용 예제를 담고 있습니다.

<a href="https://www.devicemart.co.kr/goods/view?no=14823642"><img src="https://user-images.githubusercontent.com/67400790/201601309-cd74a7be-c835-4578-9765-07a0821b2524.png" border="10" width="70%"></a>
<br>
<br>

## Module Description
![wifibridge_pin](https://user-images.githubusercontent.com/67400790/201601574-8290b2ca-ffde-4c62-b4ed-3c66418d3b12.png)

- Pin Header<br>

|Pin Name|Description|
|:------:|:---------:|
|VCC|전원 5V 입력|
|TX|UART 출력 (3.3V or 5V)|
|RX|UART 입력 (3.3V or 5V)|
|CTS|Software Reset<br>(Falling Edge Trigger)|
|RTS|Socket 연결 상태 출력<br>(Active 0V, Inactive 5V)|
|GND| 전원 Ground|

> CTS 및 RTS pin은 부팅이 완료된 후 사용 가능<br>
> (HW Reset에 의한 부팅 시간 약 2초 소요)

<br>

- Switch Mode<br>

|SW1|SW2|Mode|Description|
|:---:|:---:|:---:|:---:|
|OFF|OFF|수동|부팅 후 수동 모드 진입<br>SW2 상태 무시|
|OFF|ON|수동|부팅 후 수동 모드 진입<br>SW2 상태 무시|
|ON|OFF|자동<br>STA|부팅 후 자동, STA 모드 동작<br>설정된 Configuration Mode 무시|
|ON|ON|자동<br>AP|부팅 후 자동, AP 모드 동작<br>설정된 Configuration Mode 무시|

<br>

- LED Indicator<br>

|LED|Boot State|Operation State|
|:---:|:---:|:---|
|Blue|ON: SW1 ON<br>OFF: SW1 OFF|ON: WLAN(WiFi) 연결 성공<br>Blink: WLAN(WiFi) 연결 시도 또는 대기<br>OFF: WLAN(WiFi) 연결 안됨|
|Red|ON: SW2 ON<br>OFF: SW2 OFF|ON: 에러 발생<br>OFF: 정상|
|Green|ON: 전원 ON<br>OFF: 전원 OFF|ON: Socket 연결 성공<br>Blink: Socket 연결 시도 또는 대기<br>OFF: Socket 연결 안됨|

> Boot State는 부팅 후 1초간 표시 됨.

<br>
<br>

## STM32 예제
STM32를 사용한 WiFiBridge 사용 예제
[[README]](https://github.com/dauera80/WiFiBridgeExample/tree/master/STM32)  

1. WiFiBridgeSimpleExample [[예제]](https://github.com/dauera80/WiFiBridgeExample/tree/master/STM32/WiFiBridgeSimpleExample)  
2. WiFiBridgeAWSIoTExample [[예제]](https://github.com/dauera80/WiFiBridgeExample/tree/master/STM32/WiFiBridgeAWSIoTExample)  
