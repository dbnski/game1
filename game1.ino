/*
 * game1 - My first attempt to create a Gamebuino Classic game
 * 
 * Copyright (c) 2018 Maciej Dobrzanski
 * 
 * Under GPLv3. See LICENSE file details.
 * 
 */
 
#include <SPI.h>
#include <EEPROM.h>

#include <Gamebuino.h>

Gamebuino gb;

const byte LOGO[] PROGMEM = {64,30,
B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,
B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,
B00000000,B00000000,B00000000,B00000000,B00010000,B00000000,B00000000,B00000000,
B00000000,B00000000,B00000000,B00000000,B00001111,B00000000,B00000000,B00000000,
B00000000,B00000000,B00000000,B00001100,B01111111,B10000000,B00000000,B00000000,
B00000000,B00000000,B00000000,B00000111,B11111111,B10000000,B00000000,B00000000,
B00000000,B00000000,B00000000,B00000111,B11111111,B11000000,B00000000,B00000000,
B00000000,B00000000,B00000000,B00001111,B11110000,B11000000,B00000000,B00000000,
B00000000,B00000000,B00000000,B00000111,B11000000,B01000000,B00000000,B00000000,
B00000000,B00000000,B00000000,B00000111,B10000100,B01000000,B00000000,B00000000,
B00000000,B00000000,B00000000,B00000011,B00000001,B01000000,B00000000,B00000000,
B00000000,B00000000,B00000000,B00000111,B10000000,B10000000,B00000000,B00000000,
B00000000,B00000000,B00000000,B01111001,B11000001,B00000000,B00000000,B00000000,
B00000000,B00000000,B00000001,B10000001,B11110010,B00000000,B00000000,B00000000,
B00000000,B00000000,B00000010,B00000001,B11111100,B00000000,B00000000,B00000000,
B00000000,B00000000,B00000010,B01111111,B11111010,B00000000,B00000000,B00000000,
B00000000,B00000000,B00000001,B10001111,B11110010,B01100000,B00000000,B00000000,
B00000000,B00000000,B00000000,B00011111,B11110001,B10100000,B00000000,B00000000,
B00000000,B00000000,B00000000,B00011111,B11111100,B00100000,B00000000,B00000000,
B00000000,B00000000,B00000000,B00111111,B11010011,B11000000,B00000000,B00000000,
B00000000,B00000000,B00000000,B01011111,B11010000,B00000000,B00000000,B00000000,
B00000000,B00000000,B00000000,B10001111,B11110000,B00000000,B00000000,B00000000,
B00000000,B00000000,B00000001,B00000111,B11111001,B10000000,B00000000,B00000000,
B00000000,B00000000,B00000010,B00001111,B11111111,B10000000,B00000000,B00000000,
B00000000,B00000000,B00000100,B00010001,B11111111,B10000000,B00000000,B00000000,
B00000000,B00000000,B00000100,B00100000,B01111111,B10000000,B00000000,B00000000,
B00000000,B00000000,B00000010,B00010000,B00011111,B00000000,B00000000,B00000000,
B00000000,B00000000,B00000001,B11100000,B00000110,B00000000,B00000000,B00000000,
B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,
B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,
};

const byte PLAYER[][10] PROGMEM = {
{
  8, 8,
  B00111000,
  B01110100,
  B01111000,
  B01010000,
  B00011000,
  B00010000,
  B00010000,
  B00010000
},{
  8, 8,
  B00111000,
  B01110100,
  B10111000,
  B00010000,
  B00111000,
  B00010000,
  B00011000,
  B00010100
},{
  8, 8,
  B00111000,
  B11110100,
  B00111000,
  B00010000,
  B00011000,
  B00010000,
  B00101000,
  B01000100
},{
  8, 8,
  B00111000,
  B01110100,
  B10111000,
  B00010000,
  B00111000,
  B00010000,
  B00110000,
  B01010000
},{
  8, 6,
  B00111000,
  B01110100,
  B01111000,
  B01010000,
  B00011000,
  B00111000
}};

const int PLAYER_LEN = 5;

const byte BLOCK[] PROGMEM = {
  8,
  3,
  B11111111,
  B10011001,
  B11111111
};

const byte COIN[] PROGMEM = {
  8,
  5,
  B01110000,
  B10001000,
  B10111000,
  B10001000,
  B01110000
};

const byte BOMB[][10] PROGMEM = {
{
  8, 5,
  B00111000,
  B00010000,
  B00111000,
  B00111000,
  B00010000,
  B00000000,
  B00000000,
  B00000000
},{
  8, 4,
  B00111000,
  B00010000,
  B00111000,
  B01111100,
  B00000000,
  B00000000,
  B00000000,
  B00000000
},{
  8, 3,
  B10111010,
  B11010110,
  B01111100,
  B00000000,
  B00000000,
  B00000000,
  B00000000,
  B00000000
},{
  8, 3,
  B10111010,
  B11010110,
  B01111100,
  B00000000,
  B00000000,
  B00000000,
  B00000000,
  B00000000
},{
  8, 4,
  B10101010,
  B01010111,
  B10111011,
  B11010110,
  B00000000,
  B00000000,
  B00000000,
  B00000000
},{
  8, 4,
  B10101010,
  B01010111,
  B10111011,
  B11010110,
  B00000000,
  B00000000,
  B00000000,
  B00000000
},{
  8, 2,
  B10101010,
  B01010111,
  B00000000,
  B00000000,
  B00000000,
  B00000000,
  B00000000,
  B00000000
}};


byte high_score = EEPROM.read(0);

byte frame = 0;
byte flip = NOFLIP;

int ground_y = LCDHEIGHT - 4;

int player_x = LCDWIDTH / 2 - 4;
int player_x_mod = 0;
int player_y = ground_y - 8;
int player_y_mod = 0;
int player_vy = 2;
int player_vx = 1;
int player_moving = -1;
int player_jumping = 0;

int projectile_x = -1;
int projectile_y = ground_y - 4;
int projectile_vx = 1;
int projectile_wait = 60;

int coin_x = 0;
int coin_y = ground_y - 12;
int coin_wait = 0;
int coin_expire = 0;

int bomb_x = -1;
int bomb_y = -1;
int bomb_frame;
int bomb_wait = 0;

byte lives = 3;
byte score = 0;

void wait(int frames) {
  bool p = gb.display.persistence;

  gb.display.persistence = true;

  frames = frames + gb.frameCount;
  while (gb.frameCount < frames) {
    while(!gb.update());
  }

  gb.display.persistence = p;
}

void setup() {
  if (high_score == 255) {
    high_score = 0;
  }
  
  gb.begin();

  gb.pickRandomSeed();
  gb.titleScreen(F("GAME1"), LOGO);

  gb.battery.show = false;
}

void loop(){
  if(gb.update()){
    if (!lives) {
      gb.display.clear();

      gb.display.fillScreen(BLACK);

      gb.display.setColor(WHITE, BLACK);
      gb.display.cursorX = 26;
      gb.display.cursorY = 15;
      gb.display.println("YOU DIED");

      gb.display.cursorX = 20;
      gb.display.cursorY = 25;
      gb.display.print("SCORE: ");
      if (score < 10) {
        gb.display.print("000");
      }
      else if (score < 100) {
        gb.display.print("00");
      }
      else if (score < 100) {
        gb.display.print("0");
      }
      gb.display.println(score);

      if (score && score == high_score) {
        gb.display.cursorX = 20;
        gb.display.cursorY = 35;
        gb.display.println("HIGH SCORE!");        
      }

      gb.display.setColor(BLACK, WHITE);

      EEPROM.write(0, high_score);

      wait(60);

      player_x = LCDWIDTH / 2 - 4;
      player_y = ground_y - 8;
      player_moving = -1;
      player_jumping = 0;

      projectile_x = -1;
      projectile_wait = 0;

      coin_x = 0;
      coin_wait = 0;
      coin_expire = 0;

      lives = 3;
      score = 0;

      gb.titleScreen(F("GAME1"), LOGO);

      gb.battery.show = false;

      return;
    }

    gb.display.clear();

    for (byte i = 0; i < lives; i++) {
      gb.display.print("\3");    
    }

    gb.display.cursorX = 35;
    gb.display.print(score);

    gb.display.cursorX = 60;
    gb.display.print("HI ");
    gb.display.print(high_score);

    for (int x = 0; x < LCDWIDTH + 8; x += 8) {
      gb.display.drawBitmap(x, ground_y, BLOCK, NOROT, NOFLIP);
    }

    if (!coin_wait) {
      coin_wait = gb.frameCount + rand() % 100 + 100;
    }

    if (!coin_x && gb.frameCount >= coin_wait) {
      coin_x = player_x > LCDWIDTH - player_x ? 10 + rand() % 15 : LCDWIDTH - (10 + rand() % 15);
      coin_expire = gb.frameCount + 200;
    }

    if (coin_x) {
      gb.display.drawBitmap(coin_x, coin_y, COIN, NOROT, NOFLIP);

      if (gb.collideRectRect(player_x + player_x_mod, player_y + player_y_mod, 8 - player_x_mod, 8 - player_y_mod, coin_x, coin_y, 5, 5)) {
        gb.sound.playOK();
        score = score + 2;
        coin_x = 0;
        coin_wait = 0;
      }

      if (gb.frameCount >= coin_expire) {
        coin_x = 0;
        coin_wait = 0;
        coin_expire = 0;
      }
    }

    if (!bomb_wait) {
      bomb_wait = gb.frameCount + rand() % 100 + 100;
    }

    if (bomb_wait && gb.frameCount >= bomb_wait) {
      if (bomb_x < 0) {
        bomb_x = rand() % (LCDWIDTH - 40) + 20;
      }

      if (bomb_y < 0) {
        bomb_y = 6;
      }

      bomb_y = bomb_y + 1;

      if (bomb_y <= ground_y - 5) {
        bomb_frame = 0;
      }
      else {
        bomb_frame = bomb_frame + 1;
      }

      if (bomb_frame < 7) {
        int _bomb_y = bomb_frame == 0 ? bomb_y : ground_y - BOMB[bomb_frame][1] - 4;

        gb.display.drawBitmap(bomb_x, _bomb_y, BOMB[bomb_frame], NOROT, NOFLIP);     

        if (gb.collideRectRect(player_x + player_x_mod, player_y + player_y_mod, 8 - player_x_mod, 8 - player_y_mod, bomb_x, _bomb_y, 8, BOMB[bomb_frame][1])) {
          gb.sound.playTick();
  
          bomb_x = -1;
          bomb_y = -1;
          bomb_wait = 0;

          projectile_x = -1;
          projectile_wait = 0;
  
          lives = lives - 1;
        }
      }
      else {
        bomb_x = -1;
        bomb_y = -1;
        bomb_wait = 0;
      }
    }


    if (!projectile_wait) {
      projectile_wait = gb.frameCount + rand() % 40 + 10;
    }

    if ((projectile_x < 0 || projectile_x > LCDWIDTH) && gb.frameCount >= projectile_wait) {
      gb.sound.playCancel();
      projectile_vx = rand() % 2 + 2;
      projectile_y = ground_y - 4 * (rand() % 3 + 1); 
      if (rand() % 2 == 0) {
        projectile_x = 0;
      }
      else{
        projectile_x = LCDWIDTH;
        projectile_vx = -projectile_vx;
      }
    }

    if (projectile_x >= 0 && projectile_x <= LCDWIDTH) {
      projectile_x = projectile_x + projectile_vx;
      gb.display.fillCircle(projectile_x, projectile_y, 1);

      if (gb.collideRectRect(player_x + player_x_mod, player_y + player_y_mod, 8 - player_x_mod, 8 - player_y_mod, projectile_x, projectile_y, 2, 2)) {
        gb.sound.playTick();

        projectile_x = -1;
        projectile_wait = 0;

        lives = lives - 1;
      }
    }

    if (projectile_x == 0 || projectile_x == LCDWIDTH) {
      score = score + 1;
      projectile_wait = 0;
    }

    if (!player_jumping) {
      player_moving = -1;
      player_x_mod = 0;
      player_y_mod = 0;

      if (gb.buttons.repeat(BTN_LEFT, 1)) {
        player_moving = BTN_LEFT;
        flip = FLIPH;
        player_vx = -1;
      }

      if (gb.buttons.repeat(BTN_RIGHT, 1)) {
        player_moving = BTN_RIGHT;
        flip = NOFLIP;
        player_vx = 1;
      }

      if (gb.buttons.repeat(BTN_UP, 1)) {
        player_jumping = player_y;
      }

      if (gb.buttons.repeat(BTN_DOWN, 1)) {
        player_moving = BTN_DOWN;
        player_vx = 0;
        player_y_mod = 2;
      }
    }

    if (player_moving >= 0) {
      player_x = player_x + player_vx;

      if (player_x < 0) {
        player_x = 0;
      }
      if (player_x > LCDWIDTH - 8) {
        player_x = LCDWIDTH - 8;
      }
    }

    if (player_jumping) {
      player_y = player_y - player_vy;

      if (player_y == 0 || player_jumping - player_y == 16) {
        player_vy = -player_vy;
      }

      if (player_y > ground_y - 8) {
        player_y = ground_y - 8;
        player_vy = -player_vy;
        player_jumping = 0;
      }
    }

    if (player_moving < 0 && !player_jumping) {
      frame = 0;
    }
    else if (player_jumping) {
      frame = 2;
    }
    else if (player_moving == BTN_DOWN) {
      frame = 4;
    }
    else {
      if (gb.frameCount % 2 == 0) {
        frame++;

        if (frame >= PLAYER_LEN - 1) {
          frame = 0;
        }
      }
    }

    gb.display.drawBitmap(player_x + player_x_mod, player_y + player_y_mod, PLAYER[frame], NOROT, flip);

    if (score > high_score) {
      high_score = score;
    }
  }
}
