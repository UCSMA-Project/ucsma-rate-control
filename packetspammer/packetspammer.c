// (c)2007 Andy Green <andy@warmcat.com>

/*
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; version 2.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License along
 *   with this program; if not, write to the Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

// Thanks for contributions:
// 2007-03-15 fixes to getopt_long code by Matteo Croce rootkit85@yahoo.it

#include "packetspammer.h"
#include "radiotap.h"
#include <pthread.h>
#include<stdio.h>
#include<stdlib.h>
#include<errno.h>
#include<fcntl.h>
#include<string.h>
#include<unistd.h>


/* wifi bitrate to use in 500kHz units */

static const u8 u8aRatesToUse[] = {

	54*2,
	48*2,
	36*2,
	24*2,
	18*2,
	12*2,
	9*2,
	11*2,
	11, // 5.5
	2*2,
	1*2
};

/* this is the template radiotap header we send packets out with */

static const u8 u8aRadiotapHeader[] = {

	0x00, // <-- radiotap version
	0x00, // <-- pad
	0x19, 0x00, // <- radiotap header length
	0x6f, 0x08, 0x00, 0x00, // <-- bitmap
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // <-- timestamp
	0x02, // <-- flags (Offset +0x10)
	0x16, // <-- rate (0ffset +0x11)
	0x8f, 0x09, // <-- channel frequency
	0xa0, 0x00, // <-- channel flags
	0x00, // <-- antsignal
	0x00, // <-- antnoise
	0x00, // <-- antenna

};
#define	OFFSET_FLAGS 0x10
#define	OFFSET_RATE 0x11

/* Penumbra IEEE80211 header */

static u8 u8aIeeeHeader[] = {
	0x08, 0x01, 0x00, 0x00,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,	//Receiver Address
	0x13, 0x22, 0x33, 0x44, 0x55, 0x66,	//Destination Address
	0x13, 0x22, 0x33, 0x44, 0x55, 0x66,	//Source Address
	0x10, 0x86,
};

// this is where we store a summary of the
// information from the radiotap header

typedef struct  {
	int m_nChannel;
	int m_nChannelFlags;
	int m_nRate;
	int m_nAntenna;
	int m_nRadiotapFlags;
} __attribute__((packed)) PENUMBRA_RADIOTAP_DATA;

typedef struct {
	int sent_byte;
	int sleep_interval;
	int sent_packet;
} statistics;

int wait(int);

void *print_speed(void *stats_ptr) {
	puts("sub-thread initialized");
	statistics *stats = (statistics *) stats_ptr;
	int s_byte, e_byte, s_packet, e_packet;
	double speed;
	while (1) {
		s_byte = stats->sent_byte;
		s_packet = stats->sent_packet;
		usleep(stats->sleep_interval * 1000000);
		e_byte = stats->sent_byte;
		e_packet = stats->sent_packet;
		speed = (e_byte - s_byte) / (float)(stats->sleep_interval * 1024);
		printf("sending rate: %.2fKB/s %.2fpps\n", speed,
				(e_packet - s_packet) / (float)stats->sleep_interval);
	}
}

int flagHelp = 0, flagMarkWithFCS = 0;

void
Dump(u8 * pu8, int nLength)
{
	char sz[256], szBuf[512], szChar[17], *buf, fFirst = 1;
	unsigned char baaLast[2][16];
	uint n, nPos = 0, nStart = 0, nLine = 0, nSameCount = 0;

	buf = szBuf;
	szChar[0] = '\0';

	for (n = 0; n < nLength; n++) {
		baaLast[(nLine&1)^1][n&0xf] = pu8[n];
		if ((pu8[n] < 32) || (pu8[n] >= 0x7f))
			szChar[n&0xf] = '.';
		else
			szChar[n&0xf] = pu8[n];
		szChar[(n&0xf)+1] = '\0';
		nPos += sprintf(&sz[nPos], "%02X ",
			baaLast[(nLine&1)^1][n&0xf]);
		if ((n&15) != 15)
			continue;
		if ((memcmp(baaLast[0], baaLast[1], 16) == 0) && (!fFirst)) {
			nSameCount++;
		} else {
			if (nSameCount)
				buf += sprintf(buf, "(repeated %d times)\n",
					nSameCount);
			buf += sprintf(buf, "%04x: %s %s\n",
				nStart, sz, szChar);
			nSameCount = 0;
			printf("%s", szBuf);
			buf = szBuf;
		}
		nPos = 0; nStart = n+1; nLine++;
		fFirst = 0; sz[0] = '\0'; szChar[0] = '\0';
	}
	if (nSameCount)
		buf += sprintf(buf, "(repeated %d times)\n", nSameCount);

	buf += sprintf(buf, "%04x: %s", nStart, sz);
	if (n & 0xf) {
		*buf++ = ' ';
		while (n & 0xf) {
			buf += sprintf(buf, "   ");
			n++;
		}
	}
	buf += sprintf(buf, "%s\n", szChar);
	printf("%s", szBuf);
}



void
usage(void)
{
	printf(
	    "(c)2006-2007 Andy Green <andy@warmcat.com>  Licensed under GPL2\n"
	    "\n"
	    "Usage: packetspammer [options] <interface>\n\nOptions\n"
	    "-d/--delay <delay> Delay between packets\n"
	    "-s/--src <mac> set source mac address\n"
	    "-t/--dst <mac> set destination mac address\n"
	    "-l/--len <length> set frame length (include FCS)\n\n"
	    "-f/--fcs           Mark as having FCS (CRC) already\n"
		"-c/--count <num>	Number of packet to send (0 for infinity)\n"
	    "                   (pkt ends with 4 x sacrificial - chars)\n"
		"-i/--intval <int>  the frequency to print rate stats (default 1s)\n"
	    "Example:\n"
	    "  echo -n mon0 > /sys/class/ieee80211/phy0/add_iface\n"
	    "  iwconfig mon0 mode monitor\n"
	    "  ifconfig mon0 up\n"
	    "  packetspammer mon0        Spam down mon0 with\n"
	    "                            radiotap header first\n"
	    "\n");
	exit(1);
}


int
main(int argc, char *argv[])
{
	u8 u8aSendBuffer[2500];
	char szErrbuf[PCAP_ERRBUF_SIZE];
	int nCaptureHeaderLength = 0, n80211HeaderLength = 0, nLinkEncap = 0;
	int nOrdinal = 0, r, nDelay = 100000, RATE_STAT_INTERVAL = 1;
	u32 frame_len = 1000;
	u32 count = 0;
	pcap_t *ppcap = NULL;
	struct bpf_program bpfprogram;
	char * szProgram = "", fBrokenSocket = 0;
	char szHostname[PATH_MAX];

	if (gethostname(szHostname, sizeof (szHostname) - 1)) {
		perror("unable to get hostname");
	}
	szHostname[sizeof (szHostname) - 1] = '\0';


	printf("Packetspammer (c)2007 Andy Green <andy@warmcat.com>  GPL2\n");

	while (1) {
		int nOptionIndex;
		static const struct option optiona[] = {
			{ "delay", required_argument, NULL, 'd' },
			{ "fcs", no_argument, &flagMarkWithFCS, 1 },
			{ "help", no_argument, &flagHelp, 1 },
			{ "src", required_argument, NULL, 's' },
			{ "dst", required_argument, NULL, 't' },
			{ "len", required_argument, NULL, 'l' },
			{ "count", required_argument, NULL, 'c' },
			{ "intval", required_argument, NULL, 'i' },
			{ 0, 0, 0, 0 }
		};
		int c = getopt_long(argc, argv, "d:hfs:t:l:c:i:",
			optiona, &nOptionIndex);

		u8 mac[6];

		if (c == -1)
			break;
		switch (c) {
		case 0: // long option
			break;

		case 'h': // help
			usage();

		case 'd': // delay
			nDelay = atoi(optarg);
			break;

		case 'l': // frame length
			frame_len = atoi(optarg);
			break;

		case 'f': // mark as FCS attached
			flagMarkWithFCS = 1;
			break;

		case 'c': // number of packets to send
			count = atoi(optarg);
			break;

		case 'i': // freqency of printing speed stats
			RATE_STAT_INTERVAL = atoi(optarg);
			break;

		case 's': // set source mac address
			for (int i = 0; i < 6; i++) {
				int pos = i * 3;
				u8 tmp;
				if (optarg[pos] <= '9' && optarg[pos] >= '0') {
					tmp = optarg[pos] - '0';
				}
				else {
					tmp = optarg[pos] - 'a' + 10;
				}
				if (optarg[pos + 1] <= '9' && optarg[pos + 1] >= '0') {
					tmp = tmp * 16 + optarg[pos + 1] - '0';
				}
				else {
					tmp = tmp * 16 + optarg[pos + 1] - 'a' + 10;
				}
				mac[i] = tmp;
			}
			memcpy(u8aIeeeHeader + 16, mac, sizeof(mac));
			memcpy(u8aIeeeHeader + 10, mac, sizeof(mac));
			break;

		case 't': // set destination mac address
			printf("%s\n", optarg);
			for (int i = 0; i < 6; i++) {
				int pos = i * 3;
				u8 tmp;
				if (optarg[pos] <= '9' && optarg[pos] >= '0') {
					tmp = optarg[pos] - '0';
				}
				else {
					tmp = optarg[pos] - 'a' + 10;
				}
				if (optarg[pos + 1] <= '9' && optarg[pos + 1] >= '0') {
					tmp = tmp * 16 + optarg[pos + 1] - '0';
				}
				else {
					tmp = tmp * 16 + optarg[pos + 1] - 'a' + 10;
				}
				mac[i] = tmp;
			}
			memcpy(u8aIeeeHeader + 4, mac, sizeof(mac));
			break;

		default:
			printf("unknown switch %c\n", c);
			usage();
			break;
		}
	}

	if (optind >= argc)
		usage();


		// open the interface in pcap

	szErrbuf[0] = '\0';
	ppcap = pcap_open_live(argv[optind], 800, 1, 20, szErrbuf);
	if (ppcap == NULL) {
		printf("Unable to open interface %s in pcap: %s\n",
		    argv[optind], szErrbuf);
		return (1);
	}

	nLinkEncap = pcap_datalink(ppcap);
	nCaptureHeaderLength = 0;

	switch (nLinkEncap) {

		case DLT_PRISM_HEADER:
			printf("DLT_PRISM_HEADER Encap\n");
			nCaptureHeaderLength = 0x40;
			n80211HeaderLength = 0x20; // ieee80211 comes after this
			szProgram = "radio[0x4a:4]==0x13223344";
			break;

		case DLT_IEEE802_11_RADIO:
			printf("DLT_IEEE802_11_RADIO Encap\n");
			nCaptureHeaderLength = 0x40;
			n80211HeaderLength = 0x18; // ieee80211 comes after this
			szProgram = "ether[0x0a:4]==0x13223344";
			break;

		default:
			printf("!!! unknown encapsulation on %s !\n", argv[1]);
			return (1);

	}

	if (pcap_compile(ppcap, &bpfprogram, szProgram, 1, 0) == -1) {
		puts(szProgram);
		puts(pcap_geterr(ppcap));
		return (1);
	} else {
		if (pcap_setfilter(ppcap, &bpfprogram) == -1) {
			puts(szProgram);
			puts(pcap_geterr(ppcap));
		} else {
			printf("RX Filter applied\n");
		}
		pcap_freecode(&bpfprogram);
	}

	pcap_setnonblock(ppcap, 1, szErrbuf);

	printf("   (delay between packets %dus)\n", nDelay);

	memset(u8aSendBuffer, 0, sizeof (u8aSendBuffer));

	statistics *stats = malloc(sizeof(statistics));
	stats->sent_byte = 0;
	stats->sent_packet = 0;
	stats->sleep_interval = RATE_STAT_INTERVAL;
	pthread_t speed_statistic_thread;
	pthread_create(&speed_statistic_thread, NULL, print_speed, (void *)stats);

	u8 * pu8 = u8aSendBuffer;
	int nRate;

	// transmit

	memcpy(u8aSendBuffer, u8aRadiotapHeader,
		sizeof (u8aRadiotapHeader));
	if (flagMarkWithFCS)
		pu8[OFFSET_FLAGS] |= IEEE80211_RADIOTAP_F_FCS;
	nRate = 22;
	pu8 += sizeof (u8aRadiotapHeader);

	memcpy(pu8, u8aIeeeHeader, sizeof (u8aIeeeHeader));
	pu8 += sizeof (u8aIeeeHeader);

	sprintf((char *)pu8,
	    "Packetspammer %02d"
	    "broadcast packet"
	    "#%05d -- :-D --%s ----",
	    nRate/2, nOrdinal++, szHostname);

	pu8 += frame_len - sizeof(u8aIeeeHeader) - 4;	// 4 bytes of FCS, not included since calculated by driver
	if (flagMarkWithFCS)
		pu8 += 4;	// in the case that FCS is already included
	u32 send_length = pu8 - u8aSendBuffer;
	printf("packet length: %ubytes\n", frame_len);

	u32 sent_packet = 0;

    int fd = open("/dev/ebbchar", O_RDWR);

	while (!fBrokenSocket) {
		r = pcap_inject(ppcap, u8aSendBuffer, send_length);
		stats->sent_byte += frame_len;
		stats->sent_packet++;
		if (r != send_length) {
			perror("Trouble injecting packet");
			return (1);
		}
		int buf_free;
		read(fd, &buf_free, sizeof(int));
		if (wait(buf_free))
			printf("Waiting for: %d\n": wait(buf_free));
			usleep(wait(buf_free));
		if (++sent_packet == count)
			break;
	}

	pthread_cancel(speed_statistic_thread);
	pthread_exit(NULL);
	return (0);
}

int wait(int buf) {
	int use = 80 - buf;
	return  * use;
}