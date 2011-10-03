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


#include "stdio.h"
#include "inttypes.h"
#include "string.h"
#include "file.h"
#include <assert.h>
#include <algorithm>

EFile::EFile():
eeprom(NULL),
eeprom_size(0),
eeFs(NULL)
{
}

void EFile::EeFsInit(uint8_t *eeprom, int size, bool format)
{
  this->eeprom = eeprom;
  this->eeprom_size = size;
  eeFs = (EeFs *)eeprom;

  if (format) {
    memset(eeprom, 0, size);
    eeFs->version  = EEFS_VERS;
    eeFs->mySize   = sizeof(EeFs);
    eeFs->freeList = 0;
    eeFs->bs       = BS;
    for (int i=FIRSTBLK; i<std::min(eeprom_size/BS, 255)-1; i++) EeFsSetLink(i, i+1);
    EeFsSetLink(std::min(eeprom_size/BS, 255)-1, 0);
    eeFs->freeList = FIRSTBLK;
    EeFsFlush();
  }
}

void EFile::eeprom_read_block (void *pointer_ram, uint16_t pointer_eeprom, size_t size)
{
  memcpy(pointer_ram,&eeprom[pointer_eeprom],size);
}

void EFile::eeWriteBlockCmp(void *i_pointer_ram, uint16_t i_pointer_eeprom, size_t size)
{
  memcpy(&eeprom[i_pointer_eeprom],i_pointer_ram,size);
}

uint8_t EFile::EeFsRead(uint8_t blk,uint8_t ofs)
{
  uint8_t ret;
  eeprom_read_block(&ret,(uint16_t)(blk*BS+ofs),1);
  return ret;
}

void EFile::EeFsWrite(uint8_t blk,uint8_t ofs,uint8_t val)
{
  eeWriteBlockCmp(&val, (uint16_t)(blk*BS+ofs), 1);
}

uint8_t EFile::EeFsGetLink(uint8_t blk)
{
  return EeFsRead(blk, 0);
}

void EFile::EeFsSetLink(uint8_t blk, uint8_t val)
{
  EeFsWrite(blk, 0, val);
}

uint8_t EFile::EeFsGetDat(uint8_t blk,uint8_t ofs)
{
  return EeFsRead(blk,ofs+1);
}

void EFile::EeFsSetDat(uint8_t blk, uint8_t ofs, uint8_t *buf, uint8_t len)
{
  //EeFsWrite( blk,ofs+1,val);
  eeWriteBlockCmp(buf, (uint16_t)(blk*BS+ofs+1), len);
}

void EFile::EeFsFlushFreelist()
{
  // eeWriteBlockCmp(&eeFs->freeList, eeFs->freeList, sizeof(eeFs->freeList));
}

void EFile::EeFsFlush()
{
  // eeWriteBlockCmp(eeFs, 0, sizeof(eeFs));
}

uint16_t EFile::EeFsGetFree()
{
  uint16_t  ret = 0;
  uint8_t i = eeFs->freeList;
  while (i) {
    ret += BS-1;
    i = EeFsGetLink(i);
  }
  return ret;
}

/*
 * free one or more blocks
 */
void EFile::EeFsFree(uint8_t blk)
{
  uint8_t i = blk;
  while( EeFsGetLink(i)) i = EeFsGetLink(i);
  EeFsSetLink(i,eeFs->freeList);
  eeFs->freeList = blk; //chain in front
  EeFsFlushFreelist();
}

/*
 * alloc one block from freelist
 */
uint8_t EFile::EeFsAlloc()
{
  uint8_t ret=eeFs->freeList;
  if (ret){
    eeFs->freeList = EeFsGetLink(ret);
    EeFsFlushFreelist();
    EeFsSetLink(ret,0);
  }
  return ret;
}

bool EFile::EeFsOpen()
{
  eeprom_read_block(&eeFs, 0, sizeof(eeFs));
  return eeFs->version == EEFS_VERS && eeFs->mySize == sizeof(eeFs);
}

bool EFile::exists(uint8_t i_fileId)
{
  return eeFs->files[i_fileId].startBlk;
}

void EFile::swap(uint8_t i_fileId1,uint8_t i_fileId2)
{
  DirEnt            tmp = eeFs->files[i_fileId1];
  eeFs->files[i_fileId1] = eeFs->files[i_fileId2];
  eeFs->files[i_fileId2] = tmp;;
  EeFsFlush();
}

void EFile::rm(uint8_t i_fileId)
{
  uint8_t i = eeFs->files[i_fileId].startBlk;
  memset(&(eeFs->files[i_fileId]), 0, sizeof(eeFs->files[i_fileId]));
  EeFsFlush(); //chained out

  if(i) EeFsFree( i ); //chain in
}

uint16_t EFile::size(uint8_t id)
{
  return eeFs->files[id].size;
}


uint8_t EFile::openRd(uint8_t i_fileId)
{
  m_fileId = i_fileId;
  m_pos      = 0;
  m_currBlk  = eeFs->files[m_fileId].startBlk;
  m_ofs      = 0;
  m_zeroes   = 0;
  m_bRlc     = 0;
  m_err      = ERR_NONE;       //error reasons
  return eeFs->files[m_fileId].typ;
}

uint8_t EFile::read(uint8_t*buf, uint16_t i_len)
{
  uint16_t len = eeFs->files[m_fileId].size - m_pos;
  if (len < i_len) i_len = len;
  len = i_len;
  while(len) {
    if(!m_currBlk) break;
    *buf++ = EeFsGetDat(m_currBlk, m_ofs++);
    if (m_ofs >= (BS-1)){
      m_ofs=0;
      m_currBlk=EeFsGetLink(m_currBlk);
    }
    len--;
  }
  m_pos += i_len - len;
  return i_len - len;
}

// G: Read runlength (RLE) compressed bytes into buf.
uint16_t EFile::readRlc12(uint8_t*buf, uint16_t i_len, bool rlc2)
{
  uint16_t i=0;
  for( ; 1; ) {
    uint8_t l = std::min<uint16_t>(m_zeroes, i_len-i);
    memset(&buf[i],0,l);
    i        += l;
    m_zeroes -= l;
    if(m_zeroes) break;

    l=std::min<uint16_t>(m_bRlc, i_len-i);
    uint8_t lr = read(&buf[i], l);
    i        += lr ;
    m_bRlc   -= lr;
    if(m_bRlc) break;

    if (read(&m_bRlc,1)!=1) break; //read how many bytes to read

    assert(m_bRlc & 0x7f);
    if (rlc2) {
      if(m_bRlc&0x80){ // if contains high byte
        m_zeroes  =(m_bRlc>>4) & 0x7;
        m_bRlc    = m_bRlc & 0x0f;
      }
      else if(m_bRlc&0x40){
        m_zeroes  = m_bRlc & 0x3f;
        m_bRlc    = 0;
      }
      //else   m_bRlc
    }
    else {
      if(m_bRlc&0x80){ // if contains high byte
        m_zeroes  = m_bRlc & 0x7f;
        m_bRlc    = 0;
      }
    }
  }
  return i;
}

uint8_t EFile::write1(uint8_t b)
{
  return write(&b, 1);
}

uint8_t EFile::write(uint8_t *buf, uint8_t i_len)
{
  uint8_t len = i_len;
  if (!m_currBlk && m_pos==0)
  {
    eeFs->files[m_fileId].startBlk = m_currBlk = EeFsAlloc();
  }
  while (len)
  {
    if (!m_currBlk) {
      m_err = ERR_FULL;
      return 0;
    }
    if (m_ofs>=(BS-1)) {
      m_ofs=0;
      if (!EeFsGetLink(m_currBlk) ){
        EeFsSetLink(m_currBlk, EeFsAlloc());
      }
      m_currBlk = EeFsGetLink(m_currBlk);
    }
    if (!m_currBlk) {
      m_err = ERR_FULL;
      return 0;
    }
    uint8_t l = BS-1-m_ofs; if(l>len) l=len;
    EeFsSetDat(m_currBlk, m_ofs, buf, l);
    buf   +=l;
    m_ofs +=l;
    len   -=l;
  }
  m_pos += i_len - len;
  return i_len - len;
}

void EFile::create(uint8_t i_fileId, uint8_t typ)
{
  openRd(i_fileId); //internal use
  eeFs->files[i_fileId].typ      = typ;
  eeFs->files[i_fileId].size     = 0;
}

void EFile::closeTrunc()
{
  uint8_t fri=0;
  eeFs->files[m_fileId].size     = m_pos;
  if (m_currBlk && ( fri = EeFsGetLink(m_currBlk))) EeFsSetLink(m_currBlk, 0);
  EeFsFlush(); //chained out

  if(fri) EeFsFree( fri );  //chain in
}

uint16_t EFile::writeRlc1(uint8_t i_fileId, uint8_t typ, uint8_t *buf, uint16_t i_len)
{
  create(i_fileId, typ);
  bool state0 = true;
  uint8_t cnt = 0;
  uint16_t i;

  //RLE compression:
  //rb = read byte
  //if (rb | 0x80) write rb & 0x7F zeros
  //else write rb bytes
  for (i=0; i<=i_len; i++)
  {
    bool nst0 = buf[i] == 0;                   
    if (nst0 && !state0 && buf[i+1]!=0) nst0 = false ;
    if (nst0 != state0 || cnt>=0x7f || i==i_len) {
      if (state0) {
        if(cnt>0) {
          cnt |= 0x80;
          if (write(&cnt,1)!=1)           goto error;
          cnt=0;
        }
      }
      else {
        if (cnt>0) {
          if (write(&cnt,1) !=1)            goto error;
          uint8_t ret=write(&buf[i-cnt],cnt);
          if (ret!=cnt) { cnt-=ret;        goto error;}
          cnt=0;
        }
      }
      state0 = nst0;
    }
    cnt++;
  }
  if(0){
    error:
    i_len = i - (cnt & 0x7f);
  }
  closeTrunc();
  return i_len;
}

/*
 * Write runlength (RLE) compressed bytes
 */
uint16_t EFile::writeRlc2(uint8_t i_fileId, uint8_t typ, uint8_t *buf, uint16_t i_len)
{
  create(i_fileId, typ);
  bool    run0   = buf[0] == 0;
  uint8_t cnt    = 1;
  uint8_t cnt0   = 0;
  uint16_t i     = 0;
  if (i_len==0) goto close;

  //RLE compression:
  //rb = read byte
  //if (rb | 0x80) write rb & 0x7F zeros
  //else write rb bytes
  for (i=1; 1; i++) { // !! laeuft ein byte zu weit !!
    bool cur0 = buf[i] == 0;
    if (cur0 != run0 || cnt==0x3f || (cnt0 && cnt==0xf)|| i==i_len){
      if (run0){
        assert(cnt0==0);
        if (cnt<8 && i!=i_len)
          cnt0 = cnt; //aufbew fuer spaeter
        else {
          if (write1(cnt|0x40)!=1)                goto error;//-cnt&0x3f
        }
      }
      else {
        if (cnt0) {
          if (write1(0x80 | (cnt0<<4) | cnt)!=1)  goto error;//-cnt0xx-cnt
          cnt0 = 0;
        }
        else {
          if (write1(cnt)!=1)                    goto error;//-cnt
        }
        uint8_t ret = write(&buf[i-cnt], cnt);
        if (ret != cnt) { cnt-=ret;                goto error;}//-cnt
      }
      cnt=0;
      if (i==i_len) break;
      run0 = cur0;
    }
    cnt++;
  }
  if (0) {
    error:
    i-=cnt+cnt0;
  }

  close:
  closeTrunc();
  return i;
}

