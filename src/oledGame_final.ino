#include <IRremote.h>
#include <U8glib.h>
#include <EEPROM.h>

U8GLIB_SH1106_128X64 u8g(13, 11, 10, 9, 8);

/*
 * Arduino UNO Board 와 OLED는 SPI 통신
 * 13 : SCK
 * 11 : SDA
 * 9 : DC
 * 8 : RST
 */

// IR Sensor는 14번 핀에 연결
IRrecv irrecv(14);
decode_results results;

unsigned long ifrRcv;
int rcvCtrl;

// 몬스터의 총 개수는 22개 이다.
// timeRandom[22] 은 각 몬스터들이 움직이는 시간 간격 정보를 가지고 있다. (각 몬스터들의 속도는 모두 다르다.)
// ex) timeRandom[0] 이 30 ms 라면, 0 번째 몬스터는 30ms 마다 움직이는 것이다.
// timeRandom[0] 이 가장 빠른 몬스터의 시간 정보를 가지고, timeRandom[21] 이 가장 느린 몬스터의 시간 정보를 가진다.
// 시간 값이 작다는 것은 그만큼 몬스터의 속도가 빠르다는 것이다.
// timeLimit 은 각 몬스터의 timeRandom 값 마다 if문 조건을 만족시키기 위한 역할이다.
// timeRandom[0 ~ 10], timeLimit[0 ~ 10] 은 오른쪽 몬스터, timeRandom[11 ~ 21], timeLimit[11 ~ 21] 은 왼쪽 몬스터들에 대한 정보이다.
int timeRandom[22];
unsigned long timeLimit[22];

// 메뉴 화면에 나타나는 총 게임 플레이 시간이다.
int play_time_sec;
int play_time_min = 0;

// 게임 처음 실행 후 로딩 화면에서 게임 캐릭터가 달려가는 모습을 애니메이션 처럼 효과를 내었다.
// load_sct 가 0 ~ 3 까지 반복적으로 돌면서 4개의 캐릭터 모습을 반복적으로 보여준다. 그렇게 함으로써 캐릭터가 달려가는 모습을 애니메이션 처럼 보여준다.
char load_sct = 0;

// 로딩 화면에서 게이지가 0 ~ 100 % 까지 차는 것을 표현하였다.
// load_gage 가 0 ~ 100 정도 까지 증가하고 그에 맞춰 게이지가 차 오른다.
char load_gage = 0;

int yspaR[11]; // 오른쪽에서 나오는 몬스터의 y좌표 정보
int xspaR[11]; // 오른쪽에서 나오는 몬스터의 x좌표 정보
int yspaL[11]; // 왼쪽에서 나오는 몬스터의 y좌표 정보
int xspaL[11]; // 왼쪽에서 나오는 몬스터의 x좌표 정보
int Length = sizeof(timeRandom) / sizeof(timeRandom[0]);

// 게임 난이도는 1 ~ 4 단계가 있다. 난이도가 높아질 수록 몬스터 이동 속도가 빨라진다.
// timeLevel을 낮출수록 몬스터의 이동 속도가 빨라진다.
int timeLevel;

int yCtrl = u8g.getHeight() / 2; // 캐릭터의 y좌표 정보
int xCtrl = u8g.getWidth() / 2; // 캐릭터의 x좌표 정보

int attack_xspa[4]; // 총알의 x좌표
int attack_yspa[4]; // 총알의 y좌표

// 총알은 최대 4개 까지만 연속 발사할 수 있다.
// attack_seek 배열은 각 총알이 현재 발사된 상태인지 아닌지, 오른쪽 방향으로 발사 됬는지 왼쪽 방향으로 발사 됬는지의 정보를 가지고 있다.
// 0이면 발사되지 않은 것, 1이면 오른쪽 방향, 2이면 왼쪽 방향으로 발사된 것이다.
int attack_seek[4];

int attack_size = sizeof(attack_seek)/sizeof(attack_seek[0]);

// 현재 몇 번째 총알이 발사됬는지에 대한 정보를 가진다.
// 0 ~ 3 사이의 값을 가질 것이다.
int attack_idx = -1;

int obs_kill = 0; // 몬스터 킬 수를 의미한다.

char *menuStr[3] = {"STRAT", "OPTION", "SCORES"}; // 메뉴 화면에서 나타나는 문자열
char *optionStr[3] = {"LEVEL", "SCORES RESET", "Back To Menu"}; // 옵션 화면에서 나타나는 문자열
char *scoresResetS[4] = {"Do You Want", "Scores Reset?", "YES", "NO"}; // 리셋 화면에서 나타나는 문자열

int menuCur = 0; // 현재 메뉴 커서 위치 정보를 가진다. (0 ~ 2 사이의 값)
int optionCur = 0; // 현재 옵션 커서 위치 정보를 가진다. (0 ~ 2 사이의 값)
int resetCur = 2; // 현재 리셋 화면의 커서 위치 정보를 가진다. (2 ~ 3 사이의 값, YES 또는 NO)
int level = 1; // 현재 게임 난이도 정보를 가진다. (1 ~ 4 사이의 값)

// 현재 어떤 화면을 보여주고 있는지를 나타내는 flag들 이다.
// ex) reset_state == 1 이면, 현재 리셋 화면을 나타내고 있는 중인 것이다.
int menu_state = 1; // 메뉴 화면 flag
int option_state = 0; // 옵션 화면 flag
int reset_state = 0; // 리셋 화면 flag
int scores_state = 0; // 점수 화면 flag

// timeM_score 는 게임 실행 시, (게임을 킨 순간 ~ 게임 실행 직전) 까지의 시간 정보를 담는다. 이 정보를 이용하여 게임 버틴 시간을 구한다.
// time_score 는 게임 버틴 시간을 second 단위로 저장한다.
// time_totalScore 는 게임 버틴 시간에 (몬스터 킬 수 * 1) second 까지 더한 최종 값을 저장한다.
unsigned long timeM_score, time_score, time_totalScore;

int score_num = 6; // 점수 화면에 나타낼 기록 개수이다. 상위 6개 기록만 나타낼 것이다.
unsigned long total_score_arr[6]; // 6개 점수 기록을 담을 배열

// inGame_sec 은 게임 실행 화면에서 나타나는 second 시간 정보를 가진다.
// inGame_sec_limit 은 1 second 단위로 inGame_sec 을 증가시키기 위한 변수이다.
// inGame_mil 은 게임 실행 화면에서 나타나는 milli second 시간 정보를 가진다. 10 milli second 단위이다.
// inGame_mil_limit 은 10 milli second 단위로 inGame_mil 을 증가시키기 위한 변수이다.
unsigned long inGame_sec = 0;
unsigned long inGame_sec_limit = 0;
unsigned long inGame_mil = 0;
unsigned long inGame_mil_limit = 0;

// 캐릭터의 현재 방향을 의미한다. 1이면 캐릭터가 오른쪽을 보고 있는 것이고, 2이면 왼쪽을 보고 있는 것이다.
int avoid_drt = 1;

// 메뉴 화면에서 왼쪽 캐릭터의 HEX Array 이다.
// 그림판으로 흑백 캐릭터를 그린 후, LCD Assistant 프로그램을 이용하여 비트맵 이미지를 HEX Array 타입으로 반환하였다.
// 사이즈가 크기 때문에 PROGMEM 키워드를 사용하여 SRAM이 아니라 Flash Memory에 저장하였다.
const unsigned char bull[] PROGMEM = {
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFC, 0x00, 0x00, 0x03, 0xFE, 0x00, 0x00, 0x03, 0xFE, 0x00,
0x00, 0x07, 0xFF, 0x00, 0x00, 0x07, 0xFF, 0x00, 0x00, 0x07, 0xFF, 0x00, 0x00, 0x07, 0xFE, 0x00,
0x00, 0x0F, 0xFF, 0x00, 0x00, 0x1F, 0xFF, 0x80, 0x00, 0x3E, 0xF9, 0x80, 0x00, 0x77, 0xB7, 0xC0,
0x00, 0x75, 0xCE, 0xC0, 0x00, 0x7C, 0xD8, 0xC0, 0x00, 0xFC, 0xD8, 0xC0, 0x03, 0xFE, 0x01, 0xC0,
0x06, 0x3E, 0xC9, 0xC0, 0x0C, 0x1E, 0x3B, 0xF8, 0x08, 0x1F, 0x07, 0x48, 0x18, 0x1E, 0x0F, 0xAC,
0x20, 0x1C, 0x0C, 0x84, 0x30, 0x3C, 0x0E, 0x04, 0x20, 0x3C, 0xAE, 0x06, 0x20, 0x7C, 0x4F, 0x0E,
0x39, 0x3C, 0xAF, 0xFC, 0x19, 0x7C, 0x0F, 0xF8, 0x0F, 0xDF, 0x1F, 0xF0, 0x00, 0x1F, 0xFF, 0x00,
0x00, 0x0F, 0xFF, 0x00, 0x00, 0x0F, 0x5E, 0x00, 0x00, 0x0F, 0xFE, 0x00, 0x00, 0x0F, 0xBF, 0x00,
0x00, 0x0F, 0xBF, 0x00, 0x00, 0x0F, 0x9F, 0x00, 0x00, 0x1F, 0x9F, 0x00, 0x00, 0x1F, 0x9F, 0x00,
0x00, 0x1F, 0x9F, 0x00, 0x00, 0x1F, 0x9F, 0x80, 0x00, 0x1F, 0x9F, 0x80, 0x00, 0x3F, 0x9F, 0x80,
0x00, 0x00, 0x00, 0x00, 
};

// 메뉴 화면에서 오른쪽 캐릭터의 HEX Array 이다.
const unsigned char brawl[] PROGMEM = {
0x00, 0x3C, 0x00, 0x00, 0xFF, 0x80, 0x03, 0xFF, 0xC0, 0x07, 0xC3, 0xE0, 0x0F, 0x00, 0xF0, 0x1E,
0x00, 0x78, 0x1C, 0x00, 0x38, 0x38, 0x00, 0x1C, 0x70, 0x00, 0x0C, 0x70, 0x00, 0x0E, 0x60, 0xC0,
0x06, 0x60, 0xE0, 0x06, 0xE0, 0x61, 0x87, 0xE7, 0xB3, 0x07, 0xE7, 0xF7, 0x07, 0xEF, 0xCE, 0x03,
0xEF, 0xC3, 0xC3, 0xEF, 0xC7, 0xC7, 0xE7, 0xC7, 0xE7, 0x77, 0x87, 0xE7, 0x73, 0x37, 0xE6, 0x78,
0x77, 0xEE, 0x7C, 0x33, 0xCE, 0x3C, 0x03, 0x9C, 0x3C, 0x00, 0x3C, 0x1C, 0x00, 0x78, 0x0F, 0x03,
0xF0, 0x0F, 0xE3, 0xE0, 0x07, 0xFF, 0xC0, 0x01, 0xFF, 0x80, 0x00, 0xFE, 0x00, 0x00, 0x00, 0x00
};


// 로딩 화면에서 뛰어가는 캐릭터의 첫 번째 포즈이다.
const unsigned char loading1[] PROGMEM = {
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF0, 0x00,
0x00, 0x01, 0xFC, 0x00, 0x00, 0x03, 0xFC, 0x00, 0x00, 0x03, 0xBE, 0x00, 0x00, 0x03, 0xFE, 0x00,
0x00, 0x07, 0xFE, 0x00, 0x00, 0x0F, 0xFF, 0x00, 0x00, 0x0F, 0xFF, 0x00, 0x00, 0x0F, 0xFE, 0x00,
0x00, 0x7B, 0x03, 0x00, 0x00, 0xD1, 0x01, 0x00, 0x03, 0x9A, 0x31, 0x00, 0x07, 0x9A, 0x1D, 0x00,
0x0F, 0x8E, 0x18, 0x7C, 0x1F, 0x8E, 0x01, 0xFE, 0x1F, 0xD6, 0x33, 0x7E, 0x1F, 0xF3, 0x1A, 0x7E,
0x1F, 0xF3, 0x1A, 0x7E, 0x1F, 0x73, 0x84, 0x7E, 0x1D, 0x91, 0xFC, 0x3C, 0x07, 0x30, 0x70, 0x00,
0x00, 0x30, 0x0A, 0x60, 0x7E, 0xF0, 0x1E, 0x00, 0x7F, 0xB0, 0x27, 0x40, 0x7E, 0x18, 0x30, 0x00,
0x3E, 0x0F, 0x18, 0x00, 0x1E, 0x04, 0x08, 0x00, 0x07, 0x8C, 0x00, 0x00, 0x00, 0xF8, 0x08, 0x00,
0x00, 0x03, 0x0C, 0x00, 0x00, 0x03, 0x07, 0x00, 0x00, 0x04, 0x01, 0x00, 0x00, 0x07, 0xFF, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 
};

// 로딩 화면에서 뛰어가는 캐릭터의 두 번째 포즈이다.
const unsigned char loading2[] PROGMEM = {
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFC, 0x00,
0x00, 0x01, 0xFE, 0x00, 0x00, 0x01, 0xFF, 0x00, 0x00, 0x03, 0xFF, 0x00, 0x00, 0x07, 0xFF, 0x80,
0x00, 0x0F, 0xFF, 0x80, 0x00, 0x1F, 0xFF, 0x80, 0x00, 0x1F, 0xFF, 0x80, 0x00, 0x1F, 0xFF, 0x80,
0x00, 0x3F, 0xFF, 0x80, 0x00, 0x26, 0x00, 0x80, 0x00, 0x26, 0x38, 0x80, 0x00, 0x6E, 0x1E, 0x80,
0x00, 0xFC, 0x0C, 0x80, 0x01, 0x98, 0x00, 0x80, 0x03, 0x08, 0x38, 0x80, 0x06, 0x0C, 0x0D, 0x80,
0x04, 0x26, 0x0D, 0x80, 0x04, 0x12, 0x03, 0x00, 0x06, 0x3F, 0xFF, 0x80, 0x02, 0x3F, 0x07, 0xC0,
0x03, 0x3F, 0x87, 0xC0, 0x01, 0xFF, 0x8F, 0xC0, 0x00, 0x7F, 0x9F, 0x80, 0x00, 0x3F, 0x0F, 0x80,
0x00, 0x1E, 0x0E, 0x00, 0x00, 0x18, 0x48, 0x00, 0x00, 0x30, 0x78, 0x00, 0x00, 0x60, 0xF0, 0x00,
0x00, 0xE0, 0x80, 0x00, 0x00, 0xF0, 0x80, 0x00, 0x01, 0xFF, 0x80, 0x00, 0x03, 0xFF, 0xC0, 0x00,
0x03, 0xFF, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 
};

// 로딩 화면에서 뛰어가는 캐릭터의 세 번째 포즈이다.
const unsigned char loading3[] PROGMEM = {
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFC, 0x00, 0x00, 0x01, 0xFF, 0x00,
0x00, 0x01, 0xFF, 0x00, 0x00, 0x03, 0xFF, 0x00, 0x00, 0x03, 0xFF, 0x80, 0x00, 0x07, 0xFF, 0x80,
0x00, 0x07, 0xFF, 0x40, 0x00, 0x07, 0xFF, 0xC0, 0x00, 0x0F, 0xFF, 0xC0, 0x00, 0x09, 0x00, 0x40,
0x00, 0x39, 0x1C, 0x40, 0x00, 0xFD, 0x07, 0x40, 0x01, 0xC7, 0x06, 0x40, 0x03, 0xC3, 0x00, 0x40,
0x07, 0xE7, 0x1C, 0xC0, 0x07, 0xFF, 0x86, 0x80, 0x07, 0xFD, 0x80, 0xE0, 0x07, 0xF8, 0xF7, 0xF0,
0x03, 0xF8, 0x3F, 0xF0, 0x03, 0xF0, 0x07, 0xF0, 0x07, 0xF0, 0x07, 0xF0, 0x0E, 0x20, 0x07, 0xF0,
0x1E, 0x00, 0x07, 0xE0, 0x3E, 0x00, 0x04, 0x00, 0x7E, 0x03, 0x86, 0x00, 0x7E, 0x04, 0x42, 0x00,
0x3F, 0x18, 0x7E, 0x00, 0x07, 0xB0, 0xFF, 0x00, 0x00, 0xE1, 0xFF, 0x80, 0x00, 0x01, 0xFF, 0x80,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 
};

// 게임 실행 화면에서 캐릭터가 오른쪽을 보고 있을 때의 HEX Array 이다.
const unsigned char avoidR[] PROGMEM = {
0xFF, 0x81, 0x99, 0x8D, 0x8D, 0x81, 0x81, 0xFF, 
};

// 게임 실행 화면에서 캐릭터가 왼쪽을 보고 있을 때의 HEX Array 이다.
const unsigned char avoidL [] PROGMEM = {
0xFF, 0x81, 0x99, 0xB1, 0xB1, 0x81, 0x81, 0xFF, 
};

// 게임 실행 화면에서 오른쪽에서 오는 몬스터의 HEX Array 이다.
const unsigned char obs1 [] PROGMEM = {
0xE0, 0x35, 0xE0, 
};

// 게임 실행 화면에서 왼쪽에서 오는 몬스터의 HEX Array 이다.
const unsigned char obs2 [] PROGMEM = {
0x07, 0xAC, 0x07, 
};

// 게임 실행 화면에서 총알의 HEX Array 이다.
const unsigned char attack_obs [] PROGMEM = {
0xFF,
};

// 게임 실행 화면에서 킬 수 옆에 그려진 해골 모양의 HEX Array 이다.
const unsigned char kill_draw [] PROGMEM = {
0x00, 0xA5, 0x42, 0xA5, 0x00, 0x7E, 0x42, 0x7E, 
};

// 게임 실행 시 OLED 화면에 그리는 함수이다. (게임 실행 화면)
void draw() { 
     u8g.setFont(u8g_font_04b_03);
     
     // 게임 버틴 시간을 나타내기 위한 위치
     u8g.setPrintPos(u8g.getWidth() - u8g.getStrWidth("0000:00"), u8g.getHeight());

     // 게임 실행 직 후 millis() - timeM_score 는 0 이다. 10 milli second 마다 이 if문은 참이 된다.
     if ((millis() - timeM_score) - inGame_mil_limit > 10) {
      inGame_mil_limit = millis() - timeM_score;
      inGame_mil++; // 10 milli second 마다 inGame_mil 이 1씩 증가한다.
      if (inGame_mil == 100) {
        inGame_mil = 0;
      }
     }
     
     // 게임 실행 직 후 millis() - timeM_score 는 0 이다. 1 second 마다 이 if문은 참이 된다.
     if ((millis() - timeM_score) - inGame_sec_limit > 1000) {
      inGame_sec_limit = millis() - timeM_score;
      inGame_sec++; // 1 second 마다 inGame_sec 이 1씩 증가한다.
     }

     // 오른쪽 아래에 게임 버틴 시간 기록을 나타낸다.
     if (inGame_sec < 10) {
      u8g.print("000" + String(inGame_sec) + ":" + String(inGame_mil));
     } else if (inGame_sec < 100) {
      u8g.print("00" + String(inGame_sec) + ":" + String(inGame_mil));
     } else if (inGame_sec < 1000) {
      u8g.print("0" + String(inGame_sec) + ":" + String(inGame_mil));
     }

     // 왼쪽 아래에 가로 8픽셀, 세로 8픽셀 해골 모양을 그린다.
     u8g.drawBitmapP(5, u8g.getHeight() - 8, 1, 8, kill_draw);
     
     // 해골 모양 옆에 킬 수를 나타낸다.
     u8g.setPrintPos(15, u8g.getHeight());
     if (obs_kill < 10) {
      u8g.print("0x0" + String(obs_kill));
     } else {
      u8g.print("0x" + String(obs_kill));
     }
     
     u8g.setFont(u8g_font_unifont);
     
     // 캐릭터와 몬스터들이 움직이는 영역을 프레임으로 그린다.
     u8g.drawFrame(0, 0, u8g.getWidth(), u8g.getHeight() - 8);

     // avoid_drt == 1 인 경우 캐릭터가 오른쪽을 보고있는 경우이다. 8 X 8 픽셀 크기로 오른쪽을 보고 있는 캐릭터를 그린다.
     if (avoid_drt == 1) {
      u8g.drawBitmapP(xCtrl, yCtrl, 1, 8, avoidR);
          
     // avoid_drt == 2 인 경우 캐릭터가 왼쪽을 보고있는 경우이다. 8 X 8 픽셀 크기로 왼쪽을 보고 있는 캐릭터를 그린다.     
     } else if (avoid_drt == 2) {
      u8g.drawBitmapP(xCtrl, yCtrl, 1, 8, avoidL);
     }
     
     for (int i=0; i<Length / 2; i++) {
          
     // 오른쪽에서 나오는 총 11개의 몬스터들의 각 위치에 맞게 3 X 8 픽셀 크기로 몬스터들을 그린다.
     u8g.drawBitmapP(xspaR[i], yspaR[i], 1, 3, obs1);
          
     // 왼쪽에서 나오는 총 11개의 몬스터들의 각 위치에 맞게 3 X 8 픽셀 크기로 몬스터들을 그린다.
     u8g.drawBitmapP(xspaL[i], yspaL[i], 1, 3, obs2);
    }

    // 캐릭터의 총알들의 위치를 최신화 하는 함수이다.
    attack_draw();

    // 각 총알의 위치에 맞게 1 X 8 픽셀 크기로 총알을 그린다.
    for (int i=0; i<attack_size; i++) {
    u8g.drawBitmapP(attack_xspa[i], attack_yspa[i], 1, 1, attack_obs);
    }
    
  }

// 게임 종료 시 OLED 화면에 그리는 함수이다. (게임 종료 화면)
void end_draw() {
  u8g.setFont(u8g_font_unifont);
  int h = u8g.getFontAscent() - u8g.getFontDescent();
     
  // minu 는 최종 기록의 minute 단위 정보를 저장한다.
  // sec 는 최종 기록의 second 단위 정보를 저장한다.
  int minu = time_totalScore / 60, sec = time_totalScore % 60;

  // 최종 기록 겉에 표시되는 프레임이다.
  u8g.drawFrame(15, 2*(h+1) - (h/2) - 2, 95, 2*h + 3);
     
  u8g.setPrintPos((u8g.getWidth() - u8g.getStrWidth("Game Over"))/2, h);
  u8g.print("Game Over");
  u8g.drawBitmapP(25, 2*(h+1) - (h/2), 1, 8, kill_draw);
  u8g.setPrintPos(40, 3*h - (h/2));
  u8g.print(String(obs_kill) + "x 1sec");
  u8g.setPrintPos(64, 4*h - (h/2));
  u8g.print(String(time_score) + "sec");
  u8g.setPrintPos(60, 5*h - (h/2) + 2);
  
  if(minu > 9) {
    if (sec > 9) {
      u8g.print(String(minu) + ":" + String(sec));
    } else {
      u8g.print(String(minu) + ":0" + String(sec));
    }
  } else {
    if (sec > 9) {
      u8g.print("0" + String(minu) + ":" + String(sec));
    } else {
      u8g.print("0" + String(minu) + ":0" + String(sec));
    }
  }
}

// 캐릭터의 총알을 생성하는 함수이다.
void attack() {
     
  // 현재 발사 되어 있는 총알의 개수 정보를 가진다.
  int attack_num = 0;
  
  for (int i=0; i<attack_size; i++)
  {
    if (attack_seek[i] != 0) attack_num++;
  }

  // 현재 발사 되어 있는 총알의 개수가 4개이면 총알을 생성하지 않는다.
  if (attack_num == 4) return;
  
  // 현재 발사되는 총알의 idx를 최신화 한다. 0 ~ 3 사이의 값만 가진다.
  attack_idx++;
  attack_idx &= 0x03;

  if (avoid_drt == 1) {
    attack_seek[attack_idx] = 1;
    attack_xspa[attack_idx] = xCtrl + 7;
    attack_yspa[attack_idx] = yCtrl + 3;
  } else if (avoid_drt == 2) {
    attack_seek[attack_idx] = 2;
    attack_xspa[attack_idx] = xCtrl - 1;
    attack_yspa[attack_idx] = yCtrl + 3;
  } else {
    attack_xspa[attack_idx] = 0;
    attack_yspa[attack_idx] = 0;
  }
}

// 캐릭터의 총알들의 위치를 최신화 하는 함수이다.
void attack_draw() {
  for (int i=0; i<attack_size; i++) {
    
    // attack_seek[i] == 1 이면 총알이 오른쪽 방향으로 발사된 것이다. 따라서 총알의 x좌표는 증가한다.
    if (attack_seek[i] == 1) {
      attack_xspa[i]++;
      if (attack_xspa[i] == u8g.getWidth()) {
        attack_seek[i] = 0;
      }
         
    // attack_seek[i] == 2 이면 총알이 왼쪽 방향으로 발사된 것이다. 따라서 총알의 x좌표는 감소한다.
    } else if (attack_seek[i] == 2) {
      attack_xspa[i]--;
      if (attack_xspa[i] == -8) {
        attack_seek[i] = 0;
      }
    
    // attack_seek[i] == 0 이면 총알이 발사되지 않은 것이다.
    } else {
      attack_xspa[i] = 0;
      attack_yspa[i] = 0;
    }
  }
}

void swap(int x, int y) {
  int temp;
  temp = timeRandom[x];
  timeRandom[x] = timeRandom[y];
  timeRandom[y] = temp;
}

// 인자 값으로 timeRandom[22] 값이 들어올 것이다. timeRandom[22] 를 오름차순으로 정렬하여 빠른 몬스터 순으로 정렬한다.
void lineUp(int timeRand[]) {
  int x;
  for(int a=0; a<Length - 1; a++) {
    x=a;
    for(int b=a+1; b<Length; b++) {
      if(timeRand[x] > timeRand[b]) {
        x=b;
      }
    }
    swap(x, a);
  }
}

// 어떤 몬스터를 새로 생성하였을 때, 그 몬스터의 속도가 0 번째 몬스터 보다 빠르다면 값을 서로 교체해준다.
void Switch () {
  for (int a=1; a<Length; a++) {
    if (timeRandom[0] > timeRandom[a]) {
      swap(0, a);
    }
  }
}

// 처음 게임을 켰을 때 OLED 화면에 로딩 상태를 그리는 함수이다. (로딩 중 화면)
void loading() { 
  u8g.setFont(u8g_font_unifont);
     
  int w = u8g.getWidth();
  int d = (w - u8g.getStrWidth("Obstacles Game")) / 2;
  int h = u8g.getFontAscent() - u8g.getFontDescent();
     
  // "Obstacles Game" 이라는 문자열을 3개 중첩하여 나타냄으로써 약간 입체감을 표현하였다.
  u8g.drawStr(d, h, "Obstacles Game");
  u8g.drawStr(d+1, h+1, "Obstacles Game");
  u8g.drawStr(d+1, h, "Obstacles Game");

  // 로딩 게이지가 점점 차는 것을, 프레임을 박스가 점점 덮어가는 것으로 표현하였다.
  // load_gage 변수가 0 ~ 100 정도까지 점점 증가하면서 박스가 프레임을 덮는다.
  u8g.drawFrame(14, u8g.getHeight()-h-1, w-28, h/2);
  u8g.drawBox(14, u8g.getHeight()-h-1, load_gage, h/2);
  
  u8g.setFont(u8g_font_04b_03);
  u8g.drawStr(w-u8g.getStrWidth("Ver 1.0"), u8g.getHeight()-1, "Ver 1.0");
  u8g.drawStr((w-u8g.getStrWidth("loading..."))/2, u8g.getHeight()-2, "loading...");
  
  // load_sct 변수는 0, 1, 2, 3, 0, 1, 2 ... 값을 반복적으로 가질 것이다.
  // 이 값에 따라서 로딩 중 캐릭터가 뛰어 가는 듯한 애니메이션을 표현하였다.
  switch(load_sct) {
    case 0 :
      u8g.drawBitmapP(40, 13, 4, 41, loading1);
      break;
    case 1 :
      u8g.drawBitmapP(40, 13, 4, 41, loading2);
      break;
    case 2 :
      u8g.drawBitmapP(40, 13, 4, 41, loading3);
      break;
    case 3 :
      u8g.drawBitmapP(40, 13, 4, 41, loading2);
      break;
  }
}

// 메뉴 화면의 상단 부분을 그리는 함수이다. (총 게임 플레이 시간, 배터리 표시 등등)
void upper() {
  u8g.setDefaultForegroundColor();
  u8g.setFont(u8g_font_04b_03);
     
  int w = u8g.getWidth();
  int h = u8g.getFontAscent() - (u8g.getFontDescent() - 1);

  // 왼쪽 상단에 기지국 이미지를 표현하였다. (꾸미기 용도)
  for (int i=0; i<4; i++) {
    if(i == 3) {
      u8g.drawFrame(12, 0, 3, 6);
      break;
    }
    u8g.drawBox((i*4), 3-i, 3, i+3);
  }

  u8g.drawStr((w-u8g.getStrWidth("Play Time"))/2, h-1, "Play Time");
  
  play_time_sec = millis() / 1000 - (play_time_min * 60);
  
  if(play_time_sec >= 60) {
    play_time_min++;
    play_time_sec = millis() / 1000 - (play_time_min * 60);
  }
  
  // 가운데 상단에 총 게임 플레이 시간을 표현하였다.
  u8g.setPrintPos((w-u8g.getStrWidth("00:00"))/2, 2*h-1);
  if (play_time_min < 10) {
    if (play_time_sec < 10) {
      u8g.print("0" + String(play_time_min) + ":" + "0" + String(play_time_sec));
    } else {
      u8g.print("0" + String(play_time_min) + ":" + String(play_time_sec));
    }
  } else {
    if (play_time_sec < 10) {
      u8g.print(String(play_time_min) + ":" + "0" + String(play_time_sec));
    } else {
      u8g.print(String(play_time_min) + ":" + String(play_time_sec));
    }
  }

  // 오른쪽 상단에 배터리 이미지를 표현하였다. (꾸미기 용도)
  u8g.drawStr(w-u8g.getStrWidth("85%")-15, h-1, "85%");
  u8g.drawFrame(w-13, 2, 13, h-2);
  u8g.drawBox(w-13, 2, 8, h-2);
}

// OLED 화면에 메뉴 화면을 그리는 함수이다. (메뉴 화면)
void menu () {
  upper();
  
  int w = u8g.getWidth();
  int d;
  int d1;
  
  // 메뉴 화면 왼쪽에는 캐릭터 이미지를, 오른쪽에는 해골 이미지를 나타내었다.
  u8g.drawBitmapP(-2, 20, 4, 41, bull);
  u8g.drawBitmapP(103, 20, 3, 32, brawl);
  
  u8g.drawStr(w-u8g.getStrWidth("Ver 1.0"), u8g.getHeight()-1, "Ver 1.0");
  
  u8g.setFont(u8g_font_unifont);
  int h = u8g.getFontAscent() - (u8g.getFontDescent() - 1);

  // 큰 프레임을 그리고, 그 안에 "START", "OPTION", "SCORES" 문자열을 나타낼 것이다.
  for (int i=0; i<3; i++) {
   d = (w - u8g.getStrWidth(menuStr[i])) / 2;
   d1 = (w - u8g.getStrWidth(menuStr[1])) / 2;
   u8g.setDefaultForegroundColor();
       
   // 가장 긴 문자열에 맞춰서 프레임 가로 너비를 정한다.
   u8g.drawFrame(d1-8, 1.5*h + 1, u8g.getStrWidth(menuStr[1]) + 16, 3*h);
       
    // 사용자가 현재 가리키고 있는 커서는 하얀 바탕에 검정 글씨로 표현하고, 그 외에는 검정 바탕에 하얀 글씨로 표현한다.
    // menuCur 변수는 0 ~ 2 사이의 값을 가진다.
    if (menuCur == i) {
      u8g.drawBox(d1-8, (i+1.5)*h + 1, u8g.getStrWidth(menuStr[1]) + 16, h);
      u8g.setDefaultBackgroundColor();
    } 
      u8g.drawStr(d, (i+2.5)*h, menuStr[i]);
  }
}

// OLED 화면에 옵션 화면을 그리는 함수이다. (옵션 화면)
void option() {
  upper();
  
  u8g.setFont(u8g_font_unifont);
     
  int w, w0, w1;
  int h = u8g.getFontAscent() - (u8g.getFontDescent() - 1);
  int d;

  for (int i=0; i<3; i++) {
   w = u8g.getStrWidth(optionStr[i]);
   w0 = u8g.getStrWidth(optionStr[0]);
   w1 = u8g.getStrWidth(optionStr[1]);
   d = (u8g.getWidth() - w) / 2;
   
   u8g.setDefaultForegroundColor();
   u8g.setPrintPos((u8g.getWidth() - (w0+15)) / 2 + (w0+15), 2*h);
   u8g.print(level);
   u8g.drawStr(w0 + 17, 2*h, "<");
   u8g.drawStr(u8g.getWidth() - 6, 2*h, ">");
   u8g.drawStr(w1 + 12, 3*h, "[]");
   
   
    // 사용자가 현재 가리키고 있는 커서는 하얀 바탕에 검정 글씨로 표현하고, 그 외에는 검정 바탕에 하얀 글씨로 표현한다.
    // optionCur 변수는 0 ~ 2 사이의 값을 가진다.
    if (optionCur == i) {
      if (i == 2) {
        u8g.drawBox(d-5, u8g.getHeight() - h, w + 10, h);
        u8g.setDefaultBackgroundColor();
      } else {
          u8g.drawBox(0, (i+1)*h + 1, w + 10, h);
          u8g.setDefaultBackgroundColor();
      }
    } 
    if (i == 2) {
      u8g.drawStr(d, u8g.getHeight() - 1, optionStr[i]);
    } else {
        u8g.drawStr(5, (i+2)*h, optionStr[i]);
    }
  }
}

// OLED 화면에 점수 순위 화면을 그리는 함수이다. (점수 순위 화면)
void scores() { 
  u8g.setDefaultForegroundColor();
  u8g.setFont(u8g_font_04b_03);
 
  int w = u8g.getWidth();
  int h = u8g.getFontAscent() - (u8g.getFontDescent() - 1);
  
  // 왼쪽 상단에 기지국 이미지를 표현하였다. (꾸미기 용도)
  for (int i=0; i<4; i++) {
    if(i == 3) {
      u8g.drawFrame(12, 0, 3, 6);
      break;
    }
    u8g.drawBox((i*4), 3-i, 3, i+3);
  }

  // 오른쪽 상단에 배터리 이미지를 표현하였다. (꾸미기 용도)
  u8g.drawStr(w-u8g.getStrWidth("85%")-15, h-1, "85%");
  u8g.drawFrame(w-13, 2, 13, h-2);
  u8g.drawBox(w-13, 2, 8, h-2);

  u8g.setFont(u8g_font_unifont);
  
  h = u8g.getFontAscent() - (u8g.getFontDescent() - 1);
  u8g.drawStr((w - u8g.getStrWidth("SCORES")) / 2, h - 1, "SCORES");
  u8g.drawBox(((w - u8g.getStrWidth("OK")) / 2) - 5, u8g.getHeight() - h, u8g.getStrWidth("OK") + 10, h);
  u8g.setDefaultBackgroundColor();
  u8g.drawStr((w - u8g.getStrWidth("OK")) / 2, u8g.getHeight() - 1, "OK");
  
  u8g.setDefaultForegroundColor();
  
  u8g.setFont(u8g_font_6x10);
  h = u8g.getFontAscent() - (u8g.getFontDescent() - 1);

  for (int i=0; i<score_num; i++) {

    // 왼쪽에 1. ~ 6. 까지 표시한다.
    u8g.setPrintPos(3, (h-1)*(i+2));
    u8g.print(String(i+1) + ".");
       
    u8g.setPrintPos(w - u8g.getStrWidth("00:00"), (h-1)*(i+2));

    // EEPROM의 0 ~ 5 번지, 10 ~ 15 번지 에는 1 ~ 6 순위 기록자들의 시간 정보가 존재한다.
    // 1순위의 minute 정보는 0 번지에 존재하고, second 정보는 10번지에 존재한다.
    // 2순위의 minute 정보는 1 번지에 존재하고, second 정보는 11번지에 존재한다.
    // ...
    // 각 번지마다 1 Byte 씩 밖에 저장하지 못하기 때문에 시간 기록을 second 단위로 통째로 저장하기에는 범위의 한계가 있을 수 있다.
    // 따라서 second 정보와 minute 정보를 따로 나누어 저장하였다.
    if (EEPROM.read(i) > 9) {
      if (EEPROM.read(i+10) > 9) {
        u8g.print(String(EEPROM.read(i)) + ":" + String(EEPROM.read(i+10)));
      } else {
        u8g.print(String(EEPROM.read(i)) + ":0" + String(EEPROM.read(i+10)));
      }
    } else {
      if (EEPROM.read(i+10) > 9) {
        u8g.print("0" + String(EEPROM.read(i)) + ":" + String(EEPROM.read(i+10)));
      } else {
        u8g.print("0" + String(EEPROM.read(i)) + ":0" + String(EEPROM.read(i+10)));
      }
    }
  }
}

// OLED 화면에 옵션 -> 점수 리셋 화면을 그리는 함수이다. (점수 리셋 화면)
void scoresReset () {
  u8g.setFont(u8g_font_unifont);
     
  int w;
  int h = u8g.getFontAscent() - (u8g.getFontDescent() - 1);
  int d;

  // 사용자가 현재 가리키고 있는 커서는 하얀 바탕에 검정 글씨로 표현하고, 그 외에는 검정 바탕에 하얀 글씨로 표현한다.
  // resetCur 변수는 2 ~ 3 사이의 값을 가진다.
  for (int i=0; i<4; i++) {
    w = u8g.getStrWidth(scoresResetS[i]);
    d = (u8g.getWidth() - w) / 2;
    u8g.setDefaultForegroundColor();

    if(resetCur == i) {
      u8g.drawBox(d-5, i*h+1, w+10, h);
      u8g.setDefaultBackgroundColor();
    }
    u8g.drawStr(d, (i+1)*h, scoresResetS[i]);
  }
}

// 게임 환경을 셋팅하는 함수이다.
void reset() {
   
  // 게임 난이도에 따라 몬스터 이동 속도 지정 (timeLevel은 몬스터가 한번 스탭 하는데 걸리는 시간과 .)
  switch (level) {
    case 1 :
      timeLevel = 75;
      break;
    case 2 :
      timeLevel = 60;
      break;
    case 3 :
      timeLevel = 45;
      break;
    case 4 :
      timeLevel = 35;
      break;
  }
  
  // 몬스터들의 x 위치는 출발선상으로 위치시키고, y 위치는 무작위로 지정한다.
  for(int a=0; a<Length / 2; a++) {
    yspaR[a] = random(u8g.getHeight() - 12) + 1;
    xspaR[a] = u8g.getWidth();
    yspaL[a] = random(u8g.getHeight() - 12) + 1;
    xspaL[a] = -8;
  }

  // 몬스터들의 속도와 관련된 시간 정보를 초기화 한다.
  for(int a=0; a<Length; a++) {
    timeRandom[a] = random(timeLevel, timeLevel+30);
  }

  // 캐릭터의 위치를 초기화 시킨다.
  yCtrl = u8g.getHeight() / 2;
  xCtrl = u8g.getWidth() / 2;

  rcvCtrl = 999;

  // 총알들을 초기화 시킨다.
  for (int i=0; i<attack_size; i++) {
    attack_seek[i] = 0;
  }
     
  // 킬 수를 초기화 시킨다.
  obs_kill = 0;
}

// 현재 EEPROM 0 ~ 5 번지, 10 ~ 15 번지에 있는 기록들을 1 ~ 6 순위로 정렬하는 함수이다.
void lineUp_eep() {
  for (int i=0; i<score_num; i++) {
    total_score_arr[i] = (EEPROM.read(i) * 60) + EEPROM.read(i+10);
  }
     
  int x;
  for(int a=0; a<score_num-1; a++) {
    x=a;
    for(int b=a+1; b<score_num; b++) {
      if(total_score_arr[x] < total_score_arr[b]){
        x=b;
      }
    }
    swap_eep(x, a);
  }

  for (int k=0; k<score_num; k++) {
    EEPROM.write(k, total_score_arr[k] / 60);
    EEPROM.write(k+10, total_score_arr[k] % 60);
  }
}

void swap_eep(int a, int b) {
  unsigned long temp;
  temp = total_score_arr[a];
  total_score_arr[a] = total_score_arr[b];
  total_score_arr[b] = temp;
}

// 가장 최근에 생성된 버틴 시간 기록을 EEPROM에 기록할 지 말지 결정하는 함수이다.
// 현재 6 순위 기록과 비교하여 더욱 우수한 기록이면 순위를 교체한다.
void score_eep() {
  int compare_score = (EEPROM.read(5) * 60) + EEPROM.read(15);
  if (time_totalScore > compare_score) {
      EEPROM.write(5, time_totalScore / 60);
      EEPROM.write(15, time_totalScore % 60);
      lineUp_eep();
  }
}

// 1 ~ 6 순위 기록을 초기화 하는 함수이다.
void reset_eep() {
  for (int i=0; i<score_num; i++) {
    EEPROM.write(i, 0);
    EEPROM.write(i+10, 0);
    total_score_arr[i] = 0;
  }
}

// 처음에 1번 실행된다.
void setup() {  
  irrecv.enableIRIn();

  reset();
  lineUp(timeRandom);

  // 게임을 처음 키고 7초 동안은 로딩 화면을 OLED 화면에 나타낸다. (꾸미기 용도)
  while (millis() < 7000) {
    u8g.firstPage();
    do {
    loading();
    } while(u8g.nextPage());
    load_sct++;
    load_sct = load_sct & 0x03;

    // millis() 가 0 ~ 7000 까지 가면서 load_gaeg 는 0 ~ 100 정도 까지의 값을 가진다.
    load_gage = millis() / 68;
    delay(50);
  }
}

// 무한 반복 된다.
void loop() {
 
 // 메뉴 화면 flag 가 1이면 아래 while 문이 반복 실행 된다.
 // 메뉴 화면을 OLED 화면에 나타낸다.
 while (menu_state) {

   reset();
   u8g.firstPage();
   do {
     menu();
   }while(u8g.nextPage());
   
  // 적외선 리모컨으로부터 신호를 받은 경우
  if (irrecv.decode(&results)) {
    ifrRcv = results.value & 0xFF;
    irrecv.resume();

    switch (ifrRcv) {
      case 0xE7 : // 위 버튼
        menuCur--;
        if (menuCur == -1) {
          menuCur = 2;
        }
      break;
      case 0xB5 : // 아래 버튼
        menuCur++;
        if(menuCur == 3) {
          menuCur = 0;
        }
      break;
      case 0xC7 : // ok 버튼
        if(menuCur == 0) { // 메뉴 커서가 0 일 때 ok 버튼을 누른 경우
          menu_state = 0;
          timeM_score = millis(); // 게임 시작 전까지의 경과 시간을 timeM_score 변수에 담아둔다.
        } else if (menuCur == 1) { // 메뉴 커서가 1 일 때 ok 버튼을 누른 경우
          option_state = 1;
        } else if (menuCur == 2) { // 메뉴 커서가 2 일 때 ok 버튼을 누른 경우
          scores_state = 1;
        }
      break;
    }
  }

  // 옵션 화면 flag 가 1이면 아래 while 문이 반복 실행 된다.
  // 옵션 화면을 OLED 화면에 나타낸다.
  while(option_state) {
    u8g.firstPage();
    do {
     option();
    }while(u8g.nextPage());

    if(irrecv.decode(&results)) {
      ifrRcv = results.value & 0xFF;
      irrecv.resume();

      switch(ifrRcv) {
        case 0xE7 : // 위 버튼
          optionCur--;
          if (optionCur == -1) {
            optionCur = 2;
          }
          break;
        case 0xB5 : // 아래 버튼
          optionCur++;
          if(optionCur == 3) {
            optionCur = 0;
          }
          break;
        case 0xA5 : // 오른쪽 버튼
          if (optionCur == 0) {
            level++;
            if (level == 5) {
              level = 4;
            }
          }
          break;
        case 0xEF : // 왼쪽 버튼
          if (optionCur == 0) {
            level--;
            if (level == 0) {
              level = 1;
            }
          }
          break;
        case 0xC7 : // ok 버튼
          if (optionCur == 2) {
            optionCur = 0;
            option_state = 0;
          } else if (optionCur == 1) {
              reset_state = 1;
          }
          break;
      }
    }
       
            // 리셋 화면 flag 가 1이면 아래 while 문이 반복 실행 된다.
            // 리셋 화면을 OLED 화면에 나타낸다.
            while(reset_state){
              u8g.firstPage();
               do {
               scoresReset();
               }while(u8g.nextPage());
               
              if(irrecv.decode(&results)) {
                ifrRcv = results.value & 0xFF;
                irrecv.resume();

                switch (ifrRcv) {
                  case 0xE7 : // 위 버튼
                    resetCur--;
                    if (resetCur == 1) {
                      resetCur = 3;
                    }
                    break;
                  case 0xB5 : // 아래 버튼
                    resetCur++;
                    if (resetCur == 4) {
                      resetCur = 2;
                    }
                    break;
                  case 0xC7 : // ok 버튼
                    if (resetCur == 2) {
                      reset_eep();
                    }
                    reset_state = 0;
                    break;
                }
              }
              }  
  }

  // 점수 화면 flag 가 1이면 아래 while 문이 반복 실행 된다.
  // 점수 화면을 OLED 화면에 나타낸다.
  while(scores_state) {
    u8g.firstPage();
    do {
      scores();
    }while(u8g.nextPage());

    if (irrecv.decode(&results)) {
      ifrRcv = results.value & 0xFF;
      irrecv.resume();

      switch(ifrRcv) {
        case 0xC7 : // ok 버튼
          scores_state = 0;
          break;
      }
    } 
  }
 }
  
  // 메뉴 화면 flag 가 0이 되어 while 문 바깥으로 나오면 아래 for 문이 실행된다. (게임 실행)
  for(xspaR[0]=u8g.getWidth(); xspaR[0]>-9; xspaR[0]--) {

  // 가장 빠른 몬스터를 기준으로 게임 화면 최신화가 진행된다.
  // ex) timeRandom[0] 이 30 ms 라면, 게임 화면 최신화는 30 ms 마다 진행된다.
  if (millis() - timeLimit[0] > timeRandom[0]) {
    timeLimit[0] = millis();
    
    u8g.firstPage();
    do {
     draw();
    } while(u8g.nextPage());
    
  } else {
      while (true) {
        if(millis() - timeLimit[0] > timeRandom[0]) {
          break;
        }
      }
   }

    // 나머지 1 ~ 21 번째 몬스터들은 자신의 스탭 시간 정보에 맞춰서 위치를 최신화 한다.
    for(int i=1; i<Length; i++) {
    if (millis() - timeLimit[i] > timeRandom[i]) {
      timeLimit[i] = millis();
      
      if (i < Length/2) { // 오른쪽 몬스터들의 x좌표는 점점 감소한다.
        xspaR[i]--;
      } else { // 왼쪽 몬스터들의 x좌표는 점점 증가한다.
        xspaL[i - Length/2]++;
      }

      // 오른쪽 몬스터들이 왼쪽 끝에 도달하였으면 새로 몬스터를 생성해준다.
      if (i < Length/2) {
        if (xspaR[i] == -8) {
          timeRandom[i] = random(timeLevel, timeLevel+30);
          yspaR[i] = random(u8g.getHeight() - 12) + 1;
          xspaR[i] = u8g.getWidth();
        }
           
      // 왼쪽 몬스터들이 오른쪽 끝에 도달하였으면 새로 몬스터를 생성해준다.
      } else {
        if (xspaL[i - Length/2] == u8g.getWidth() + 1) {
          timeRandom[i] = random(timeLevel, timeLevel+30);
          xspaL[i - Length/2] = -8;
          yspaL[i - Length/2] = random(u8g.getHeight() - 12) + 1;
        }
      }
    }
    }
    
  if(irrecv.decode(&results)) {
    ifrRcv = results.value & 0xFF;
    
    irrecv.resume();

    switch (ifrRcv) {
      case 0xE7 : // 위 버튼 (캐릭터 위로 이동)
        rcvCtrl = 0;
        break;
      case 0xB5 : // 아래 버튼 (아래로 이동)
        rcvCtrl = 1;
        break;
      case 0xFF : // 실수로 또는 고의로 버튼을 연속으로 누르고 있는 경우 (rcvCtrl 값 유지)
        
        break;
      case 0xA5 : // 오른쪽 버튼 (오른쪽으로 이동)
        rcvCtrl = 2;
        break;
      case 0xEF : // 왼쪽 버튼 (왼쪽으로 이동)
        rcvCtrl = 3;
        break;
      case 0xC7 : // ok 버튼 (총알 생성)
        attack();
        rcvCtrl = 999;
        break;
      default :
        rcvCtrl = 999;
    }

    switch (rcvCtrl) {
    case 0 : 
      yCtrl -= 3;
      if (yCtrl < 0) {
        yCtrl = 0;
      }
      break;
    case 1 : 
      yCtrl += 3;
      if (yCtrl > u8g.getHeight() - 16) {
        yCtrl = u8g.getHeight() - 16;
      }
      break;
    case 2 :
      avoid_drt = 1;
      xCtrl += 3;
      if (xCtrl > u8g.getWidth() - 8) {
        xCtrl = u8g.getWidth() - 8;
      }
      break;
     case 3 :
      avoid_drt = 2;
      xCtrl -= 3;
      if (xCtrl < 0) {
        xCtrl = 0;
      }
      break;
  }
  }

  // 몬스터가 총알에 닿은 경우 또는 캐릭터가 몬스터에 닿은 경우에 대한 처리 부분이다.
  for(int i=0; i<Length / 2; i++) {

    // 캐릭터의 주변 영역이다.
    int yP = yCtrl + 8;
    int yM = yCtrl - 3;
    int xP = xCtrl + 8;
    int xM = xCtrl - 8;

    int at_y;
    int at_xP;
    int at_xM;

    // 총알이 몬스터에 닿은 경우에 대한 처리 부분이다.
    for (int k=0; k<attack_size; k++) {
      
      // 총알의 주변 영역이다.
      at_y = attack_yspa[k];
      at_xP = attack_xspa[k] + 8;
      at_xM = attack_xspa[k] - 8;
         
      // 오른쪽에서 나오는 몬스터가 총알에 맞은 경우
      if((at_xM < xspaR[i]) && (at_xP > xspaR[i]) && (at_y < yspaR[i]+3) && (at_y > yspaR[i]-1)) {
        obs_kill++;
        attack_seek[k] = 0;
        xspaR[i] = u8g.getWidth();
        yspaR[i] = random(u8g.getHeight() - 12) + 1;
      }
      
      // 왼쪽에서 나오는 몬스터가 총알에 맞은 경우
      if ((at_y > yspaL[i]-1) && (at_y < yspaL[i]+3) && (at_xP > xspaL[i]) && (at_xM < xspaL[i])){
        obs_kill++;
        attack_seek[k] = 0;
        xspaL[i] = -8;
        yspaL[i] = random(u8g.getHeight() - 12) + 1;
      }
    }

    // 캐릭터가 몬스터에 닿은 경우에 대한 처리 부분이다.
    if (((xM < xspaR[i]) && (xP > xspaR[i]) && (yM < yspaR[i]) && (yP > yspaR[i])) || ((yP > yspaL[i]) && (yM < yspaL[i]) && (xP > xspaL[i]) && (xM < xspaL[i]))) {

      time_score = (millis() - timeM_score) / 1000; // 게임 버틴 시간 (단위 : second)
      time_totalScore = time_score + obs_kill; // 게임 버틴 시간에 킬 수를 더한 총 시간 기록
      score_eep();
      inGame_sec = 0;
      inGame_mil = 0;
      inGame_sec_limit = 0;
      inGame_mil_limit = 0;
      
      // 게임 종료 화면을 OLED 화면에 나타낸다.
      while(true) {        
        u8g.firstPage();
           do {
            end_draw();
           } while(u8g.nextPage());
           
        if(irrecv.decode(&results)) {
          ifrRcv = results.value & 0xFF;
          irrecv.resume();

          if(ifrRcv == 0xC7) { // ok 버튼      
          menu_state = 1;
          i = Length / 2; // 1049 line 의 for문을 바로 나가기 위한 값
          xspaR[0] = -9; // 939 line 의 for문을 바로 나가기 위한 값
          break;
        }
        }        
     }
    }    
    }        
  }

  // 0 번째 몬스터가 끝 라인까지 도달한 경우, 속도와 y좌표를 무작위로 새로 부여한다.
  timeRandom[0] = random(timeLevel, timeLevel+30);
  yspaR[0] = random(u8g.getHeight() - 12) + 1;
  Switch();
}
