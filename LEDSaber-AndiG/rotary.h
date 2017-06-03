#ifdef CONTROL_ROTARY

#define BUTTON_DEBOUNCE  3
#define ROTARY_DEBOUNCE  2


// use encoder library
#include "encoder.h"
Encoder knob(ROTARY_D1_PIN, ROTARY_D2_PIN);

// debounce state
byte sw_state = 1; byte sw_bounce = 0;
byte d1_state = 1; byte d1_bounce = 0;
byte d2_state = 1; byte d2_bounce = 0;


byte debounce_change(byte input, byte * state, byte * bounce, byte debounce) {
  // different from state?
  if(input == *(state) ) {
    // reduce the counter
    if( *(bounce) > 0) *(bounce)--;
    return 0;
  } else {
    // still bouncing?
    if(++(*(bounce)) >= BUTTON_DEBOUNCE) {
      // commit the change
      *(state) = input;
      *(bounce) == 0;
      // return the delta code
      if(input == HIGH) {
        return 2; // now up
      } else {
        return 1; // now down
      }
    } 
  }
}


void button_click() {
  // if we are properly off, then ignite
  switch(blade_mode) {
    case BLADE_MODE_OFF: 
      ignite();
      break;
  }
}

void button_held() {
  // toggle the blade
  switch(blade_mode) {
    case BLADE_MODE_OFF: 
    case BLADE_MODE_EXTINGUISH: 
      ignite();
      break;
    case BLADE_MODE_ON: 
    case BLADE_MODE_IGNITE: 
      extinguish();
      break;
  }
}

void rotary_delta(int d) {
  // Serial.print("delta "); Serial.println(d);
  switch(button_state) {
    case 0: // button up
      // modify property
      switch(button_mode) {
        // blade extension
        case 0: 
          extend_speed = value_delta(extend_speed, d, 1, 8);
          // turn blade on and off
          switch(blade_mode) {
            case BLADE_MODE_OFF: 
            case BLADE_MODE_EXTINGUISH: 
              if(d>0) ignite();
              break;
            case BLADE_MODE_ON: 
              if(d<0) extinguish();
              break;
            case BLADE_MODE_IGNITE: 
              if((d<0) && (extend_speed==1)) extinguish();
              break;
            case BLADE_MODE_UNDERVOLT:
              if(d<0) blade_mode = BLADE_MODE_OFF;
              break; 
          }
          break;
        // global volume
        case 1: global_volume = value_delta(global_volume, d*16, 0, 255); break;
        // blade preset
        case 2: 
          // change preset number
          blade_preset = value_delta(blade_preset, d, 0, 7);
          // load preset values
          blade_hue = preset_hue[blade_preset];
          blade_saturation = preset_saturation[blade_preset];
          extend_speed = preset_speed[blade_preset];
          update_blade(); 
          snd_buzz_freq = preset_buzz[blade_preset];
          snd_hum1_freq = preset_hum1[blade_preset];
          snd_hum2_freq = preset_hum2[blade_preset];
          snd_hum2_doppler = preset_doppler[blade_preset];
          snd_echo_decay = preset_echo[blade_preset];
          break;
        // blade properties
        case 3: blade_brightness = value_delta(blade_brightness, d*8, 7, 255); update_blade();  break;      
        case 4: blade_hue += d*4; update_blade();  break;
        case 5: blade_saturation = value_delta(blade_saturation, d*8, 0, 255); update_blade();  break;
        // sound properties
        case 6: snd_buzz_freq += d; break;
        case 7: snd_hum1_freq += d; break;
        case 8: snd_hum2_freq += d; break;
        case 9: snd_hum2_doppler += d; break;
        case 10: snd_echo_decay = value_delta(snd_echo_decay, d, 0, 255); break;
        
      }
      break;
    case 1: // button down
       // we're selecting an option now
       button_state = 2;
    case 2: // selecting option
      // modify mode
      button_mode = value_delta(button_mode, d, 0, MODE_COUNT-1);
      update_blade(); 
      break;
  }
}

byte button_last = 0;
byte button_bounce = 0;
int button_longpress = 0;
#define LONGPRESS_DELAY 200

void check_button() {
  // check for changes
  byte delta = debounce_change( digitalRead(ROTARY_SW_PIN), &sw_state, &sw_bounce, BUTTON_DEBOUNCE);
  // call button actions
  switch(delta) {
    case 1: 
      button_state = 1;
      //update_blade(); 
      button_longpress = LONGPRESS_DELAY;
      break;
    case 2:
      // did we ever select a menu option?
      if(button_state==1) {
        // no, so this was a short click.
        button_click();
      }
      button_state = 0;
      update_blade();
      button_longpress = 0;
      break;
    default:
      if(button_longpress > 0) {
        if(--button_longpress == 0) {
          // long press event
          button_held();
          button_state = 4; // so we don't single-click afterwards
        }
      }
  }
}

void check_rotary() {
  // read the 4x encoder value
  int v = knob.read();
  // divide down to single increment per notch
  int delta = v/4;
  if(delta != 0) {
    knob.write( v - (delta*4) );
    // then the delta determines the direction
    rotary_delta( ROTARY_DIR_A * delta  );
    // and reset the long press counter
    button_longpress = 0;
  }
}

void start_inputs() {
  pinMode(ROTARY_SW_PIN, INPUT_PULLUP); 
  pinMode(ROTARY_D1_PIN, INPUT_PULLUP); 
  pinMode(ROTARY_D2_PIN, INPUT_PULLUP); 
#ifdef ROTARY_VCC_PIN
  pinMode(ROTARY_VCC_PIN, OUTPUT);  digitalWrite(ROTARY_VCC_PIN, HIGH);
#endif
#ifdef ROTARY_GND1_PIN
  pinMode(ROTARY_GND1_PIN, OUTPUT);  digitalWrite(ROTARY_GND1_PIN, LOW);
#endif
#ifdef ROTARY_GND2_PIN
  pinMode(ROTARY_GND2_PIN, OUTPUT);  digitalWrite(ROTARY_GND2_PIN, LOW);
#endif
  // initial state
  knob.read();
  knob.write(0);
  sw_state = digitalRead(ROTARY_SW_PIN);
  update_blade();
}


#endif

