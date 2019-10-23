#include <iostream>
#include <cstdint>
#include <array>

typedef uint_least8_t  u8 ;
typedef uint_least16_t u16;

class cpu {
  const u8 of = 0b00000001,
           zf = 0b00000010,
           if = 0b10000000;

  u8 x  = 0x00, // X register
     y  = 0x00, // Y register
     z  = 0x00, // Flags register
     io = 0x00, // External I/O register
     sp = 0x00, // Stack Pointer
     pc = 0x00; // Program Counter

  u8 fetched = 0x00;
  u8& a, b;
  u16 opcode = 0x0000;
  u8  instruction = 0x00;
  u8  imm = 0x00;

  std::array <u8, 0xff> m;

  u8& address(u16 addr) { return m[addr]; }
  u8  read(u16 addr) { return m[addr]; }

  u8 rset (u8& a, u8& b) { a  = b; return a; }
  u8 radd (u8& a, u8& b) { a += b; return a; }
  u8 rsub (u8& a, u8& b) { a -= b; return a; }
  u8 rmul (u8& a, u8& b) { a *= b; return a; }
  u8 rdiv (u8& a, u8& b) { a /= b; return a; }
  u8 rand (u8& a, u8& b) { a &= b; return a; }
  u8 ror  (u8& a, u8& b) { a |= b; return a; }
  u8 rxor (u8& a, u8& b) { a ^= b; return a; }
  u8 rsetd(u8& a, u8  b) { a  = b; return a; }
  u8 raddd(u8& a, u8  b) { a += b; return a; }
  u8 rsubd(u8& a, u8  b) { a -= b; return a; }
  u8 rmuld(u8& a, u8  b) { a *= b; return a; }
  u8 rdivd(u8& a, u8  b) { a /= b; return a; }
  u8 randd(u8& a, u8  b) { a &= b; return a; }
  u8 rord (u8& a, u8  b) { a |= b; return a; }
  u8 rxord(u8& a, u8  b) { a ^= b; return a; }

  void fetch() {
    #define rw(addr) (read(addr) | (read(addr) << 8))
    opcode      = rw(pc);
    instruction = opcode & 0xff;
    imm         = (opcode & 0xff00)>>8;
  }

  #define testFlags(o, e) \
    u16 t = o; \
    if ((t&0xff00) >> 8) { z |= of; } else { z &= not(of); } \
    if ((t&0x0100) >> 8) { z |= cf; } else { z &= not(cf); } \

  void execute() {
    switch(instruction) {
      case 0x00: set(x, imm); ++pc; break;
      case 0x01: set(y, imm); ++pc; break;
      case 0x02: set(x, address(imm)); ++pc; break;
      case 0x03: set(y, address(imm)); ++pc; break;
      case 0x04: set(address(imm), x); ++pc; break;
      case 0x05: set(address(imm), y); ++pc; break;
      case 0x06: set(x, y); break;
      case 0x07: set(y, x); break;
      case 0x08: set(sp, x); break;
      case 0x09: set(sp, y); break;
      case 0x0a: set(sp, address(x)); break;
      case 0x0b: set(sp, address(y)); break;
      case 0x0c: set(sp, address(imm)); ++pc; break;

      case
    }
  }
};
