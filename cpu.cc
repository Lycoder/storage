#include <iostream>
#include <iomanip>
#include <cstdint>
#include <utility>
#include <thread>
#include <chrono>
#include <array>

#define _CPU_LOG
#define _CPU_EXIT_ON_BRK
#define _CPU_REALTIME
#define _CPU_FREQ 1 // In Hz

#undef _CPU_REALTIME

#define log(d) std::cout << "[d: " << __FUNCTION__ << "] " << d << "\n";
#define hex(n, w) std::hex << std::setfill('0') << std::setw(w) << (int)n

class cpu {
  typedef uint_least8_t  u8 ;
  typedef uint_least16_t u16;
  typedef  int_least8_t  s8 ;

  const u8 of = 0b00000001, // Overflow flag
           zf = 0b00000010, // Zero flag
	   cf = 0b00000100, // Carry flag
           tf = 0b10000000; // Interrupt flag 

  u8 x  = 0x00, // X register
     y  = 0x00, // Y register
     z  = 0x00, // Flags register
     sp = 0x00, // Stack Pointer
     pc = 0x00, // Program Counter
     io = 0x00; // External I/O register

  u16 opcode = 0x0000;
  u8  instruction = 0x00;
  u8  imm = 0x00;
  bool jump = false;

  std::array <u8, 0xff> m;
  
  u8& address(u16 addr) { return m[addr]; }
  u8  read(u16 addr) { return m[addr]; }

  u8 rset (u8& a, u8& b) 	{ a  = b; return a; }
  u8 radd (u8& a, u8& b) 	{ a += b; return a; }
  u8 rsub (u8& a, u8& b) 	{ a -= b; return a; }
  u8 rmul (u8& a, u8& b) 	{ a *= b; return a; }
  u8 rdiv (u8& a, u8& b) 	{ a /= b; return a; }
  u8 rand (u8& a, u8& b) 	{ a &= b; return a; }
  u8 ror  (u8& a, u8& b) 	{ a |= b; return a; }
  u8 rxor (u8& a, u8& b)	{ a ^= b; return a; }
  u8 radd2(u8& a, u8& b, u8& c) { a = b + c; return a; }
  u8 rsub2(u8& a, u8& b, u8& c) { a = b - c; return a; }
  u8 rmul2(u8& a, u8& b, u8& c) { a = b * c; return a; }
  u8 rdiv2(u8& a, u8& b, u8& c) { a = b / c; return a; }
  u8 rand2(u8& a, u8& b, u8& c) { a = b & c; return a; }
  u8 ror2 (u8& a, u8& b, u8& c) { a = b | c; return a; }
  u8 rxor2(u8& a, u8& b, u8& c) { a = b ^ c; return a; }
  u8 rsetd(u8& a, u8  b) 	{ a  = b; return a; }
  u8 raddd(u8& a, u8  b) 	{ a += b; return a; }
  u8 rsubd(u8& a, u8  b) 	{ a -= b; return a; }
  u8 rmuld(u8& a, u8  b) 	{ a *= b; return a; }
  u8 rdivd(u8& a, u8  b) 	{ a /= b; return a; }
  u8 randd(u8& a, u8  b) 	{ a &= b; return a; }
  u8 rord (u8& a, u8  b) 	{ a |= b; return a; }
  u8 rxord(u8& a, u8  b) 	{ a ^= b; return a; }
  u8 radds(u8& a, s8  b) 	{ a += b; return a; }
  u8 rsubs(u8& a, s8  b) 	{ a -= b; return a; }
  u8 rmuls(u8& a, s8  b) 	{ a *= b; return a; }
  u8 rdivs(u8& a, s8  b) 	{ a /= b; return a; }

public:
  cpu() { std::fill(m.begin(), m.end(), (u8)0xff); 
m[0] = 0x00; 
m[1] = 0xaa; // ld x, $aa
m[2] = 0x08; 
m[3] = 0xd0; // ld sp, $d0
m[4] = 0xe4; // pushx
m[5] = 0xe8; // popy
};
  
  void loop() {
  #ifdef _CPU_LOG
    log("Launched");
  #endif
    while (true) {
      this->fetch();
      this->execute();
      #ifndef _CPU_REALTIME
         std::this_thread::sleep_for(std::chrono::seconds(1/_CPU_FREQ));
      #endif
      system("clear");
    }
  }

  void fetch() {
    #define rw(addr) (read(addr) | (read(addr+1) << 8))
    opcode      = rw(pc);
    instruction = opcode & 0xff;
    imm         = (opcode & 0xff00)>>8;
    #ifdef _CPU_LOG
      log("List:" <<
	"\nopcode      = $" << hex(opcode, 4) << "\n" <<
	"instruction = $" << hex(instruction, 2) << "\n" <<
	"imm         = $" << hex(imm, 2) << "\n"
      );
    #endif
  }

  #define testFlags(c, o) if (this->z && c) { o; }

  #define applyFlags(o) { \
    cpu::u16 t = o; \
    if ((t&0xff00) >> 8) { this->z |= this->of; } else { this->z &= not(this->of); } \
    if ((t&0x0100) >> 8) { this->z |= this->cf; } else { this->z &= not(this->cf); } \
    if ((t&0xff  ) == 0) { this->z |= this->zf; } else { this->z &= not(this->zf); } \
    }

  void execute() {
    switch(instruction) {
      // Sets
      case 0x00: rset(x, imm); ++pc; break;
      case 0x01: rset(y, imm); ++pc; break;
      case 0x02: rset(x, address(imm)); ++pc; break;
      case 0x03: rset(y, address(imm)); ++pc; break;
      case 0x04: rset(address(imm), x); ++pc; break;
      case 0x05: rset(address(imm), y); ++pc; break;
      case 0x06: rset(x, y); break;
      case 0x07: rset(y, x); break;
      case 0x08: rset(sp, imm); ++pc; break;
      case 0x09: rset(sp, x); break;
      case 0x0a: rset(sp, y); break;
      case 0x0b: rset(sp, address(x)); break;
      case 0x0c: rset(sp, address(y)); break;
      case 0x0d: rset(sp, address(imm)); ++pc; break;
      
      // ALU: Basic arithmetic
      case 0x10: applyFlags(radd2(x, x, imm)); ++pc; break;
      case 0x11: applyFlags(radd2(x, y, imm)); ++pc; break;
      case 0x12: applyFlags(radd2(x, x, y)); break;
      case 0x13: applyFlags(radd2(x, y, x)); break;
      case 0x14: applyFlags(radd2(y, x, imm)); ++pc; break;
      case 0x15: applyFlags(radd2(y, y, imm)); ++pc; break;
      case 0x16: applyFlags(radd2(y, x, y)); break;
      case 0x17: applyFlags(radd2(y, y, x)); break;
      case 0x20: applyFlags(rsub2(x, x, imm)); ++pc; break;
      case 0x21: applyFlags(rsub2(x, y, imm)); ++pc; break;
      case 0x22: applyFlags(rsub2(x, x, y)); break;
      case 0x23: applyFlags(rsub2(x, y, x)); break;
      case 0x24: applyFlags(rsub2(y, x, imm)); ++pc; break;
      case 0x25: applyFlags(rsub2(y, y, imm)); ++pc; break;
      case 0x26: applyFlags(rsub2(y, x, y)); break;
      case 0x27: applyFlags(rsub2(y, y, x)); break;
      case 0x30: applyFlags(rmul2(x, x, imm)); ++pc; break;
      case 0x31: applyFlags(rmul2(x, y, imm)); ++pc; break;
      case 0x32: applyFlags(rmul2(x, x, y)); break;
      case 0x33: applyFlags(rmul2(x, y, x)); break;
      case 0x34: applyFlags(rmul2(y, x, imm)); ++pc; break;
      case 0x35: applyFlags(rmul2(y, y, imm)); ++pc; break;
      case 0x36: applyFlags(rmul2(y, x, y)); break;
      case 0x37: applyFlags(rmul2(y, y, x)); break;
      case 0x40: applyFlags(rdiv2(x, x, imm)); ++pc; break;
      case 0x41: applyFlags(rdiv2(x, y, imm)); ++pc; break;
      case 0x42: applyFlags(rdiv2(x, x, y)); break;
      case 0x43: applyFlags(rdiv2(x, y, x)); break;
      case 0x44: applyFlags(rdiv2(y, x, imm)); ++pc; break;
      case 0x45: applyFlags(rdiv2(y, y, imm)); ++pc; break;
      case 0x46: applyFlags(rdiv2(y, x, y)); break;
      case 0x47: applyFlags(rdiv2(y, y, x)); break;
      
      // ALU: Increments & decrements
      case 0x50: applyFlags(raddd(x, 1)); break;
      case 0x51: applyFlags(rsubd(x, 1)); break;
      case 0x52: applyFlags(raddd(y, 1)); break;
      case 0x53: applyFlags(rsubd(y, 1)); break;
      case 0x54: applyFlags(raddd(address(imm), 1)); ++pc; break;
      case 0x55: applyFlags(rsubd(address(imm), 1)); ++pc; break;
      case 0x56: applyFlags(raddd(address(x), 1)); break;
      case 0x57: applyFlags(rsubd(address(x), 1)); break;
      case 0x58: applyFlags(raddd(address(y), 1)); break;
      case 0x59: applyFlags(rsubd(address(y), 1)); break;      

      // ALU: Logic      
      case 0x60: applyFlags(rand2(x, x, imm)); ++pc; break;
      case 0x61: applyFlags(rand2(x, y, imm)); ++pc; break;
      case 0x62: applyFlags(rand2(x, x, y)); break;
      case 0x63: applyFlags(rand2(x, y, x)); break;
      case 0x64: applyFlags(rand2(y, x, imm)); ++pc; break;
      case 0x65: applyFlags(rand2(y, x, imm)); ++pc; break;
      case 0x66: applyFlags(rand2(y, x, y)); break;
      case 0x67: applyFlags(rand2(y, y, x)); break;
      case 0x70: applyFlags(ror2 (x, x, imm)); ++pc; break;
      case 0x71: applyFlags(ror2 (x, y, imm)); ++pc; break;
      case 0x72: applyFlags(ror2 (x, x, y)); break;
      case 0x73: applyFlags(ror2 (x, y, x)); break;
      case 0x74: applyFlags(ror2 (y, x, imm)); ++pc; break;
      case 0x75: applyFlags(ror2 (y, x, imm)); ++pc; break;
      case 0x76: applyFlags(ror2 (y, x, y)); break;
      case 0x77: applyFlags(ror2 (y, y, x)); break;
      case 0x80: applyFlags(rxor2(x, x, imm)); ++pc; break;
      case 0x81: applyFlags(rxor2(x, y, imm)); ++pc; break;
      case 0x82: applyFlags(rxor2(x, x, y)); break;
      case 0x83: applyFlags(rxor2(x, y, x)); break;
      case 0x84: applyFlags(rxor2(y, x, imm)); ++pc; break;
      case 0x85: applyFlags(rxor2(y, x, imm)); ++pc; break;
      case 0x86: applyFlags(rxor2(y, x, y)); break;
      case 0x87: applyFlags(rxor2(y, y, x)); break;
      
      // ALU: Resets
      case 0x88: rxor(x, x); break;
      case 0x89: rxor(y, y); break;
      case 0x8a: rxor(address(imm), address(imm)); ++pc; break;
      case 0x8b: rxor(address(x), address(x)); ++pc; break;
      case 0x8c: rxor(address(y), address(y)); break;

      // Calls & Return
      case 0x90: rset(address(sp++), pc); rset(pc, imm); this->jump = true; break;
      case 0x91: rset(address(sp++), pc); rset(pc, x); this->jump = true; break;
      case 0x92: rset(address(sp++), pc); rset(pc, y); this->jump = true; break;
      case 0x93: rset(pc, address(--sp)); this->jump = true; break;

      // Unconditioned jumps
      case 0xa0: rsetd(pc, imm); break;
      case 0xa1: rsetd(pc, x); break;
      case 0xa2: rsetd(pc, y); break;

      // Conditioned jumps
      case 0xa3: testFlags(this->of, rsetd(pc, imm)); this->jump = true; break;
      case 0xa4: testFlags(this->of, rsetd(pc, x));   this->jump = true; break;
      case 0xa5: testFlags(this->of, rsetd(pc, y));   this->jump = true; break;
      case 0xa6: testFlags(this->zf, rsetd(pc, imm)); this->jump = true; break;
      case 0xa7: testFlags(this->zf, rsetd(pc, x));   this->jump = true; break;
      case 0xa8: testFlags(this->zf, rsetd(pc, y));   this->jump = true; break;
      case 0xa9: testFlags(this->tf, rsetd(pc, imm)); this->jump = true; break;
      case 0xaa: testFlags(this->tf, rsetd(pc, x));   this->jump = true; break;
      case 0xab: testFlags(this->tf, rsetd(pc, y));   this->jump = true; break;
      case 0xac: testFlags(this->cf, rsetd(pc, imm)); this->jump = true; break;
      case 0xad: testFlags(this->cf, rsetd(pc, x));   this->jump = true; break;
      case 0xae: testFlags(this->cf, rsetd(pc, y));   this->jump = true; break;
      case 0xaf: testFlags(not(this->of), rsetd(pc, imm)); this->jump = true; break;
      case 0xb0: testFlags(not(this->of), rsetd(pc, x));   this->jump = true; break;
      case 0xb1: testFlags(not(this->of), rsetd(pc, y));   this->jump = true; break;
      case 0xb2: testFlags(not(this->zf), rsetd(pc, imm)); this->jump = true; break;
      case 0xb3: testFlags(not(this->zf), rsetd(pc, x));   this->jump = true; break;
      case 0xb4: testFlags(not(this->zf), rsetd(pc, y));   this->jump = true; break;
      case 0xb5: testFlags(not(this->tf), rsetd(pc, imm)); this->jump = true; break;
      case 0xb6: testFlags(not(this->tf), rsetd(pc, x));   this->jump = true; break;
      case 0xb7: testFlags(not(this->tf), rsetd(pc, y));   this->jump = true; break;
      case 0xb8: testFlags(not(this->cf), rsetd(pc, imm)); this->jump = true; break;
      case 0xb9: testFlags(not(this->cf), rsetd(pc, x));   this->jump = true; break;
      case 0xba: testFlags(not(this->cf), rsetd(pc, y));   this->jump = true; break;

      // Unconditioned branches
      case 0xbb: applyFlags(radds(pc, (s8)imm)); this->jump = true; break;
      case 0xbc: applyFlags(radds(pc, (s8)x)); this->jump = true; break;
      case 0xbd: applyFlags(radds(pc, (s8)y)); this->jump = true; break;

      // Conditioned branches
      case 0xbe: testFlags(this->of, applyFlags(radds(pc, (s8)imm))); this->jump = true; break;
      case 0xbf: testFlags(this->of, applyFlags(radds(pc, (s8)x))); this->jump = true; break;
      case 0xc0: testFlags(this->of, applyFlags(radds(pc, (s8)y))); this->jump = true; break;
      case 0xc1: testFlags(this->zf, applyFlags(radds(pc, (s8)imm))); this->jump = true; break;
      case 0xc2: testFlags(this->zf, applyFlags(radds(pc, (s8)x))); this->jump = true; break;
      case 0xc3: testFlags(this->zf, applyFlags(radds(pc, (s8)y))); this->jump = true; break;
      case 0xc4: testFlags(this->tf, applyFlags(radds(pc, (s8)imm))); this->jump = true; break;
      case 0xc5: testFlags(this->tf, applyFlags(radds(pc, (s8)x))); this->jump = true; break;
      case 0xc6: testFlags(this->tf, applyFlags(radds(pc, (s8)y))); this->jump = true; break;
      case 0xc7: testFlags(this->cf, applyFlags(radds(pc, (s8)imm))); this->jump = true; break;
      case 0xc8: testFlags(this->cf, applyFlags(radds(pc, (s8)x))); this->jump = true; break;
      case 0xc9: testFlags(this->cf, applyFlags(radds(pc, (s8)y))); this->jump = true; break;
      case 0xca: testFlags(not(this->of), applyFlags(radds(pc, (s8)imm))); this->jump = true; break;
      case 0xcb: testFlags(not(this->of), applyFlags(radds(pc, (s8)x))); this->jump = true; break;
      case 0xcc: testFlags(not(this->of), applyFlags(radds(pc, (s8)y))); this->jump = true; break;
      case 0xcd: testFlags(not(this->zf), applyFlags(radds(pc, (s8)imm))); this->jump = true; break;
      case 0xce: testFlags(not(this->zf), applyFlags(radds(pc, (s8)x))); this->jump = true; break;
      case 0xcf: testFlags(not(this->zf), applyFlags(radds(pc, (s8)y))); this->jump = true; break;
      case 0xd0: testFlags(not(this->tf), applyFlags(radds(pc, (s8)imm))); this->jump = true; break;
      case 0xd1: testFlags(not(this->tf), applyFlags(radds(pc, (s8)x))); this->jump = true; break;
      case 0xd2: testFlags(not(this->tf), applyFlags(radds(pc, (s8)y))); this->jump = true; break;
      case 0xd3: testFlags(not(this->cf), applyFlags(radds(pc, (s8)imm))); this->jump = true; break;
      case 0xd4: testFlags(not(this->cf), applyFlags(radds(pc, (s8)x))); this->jump = true; break;
      case 0xd5: testFlags(not(this->cf), applyFlags(radds(pc, (s8)y))); this->jump = true; break;

      // Stack operations
      case 0xe0: rset(address(sp++), imm); ++pc; break;
      case 0xe1: rset(address(sp++), address(imm)); ++pc; break;
      case 0xe2: rset(address(sp++), address(x)); break;
      case 0xe3: rset(address(sp++), address(y)); break;
      case 0xe4: rset(address(sp++), x); break;
      case 0xe5: rset(address(sp++), y); break;
      case 0xe6: rset(address(sp++), x); rset(address(sp++), y); break;
      case 0xe7: rset(x, address(--sp)); break;
      case 0xe8: rset(y, address(--sp)); break;
      case 0xe9: rset(x, address(--sp)); rset(y, address(--sp)); break;

      // I/O operations
      case 0xf0: rset(x, io); break;
      case 0xf1: rset(io, x); break;
      case 0xf2: rset(y, io); break;
      case 0xf3: rset(io, y); break;
      case 0xf4: rset(address(imm), io); ++pc; break;
      case 0xf5: rset(io, address(imm)); ++pc; break;
      case 0xf6: rset(address(x), io); break;
      case 0xf7: rset(io, address(x)); break;
      case 0xf8: rset(address(y), io); break;
      case 0xf9: rset(io, address(y)); break;

      // Misc.
      case 0xfe: break;
      case 0xff:
      #ifndef _CPU_EXIT_ON_BRK	 
        while (!(this->z && this->tf)) {}
      #endif 
      break;
      default:
        #ifdef _CPU_LOG
          log("Unimplemented instruction $" << hex(instruction, 2));
        #endif
      break;
    } if (!jump) { ++this->pc; } else { this->jump = false; };
    #ifdef _CPU_LOG
      log("List:" <<
	"\npc = $" << hex(pc, 2) << "\n" <<
	"sp = $" << hex(sp, 2) << "\n" <<
	"x  = $" << hex(x , 2) << "\n" <<
        "y  = $" << hex(y , 2) << "\n" <<
        "z  = $" << hex(z , 2) << "\n" <<
        "io = $" << hex(io, 2) << "\n"
      )
    #endif
  }
};

int main() {
  cpu myCPU;
  system("clear");
  std::thread _cpuloop(&cpu::loop, &myCPU);
  _cpuloop.join();
  return 0;
}
