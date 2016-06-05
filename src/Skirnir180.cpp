#include "Skirnir180.hpp"
#include "base64.hpp"

Skirnir180::Skirnir180(HardwareSerial* port_) : Skirnir(port) {
  port = port_;
  fsm_state = START;
  receive_buffer[240] = '\0';
}

bool Skirnir180::fsmGlobals(uint8_t payload[], uint8_t next, uint8_t input_buffer[]) {
  switch(next) {
    case '&':
      fsm_state = LPACKET_INTERMEDIATE;
      fsm_repeats = 0;
      return true;
    default:
      return Skirnir::fsmGlobals(payload, next, input_buffer);
  }
}

uint8_t Skirnir180::fsmLocals(uint8_t payload[], uint8_t next, uint8_t input_buffer[]) {
  switch(fsm_state) {
    case LPACKET_INTERMEDIATE:
      input_buffer[fsm_repeats] = next;
      if(++fsm_repeats >= 240) fsm_state = LPACKET_END;
      return 0;
    case LPACKET_END:
      fsm_state = START;
      if(next == '\n') {
        decode_base64(input_buffer, payload);
        return 180;
      }
      return 0;
    default:
      return Skirnir::fsmLocals(payload, next, input_buffer);
  }
}

uint8_t Skirnir180::receive(uint8_t payload[], uint8_t next) {
  if(fsmGlobals(payload, next, receive_buffer)) {
    return 0;
  } else {
    return fsmLocals(payload, next, receive_buffer);
  }
}

uint8_t Skirnir180::receive_until_packet(uint8_t payload[]) {
  uint8_t result = 0;
  
  while(port -> available()) {
    result = receive(payload, port -> read());
    
    if(result) {
      return result;
    }
  }
  
  return result;
}
