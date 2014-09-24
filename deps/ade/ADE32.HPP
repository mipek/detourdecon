
// ADE32 v2.02

#ifndef __ADE32_HPP__
#define __ADE32_HPP__

#define C_ERROR   0xFFFFFFFF
#define C_ADDR1   0x00000001
#define C_ADDR2   0x00000002
#define C_ADDR4   0x00000004
#define C_LOCK    0x00000008
#define C_67      0x00000010
#define C_66      0x00000020
#define C_REP     0x00000040
#define C_SEG     0x00000080
#define C_ANYPREFIX  (C_66+C_67+C_LOCK+C_REP+C_SEG)
#define C_DATA1   0x00000100
#define C_DATA2   0x00000200
#define C_DATA4   0x00000400
#define C_SIB     0x00000800
#define C_ADDR67  0x00001000
#define C_DATA66  0x00002000
#define C_MODRM   0x00004000
#define C_BAD     0x00008000
#define C_OPCODE2 0x00010000
#define C_REL     0x00020000
#define C_STOP    0x00040000

#pragma pack(push)
#pragma pack(1)

struct disasm_struct
{
  unsigned char  disasm_defaddr;         // 00
  unsigned char  disasm_defdata;         // 01
  unsigned long disasm_len;             // 02 03 04 05
  unsigned long disasm_flag;            // 06 07 08 09
  unsigned long disasm_addrsize;        // 0A 0B 0C 0D
  unsigned long disasm_datasize;        // 0E 0F 10 11
  unsigned char  disasm_rep;             // 12
  unsigned char  disasm_seg;             // 13
  unsigned char  disasm_opcode;          // 14
  unsigned char  disasm_opcode2;         // 15
  unsigned char  disasm_modrm;           // 16
  unsigned char  disasm_sib;             // 17
  union
  {
  unsigned char  disasm_addr_b[8];       // 18 19 1A 1B  1C 1D 1E 1F

  unsigned short  disasm_addr_w[4];
  unsigned long disasm_addr_d[2];
  char  disasm_addr_c[8];
  short disasm_addr_s[4];
  long  disasm_addr_l[2];
  };
  union
  {
  unsigned char  disasm_data_b[8];       // 20 21 22 23  24 25 26 27
  unsigned short  disasm_data_w[4];
  unsigned long disasm_data_d[2];
  char  disasm_data_c[8];
  short disasm_data_s[4];
  long  disasm_data_l[2];
  };
}; // disasm_struct

#pragma pack(pop)

int ade32_disasm(unsigned char* opcode0, disasm_struct* diza);

#endif // __ADE32_HPP__

