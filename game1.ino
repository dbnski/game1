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

#include "sprites.h"

Gamebuino gb;

byte high_score = EEPROM.read(0);

int ground_y = LCDHEIGHT - 4;

// Player sprite index.
byte player_frame;
// Player sprite orientation.
byte player_flip;
// Player position on the x-axis.
int player_x;
// Player sprite width modifier.
int player_x_mod;
// Player position on the y-axis.
int player_y;
// Player sprite height modifier.
int player_y_mod;
// Player's vertical velocity.
int player_vy;
// Player's horizontal velocity.
int player_vx;
// Is player moving in any direction?
int player_moving;
// Is player jumping?
int player_jumping;

// Is there a projectile on the screen?
bool projectile;
// Projectile position on the x-axis.
int projectile_x;
// Projectile position on the y-axis.
int projectile_y;
// Projectile's horizontal velocity.
int projectile_vx;
// Delay until we can shoot another projectile.
int projectile_wait;
//
int projectile_slow;

// Is there a coin on the screen?
bool coin;
// Coin type
byte coin_type;
// Coin position on the x-axis.
int coin_x;
// Coin position on the y-axis.
int coin_y;
// Delay until we can show another coin.
int coin_wait = 0;
// How long do we wait for the player to pick the coin?
int coin_expire = 0;

// Is there a bomb on the screen?
bool bomb;
// Bomb position on the x-axis.
int bomb_x;
// Bomb position on the y-axis.
int bomb_y;
// Bomb sprite index.
int bomb_frame;
// Delay until we can drop another bomb.
int bomb_wait;

// Lives remaining.
byte lives;
// Current score.
byte score;

void wait(int frames) {
  bool p = gb.display.persistence;

  // Do not clear frames during pause.
  gb.display.persistence = true;

  frames = frames + gb.frameCount;
  while (gb.frameCount < frames) {
    while(!gb.update());
  }

  // Revert the original configuration.
  gb.display.persistence = p;
}

bool is_visible_x(int x) {
  return x >= 0 && x < LCDWIDTH;
}

bool is_visible_y(int y) {
  return y >= 0 && y < LCDHEIGHT;
}

bool is_visible(int x, int y) {
  return is_visible_x(x) && is_visible_y(y);
}

void game_reset() {
  player_flip = NOFLIP;
  player_x = LCDWIDTH / 2 - 4;
  player_y = ground_y - 8;
  player_vx = 1;
  player_vy = 2;
  player_moving = -1;
  player_jumping = 0;

  projectile = false;
  projectile_x = -1;
  projectile_y = ground_y - 4;
  projectile_vx = 2;
  projectile_wait = 0;
  projectile_slow = 0;
  
  bomb = false;
  bomb_x = -1;
  bomb_y = -1;
  bomb_wait = 0;

  coin = false;
  coin_type = 0;
  coin_x = -1;
  coin_y = ground_y - 12;
  coin_wait = 0;
  coin_expire = 0;
  
  lives = 3;
  score = 0;
}

void setup() {
  // EEPROM.read() returns 0xFF when cell was never set.
  if (high_score == 0xFF) {
    high_score = 0;
  }
  
  gb.begin();

  gb.pickRandomSeed();
  gb.titleScreen(F("GAME1"), LOGO);

  gb.battery.show = false;

  game_reset();
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

      // Give user 3 seconds to read the screen.
      wait(60);

      // Reset the game.
      game_reset();

      gb.titleScreen(F("GAME1"), LOGO);

      gb.battery.show = false;

      // Stay on the title screen.
      return;
    }

    gb.display.clear();

    // Print lives remaining.
    gb.display.print("\3");
    gb.display.print(lives);

    // Print score.
    gb.display.cursorX = 35;
    gb.display.print(score);

    // Print high score.
    gb.display.cursorX = 60;
    gb.display.print("HI ");
    gb.display.print(high_score);

    /*
     * Ground, a.k.a. game scene.
     * 
     * The flat surface at the bottom on of the screen on which the player can walk.
     * 
    */

    for (int x = 0; x < LCDWIDTH + 8; x += 8) {
      gb.display.drawBitmap(x, ground_y, BLOCK, NOROT, NOFLIP);
    }

    /*
     * Player
     * 
     * Self-explanatory.
     * 
     */

    // The player has to be on the ground in order to be able to move.
    if (!player_jumping) {
      /* 
       * Holds directional button value (e.g. BTN_LEFT).
       * -1 is used as a no-op, i.e. when no button is being held.
       */
      player_moving = -1;

      /*
       * player_x_mod and player_y_mod modifiers help with detecting 
       * player collisions with other objects when the player sprite 
       * is smaller than the default sprite (i.e. less than 8x8).
       * For example, when the player ducks.
       * 
       */
      player_x_mod = 0;
      player_y_mod = 0;

      // Should the player be going left?
      if (gb.buttons.repeat(BTN_LEFT, 1)) {
        player_moving = BTN_LEFT;
        // Flip the sprite horizontally to make it face left.
        player_flip = FLIPH;
        // Negative x velocity means left-hand side direction.
        player_vx = -1;
      }

      // Should the player be going right?
      if (gb.buttons.repeat(BTN_RIGHT, 1)) {
        player_moving = BTN_RIGHT;
        player_flip = NOFLIP;
        // Positive x velocity means right-hand side direction.
        player_vx = 1;
      }

      // Should the player jump?
      if (gb.buttons.repeat(BTN_UP, 1)) {
        /*
         * Flags the player as "in mid-air", but also remembers 
         * its original y position, which is necessary for calculating
         * the jump.
         */
        player_jumping = player_y;
      }

      // Should the player duck?
      if (gb.buttons.repeat(BTN_DOWN, 1)) {
        player_moving = BTN_DOWN;

        // Crouching disables horizontal movement.
        player_vx = 0;
      }
    }

    // Is the player moving left, right or down?
    if (player_moving >= 0) {
      // Apply the horizontal velocity.
      player_x = player_x + player_vx;

      // We can't allow the player to move off the screen...
      if (player_x < 0) {
        player_x = 0;
      }
      // ...in either direction.
      if (player_x > LCDWIDTH - 8) {
        player_x = LCDWIDTH - 8;
      }
    }

    // Is the player jumping?
    if (player_jumping) {
      // Apply the vertical velocity.
      player_y = player_y - player_vy;

      /*
       * Determine if it's time for the player to start falling down.
       * This should occur either after ascending by 16px or 
       * if the player hits the top edge of the screen.
       */
      if (player_y == 0 || player_jumping - player_y == 16) {
        // Reverse the vertical velocity.
        player_vy = -player_vy;
      }

      // Is the player touching the ground yet?
      if (player_y + player_y_mod > ground_y - 8) {
        // Do not let it fall below the ground level.
        player_y = ground_y - 8 - player_y_mod;
        // Reset the vertical velocity.
        player_vy = -player_vy;
        // Not in mid-air any more.
        player_jumping = 0;
      }
    }

    // Use the default sprite if the player is staing still.
    if (player_moving < 0 && !player_jumping) {
      player_frame = 0;
    }
    // Use the jumping sprite when the player is jumping.
    else if (player_jumping) {
      player_frame = 2;
    }
    // Use the crouching sprite when the player is crouching.
    else if (player_moving == BTN_DOWN) {
      player_frame = 4;
    }
    // Walking animation.
    else {
      if (gb.frameCount % 2 == 0) {
        player_frame++;

        // Skip the last frame as it's the crouching sprite.
        if (player_frame >= PLAYER_LEN - 1) {
          player_frame = 0;
        }
      }
    }

    // Read sprite width and height.
    player_x_mod = 8 - pgm_read_byte(&(PLAYER[player_frame][0]));
    player_y_mod = 8 - pgm_read_byte(&(PLAYER[player_frame][1]));

    gb.display.drawBitmap(player_x + player_x_mod, player_y + player_y_mod, PLAYER[player_frame], NOROT, player_flip);

    /*
     * Coins
     * 
     * Collecting coins provides the opportunity to score extra points, but
     * at a risk of moving the player closer to either edge of the screen 
     * where it may be more diffuclt to avoid projectiles.
     * 
     */

    if (!coin && !coin_wait) {
      // Calculate when a new coin should appear.
      coin_wait = gb.frameCount + rand() % 100 + 100;
    }

    // Can we show a new coin?
    if (!coin && gb.frameCount >= coin_wait) {
      int coin_weights[] = {
        50,
        25,
        25
      };
      int rnd = rand() % 100 + 1;

      coin = true;
      coin_type = 0;

      for (byte i = 0; i < COIN_LEN; i++) {
        rnd = rnd - coin_weights[i];

        if (rnd < 0) {
          coin_type = i;
          break;
        }
      }

      // Position is set on the opposite side of the screen to the player.
      coin_x = player_x > LCDWIDTH / 2 ? 10 + rand() % 15 : LCDWIDTH - (10 + rand() % 15);

      // Uncollected coin should disappear after 10 seconds.
      coin_expire = gb.frameCount + 200;
    }

    // Is there a coin to draw?
    if (coin) {
      // Draw the coin.
      gb.display.drawBitmap(coin_x, coin_y, COIN[coin_type], NOROT, NOFLIP);

      // Is player touching the coin?
      if (gb.collideRectRect(player_x + player_x_mod, player_y + player_y_mod, 8 - player_x_mod, 8 - player_y_mod, coin_x, coin_y, 5, 5)) {
        gb.sound.playOK();

        switch (coin_type) {
          case 0:
            // A coin is worth 2 points.
            score = score + 2;
            break;

          case 1:
            // Remove bombs for 20 seconds.
            bomb = false;
            bomb_x = -1;
            bomb_y = -1;
            bomb_wait = gb.frameCount + 400;
            break;

          case 2:
            // Slow down projectiles for 20 seconds.
            projectile_vx = projectile_vx > 0 ? 1 : -1;
            projectile_slow = gb.frameCount + 400;
            break;

          default:
            break;
        }

        // Reset coin.
        coin = false;
        coin_type = 0;
        coin_x = 0;
        coin_wait = 0;
      }

      // Expire uncollected coin.
      if (gb.frameCount >= coin_expire) {
        // Reset coin.
        coin = false;
        coin_type = 0;
        coin_x = 0;
        coin_wait = 0;
        coin_expire = 0;
      }
    }

    /*
     * Bomb
     * 
     * Bombs are added to increase game difficulty by forcing the player 
     * out of the comfort zone in the center of the screen every now 
     * and then.
     * 
     */

    if (!bomb && !bomb_wait) {
      // Calculate when a new bomb should drop.
      bomb_wait = gb.frameCount + rand() % 100 + 50;
    }

    // Can we show a new bomb?
    if (!bomb && gb.frameCount >= bomb_wait) {
      bomb = true;

      // Drop bombs somewhere close to the center.
      bomb_x = rand() % (LCDWIDTH - 40) + 12;

      // Start at the top.
      bomb_y = 6;
    }

    if (bomb) {
      // Apply the vertical velocity.
      bomb_y = bomb_y + 1;

      // Show the default bomb sprite until it touches the ground...
      if (bomb_y <= ground_y - 5) {
        bomb_frame = 0;
      }
      // ...then play the explosion animation.
      else {
        bomb_frame = bomb_frame + 1;
      }

      // Continue drawing animation frames until it finishes.
      if (bomb_frame < 7) {
        // Adjust sprite y position when different frames have different heights.
        int _bomb_y = bomb_frame == 0 ? bomb_y : ground_y - pgm_read_byte(&(BOMB[bomb_frame][1]));

        gb.display.drawBitmap(bomb_x, _bomb_y, BOMB[bomb_frame], NOROT, NOFLIP);     

        // Did the bomb hit the player?
        if (gb.collideRectRect(player_x + player_x_mod, player_y + player_y_mod, 8 - player_x_mod, 8 - player_y_mod, bomb_x, _bomb_y, 8, pgm_read_byte(&(BOMB[bomb_frame][1])))) {
          // Play the sound.
          gb.sound.playTick();

          // Take player's life.
          lives = lives - 1;

          // Remove the bomb.
          bomb = false;
          bomb_x = -1;
          bomb_y = -1;
          bomb_wait = 0;

          /*
           * A projectile could hit the player and kill him right after
           * the bomb just did resulting in loss of two lives in only 
           * a split second. Remove any bombs to prevent that.
           */
          projectile = false;
          projectile_x = -1;
          projectile_wait = 0;
        }
      }
      // Remove the bomb.
      else {
        bomb = false;
        bomb_x = -1;
        bomb_y = -1;
        bomb_wait = 0;
      }
    }

    if (!projectile && !projectile_wait) {
      // Calculate when a new projectile should be shot.
      projectile_wait = gb.frameCount + rand() % 40 + 10;
    }

    // Can we shoot a new projectile?
    if (!projectile && gb.frameCount >= projectile_wait) {
      projectile = true;

      // Play the sound.
      gb.sound.playCancel();

      // Choose vertical velocity.
      projectile_vx = !projectile_slow ? rand() % 2 + 2 : 2;

      // Choose height at which the projectile will fly.
      projectile_y = ground_y - 4 * (rand() % 3 + 1); 

      // Choose the direction in which the projecitle will fly.
      if (rand() % 2 == 0) {
        // Start at the left-hand side of the screen.
        projectile_x = 1;
      }
      else{
        // Start at the right-hand side of the screen.
        projectile_x = LCDWIDTH - 1;
        // Reverse the horizontal velocity (go right to left).
        projectile_vx = -projectile_vx;
      }
    }

    // Is there a projectile to draw?
    if (projectile) {
      // Apply the horizontal velocity.
      projectile_x = projectile_x + projectile_vx;

      // Draw the projectile.
      gb.display.fillCircle(projectile_x, projectile_y, 1);

      // Did the projectile hit the player?
      if (gb.collideRectRect(player_x + player_x_mod, player_y + player_y_mod, 8 - player_x_mod, 8 - player_y_mod, projectile_x, projectile_y, 2, 2)) {
        // Play the sound.
        gb.sound.playTick();

        // Take player's life.
        lives = lives - 1;

        // Remove the projectile.
        projectile = false;
        projectile_x = -1;
        projectile_wait = 0;

        /*
         * A bomb could hit the player and kill him right after
         * the projectile just did, resulting in loss of two lives 
         * in only a split second. Remove any bombs to prevent that.
         */
        bomb = false;
        bomb_x = -1;
        bomb_y = -1;
        bomb_wait = 0;
      }

      if (gb.frameCount > projectile_slow) {
        projectile_slow = 0;
      }
    }

    /*
     * Score
     */

    // Did the projectile reach either end of the screen?
    if (projectile && !is_visible_x(projectile_x)) {
      // The player managed to avoid the projectile, which is worth 1 point.
      score = score + 1;

      // Remove the projectile.
      projectile = false;
      projectile_x = -1;
      projectile_wait = 0;
    }

    // Update the high score, if necessary.
    if (score > high_score) {
      high_score = score;
    }
  }
}
