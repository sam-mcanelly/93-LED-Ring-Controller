#include <Adafruit_NeoPixel.h>

#define CHECK_BIT(var, pos) ((var) & (1<<(pos)))

#define PIN 2
#define RMT_LGHT_PIN 3

#define LIT_MODE 4
#define GLITCH_MODE_STEPS 10
#define HISTORY_BUFFER_CNT 3
#define COLOR_MODE_COUNT 7
#define LAYER_COUNT 6
#define LIGHT_RING_TOTAL 93

#define BTN1_PIN 5
#define BTN2_PIN 6
#define BTN3_PIN 7

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(LIGHT_RING_TOTAL, PIN);
Adafruit_NeoPixel remote_pixels = Adafruit_NeoPixel(3, RMT_LGHT_PIN);  

bool glitch_mode_on = false;
bool rotate_mode_on = false;
uint8_t glitch_step = 0;   // how far into the glitch animation are we?
uint8_t mode = 0;           // selects the pattern mode

//colors
uint32_t red = 0xFF0000;
uint32_t green = red >> 8;
uint32_t blue = green >> 8;
uint32_t white = 0xFFFFFF;
uint32_t yellow = 0xFFFF00;
uint32_t orange = 0xFF6600;

uint32_t rgb_colors[3] = {red, green, blue};
uint32_t bw_colors[3] = {blue, white, blue};
uint32_t red_colors[3] = {red, yellow, orange};
uint32_t dark_red_colors[3] = {red, 0x800000, 0x696969}; 
uint32_t rwb_colors[3] = {red, white, blue};
uint32_t grn_colors[3] = {green, yellow, white};
uint32_t color_18[18];
uint32_t *color_arrays[COLOR_MODE_COUNT];
uint32_t *curr_color_array;

const PROGMEM int rgb_arr_sz = 3;
const PROGMEM int color_18_sz = 18;

int curr_c_array_size;
int curr_c_array_idx = 0;
int color_idx = 0;

unsigned long last_btn1_press = 0;
unsigned long last_btn2_press = 0;
unsigned long last_btn3_press = 0;
unsigned long curr_time;

bool btn1_pushed = false;
bool btn2_pushed = false;
bool btn3_pushed = false;
bool fast_mode_switching = false;
bool spam_switch_modes = false;

uint32_t brightness = 10;
uint32_t delayTime = 10;

bool spiral_direction = true;

bool bool_op1 = false;
bool bool_op2 = false;

const PROGMEM int glitch_layer_counts[LAYER_COUNT] = {32, 24, 16, 12, 8, 1};
const PROGMEM int glitch_layer_offsets[LAYER_COUNT] = {0, 32, 56, 72, 84, 92};
int normal_layer_counts[LAYER_COUNT] = {32, 24, 16, 12, 8, 1};
int normal_layer_offsets[LAYER_COUNT] = {0, 32, 56, 72, 84, 92};

//ring layer index bank
int layer_0_indices[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12,
                         13, 14, 15, 16, 17, 18, 19, 20, 21, 22,
                         23, 24, 25, 26, 27, 28, 29, 30, 31}; 
int layer_1_indices[] = {32, 33, 34, 35, 36, 37, 38, 39, 40,
                         41, 42, 43, 44, 45, 46, 47, 48, 49, 
                         50, 51, 52, 53, 54, 55};
int layer_2_indices[] = {56, 57, 58, 59, 60, 61, 62, 63, 64,
                         65, 66, 67, 68, 69, 70, 71};
int layer_3_indices[] = {72, 73, 74, 75, 76, 77, 78, 79, 80,
                         81, 82, 83, 84};
int layer_4_indices[] = {85, 86, 87, 88, 89, 90, 91};
int layer_5_indices[] = {92};

int layer_indices[][LAYER_COUNT] = {layer_0_indices, layer_1_indices, layer_2_indices, layer_3_indices,
                                   layer_4_indices, layer_5_indices};

int history_idx = 0;
int transform_idx = 0;
uint32_t transform_saved_state[LIGHT_RING_TOTAL];
uint32_t history[HISTORY_BUFFER_CNT][LIGHT_RING_TOTAL];

uint8_t glitch_layer_order[6] = {1, 3, 0, 5, 2, 4};

void setup() {
  // compute the array with 18 colors
  compute_color_18();
  color_arrays[0] = rgb_colors;
  color_arrays[1] = bw_colors;
  color_arrays[2] = red_colors;
  color_arrays[3] = rwb_colors;
  color_arrays[4] = grn_colors;
  color_arrays[5] = dark_red_colors;
  color_arrays[6] = color_18;

  //default to RGB mode
  curr_color_array = color_arrays[curr_c_array_idx];
  curr_c_array_size = rgb_arr_sz;

  pixels.begin();
  pixels.setBrightness(brightness);
  for(int i = 0; i < 93; i++) pixels.setPixelColor(i, red);
  pixels.show();

  remote_pixels.begin();
  remote_pixels.setBrightness(brightness);
  update_remote();

  pinMode(BTN1_PIN, INPUT_PULLUP);
  pinMode(BTN2_PIN, INPUT_PULLUP);
  pinMode(BTN3_PIN, INPUT_PULLUP);
}

void loop() {  
  switch(mode) {
    case 0: spiral(bool_op1, (bool_op1 == true ? true : bool_op2));
            break;
    case 1: random_blink(5);
            break;
    case 2: pulse(bool_op1);
            break;
    case 3: portal(bool_op1, bool_op2);
            break;
    case 4: lit_mode();
            break;
  }
}

void lit_mode() {
  bool saved_glitch_mode = glitch_mode_on;
  glitch_mode_on = false;

  //if(mode != LIT_MODE) return;
  //spiral(bool_op1, bool_op2);

  if(mode != LIT_MODE) return;
  pulse(bool_op1);
  
  if(mode != LIT_MODE) return;
  pulse(!bool_op1);
  
  if(mode != LIT_MODE) return;
  portal(!bool_op1, !bool_op2);

  if(mode != LIT_MODE) return;
  pulse(!bool_op1);

  if(mode != LIT_MODE) return;
  pulse(bool_op1);
  
  if(mode != LIT_MODE) return;
  for(int i = 0; i < 30; i++) random_blink(10);

  if(mode != LIT_MODE) return;
  spiral(true, true);
  
  if(mode != LIT_MODE) return;
  pulse(bool_op1);
  
  if(mode != LIT_MODE) return;
  pulse(!bool_op1);

  if(mode != LIT_MODE) return;
  for(int i = 0; i < 30; i++) random_blink(10);
  
  if(mode != LIT_MODE) return;
  for(int i = 0; i < 3; i++) {
    if(i == 2) glitch_mode_on = true;
    portal(bool_op1, bool_op2);
    delayTime += 20;
  } 
  for(int i = 0; i < 4; i++) {
    if(i > 0) glitch_mode_on = false;
    if(i == 1) {
      for(int i = 0; i < LIGHT_RING_TOTAL; i++) {
        pixels.setPixelColor(i, 0);
      }
    }
    portal(!bool_op1, bool_op2);
   
    pixels.show();
    delayTime -= 20;
  } 
  delayTime += 20;
  
  next_color();
  glitch_mode_on = saved_glitch_mode;
}

void random_blink(uint32_t density) {
  uint32_t i;
  uint32_t j;
  
  i = random(LIGHT_RING_TOTAL);

  for(j = 0; j < density; j++) {
    if(check_buttons()) return;
    pixels.setPixelColor(i, curr_color_array[random(100)  % curr_c_array_size]);

    i = random(LIGHT_RING_TOTAL);
    if(j % 4 == 0) pixels.show();  
  }

  for(j = 0; j < density; j++) {
    if(check_buttons()) return;
    pixels.setPixelColor(i, 0);
    i = random(LIGHT_RING_TOTAL);
    if(j % 4 == 0) pixels.show();  
  }
}

void pulse(bool rainbow) {
  next_color();

  //zoom in and turn on
  for(int i = 0; i < LAYER_COUNT; i++) {
    if(check_buttons()) return;
    
    set_layer_solid(i, false, false);

    if(rainbow) {
      next_color();
    }
    pixels.show();
    delay(delayTime * 3);
  }

  //zoom out and turn off
  for(int i = 0; i < LAYER_COUNT - 1; i++) {
    if(check_buttons()) return;
    set_layer_solid((LAYER_COUNT - 1) - i, true, false);
    
    pixels.show();
    delay(delayTime * 3);
  }
}

void portal(bool rainbow, bool continuous) {
  next_color();

  //zoom in and turn on
  for(int i = 0; i < LAYER_COUNT; i++) {
    if(check_buttons()) return;
    if(!glitch_mode_on) set_layer_solid(i, false, false);
    else {
      if(i % 2 == 0){
        glitch_layer_solid(i, false, false);
        //next_color();
      } 
    }
    //set_layer_solid(i, false, false);
    
    if(!continuous && i > 0) {
      if(!glitch_mode_on) set_layer_solid(i - 1, true, false);
      else glitch_layer_solid(i - 1, true, false);
      //set_layer_solid(i - 1, true, false);
    }
    if(rainbow) {
      next_color();
    }

    pixels.show();
    if(glitch_mode_on) delay(delayTime * 1.5);
    else delay(delayTime * 3);
  }

  if(continuous) {
    //zoom out and turn off
    for(int i = 0; i < LAYER_COUNT; i++) {
    
      if(check_buttons()) return;
  
      if(!glitch_mode_on) set_layer_solid(i, true, false);
      else glitch_layer_solid(i, true, false);

      pixels.show();
      //if(glitch_mode_on) delay(delayTime);
      /*else*/ delay(delayTime * 3);
    }
  }
}

void spiral(bool half_cycle, bool spin) {  
  int start_color_idx = color_idx;
  
  for(int i = 0; i < LIGHT_RING_TOTAL; i++) {
    if(check_buttons()) return;
    pixels.setPixelColor(i, curr_color_array[color_idx]);
    pixels.show();
    
    next_color();
    
    delay(delayTime / 3);
  }

  if(spin) {
    shutoff_half_layer(0, 0, true);
    if(spiral_lights(start_color_idx)) return;
  }

  if(half_cycle) return;

  for(int i = 92; i >= 0; i--) {
    if(check_buttons()) return;
    pixels.setPixelColor(i, 0);
    pixels.show();
    delay(delayTime / 3);
  }
}

bool spiral_lights(int start_color_idx) {
  for(int i = 0; i < LIGHT_RING_TOTAL; i++) {
    transform_saved_state[i] = pixels.getPixelColor(i);
  }
  //do one full rotation
  transform_idx = 0;
  for(int j = 0; j < 93; j++) {
    if(check_buttons()) return true;
    rightSpiral();
    
    if(curr_color_array != color_18) delay(delayTime * 4);
    else delay(delayTime * 3);
    pixels.show();
  }

  return false;
}

void spinning() {
  
  
}

void execute_glitch() {
  glitch_mode_on = true;
  glitch_step = 0;
  history_idx = 0;
}

void execute_glitch_step() {
  switch(glitch_step) {
    case 0: 
      delayTime += 30;
      //save_frame(history, history_idx++);
      glitch_step++;
      break;
    case 1: 
      delayTime += 30;
      //save_frame(history, history_idx++);
      glitch_step++;
      break;
    case 2: 
      delayTime += 30;
      //save_frame(history, history_idx++);
      glitch_step++;
      break;
    case 3:
      switch(mode){
        case 0:
          execute_glitch_decay();
          break;
        case 1:
          break;
        case 2:
        case 3:
          break;
        case 4:
          break;
      }
      break;
    case 4:
      break;
    case 5:
      break;
    case 6:
      break;
    case 7:
      break;
    case 8:
      break;
    case 9:
      break;
    case 10:
      break;
  }
}

void execute_glitch_decay() {
  int start_point;
  for(int i = 0; i < LAYER_COUNT; i++) {
    // pick a random starting point on the given ring
    start_point = random(normal_layer_counts[i]);

    //shut off half that ring
    //shutoff_half_layer(glitch_layer_order[i], start_point, flash_circle);
    //delay(2);

  }
}

void decay_layer(int layer) {

}

//degree - how many places are we shifting?
//right - twitch right or left
/*
void twitch_layer(int layer, int degree, bool right) {
  if(right) {

  }
  else {
    leftRotateLayer(layer, degree);
  }

}
*/

void leftSpiral() {
  int i;

  for(i = 0; i < LIGHT_RING_TOTAL; i++) {
    pixels.setPixelColor(i, transform_saved_state[(i + transform_idx) % LIGHT_RING_TOTAL]);
  }

  transform_idx++;
  transform_idx %= LIGHT_RING_TOTAL;
  
}


void rightSpiral() {
  int i;

  for(i = LIGHT_RING_TOTAL - 1; i >= 0; i--) {
    pixels.setPixelColor(i, transform_saved_state[(i + (92 - transform_idx)) % LIGHT_RING_TOTAL]);
  }

  transform_idx++;
  transform_idx %= LIGHT_RING_TOTAL;
}

void set_layer_solid(int layer, bool off, bool one_at_a_time) {
  uint32_t color;

  if(off) color = 0;
  else color =  curr_color_array[color_idx];

  for(int i = 0; i < normal_layer_counts[layer]; i++) {
    pixels.setPixelColor(normal_layer_offsets[layer] + i, color); 
    if(one_at_a_time) { 
      pixels.show();
      delay(delayTime / 4);
    }
  }
}

void glitch_layer_solid(int layer, bool off, bool one_at_a_time) {
  uint32_t color;

  if(off) color = 0;
  else color =  random(0xFFFFFF);

  for(int i = 0; i < glitch_layer_counts[layer]; i++) {
    pixels.setPixelColor(glitch_layer_offsets[layer] + i, color); 
    if(one_at_a_time) { 
      pixels.show();
      delay(delayTime / 4);
    }
    
  }
}

void shutoff_half_layer(int layer, int start_pt, bool continuous) {
  start_pt += normal_layer_offsets[layer];
  for(int j = 0; j < normal_layer_counts[layer] / 2; j++) {
    pixels.setPixelColor(start_pt++ , 0);
      
    if(continuous) {
      delay(delayTime);
      pixels.show();
    }

      //reset the next point to the start of the layer
    if(start_pt > normal_layer_offsets[layer] + (normal_layer_counts[layer] - 1)) start_pt = normal_layer_offsets[layer];
  }
  pixels.show();
}

void seton_half_layer(int layer, int start_pt) {
  for(int j = 0; j < normal_layer_counts[layer]; j++) {

  }
}



/*--------------------------------
 * Button handlers
 * 
 * 
 * 
 * -------------------------------
 */
bool check_buttons() {
  curr_time = millis();

  check_speed_button();
  check_function_button();
  return check_mode_button();
}

bool check_mode_button() {
  if(digitalRead(BTN1_PIN) == LOW){
    if(curr_time - last_btn1_press < 100) return;

    last_btn1_press = curr_time;
    //changing op1 logic (rainbow mode)
    //initiated by pressing first two buttons
    if(check_op1_logic()) return false;

    //change colors
    //initiated by pressing first and last button
    if(check_color_array_logic()) return false;

    //check for long hold
    if(check_long_hold(BTN1_PIN)) {
      uint32_t c = (fast_mode_switching) ? red : green;
      flash_circle(c);
      flash_remote(c);
      fast_mode_switching = !fast_mode_switching;
      update_remote();
      last_btn1_press = millis();
      
      return true;
    }

    next_mode();
    return true;
  }
  return false;
}

void check_speed_button() {
  if(digitalRead(BTN2_PIN) == LOW){
    if(curr_time - last_btn2_press < 100) return;

    if(digitalRead(BTN3_PIN) == LOW) {
      increment_brightness();
      last_btn2_press = curr_time;
    } else {
      bool long_hold = check_long_hold(BTN2_PIN);
      if(long_hold && !fast_mode_switching) {
        delayTime = 18;
        flash_remote(blue);
        update_remote();
        return;
      } else if(long_hold && fast_mode_switching) {
        if(mode != LIT_MODE) glitch_mode_on = !glitch_mode_on;
      }
    }

    if(fast_mode_switching) { 
      prev_mode();
      last_btn2_press = curr_time;
      return;
    } else {
      increment_delay_time();
    }
    last_btn2_press = curr_time;
  }
}

void check_function_button() {
  if(digitalRead(BTN3_PIN) == LOW) {
    if(curr_time - last_btn3_press < 100) return;
    //execute_glitch();
    if(check_long_hold(BTN3_PIN)) {
      next_color_mode();
      last_btn3_press = curr_time;
      return;
    }
    if(mode != LIT_MODE && !fast_mode_switching) bool_op1 = !bool_op1;
    else if(mode != LIT_MODE && fast_mode_switching) increment_brightness();
    last_btn3_press = curr_time;
  }
}

void increment_brightness() {
    if(brightness < 10) brightness += 1;
    else if(brightness < 100) brightness += 25;
    else brightness += 50;
    if(brightness > 240) brightness = 0;
    pixels.setBrightness(brightness);
    remote_pixels.setBrightness(brightness);
}

bool check_long_hold(int button_pin) {
  if(digitalRead(button_pin) == LOW) {
    delay(500);
    if(digitalRead(button_pin) == LOW) {
      return true;
    }
  }
  return false;
}

bool check_color_array_logic() {
  if(digitalRead(BTN3_PIN) == LOW) {
    delay(100);
    next_color_mode();
    return true;
  }
  return false;
}

void next_color_mode() {
  flash_circle(red);
  curr_c_array_idx += 1;
  curr_c_array_idx %= COLOR_MODE_COUNT;
  curr_color_array = color_arrays[curr_c_array_idx];
  if(curr_color_array != color_18) curr_c_array_size = rgb_arr_sz;
  else curr_c_array_size = color_18_sz;
}

//check if button 1 and button 2 are pressed
//if so, toggle bool_op1
bool check_op1_logic() { 
  if(digitalRead(BTN2_PIN) == LOW) {
    delay(100);
    flash_circle(green);
    bool_op1 = !bool_op1;
    return true;
  }
  return false;
}

void next_mode() {
  mode += 1;
  mode %= 5;

  update_remote();
}

void prev_mode() {
  if(mode == 0) mode = 4;
  else mode--;
  
  update_remote();
}

void next_color() {
  color_idx += 1;
  color_idx %= curr_c_array_size;
}

void increment_delay_time() {
  delayTime++;
  if(delayTime < 10) delayTime +=  1;
  else if(delayTime >=10 && delayTime < 25) delayTime += 5;
  else delayTime += 15;

  if(delayTime > 100) delayTime = 0;
}

void save_frame(uint32_t states[][LIGHT_RING_TOTAL], int idx) {
  for(int i = 0; i < LIGHT_RING_TOTAL; i++) {
    states[idx][i] = pixels.getPixelColor(i);
  }
}

//uses a binary counter scheme to show the mode
//initally supports 15 modes
//remote is a binary counter with the top light 
//being the least significant bit
//red -> 0-7
//green -> 8-15
void update_remote() {
  uint32_t color;
  uint8_t switches;
  
  if(fast_mode_switching) color = green;
  else color = red;;

  switches = mode;
  if(switches > 7) switches -= 7;

  set_remote_lights(switches, color);
}

void set_remote_lights(uint8_t switches, uint32_t color) {
  for(int i = 0; i < 3; i++) {
    if(CHECK_BIT(switches, i)) remote_pixels.setPixelColor(i, color);
    else remote_pixels.setPixelColor(i, 0);
  }
  remote_pixels.show();
}

void flash_circle(uint32_t c) {
    for(int i = 0; i < LAYER_COUNT; i++) {
      for(int j = 0; j < LIGHT_RING_TOTAL; j++) {
        if(i % 2 == 0) {
          pixels.setPixelColor(j, c);
        } else {
          pixels.setPixelColor(j, 0); 
        }
      }
      pixels.show();
      delay(150);
    }
}

void flash_remote(uint32_t c) {
  for(int j = 0; j < 6; j++) {
    for(int i = 0; i < 3; i++) {
      if(j % 2 == 0) remote_pixels.setPixelColor(i, c);
      else remote_pixels.setPixelColor(i, 0);
    }
    remote_pixels.show();
    delay(150);
  }
}

void compute_color_18() {
  uint32_t begin_c = 0x480000;
  int shift = 16;
  
  for(int i = 0; i < 3; i++) {
    color_18[i] = begin_c + ((0x48 * (i + 1)) << shift);
  }

  begin_c = 0xFF0000;
  shift = 8;
  for(int i = 3; i < 6; i++) {
    color_18[i] = begin_c + ((0x48 * (i - 2)) << shift);
  }

  begin_c = 0xFFFF00;
  shift = 16;
  for(int i = 6; i < 9; i++) {
    color_18[i] = begin_c - ((0x48 * (i - 5)) << shift);
    if(i == 8) color_18[i] = 0x00FF00;;
  }

  begin_c = 0x00FF00;
  for(int i = 9; i < 12; i++) {
    color_18[i] = begin_c + (0x48 * (i - 8));
  }

  begin_c = 0x00FFFF;
  shift = 8;
  for(int i = 12; i < 15; i++) {
    color_18[i] = begin_c - ((0x48 * (i - 11)) << shift);
  }
  
  begin_c = 0x00FF;
  for(int i = 15; i < 18; i++) {
    color_18[i] = begin_c - (0x48 * (i - 14));
  }

}


