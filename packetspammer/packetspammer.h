#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <resolv.h>
#include <string.h>
#include <utime.h>
#include <unistd.h>
#include <getopt.h>
#include <pcap.h>
#include <endian.h>

typedef unsigned int u32;
typedef unsigned short u16;
typedef unsigned char u8;
typedef u32 __le32;

#if __BYTE_ORDER == __LITTLE_ENDIAN
#define	le16_to_cpu(x) (x)
#define	le32_to_cpu(x) (x)
#else
#define	le16_to_cpu(x) ((((x)&0xff)<<8)|(((x)&0xff00)>>8))
#define	le32_to_cpu(x) \
((((x)&0xff)<<24)|(((x)&0xff00)<<8)|(((x)&0xff0000)>>8)|(((x)&0xff000000)>>24))
#endif
#define	unlikely(x) (x)

#define	MAX_PENUMBRA_INTERFACES 8

struct radiotap_hdr {
	uint8_t revision;
	uint8_t padding;
	uint16_t length;
	//Present flags
	uint8_t TSFT:1;
	uint8_t Flags:1;
	uint8_t Rate:1;
	uint8_t Channel:1;
	uint8_t FHSS:1;
	uint8_t dBmAntennaSignal:1;
	uint8_t dBmAntennaNoise:1;
	uint8_t LockQuality:1;
	uint8_t TXAttenuation:1;
	uint8_t dbTXAttenuation:1;
	uint8_t dBmTXPower:1;
	uint8_t Antenna:1;
	uint8_t dBAntennaSignal:1;
	uint8_t dbAntennaNoise:1;
	uint8_t RXflags:1;
	uint8_t not_used:3;
	uint8_t ChannelPlus:1;
	uint8_t MCSinfo:1;
	uint8_t A_MPDUStatus:1;
	uint8_t VHTinfo:1;
	uint8_t Reserved:7;
	uint8_t RadiotapNSnext:1;
	uint8_t VendorNSnext:1;
	uint8_t Ext:1;
	//Flags
	uint8_t Flags_CFP:1;
	uint8_t Flags_Preamble:1;	//0 for long, 1 for short
	uint8_t Flags_WEP:1;
	uint8_t Flags_Fragmentation:1;
	uint8_t Flags_FCS_at_end:1;
	uint8_t Flags_DataPad:1;
	uint8_t DataPad:1;
	uint8_t BadFCS:1;
	uint8_t ShortGI:1;

	uint8_t DataRate;
	uint16_t Channel_frequency;
	//begin channel flags
	uint8_t cf_not_used:4;
	uint8_t cf_Turbo:1;
	uint8_t cf_CCK:1;
	uint8_t cf_OFDM:1;
	uint8_t cf_2GHz:1;
	uint8_t cf_5GHz:1;
	uint8_t cf_Passive:1;
	uint8_t cf_Dynamic_CCK_OFDM:1;
	uint8_t cf_GFSK:1;
	uint8_t cf_GSM:1;
	uint8_t cf_StaticTurbo:1;
	uint8_t cf_10MHz:1;
	uint8_t cf_5MHz:1;
} __attribute__ ((packed)) ;
typedef struct radiotap_hdr radiotap_hdr_t;
