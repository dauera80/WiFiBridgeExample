# WiFiBridge STM32 Example
STM32 NUCLEO-F103RB 보드와 [WiFiBridge]() 모듈을 사용한 예제입니다.

<!-- 모듈 이미지 추가 필요
<a href="http://www.kyobobook.co.kr/product/detailViewKor.laf?ejkGb=KOR&mallGb=KOR&barcode=9791165392659&orderClick=LEa&Kc="><img src="https://image.yes24.com/goods/90611902/800x0" border="0" width="70%"></a>
-->

<!-- STM32 폴더에 내용 추가
# 개발 환경 및 실습 보드
|IDE             |Target board         |External board        |FW version     |
|:--------------:|:-------------------:|:--------------------:|:-------------:|
|<center>[STM32CubeIDE 1.7.0](https://www.st.com/content/st_com/en/products/development-tools/software-development-tools/stm32-software-development-tools/stm32-ides/stm32cubeide.html)</center>|<center>[NUCLEO64](https://www.devicemart.co.kr/goods/view?no=1346033)</center>|<center>[NUCLEOEVB](https://www.devicemart.co.kr/goods/view?no=12545343)</center>|F1_V1.8.0
|<img src="https://encrypted-tbn0.gstatic.com/images?q=tbn%3AANd9GcRGhTJJ8gbDkFXe_0Md4uHzcOrr558cVxnCbw&usqp=CAU" border="0" width="200">|<img src="https://user-images.githubusercontent.com/67400790/86118039-0cae3800-bb0b-11ea-85ba-a246ab6d0b4b.png" border="0" width="100">|<img src="https://user-images.githubusercontent.com/67400790/86117881-c953c980-bb0a-11ea-8c28-f9621f89737b.jpg" border="0" width="200">
  
* NUCLEO64 및 NUCELOEVB 보드 이미지
<img src="https://user-images.githubusercontent.com/67400790/86118831-6105e780-bb0c-11ea-80d1-72107f9bb4ff.jpg" border="0" width="85%">  
-->

## 결선도
<!-- 결선도 이미지 및 표 추가 -->

## Quick Start
메뉴얼 참고? 다시 작성?

예제 폴더의 Prebuilt_binary 폴더의 bin 파일을 NUCLEO 보드의 가상 드라이브에 복사해 FW를 업데이트

## 예제 목록
### 1. WiFiBridgeSimpleExample
WiFiBridge 모듈에 대한 간단한 사용 예제.

WiFiBridge와 연결되는 UART 데이터, PC와 연결되는 UART 데이터를 bypass 시켜준다.

기본 동작모드는 WiFiBridge 모듈의 **자동/AP 모드**를 사용해 시리얼 장치(STM32)와 무선 Station 장치를 연결.

WiFiBridge 모듈의 RTS핀을 읽어 Socket 사용 가능 상태를 판별하고, 사용 가능할 경우 WiFiBridge 모듈의 데이터를 bypass 시켜준다.

B1 Switch를 누를 경우, WiFiBridge 모듈의 CTS핀에 출력을 인가해 모듈을 리셋한다.

#define USE_AUTO에 따라 다름.

USE_AUTO를 0로 설정하면 데이터를 bypass 시켜준다.
