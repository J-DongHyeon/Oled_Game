#include <IRremote.h>
#include <U8glib.h>
#include <EEPROM.h>

U8GLIB_SH1106_128X64 u8g(13, 11, 10, 9, 8);

/*
 * 13 : SCK
 * 11 : SDA
 * 9 : DC
 * 8 : RST
 */

IRrecv irrecv(14);
decode_results results;

unsigned long ifrRcv;
int rcvCtrl;

// 몬스터의 총 개수는 22개 이다.
unsigned long timeLimit[22];
int timeRandom[22];

int play_time_sec;
int play_time_min = 0;

char load_sct = 0;
char load_gage = 0;

int yspaR[11]; // 오른쪽에서 나오는 몬스터의 y좌표 정보
int xspaR[11]; // 오른쪽에서 나오는 몬스터의 x좌표 정보
int yspaL[11]; // 왼쪽에서 나오는 몬스터의 y좌표 정보
int xspaL[11]; // 왼쪽에서 나오는 몬스터의 x좌표 정보
int Length = sizeof(timeRandom) / sizeof(timeRandom[0]);
int timeLevel;

int yCtrl = u8g.getHeight() / 2; // 캐릭터의 y좌표 정보
int xCtrl = u8g.getWidth() / 2; // 캐릭터의 x좌표 정보

int attack_xspa[4]; // 총알의 x좌표
int attack_yspa[4]; // 총알의 y좌표
int attack_seek[4]; // 총알은 최대 4개 까지만 연속 발사할 수 있다.
int attack_size = sizeof(attack_seek)/sizeof(attack_seek[0]);
int attack_idx = -1;
int obs_kill = 0;

char *menuStr[3] = {"STRAT", "OPTION", "SCORES"};
char *optionStr[3] = {"LEVEL", "SCORES RESET", "Back To Menu"};
char *scoresResetS[4] = {"Do You Want", "Scores Reset?", "YES", "NO"};

int menuCur = 0;
int optionCur = 0;
int resetCur = 2;
int level = 1;

int menu_state = 1;
int option_state = 0;
int reset_state = 0;
int scores_state = 0;

unsigned long timeM_score, time_score, time_totalScore;
int score_num = 6;
unsigned long total_score_arr[6];
unsigned long inGame_sec = 0;
unsigned long inGame_sec_limit = 0;
unsigned long inGame_mil = 0;
unsigned long inGame_mil_limit = 0;

int avoid_drt = 1;


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

const unsigned char brawl[] PROGMEM = {
0x00, 0x3C, 0x00, 0x00, 0xFF, 0x80, 0x03, 0xFF, 0xC0, 0x07, 0xC3, 0xE0, 0x0F, 0x00, 0xF0, 0x1E,
0x00, 0x78, 0x1C, 0x00, 0x38, 0x38, 0x00, 0x1C, 0x70, 0x00, 0x0C, 0x70, 0x00, 0x0E, 0x60, 0xC0,
0x06, 0x60, 0xE0, 0x06, 0xE0, 0x61, 0x87, 0xE7, 0xB3, 0x07, 0xE7, 0xF7, 0x07, 0xEF, 0xCE, 0x03,
0xEF, 0xC3, 0xC3, 0xEF, 0xC7, 0xC7, 0xE7, 0xC7, 0xE7, 0x77, 0x87, 0xE7, 0x73, 0x37, 0xE6, 0x78,
0x77, 0xEE, 0x7C, 0x33, 0xCE, 0x3C, 0x03, 0x9C, 0x3C, 0x00, 0x3C, 0x1C, 0x00, 0x78, 0x0F, 0x03,
0xF0, 0x0F, 0xE3, 0xE0, 0x07, 0xFF, 0xC0, 0x01, 0xFF, 0x80, 0x00, 0xFE, 0x00, 0x00, 0x00, 0x00
};

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

const unsigned char avoidR[] PROGMEM = {
0xFF, 0x81, 0x99, 0x8D, 0x8D, 0x81, 0x81, 0xFF, 
};

const unsigned char avoidL [] PROGMEM = {
0xFF, 0x81, 0x99, 0xB1, 0xB1, 0x81, 0x81, 0xFF, 
};

const unsigned char obs1 [] PROGMEM = {
0xE0, 0x35, 0xE0, 
};

const unsigned char obs2 [] PROGMEM = {
0x07, 0xAC, 0x07, 
};

const unsigned char attack_obs [] PROGMEM = {
0xFF,
};

const unsigned char kill_draw [] PROGMEM = {
0x00, 0xA5, 0x42, 0xA5, 0x00, 0x7E, 0x42, 0x7E, 
};

void draw() { // screen while the game is running
     u8g.setFont(u8g_font_04b_03);
     u8g.setPrintPos(u8g.getWidth() - u8g.getStrWidth("0000:00"), u8g.getHeight());

     if ((millis() - timeM_score) - inGame_mil_limit > 10) {
      inGame_mil_limit = millis() - timeM_score;
      inGame_mil++;
      if (inGame_mil == 100) {
        inGame_mil = 0;
      }
     }
     
     if ((millis() - timeM_score) - inGame_sec_limit > 1000) {
      inGame_sec_limit = millis() - timeM_score;
      inGame_sec++;
     }

     if (inGame_sec < 10) {
      u8g.print("000" + String(inGame_sec) + ":" + String(inGame_mil));
     } else if (inGame_sec < 100) {
      u8g.print("00" + String(inGame_sec) + ":" + String(inGame_mil));
     } else if (inGame_sec < 1000) {
      u8g.print("0" + String(inGame_sec) + ":" + String(inGame_mil));
     }

     u8g.drawBitmapP(5, u8g.getHeight() - 8, 1, 8, kill_draw);
     u8g.setPrintPos(15, u8g.getHeight());
     if (obs_kill < 10) {
      u8g.print("0x0" + String(obs_kill));
     } else {
      u8g.print("0x" + String(obs_kill));
     }
     
     
  
     u8g.setFont(u8g_font_unifont);
     u8g.drawFrame(0, 0, u8g.getWidth(), u8g.getHeight() - 8);

     if (avoid_drt == 1) {
      u8g.drawBitmapP(xCtrl, yCtrl, 1, 8, avoidR);
     } else if (avoid_drt == 2) {
      u8g.drawBitmapP(xCtrl, yCtrl, 1, 8, avoidL);
     }
     
     for (int i=0; i<Length / 2; i++) {
     u8g.drawBitmapP(xspaR[i], yspaR[i], 1, 3, obs1);
     u8g.drawBitmapP(xspaL[i], yspaL[i], 1, 3, obs2);
    }

    attack_draw();

    for (int i=0; i<attack_size; i++) {
    u8g.drawBitmapP(attack_xspa[i], attack_yspa[i], 1, 1, attack_obs);
    }
    
  }

void end_draw() { // end game screen
  u8g.setFont(u8g_font_unifont);
  int h = u8g.getFontAscent() - u8g.getFontDescent();
  int minu = time_totalScore / 60, sec = time_totalScore % 60;

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

void attack() { // create attack bullets
  int attack_num = 0;
  
  for (int i=0; i<attack_size; i++) {
    if (attack_seek[i] != 0) {
      attack_num++;
    }
  }

  if (attack_num == 4) {
    return;
  }
  
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

void attack_draw() { // handling for the bullet movement
  for (int i=0; i<attack_size; i++) {
    if (attack_seek[i] == 1) {
      attack_xspa[i]++;
      if (attack_xspa[i] == u8g.getWidth()) {
        attack_seek[i] = 0;
      }
    } else if (attack_seek[i] == 2) {
      attack_xspa[i]--;
      if (attack_xspa[i] == -8) {
        attack_seek[i] = 0;
      }
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

void lineUp(int timeRand[]) { // obstacle alignment
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

void Switch () {
  for (int a=1; a<Length; a++) {
    if (timeRandom[0] > timeRandom[a]) {
      swap(0, a);
    }
  }
}

void loading() { // loading screen
  
  u8g.setFont(u8g_font_unifont);
  int w = u8g.getWidth();
  int d = (w - u8g.getStrWidth("Obstacles Game")) / 2;
  int h = u8g.getFontAscent() - u8g.getFontDescent();
  u8g.drawStr(d, h, "Obstacles Game");
  u8g.drawStr(d+1, h+1, "Obstacles Game");
  u8g.drawStr(d+1, h, "Obstacles Game");

  u8g.drawFrame(14, u8g.getHeight()-h-1, w-28, h/2);
  u8g.drawBox(14, u8g.getHeight()-h-1, load_gage, h/2);
  
  u8g.setFont(u8g_font_04b_03);
  u8g.drawStr(w-u8g.getStrWidth("Ver 1.0"), u8g.getHeight()-1, "Ver 1.0");
  u8g.drawStr((w-u8g.getStrWidth("loading..."))/2, u8g.getHeight()-2, "loading...");


  
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

void upper() { // top of the main screen
  u8g.setDefaultForegroundColor();
  u8g.setFont(u8g_font_04b_03);
  int w = u8g.getWidth();
  int h = u8g.getFontAscent() - (u8g.getFontDescent() - 1);

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

  u8g.drawStr(w-u8g.getStrWidth("85%")-15, h-1, "85%");
  u8g.drawFrame(w-13, 2, 13, h-2);
  u8g.drawBox(w-13, 2, 8, h-2);
  
  
}

void menu () {// main screen
  upper();
  
  int w = u8g.getWidth();
  int d;
  int d1;
  
  u8g.drawBitmapP(-2, 20, 4, 41, bull);
  u8g.drawBitmapP(103, 20, 3, 32, brawl);
  
  u8g.drawStr(w-u8g.getStrWidth("Ver 1.0"), u8g.getHeight()-1, "Ver 1.0");
  
  u8g.setFont(u8g_font_unifont);
  int h = u8g.getFontAscent() - (u8g.getFontDescent() - 1);

  for (int i=0; i<3; i++) {
   d = (w - u8g.getStrWidth(menuStr[i])) / 2;
   d1 = (w - u8g.getStrWidth(menuStr[1])) / 2;
   u8g.setDefaultForegroundColor();
   u8g.drawFrame(d1-8, 1.5*h + 1, u8g.getStrWidth(menuStr[1]) + 16, 3*h);
    if (menuCur == i) {
      u8g.drawBox(d1-8, (i+1.5)*h + 1, u8g.getStrWidth(menuStr[1]) + 16, h);
      u8g.setDefaultBackgroundColor();
    } 
      u8g.drawStr(d, (i+2.5)*h, menuStr[i]);
  }
}

void option() { // Options menu screen
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

void scores() { // Score menu screen
  u8g.setDefaultForegroundColor();
  u8g.setFont(u8g_font_04b_03);
 
  int w = u8g.getWidth();
  int h = u8g.getFontAscent() - (u8g.getFontDescent() - 1);
  
  for (int i=0; i<4; i++) {
    if(i == 3) {
      u8g.drawFrame(12, 0, 3, 6);
      break;
    }
    u8g.drawBox((i*4), 3-i, 3, i+3);
  }

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

    u8g.setPrintPos(3, (h-1)*(i+2));
    u8g.print(String(i+1) + ".");
    u8g.setPrintPos(w - u8g.getStrWidth("00:00"), (h-1)*(i+2));

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

void scoresReset () { // option -> Score reset screen
  u8g.setFont(u8g_font_unifont);
  int w;
  int h = u8g.getFontAscent() - (u8g.getFontDescent() - 1);
  int d;

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


void reset() { // Initialize the game execution environment
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
  
  for(int a=0; a<Length / 2; a++) {
    yspaR[a] = random(u8g.getHeight() - 12) + 1;
    xspaR[a] = u8g.getWidth();
    yspaL[a] = random(u8g.getHeight() - 12) + 1;
    xspaL[a] = -8;
  }

  for(int a=0; a<Length; a++) {
    timeRandom[a] = random(timeLevel, timeLevel+30);
  }

  yCtrl = u8g.getHeight() / 2;
  xCtrl = u8g.getWidth() / 2;

  rcvCtrl = 999;

  for (int i=0; i<attack_size; i++) {
    attack_seek[i] = 0;
  }
  obs_kill = 0;
}

void lineUp_eep() { // Scores are stored in EEPROM in record order
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

void score_eep() { // Save latest records to EEPROM
  int compare_score = (EEPROM.read(5) * 60) + EEPROM.read(15);
  if (time_totalScore > compare_score) {
      EEPROM.write(5, time_totalScore / 60);
      EEPROM.write(15, time_totalScore % 60);
      lineUp_eep();
  }
}

void reset_eep() {
  for (int i=0; i<score_num; i++) {
    EEPROM.write(i, 0);
    EEPROM.write(i+10, 0);
    total_score_arr[i] = 0;
  }
}

void setup() {  
  irrecv.enableIRIn();

  reset();
  lineUp(timeRandom);

  while (millis() < 7000) {
    u8g.firstPage();
    do {
    loading();
    } while(u8g.nextPage());
    load_sct++;
    load_sct = load_sct & 0x03;

    load_gage = millis() / 68;
    delay(50);
  }
}
 
void loop() {
 while (menu_state) { // handling for the main screen

   reset();
   u8g.firstPage();
   do {
     menu();
   }while(u8g.nextPage());
   
  if (irrecv.decode(&results)) {
    ifrRcv = results.value & 0xFF;
    
    irrecv.resume();

    
    switch (ifrRcv) {
      case 0xE7 : // down button
        menuCur--;
        if (menuCur == -1) {
          menuCur = 2;
        }
      break;
      case 0xB5 : // up button
        menuCur++;
        if(menuCur == 3) {
          menuCur = 0;
        }
      break;
      case 0xC7 : // ok button
        if(menuCur == 0) {
          menu_state = 0;
          timeM_score = millis();
        } else if (menuCur == 1) {
          option_state = 1;
        } else if (menuCur == 2) {
          scores_state = 1;
        }
      break;
    }
  }

  while(option_state) { // handling for the option screen
    u8g.firstPage();
    do {
     option();
    }while(u8g.nextPage());

    if(irrecv.decode(&results)) {
      ifrRcv = results.value & 0xFF;
      irrecv.resume();

      switch(ifrRcv) {
        case 0xE7 : // down button
          optionCur--;
          if (optionCur == -1) {
            optionCur = 2;
          }
          break;
        case 0xB5 : // up button
          optionCur++;
          if(optionCur == 3) {
            optionCur = 0;
          }
          break;
        case 0xA5 : // right button
          if (optionCur == 0) {
            level++;
            if (level == 5) {
              level = 4;
            }
          }
          break;
        case 0xEF : // left button
          if (optionCur == 0) {
            level--;
            if (level == 0) {
              level = 1;
            }
          }
          break;
        case 0xC7 : // ok button
          if (optionCur == 2) {
            optionCur = 0;
            option_state = 0;
          } else if (optionCur == 1) {
              reset_state = 1;
          }
          break;
      }
    }
       
            while(reset_state){ // handling for the score reset screen
              u8g.firstPage();
               do {
               scoresReset();
               }while(u8g.nextPage());
               
              if(irrecv.decode(&results)) {
                ifrRcv = results.value & 0xFF;
                irrecv.resume();

                switch (ifrRcv) {
                  case 0xE7 : // down button
                    resetCur--;
                    if (resetCur == 1) {
                      resetCur = 3;
                    }
                    break;
                  case 0xB5 : // up button
                    resetCur++;
                    if (resetCur == 4) {
                      resetCur = 2;
                    }
                    break;
                  case 0xC7 : // ok button
                    if (resetCur == 2) {
                      reset_eep();
                    }
                    reset_state = 0;
                    break;
                }
              }
              }  
  }

  while(scores_state) { // handling for the score screen
    u8g.firstPage();
    do {
      scores();
    }while(u8g.nextPage());

    if (irrecv.decode(&results)) {
      ifrRcv = results.value & 0xFF;
      irrecv.resume();

      switch(ifrRcv) {
        case 0xC7 : // ok button
          scores_state = 0;
          break;
      }
    } 
  }
 }
  
  for(xspaR[0]=u8g.getWidth(); xspaR[0]>-9; xspaR[0]--) { // handling for the movement of obstacles

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

    for(int i=1; i<Length; i++) {
    if (millis() - timeLimit[i] > timeRandom[i]) {
      timeLimit[i] = millis();
      
      if (i < Length/2) {
        xspaR[i]--;
      } else {
        xspaL[i - Length/2]++;
      }

      if (i < Length/2) {
        if (xspaR[i] == -8) {
          timeRandom[i] = random(timeLevel, timeLevel+30);
          yspaR[i] = random(u8g.getHeight() - 12) + 1;
          xspaR[i] = u8g.getWidth();
        }
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
      case 0xE7 : // up button
        rcvCtrl = 0;
        break;
      case 0xB5 : // down button
        rcvCtrl = 1;
        break;
      case 0xFF :
        
        break;
      case 0xA5 : // right button
        rcvCtrl = 2;
        break;
      case 0xEF : // left button
        rcvCtrl = 3;
        break;
      case 0xC7 : // ok button
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

  for(int i=0; i<Length / 2; i++) {

    int yP = yCtrl + 8;
    int yM = yCtrl - 3;
    int xP = xCtrl + 8;
    int xM = xCtrl - 8;

    int at_y;
    int at_xP;
    int at_xM;

    for (int k=0; k<attack_size; k++) {
      at_y = attack_yspa[k];
      at_xP = attack_xspa[k] + 8;
      at_xM = attack_xspa[k] - 8;
      if((at_xM < xspaR[i]) && (at_xP > xspaR[i]) && (at_y < yspaR[i]+3) && (at_y > yspaR[i]-1)) { // handling for the obstacle attack
        obs_kill++;
        attack_seek[k] = 0;
        xspaR[i] = u8g.getWidth();
        yspaR[i] = random(u8g.getHeight() - 12) + 1;
      } else if ((at_y > yspaL[i]-1) && (at_y < yspaL[i]+3) && (at_xP > xspaL[i]) && (at_xM < xspaL[i])){
        obs_kill++;
        attack_seek[k] = 0;
        xspaL[i] = -8;
        yspaL[i] = random(u8g.getHeight() - 12) + 1;
      }
    }

    
    if (((xM < xspaR[i]) && (xP > xspaR[i]) && (yM < yspaR[i]) && (yP > yspaR[i])) || ((yP > yspaL[i]) && (yM < yspaL[i]) && (xP > xspaL[i]) && (xM < xspaL[i]))) { // handling for the character death

      time_score = (millis() - timeM_score) / 1000;
      time_totalScore = time_score + obs_kill;
      score_eep();
      inGame_sec = 0;
      inGame_mil = 0;
      
      while(true) {        
        u8g.firstPage();
           do {
            end_draw();
           } while(u8g.nextPage());
        if(irrecv.decode(&results)) {
          ifrRcv = results.value & 0xFF;
          irrecv.resume();

          if(ifrRcv == 0xC7) { // ok button         
          menu_state = 1;
          i = Length / 2;
          xspaR[0] = -9;
          break;
        }
        }        
     }
    }    
    }        
  }

  timeRandom[0] = random(timeLevel, timeLevel+30);
  yspaR[0] = random(u8g.getHeight() - 12) + 1;

  Switch();
}
