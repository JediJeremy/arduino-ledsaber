
int blade_preset = 0;
// presets                   aqua green yell whit blue prpl orng red  
int   preset_hue[] =        { 144,  96,  64,  144, 160, 192,  32,   0 };
int   preset_saturation[] = { 255, 255, 255,  32,  255, 255, 255, 255 };
int   preset_buzz[] =       {  47,  48,  47,  50,  47,  50,   49,  45 };
int   preset_hum1[] =       {  50,  50,  55,  60,  50,  50,   50,  47 };
int   preset_hum2[] =       {  52,  51,  54,  60,  52,  57,   50,  47 };
int   preset_doppler[] =    {  30,  30,  35,  40,  40,  45,  -10,  30 };
int   preset_echo[] =       { 180, 200, 210, 220, 180, 192,  240, 210 };
int   preset_speed[] =      {   3,   2,   2,   3,   2,   3,   2,    1 };

// blade modes
#define BLADE_MODE_OFF        0
#define BLADE_MODE_ON         1
#define BLADE_MODE_IGNITE     2
#define BLADE_MODE_EXTINGUISH 3
#define BLADE_MODE_UNDERVOLT  4

// Total LED power limit, of all RGB LEDS together.
// If your strips are back-to-back in a tube without serious extra heatsinking, don't exceed 40% sustained power
#define BLADE_POWER_LIMIT     0.30f
// Seriously. I mean it. I heat-destroyed a blade at 100% so you don't have to. 
// It will run for a few minutes and then neopixels will start dying.
// we can also define limits for the individual channels, since that last 10% of brightness usually makes more heat than light (especially for red)
#define BLADE_POWER_LIMIT_RED      0.90f
#define BLADE_POWER_LIMIT_GREEN    0.90f
#define BLADE_POWER_LIMIT_BLUE     0.90f

// blade state
int blade_mode = BLADE_MODE_OFF;
int blade_length = 0;
int blade_brightness = BLADE_BRIGHTNESS; // start at half-brightness
int blade_hue = BLADE_HUE;
int blade_saturation = BLADE_SATURATION;

int extend_speed = 3;

// rotary knob state
int  button_mode = 0;
byte button_state = 0;

// audio state
byte global_volume = 240;
int snd_buzz_freq = 47; 
int snd_hum1_freq = 50; 
int snd_hum2_freq = 52; 
int snd_hum2_doppler = 40;
int snd_echo_decay = 128;
unsigned int entropy = 0;
void add_entropy(byte e, byte mask) {
  entropy = entropy << 1 ^ (e & mask);
}

// how many modes
#define MODE_COUNT 12

// mode light colour list
CRGB mode_color[] = {
  CRGB::White,  // mode 0 : extension
  CRGB::White,  // mode 1 : volume
  CRGB::Purple, // mode 2 : presets
  CRGB::Green,  // mode 3 : blade brightness
  CRGB::Blue,   // mode 4 : blade hue
  CRGB::Blue,   // mode 5 : blade saturation
  CRGB::Yellow, // mode 6 : buzz frequency
  CRGB::Orange, // mode 7 : hum1 frequency
  CRGB::Orange, // mode 8 : hum2 frequency
  CRGB::Red,    // mode 9 : doppler shift
  CRGB::Red,    // mode 10 : echo decay
  CRGB::Black,  // mode 11 : no action
};

// add a delta to a value, and limit the result to a range
int value_delta(int value, int delta, int vmin, int vmax) {
  int rd = 0;
  if(delta>0) {
    rd = min( vmax - value, delta);
  }
  if(delta<0) {
    rd = max( vmin - value, delta);
  }
  return value + rd;
}

/*
void update_property(int * v, int delta) {
  *v += delta;
  if(*v<0) *v=0;
  if(*v>=8192) *v=8191;
}

byte get_property(int * v) {
  int r = *v;
  return (r >> 5) & 255;
}
*/

void eeprom_restore() {
  // check the first two bytes for our magic value, indicating the eeprom isn't empty
  bool valid = (EEPROM.read(0) == 42) && (EEPROM.read(1) == !42);
  // if valid, load the properties
  if(valid) {
    button_mode = EEPROM.read(2);
    // audio state
    global_volume = EEPROM.read(3);
    snd_buzz_freq = EEPROM.read(4); 
    snd_hum1_freq = EEPROM.read(5); 
    snd_hum2_freq = EEPROM.read(6); 
    snd_hum2_doppler = EEPROM.read(7) - 128;
    snd_echo_decay = EEPROM.read(12); 
    // blade color
    blade_hue = EEPROM.read(8); 
    blade_saturation = EEPROM.read(9);
    blade_brightness = EEPROM.read(10); 
    extend_speed = EEPROM.read(11);
  }
}

void eeprom_save() {
  // save the properties
  EEPROM.update(2,button_mode);
  // sound properties
  EEPROM.update(3,global_volume);
  EEPROM.update(4,snd_buzz_freq);
  EEPROM.update(5,snd_hum1_freq);
  EEPROM.update(6,snd_hum2_freq);
  EEPROM.update(7,snd_hum2_doppler);
  EEPROM.update(12,snd_echo_decay);
  // blade color
  EEPROM.update(8,blade_hue);
  EEPROM.update(9,blade_saturation);
  EEPROM.update(10,blade_brightness);
  EEPROM.update(11,extend_speed);
  // commit with the valid token
  EEPROM.update(0,42);
  EEPROM.update(1,!42);
}

void update_blade() {
  // compute base color
  int H = blade_hue;
  int S = blade_saturation;
  int V = blade_brightness;
  CRGB color = CHSV( H, S, V );
  // limit the LED power
  float scale = 1.0;
  scale = min( scale, (BLADE_POWER_LIMIT_RED*255.0f) / (float)color.r );
  scale = min( scale, (BLADE_POWER_LIMIT_GREEN*255.0f) / (float)color.g );
  scale = min( scale, (BLADE_POWER_LIMIT_BLUE*255.0f) / (float)color.b );
  int power = (int)color.r + (int)color.g + (int)color.b;
  scale = min( scale, (BLADE_POWER_LIMIT*3.0f*255.0f) / (float)power );
  // rescale brightness;
  color = CHSV( H, S, scale * (float)V ); 
  
  // start index
  int i = 0;
  // are we in menu selection mode?
  if(button_state==2) {
    // show the mode dots
    for(int m=0; m<MODE_COUNT; m++) {
      if(m==button_mode) {
        // current menu dot. both leds full brightness
        blade_leds[i++] = blade_leds[i++] = mode_color[m];
      } else {
        // not current item. low-intensity first dot
        blade_leds[i++] = CRGB( mode_color[m].r>>4, mode_color[m].g>>4, mode_color[m].b>>4);
        blade_leds[i++] =  CRGB::Black;
      }
      blade_leds[i++] =  CRGB::Black;
    }
  }
  // set the remaining strip light values
  for(; i<BLADE_LEDS_COUNT; i++) {
    blade_leds[i] = (i<blade_length) ? color : CRGB::Black;
  }
  // update the LEDS now
  LEDS.show();
}

void ignite() {
  if(blade_mode == BLADE_MODE_OFF) {
    blade_mode = BLADE_MODE_IGNITE;
  }
}

void extinguish() {
  if(blade_mode == BLADE_MODE_ON) {
    blade_mode = BLADE_MODE_EXTINGUISH;
    // since this was done gracefully, store the current blade settings for next time
    eeprom_save();
  }
}



