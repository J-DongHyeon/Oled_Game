
<h1 align="center"> OLED GAME </h1>

<h4 align="center"> Avoiding Obstacles </h4>
</br>

<p align= "center">
<img src=/docs/readme_Imgs/readme_img1.PNG width=500 height=350></p> 
</br></br>

------------------------------------------

# 1. 기획 계기

<p align= "center">
<img src=/docs/readme_Imgs/readme_img2.PNG width=500 height=350></p> 

> &nbsp;`Arduino UNO 보드` 를 이용하여 여러 센서들을 테스트 해보던 중, `OLED 모듈` 을 연결하여 테스트 할 때 흥미로웠다. `U8glib.h` 라이브러리를 이용함으로써 OLED에 내가 원하는 문양, 텍스트들이 곧바로 출력되는 것을 보니 신기했고, 시간을 들여 하나의 프로젝트를 만들면 재미있겠다는 생각이 들었다. <br><br>
> &nbsp;OLED 모듈 사용법을 익힐 겸, 재미있는 프로젝트를 진행할 겸 `나만의 OLED GAME` 을 만들기 위한 구상을 하였고, 이를 구현하기 위해 본 프로젝트를 진행하였다.
<br><br>

------------------------------------------

# 2. 사용부품 & 개발환경

- ## 사용부품

<p align= "center">
<img src=/docs/readme_Imgs/readme_img3.PNG width=700 height=170></p>

-
  - Arduino UNO Board
  - IR Sensor
  - IR Remote Control
  - OLED Display
<br>

- ## 개발환경

<p align= "center">
<img src=/docs/readme_Imgs/readme_img4.PNG width=700 height=170></p>

-
  - OLED에 draw를 하기 위한 그래픽 라이브러리로 `u8glib` 을 사용하였다.
  - Bitmap 파일을 HEX Array 타입 파일로 변환시켜주는 Tool로 `LCD Assistant` 를 사용하였다.
<br><br>

------------------------------------------

# 3. 회로도

<img width="471" alt="2021-07-18 22;59;03" src="https://user-images.githubusercontent.com/86474141/126070102-5cddf993-82ab-41b4-a8d0-9e4b2ae5b4e1.PNG">

<img width="537" alt="회로도" src="https://user-images.githubusercontent.com/86474141/126897813-1ae3a291-f495-41cb-ab88-cad2370c5893.PNG">

적외선 센서 <-> 적외선 송신기 : 적외선 통신

UNO 보드 R3 <-> Oled : SPI 통신

u8glib 라이브러리를 사용하여 Oled에 그림


### 게임 플레이 영상

![ezgif com-gif-maker](https://user-images.githubusercontent.com/86474141/126072594-909f29d7-0fe1-4cb9-9153-c00f8f63c711.gif)


### 조작 방법
- 좌 우 양 옆에서 장애물들이 출현합니다.
- 리모컨 (적외선 송신기) 으로 상, 하, 좌, 우 키를 누름으로써 캐릭터를 움직이고 장애물을 피합니다.
- 4키 : 좌, 6키 : 우, 2키 : 상, 8키 : 하, 5키 : ok!
- ok키를 누르면 총알이 나가고 장애물을 제거할 수 있습니다.
- 총알은 연속으로 최대 4개 발사 가능합니다. (개수 제한 없음)
- 게임 종료 시 버틴 기록이 나옵니다. (장애물 킬 수 * 1) 초가 버틴 기록에 추가됩니다.
- 기록이 1 ~ 6 위 사이에 해당하면 EEPROM에 저장됩니다.
- 게임 순위는 'scores' 옵션에서 확인 가능합니다.
- 'option' 옵션에서는 게임 난이도 설정과 게임 기록 리셋이 가능합니다.
- 게임 난이도를 올릴수록 게임 플레이 속도가 빨라집니다.
