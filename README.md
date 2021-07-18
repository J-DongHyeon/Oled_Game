# Oled-Game
oled Game

# 준비물
- 아두이노 UNO 보드 R3
- 적외선 센서, 적외선 송신기 (Car MP3 remote controller)
- Oled (SH1106 SPI 통신 기반 모듈)

적외선 센서 <-> 적외선 송신기 : 적외선 통신

UNO 보드 R3 <-> Oled : SPI 통신

u8glib 라이브러리를 사용하여 Oled에 그림

# 연결 완료 사진

<img width="471" alt="2021-07-18 22;59;03" src="https://user-images.githubusercontent.com/86474141/126070102-5cddf993-82ab-41b4-a8d0-9e4b2ae5b4e1.PNG">

# 회로도

<img width="700" alt="2021-07-17 19;52;14" src="https://user-images.githubusercontent.com/86474141/126034769-8b9c1954-091c-497f-892f-857ecae49f5f.PNG">

# 게임 플레이 영상

![ezgif com-gif-maker](https://user-images.githubusercontent.com/86474141/126072594-909f29d7-0fe1-4cb9-9153-c00f8f63c711.gif)


# 조작 방법
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

[![Hits](https://hits.seeyoufarm.com/api/count/incr/badge.svg?url=https%3A%2F%2Fgithub.com%2FJ-DongHyeon%2FOled_Game.git&count_bg=%2379C83D&title_bg=%23555555&icon=grav.svg&icon_color=%23E7E7E7&title=hits&edge_flat=false)](https://hits.seeyoufarm.com)
