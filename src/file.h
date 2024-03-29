// TODO should be rle

/*
 * Author - Bertrand Songis <bsongis@gmail.com>
 * 
 * Based on th9x -> http://code.google.com/p/th9x/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
#ifndef file_h
#define file_h

#if __GNUC__
#define PACK( __Declaration__ ) __Declaration__ __attribute__((__packed__))
#else
#include "../winbuild/winbuild.h"
#endif

#include "eeprominterface.h"

#define ERR_NONE 0
#define ERR_FULL 1
#define ERR_TMO  2

PACK(struct DirEnt {
  uint8_t  startBlk;
  uint16_t size:12;
  uint16_t typ:4;
});

PACK(struct DirEntArm {
  uint16_t startBlk;
  uint16_t size:12;
  uint16_t typ:4;
});

PACK(struct EeFs{
  uint8_t  version;
  uint8_t  mySize;
  uint8_t  freeList;
  uint8_t  bs;
  DirEnt   files[36];
});

PACK(struct EeFsArm {
  uint8_t   version;
  uint16_t  mySize;
  uint16_t  freeList;
  uint8_t   bs;
  uint8_t  spare[2];
  DirEntArm files[62];
});

struct t_eeprom_header
{
  uint32_t sequence_no ;          // sequence # to decide which block is most recent
  uint16_t data_size ;                    // # bytes in data area
  uint8_t flags ;
  uint8_t hcsum ;
};

class EFile
{
  uint8_t       m_fileId;    //index of file in directory = filename
  unsigned int  m_pos;       //over all filepos
  unsigned int  m_currBlk;   //current block.id
  unsigned int  m_ofs;       //offset inside of the current block
  uint8_t       m_zeroes;    //control byte for run length decoder
  uint8_t       m_bRlc;      //control byte for run length decoder
  unsigned int  m_err;       //error reasons
  uint16_t      m_size;

  BoardEnum board;
  uint8_t *eeprom;
  unsigned int eeprom_size;
  EeFs         *eeFs;
  EeFsArm      *eeFsArm;
  unsigned int eeFsVersion;
  unsigned int eeFsBlockSize;
  unsigned int eeFsSize;
  unsigned int eeFsFirstBlock;
  unsigned int eeFsBlocksOffset;
  unsigned int eeFsBlocksMax;
  unsigned int eeFsLinkSize;

  void eeprom_read_block (void *pointer_ram, unsigned int pointer_eeprom, size_t size);
  void eeprom_write_block(const void *pointer_ram, unsigned int pointer_eeprom, size_t size);

  uint8_t EeFsRead(unsigned int blk, unsigned int ofs);
  void EeFsWrite(unsigned int blk, unsigned int ofs, uint8_t val);

  unsigned int EeFsGetLink(unsigned int blk);
  void EeFsSetLink(unsigned int blk, unsigned int val);
  uint8_t EeFsGetDat(unsigned int blk, unsigned int ofs);
  void EeFsSetDat(unsigned int blk, unsigned int ofs, const uint8_t *buf, unsigned int len);
  unsigned int EeFsGetFree();
  void EeFsFree(unsigned int blk); // free one or more blocks
  unsigned int EeFsAlloc(); // alloc one block from freelist

public:

  EFile();

  void EeFsCreate(uint8_t *eeprom, int size, BoardEnum board);

  bool EeFsOpen(uint8_t *eeprom, int size, BoardEnum board);

  ///open file for reading, no close necessary
  ///for writing use writeRlc() or create()
  unsigned int openRd(unsigned int i_fileId);
  /// create a new file with given fileId, 
  /// !!! if this file already exists, then all blocks are reused
  /// and all contents will be overwritten.
  /// after writing closeTrunc has to be called
  void    create(unsigned int i_fileId, unsigned int typ);
  /// close file and truncate the blockchain if to long.
  void    closeTrunc();

  ///open file, write to file and close it. 
  ///If file existed before, then contents is overwritten. 
  ///If file was larger before, then unused blocks are freed
  unsigned int writeRlc1(unsigned int i_fileId, unsigned int typ, const uint8_t *buf, unsigned int i_len);
  unsigned int writeRlc2(unsigned int i_fileId, unsigned int typ, const uint8_t *buf, unsigned int i_len);

  unsigned int read(uint8_t *buf, unsigned int i_len);
  unsigned int write1(uint8_t b);
  unsigned int write(const uint8_t *buf, unsigned int i_len);

  ///return size of compressed file without block overhead
  unsigned int size(unsigned int id);
  ///read from opened file and decode rlc-coded data
  unsigned int readRlc12(uint8_t *buf, unsigned int i_len, bool rlc2);
  inline unsigned int readRlc1(uint8_t *buf, unsigned int i_len)
  {
    return readRlc12(buf, i_len, false);
  }
  inline unsigned int readRlc2(uint8_t *buf, unsigned int i_len)
  {
    return readRlc12(buf, i_len, true);
  }

  uint8_t byte_checksum(uint8_t *p, unsigned int size);
  unsigned int ee32_check_header(struct t_eeprom_header *hptr);
  unsigned int get_current_block_number(unsigned int block_no, uint16_t *p_size);
};

#endif
