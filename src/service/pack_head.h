#ifndef _COMMON_H_
#define _COMMON_H_

#define MAX_MSG_SIZE 65536

#include <stdint.h>
#include <arpa/inet.h>
#include <endian.h>

#pragma pack(1)

struct PHDR
{
  PHDR():len(0), cmd(0), uid(0), sid(0){}
  uint32_t len;   //! packet length
  uint32_t cmd;   //! command
  uint64_t uid;   //! user id
  uint32_t sid;   //! session_id created by server

};

#pragma pack()

static const uint32_t HEADER_LEN = sizeof(PHDR);

void push_pack_head(char* dest, const PHDR& header) {
  *reinterpret_cast<uint32_t*>(dest) = htonl(header.len);
  dest += sizeof(uint32_t);
  *reinterpret_cast<uint32_t*>(dest) = htonl(header.cmd);
  dest += sizeof(uint32_t);
  *reinterpret_cast<uint64_t*>(dest) = htobe64(header.uid);
  dest += sizeof(uint64_t);
  *reinterpret_cast<uint32_t*>(dest) = htonl(header.sid);
}

void pop_pack_head(const char* src, PHDR& header) {
  header.len = ntohl(*reinterpret_cast<const uint32_t*>(src));
  src += sizeof(uint32_t);
  header.cmd = ntohl(*reinterpret_cast<const uint32_t*>(src));
  src += sizeof(uint32_t);
  header.uid = be64toh(*reinterpret_cast<const uint64_t*>(src));
  src += sizeof(uint64_t);
  header.sid = ntohl(*reinterpret_cast<const  uint32_t*>(src));
}

#endif
