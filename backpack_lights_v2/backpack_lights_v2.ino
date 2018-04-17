#include <Adafruit_NeoPixel.h>

#define CHECK_BIT(var, pos) ((var) & (1<<(pos)))

#define PIN 2
#define RMT_LGHT_PIN 3

#define LIT_MODE 4
#define GLITCH_MODE_STEPS 10
#define GLITCH_FRAME_BUFFER_CNT 40
#define SPIRALING_FRAME_BUFFER_CNT 4
#define COLOR_MODE_COUNT 4
#define LAYER_COUNT 6
#define LIGHT_RING_TOTAL 93

#define BTN1_PIN 5
#define BTN2_PIN 6
#define BTN3_PIN 7

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(LIGHT_RING_TOTAL, PIN);
Adafruit_NeoPixel remote_pixels = Adafruit_NeoPixel(3, RMT_LGHT_PIN);  

bool glitch_mode_on = false;
bool rotate_mode_on = false;
int spiral_current_frame = 0;
int glitch_current_frame = 0;
uint16_t glitch_step = 0;   // how far into the glitch animation are we?
uint8_t mode = 0;           // selects the pattern mode

uint32_t color = 0xFF0000;  // default color is red

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
uint32_t color_18[18];
uint32_t *color_arrays[COLOR_MODE_COUNT];
uint32_t *curr_color_array;

const int rgb_arr_sz = 3;
const int color_18_sz = 18;
int curr_c_array_size;
int color_idx = 0;
int curr_c_array_idx = 0;

uint32_t brightness = 20;
uint32_t delayTime = 10;

uint8_t spiral_direction = 0;

bool bool_op1 = false;
bool bool_op2 = false;

int layer_counts[LAYER_COUNT] = {32, 24, 16, 12, 8, 1};
int layer_offsets[LAYER_COUNT] = {0, 32, 56, 72, 84, 92}; 

uint32_t *spiral_saved_states[SPIRALING_FRAME_BUFFER_CNT];
uint32_t *glitch_saved_states[GLITCH_FRAME_BUFFER_CNT];
uint8_t glitch_layer_order[6] = {1, 3, 0, 5, 2, 4};

void setup() {
  // compute the array with 18 colors
  compute_color_18();
  color_arrays[0] = rgb_colors;
  color_arrays[1] = bw_colors;
  color_arrays[2] = red_colors;
  color_arrays[3] = color_18;

  curr_color_array = color_arrays[curr_c_array_idx];
  curr_c_array_size = rgb_arr_sz;

  init_frame_history_arrays();

  pixels.begin();
  pixels.setBrightness(brightness);

  remote_pixels.begin();
  remote_pixels.setBrightness(brightness);
  update_remote();

  pinMode(BTN1_PIN, INPUT_PULLUP);
  pinMode(BTN2_PIN, INPUT_PULLUP);
  pinMode(BTN3_PIN, INPUT_PULLUP);

  Serial.begin(9600);
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
  if(mode != LIT_MODE) return;
  spiral(bool_op1, bool_op2);

  if(mode != LIT_MODE) return;
  pulse(bool_op1);
  
  if(mode != LIT_MODE) return;
  pulse(!bool_op1);
  
  if(mode != LIT_MODE) return;
  for(int i = 0; i < 2; i++) portal(!bool_op1, !bool_op2);

  pulse(bool_op1);
  
  if(mode != LIT_MODE) return;
  for(int i = 0; i < 30; i++) random_blink(10);
  
  if(mode != LIT_MODE) return;
  pulse(bool_op1);
  
  if(mode != LIT_MODE) return;
  pulse(!bool_op1);

  if(mode != LIT_MODE) return;
  for(int i = 0; i < 30; i++) random_blink(10);
  
  if(mode != LIT_MODE) return;
  for(int i = 0; i < 3; i++) {
    portal(bool_op1, bool_op2);
    delayTime += 20;
  } 
  for(int i = 0; i < 4; i++) {
    portal(bool_op1, bool_op2);
    delayTime -= 20;
  } 
  delayTime += 20;
  
  next_color();
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
    while(true) {
      if(spiral_smear_lights(start_color_idx)) return;
    }
  }

  if(half_cycle) return;

  for(int i = 92; i >= 0; i--) {
    if(check_buttons()) return;
    pixels.setPixelColor(i, 0);
    pixels.show();
    delay(delayTime / 3);
  }
}

bool spiral_smear_lights(int start_color_idx) {
  spiral_current_frame = 0;
  //save initial frame
  //save_frame(spiral_saved_states, spiral_current_frame++);

  //spiral lights 288 times
  //288 is the LCM of all the layer sizes as well as 18 and 3 (color array sizes)
  for(int j = 0; j < 288; j++) {
    for(int i = 0; i < LAYER_COUNT; i++) {
      if(check_buttons()) return true;
      leftRotateLayer(i, 1);
    }

    //delay(delayTime / 4);
    pixels.show();
  }

  return false;
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
    
    set_layer(i, false, false);

    if(rainbow) {
      next_color();
    }
    pixels.show();
    delay(delayTime * 3);
  }

  //zoom out and turn off
  for(int i = 0; i < LAYER_COUNT - 1; i++) {
    if(check_buttons()) return;

    set_layer((LAYER_COUNT - 1) - i, true, false);
    pixels.show();
    delay(delayTime * 3);
  }
}

void portal(bool rainbow, bool continuous) {
  next_color();

  //zoom in and turn on
  for(int i = 0; i < LAYER_COUNT; i++) {
    if(check_buttons()) return;

    set_layer(i, false, false);
    
    if(!continuous && i > 0) {
      set_layer(i - 1, true, false);
    }
    if(rainbow) {
      next_color();
    }

    pixels.show();
    delay(delayTime * 3);
  }

  if(continuous) {
    //zoom out and turn off
    for(int i = 0; i < LAYER_COUNT; i++) {
    
      if(check_buttons()) return;
  
      set_layer(i, true, false);

      pixels.show();
      delay(delayTime * 3);
    }
  }
}

void spinning() {
  
  
}

void execute_glitch() {
  glitch_mode_on = true;
  glitch_step = 0;
  glitch_current_frame = 0;
}

void execute_glitch_step() {
  switch(glitch_step) {
    case 0: 
      delayTime += 30;
      save_frame(glitch_saved_states, glitch_current_frame++);
      glitch_step++;
      break;
    case 1: 
      delayTime += 30;
      save_frame(glitch_saved_states, glitch_current_frame++);
      glitch_step++;
      break;
    case 2: 
      delayTime += 30;
      save_frame(glitch_saved_states, glitch_current_frame++);
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
    start_point = random(layer_counts[i]);

    //shut off half that ring
    shutoff_half_layer(glitch_layer_order[i], start_point, flash_circle);
    delay(2);

  }
}

void decay_layer(int layer) {

}

//degree - how many places are we shifting?
//right - twitch right or left
void twitch_layer(int layer, int degree, bool right) {
  if(right) {

  }
  else {
    leftRotateLayer(layer, degree);
  }

}

void leftRotateLayer(int layer, int n) {
  int i, j, k;
  uint32_t temp;
  for (i = 0; i < gcd(layer_counts[layer], n); i++)
  {
    /* move i-th values of blocks */
    temp = pixels.getPixelColor(layer_offsets[layer] + i);
    j = layer_offsets[layer] + i;
    while(1)
    {
      k = j + n;
      if (k >= layer_counts[layer] + layer_offsets[layer])
        k = (k - layer_counts[layer]) + layer_offsets[layer];
      if (k == i)
        break;
      pixels.setPixelColor(j, pixels.getPixelColor(layer_offsets[i] + k));
      j = k;
    }
    pixels.setPixelColor(j, temp);
  }
}


/*Fuction to get gcd of a and b*/
int gcd(int a,int b) {
   if(b==0)
     return a;
   else
     return gcd(b, a%b);
}


void set_layer(int layer, bool off, bool one_at_a_time) {
  uint32_t color;

  if(off) color = 0;
  else color =  curr_color_array[color_idx];

  for(int i = 0; i < layer_counts[layer]; i++) {
    pixels.setPixelColor(layer_offsets[layer] + i, color); 

    if(one_at_a_time) { 
      pixels.show();
      delay(delayTime / 3);
    }
  }
}

void shutoff_half_layer(int layer, int start_pt, bool continuous) {
  start_pt += layer_offsets[layer];
  for(int j = 0; j < layer_counts[layer]; j++) {
    pixels.setPixelColor(start_pt++ , 0);
      
    if(continuous) {
      pixels.show();
    }

      //reset the next point to the start of the layer
    if(start_pt > layer_offsets[layer] + (layer_counts[layer] - 1)) start_pt = layer_offsets[layer];
  }
  pixels.show();
}

void seton_half_layer(int layer, int start_pt) {
  for(int j = 0; j < layer_counts[layer]; j++) {

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
  check_speed_button();
  check_function_button();
  return check_mode_button();
}

bool check_mode_button() {
  if(digitalRead(BTN1_PIN) == LOW){
    delay(400);

    //changing op1 logic (rainbow mode)
    //initiated by pressing first two buttons
    if(check_op1_logic()) return false;

    //change colors
    //initiated by pressing first and last button
    if(check_color_array_logic()) return false;

    //check for long hold
    if(check_long_hold(BTN1_PIN)) {
      flash_circle(blue);
      mode = 4;
      return true;
    }
    next_mode();
    return true;
  }
  return false;
}

void check_speed_button() {
  if(digitalRead(BTN2_PIN) == LOW){
    delay(200);

    if(digitalRead(BTN3_PIN) == LOW) {
      increment_brightness();
    }

    increment_delay_time();
  }
}

void check_function_button() {
  if(digitalRead(BTN3_PIN) == LOW) {
    delay(50);
    Serial.println("executing_glitch!");
    //execute_glitch();
  }
}

void increment_brightness() {
    if(brightness < 20) brightness += 2;
    else brightness += 10;
    brightness %= 240;
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
    flash_circle(red);
    curr_c_array_idx += 1;
    curr_c_array_idx %= 4;
    curr_color_array = color_arrays[curr_c_array_idx];
    if(curr_color_array != color_18) curr_c_array_size = rgb_arr_sz;
    else curr_c_array_size = color_18_sz;
    return true;
  }
  return false;
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

void next_color() {
  color_idx += 1;
  color_idx %= curr_c_array_size;
}

void increment_delay_time() {
  delayTime++;
  delayTime = (delayTime % 50) + 2;
}

void init_frame_history_arrays() {
  int i;

  for(i = 0; i < GLITCH_MODE_STEPS; i++) {
    glitch_saved_states[i] = new uint32_t[LIGHT_RING_TOTAL];
  }

  for(i = 0; i < SPIRALING_FRAME_BUFFER_CNT; i++) {
    spiral_saved_states[i] = new uint32_t[LIGHT_RING_TOTAL];
  }
}

void save_frame(uint32_t **states, int idx) {
  for(int i = 0; i < LIGHT_RING_TOTAL; i++) {
    states[idx][i] = pixels.getPixelColor(idx);
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
  
  if(mode == 0) color = 0;
  else color = (8 % mode == 8) ? green : red;

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
    for(int i = 0; i < 6; i++) {
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


