
<h1 align="center"> OLED GAME </h1>

<h4 align="center"> Avoiding Monsters </h4>
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

- ## 회로도 구상
<p align= "center">
<img width=450 height=300 src="https://user-images.githubusercontent.com/86474141/126897813-1ae3a291-f495-41cb-ab88-cad2370c5893.PNG"></p>
</br>

- ## 실제 연결 모습
<p align= "center">
<img width=450 height=300 src="https://user-images.githubusercontent.com/86474141/126070102-5cddf993-82ab-41b4-a8d0-9e4b2ae5b4e1.PNG"></p>
</br>

-
  - Arduino UNO Board <-> OLED : SPI 통신
  - IR Sensor <-> IR Remote Control : 적외선 통신
<br><br>

------------------------------------------

# 4. 게임 설명

- 좌, 우 양 옆에서 몬스터들이 출현한다.
- 리모컨 (적외선 송신기) 으로 상, 하, 좌, 우 키를 누름으로써 캐릭터를 움직이고 몬스터를 피할 수 있다.
  - 2키 : 상
  - 8키 : 하
  - 4키 : 좌
  - 6키 : 우
  - 5키 : ok
- ok 키를 누르면 총알이 나가고 몬스터를 제거할 수 있다.
- 총알은 연속으로 최대 4개 발사 가능하다. (개수 제한 없음)
- 게임 종료 시 버틴 시간 기록이 나온다. (몬스터 킬 수 * 1) 초가 버틴 기록에 추가된다.
- 게임 순위는 `scores 메뉴` 에서 확인 가능하다. 1 ~ 6위 기록들만 나타난다.
- `option 메뉴` 에서는 게임 난이도 설정과 게임 기록 리셋이 가능하다.
- 게임 난이도를 올릴수록 몬스터 출현 속도가 빨라진다.
<br><br>

------------------------------------------

# 5. 게임 플레이 영상

<p align= "center">
<img width=450 height=250 src=https://user-images.githubusercontent.com/86474141/126072594-909f29d7-0fe1-4cb9-9153-c00f8f63c711.gif> </p>
<br><br>

------------------------------------------

# 6. 참고 사이트

#### 📙 https://tony100.tistory.com/entry/%EC%B5%9C%EA%B3%A0%EC%9D%98-%EB%94%94%EC%8A%A4%ED%94%8C%EB%A0%88%EC%9D%B4%EC%9A%A9-%EB%9D%BC%EC%9D%B4%EB%B8%8C%EB%9F%AC%EB%A6%AC-u8glib
#### 📙 https://rottenstar.tistory.com/35
#### 📙 https://www.hardcopyworld.com/?p=1189
