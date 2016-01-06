#ifndef __REGISTRY_H_
#define __REGISTRY_H_

typedef enum {
  REG_DELETED = 0,
  REG_BRIGHTNESS_INTERVAL,
  REG_ETHERNET_MAC,
  REG_ETHERNET_DHCP,
  REG_ETHERNET_IP4_ADDR,
  REG_ETHERNET_IP4_NETMASK,
  REG_ETHERNET_IP4_GATEWAY,
  REG_ETHERNET_IP4_DNS,
  REG_CRC = 0xff,
} RID_t;

typedef enum {
  REG_STATUS_OK,
  REG_STATUS_NOT_FOUND,
  REG_STATUS_ERROR,
} REG_STATUS_t;

typedef enum {
  CRC_VERIFY,
  CRC_WRITE,
  CRC_REORG,
} REG_CRC_t;

void registry_init(void);
REG_STATUS_t registry_get(RID_t rid, void *data);
void registry_set(RID_t rid, void *data, uint16_t len);

#endif
