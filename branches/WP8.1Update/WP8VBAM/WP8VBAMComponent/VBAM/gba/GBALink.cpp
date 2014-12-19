// This file was written by denopqrihg
// with major changes by tjm
#include <string.h>
#include <stdio.h>
#include <windows.h>


// malloc.h does not seem to exist on Mac OS 10.7
#ifdef __APPLE__
#include <stdlib.h>
#else
#include <malloc.h>
#endif

#ifdef _MSC_VER
#define snprintf _snprintf
#endif

static int vbaid = 0;
const char *MakeInstanceFilename(const char *Input)
{
	if (vbaid == 0)
		return Input;

	static char *result=NULL;
	if (result!=NULL)
		free(result);

	result = (char *)malloc(strlen(Input)+3);
	char *p = strrchr((char *)Input, '.');
	sprintf(result, "%.*s-%d.%s", (int)(p-Input), Input, vbaid+1, p+1);
	return result;
}

#ifndef NO_LINK

#define LOCAL_LINK_NAME "VBA link memory"
#define IP_LINK_PORT 5738

#include "../common/Port.h"
#include "GBA.h"
#include "GBALink.h"
#include "GBASockClient.h"

#include <SFML/Network.hpp>

#ifdef ENABLE_NLS
#include <libintl.h>
#define _(x) gettext(x)
#else
#define _(x) x
#endif
#define N_(x) x



#if (defined __WIN32__ || defined _WIN32)
#include <windows.h>
#else
#include <sys/mman.h>
#include <time.h>
#include <semaphore.h>
#include <fcntl.h>
#include <errno.h>


#define ReleaseSemaphore(sem, nrel, orel) do { \
	for(int i = 0; i < nrel; i++) \
		sem_post(sem); \
} while(0)
#define WAIT_TIMEOUT -1
#ifdef HAVE_SEM_TIMEDWAIT
int WaitForSingleObject(sem_t *s, int t)
{
	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);
	ts.tv_sec += t/1000;
	ts.tv_nsec += (t%1000) * 1000000;
	do {
		if(!sem_timedwait(s, &ts))
			return 0;
	} while(errno == EINTR);
	return WAIT_TIMEOUT;
}

// urg.. MacOSX has no sem_timedwait (POSIX) or semtimedop (SYSV)
// so we'll have to simulate it..
// MacOSX also has no clock_gettime, and since both are "real-time", assume
// anyone who doesn't have one also doesn't have the other

// 2 ways to do this:
//   - poll & sleep loop
//   - poll & wait for timer interrupt loop

// the first consumes more CPU and requires selection of a good sleep value

// the second may interfere with other timers running on system, and
// requires that a dummy signal handler be installed for SIGALRM
#else
#include <sys/time.h>
#ifndef TIMEDWAIT_ALRM
#define TIMEDWAIT_ALRM 1
#endif
#if TIMEDWAIT_ALRM
#include <signal.h>
static void alrmhand(int sig)
{
}
#endif
int WaitForSingleObject(sem_t *s, int t)
{
#if !TIMEDWAIT_ALRM
	struct timeval ts;
	gettimeofday(&ts, NULL);
	ts.tv_sec += t/1000;
	ts.tv_usec += (t%1000) * 1000;
#else
	struct sigaction sa, osa;
	sigaction(SIGALRM, NULL, &osa);
	sa = osa;
	sa.sa_flags &= ~SA_RESTART;
	sa.sa_handler = alrmhand;
	sigaction(SIGALRM, &sa, NULL);
	struct itimerval tv, otv;
	tv.it_value.tv_sec = t / 1000;
	tv.it_value.tv_usec = (t%1000) * 1000;
	// this should be 0/0, but in the wait loop, it's possible to
	// have the signal fire while not in sem_wait().  This will ensure
	// another signal within 1ms
	tv.it_interval.tv_sec = 0;
	tv.it_interval.tv_usec = 999;
	setitimer(ITIMER_REAL, &tv, &otv);
#endif
	while(1) {
#if !TIMEDWAIT_ALRM
		if(!sem_trywait(s))
			return 0;
		struct timeval ts2;
		gettimeofday(&ts2, NULL);
		if(ts2.tv_sec > ts.tv_sec || (ts2.tv_sec == ts.tv_sec &&
					      ts2.tv_usec > ts.tv_usec)) {
			return WAIT_TIMEOUT;
		}
		// is .1 ms short enough?  long enough?  who knows?
		struct timespec ts3;
		ts3.tv_sec = 0;
		ts3.tv_nsec = 100000;
		nanosleep(&ts3, NULL);
#else
		if(!sem_wait(s)) {
			setitimer(ITIMER_REAL, &otv, NULL);
			sigaction(SIGALRM, &osa, NULL);
			return 0;
		}
		getitimer(ITIMER_REAL, &tv);
		if(tv.it_value.tv_sec || tv.it_value.tv_usec > 999)
			continue;
		setitimer(ITIMER_REAL, &otv, NULL);
		sigaction(SIGALRM, &osa, NULL);
		break;
#endif
	}
	return WAIT_TIMEOUT;
}
#endif
#endif

using namespace Windows::System::Threading;

#define UNSUPPORTED -1
#define MULTIPLAYER 0
#define NORMAL8 1
#define NORMAL32 2
#define UART 3
#define JOYBUS 4
#define GP 5

#define RFU_INIT 0
#define RFU_COMM 1
#define RFU_SEND 2
#define RFU_RECV 3

static ConnectionState InitIPC();
static ConnectionState InitSocket();
static ConnectionState JoyBusConnect();

static void JoyBusShutdown();
static void CloseIPC();
static void CloseSocket();

static void StartCableSocket(u16 siocnt);
static void StartRFU(u16 siocnt);
static void StartCableIPC(u16 siocnt);

static void JoyBusUpdate(int ticks);
static void UpdateCableIPC(int ticks);
static void UpdateRFUIPC(int ticks);
static void UpdateSocket(int ticks);





static ConnectionState ConnectUpdateSocket(char * const message, size_t size);

struct LinkDriver {
	typedef ConnectionState (ConnectFunc)();
	typedef ConnectionState (ConnectUpdateFunc)(char * const message, size_t size);
	typedef void (StartFunc)(u16 siocnt);
	typedef void (UpdateFunc)(int ticks);
	typedef void (CloseFunc)();

	LinkMode mode;
	ConnectFunc *connect;
	ConnectUpdateFunc *connectUpdate;
	StartFunc *start;
	UpdateFunc *update;
	CloseFunc *close;
};
static const LinkDriver linkDrivers[] =
{
	{ LINK_CABLE_IPC,			InitIPC,		NULL,					StartCableIPC,		UpdateCableIPC,	CloseIPC },
	{ LINK_CABLE_SOCKET,		InitSocket,		ConnectUpdateSocket,	StartCableSocket,	UpdateSocket,	CloseSocket },
	{ LINK_RFU_IPC,				InitIPC,		NULL,					StartRFU,			UpdateRFUIPC,	CloseIPC },
	{ LINK_GAMECUBE_DOLPHIN,	JoyBusConnect,	NULL,					NULL,				JoyBusUpdate,	JoyBusShutdown }
};


enum
{
	JOY_CMD_RESET	= 0xff,
	JOY_CMD_STATUS	= 0x00,
	JOY_CMD_READ	= 0x14,
	JOY_CMD_WRITE	= 0x15
};

#define UPDATE_REG(address, value) WRITE16LE(((u16 *)&ioMem[address]),value)

typedef struct {
	u16 linkdata[5];
	u16 linkcmd;
	u16 numtransfers;
	int lastlinktime;
	u8 numgbas;
	u8 trgbas;
	u8 linkflags;
	int rfu_q[4];
	u8 rfu_request[4];
	int rfu_linktime[4];
	u32 rfu_bdata[4][7];
	u32 rfu_data[4][32];
} LINKDATA;

typedef struct {
	sf::SocketTCP tcpsocket;
	int numslaves;
	int connectedSlaves;
	int type;
	bool server;
	bool speed;
	Windows::Foundation::IAsyncAction ^linkthread;
} LANLINKDATA;

class lserver{
	int numbytes;
	sf::Selector<sf::SocketTCP> fdset;
	//timeval udptimeout;
	char inbuffer[256], outbuffer[256];
	s32 *intinbuffer;
	u16 *u16inbuffer;
	s32 *intoutbuffer;
	u16 *u16outbuffer;
	int counter;
	int done;
public:
	int howmanytimes;
	sf::SocketTCP tcpsocket[4];
	sf::IPAddress udpaddr[4];
	lserver(void);
	void Send(void);
	void Recv(void);
};

class lclient{
	sf::Selector<sf::SocketTCP> fdset;
	char inbuffer[256], outbuffer[256];
	s32 *intinbuffer;
	u16 *u16inbuffer;
	s32 *intoutbuffer;
	u16 *u16outbuffer;
	int numbytes;
public:
	sf::IPAddress serveraddr;
	unsigned short serverport;
	int numtransfers;
	lclient(void);
	void Send(void);
	void Recv(void);
	void CheckConn(void);
};

static const LinkDriver *linkDriver = NULL;
static ConnectionState gba_connection_state = LINK_OK;

LinkMode GetLinkMode() {
	if (linkDriver && gba_connection_state == LINK_OK)
		return linkDriver->mode;
	else
		return LINK_DISCONNECTED;
}

static int linktime = 0;

static GBASockClient* dol = NULL;
static sf::IPAddress joybusHostAddr = sf::IPAddress::LocalHost;

// Hodgepodge
static u8 tspeed = 3;
static u8 transfer = 0;
static LINKDATA *linkmem = NULL;
static int linkid = 0;
#if (defined __WIN32__ || defined _WIN32)
static HANDLE linksync[4];
#else
static sem_t *linksync[4];
#endif
static int savedlinktime = 0;
#if (defined __WIN32__ || defined _WIN32)
static HANDLE mmf = NULL;
#else
static int mmf = -1;
#endif
static char linkevent[] =
#if !(defined __WIN32__ || defined _WIN32)
	"/"
#endif
	"VBA link event  ";
static int i, j;
static int linktimeout = 1000;
static LANLINKDATA lanlink;
static u16 linkdata[4];
static lserver ls;
static lclient lc;
static bool oncewait = false, after = false;

// RFU crap (except for numtransfers note...should probably check that out)
static u8 rfu_cmd, rfu_qsend, rfu_qrecv;
static int rfu_state, rfu_polarity, rfu_counter, rfu_masterq;
// numtransfers seems to be used interchangeably with linkmem->numtransfers
// in rfu code; probably a bug?
static int rfu_transfer_end;
// in local comm, setting this keeps slaves from trying to communicate even
// when master isn't
static u16 numtransfers = 0;
static u32 rfu_masterdata[32];

// time to end of single GBA's transfer, in 16.78 MHz clock ticks
// first index is GBA #
static const int trtimedata[4][4] = {
      //baudrate: 9600 38400 57600 115200
	//{34080, 8520, 5680, 2840}, //time to finish transfering data from master (#0)
	//{65536, 16384, 10923, 5461}, //time to finish transfer data from master and slave #1
	//{31440, 7860, 5240, 2620}, //time to finish transfering data from master (#0) (new calculation)
	//{62880, 15720, 10480, 5240}, //time to finish transfer data from master and slave #1 (new calculation)
	{17040, 4260, 2840, 1420}, //time to finish transfering data from master (#0)
	{32768, 8192, 5462, 2730}, //time to finish transfer data from master and slave #1
	{99609, 24903, 16602, 8301},
	{133692, 33423, 22282, 11141}
};

// time to end of transfer
// for 3 slaves, this is time to transfer machine 4
// for < 3 slaves, this is time to transfer last machine + time to detect lack
// of start bit from next slave
// first index is (# of slaves) - 1
static const int trtimeend[3][4] = {
      //baudrate: 9600 38400 57600 115200
	//{72527, 18132, 12088, 6044}, //if 2 slaves, the all transfer should be done after this
	//{34584, 8646, 11528, 5764}, //if 2 slaves, the all transfer should be done after this (new calculation)
	{36263, 9066, 6044, 3022},
	{106608, 26652, 17768, 8884},
	{133692, 33423, 22282, 11141} 
};

static int GetSIOMode(u16, u16);

static HANDLE startSendEvent; //signal start sending
static HANDLE finishSendEvent; //signal finish sending
static HANDLE startReceiveEvent; //signal start receiving
static HANDLE finishReceiveEvent; //singal stop receiving
static void LANLinkThread();
static bool endLANThread;
static bool newMasterCycle;

// The GBA wireless RFU (see adapter3.txt)
// Just try to avert your eyes for now ^^ (note, it currently can be called, tho)
static void StartRFU(u16 siocnt)
{
	switch (GetSIOMode(siocnt, READ16LE(&ioMem[COMM_RCNT]))) {
	case NORMAL8:
		rfu_polarity = 0;
		break;

	case NORMAL32:
		if (siocnt & 8)
			siocnt &= 0xfffb;	// A kind of acknowledge procedure
		else
			siocnt |= 4;

		if (siocnt & 0x80)
		{
			if ((siocnt&3) == 1)
				rfu_transfer_end = 2048;
			else
				rfu_transfer_end = 256;

			u16 a = READ16LE(&ioMem[COMM_SIODATA32_H]);

			switch (rfu_state) {
			case RFU_INIT:
				if (READ32LE(&ioMem[COMM_SIODATA32_L]) == 0xb0bb8001)
					rfu_state = RFU_COMM;	// end of startup

				UPDATE_REG(COMM_SIODATA32_H, READ16LE(&ioMem[COMM_SIODATA32_L]));
				UPDATE_REG(COMM_SIODATA32_L, a);
				break;

			case RFU_COMM:
				if (a == 0x9966)
				{
					rfu_cmd = ioMem[COMM_SIODATA32_L];
					if ((rfu_qsend=ioMem[0x121]) != 0) {
						rfu_state = RFU_SEND;
						rfu_counter = 0;
					}
					if (rfu_cmd == 0x25 || rfu_cmd == 0x24) {
						linkmem->rfu_q[vbaid] = rfu_qsend;
					}
					UPDATE_REG(COMM_SIODATA32_L, 0);
					UPDATE_REG(COMM_SIODATA32_H, 0x8000);
				}
				else if (a == 0x8000)
				{
					switch (rfu_cmd) {
					case 0x1a:	// check if someone joined
						if (linkmem->rfu_request[vbaid] != 0) {
							rfu_state = RFU_RECV;
							rfu_qrecv = 1;
						}
						linkid = -1;
						rfu_cmd |= 0x80;
						break;

					case 0x1e:	// receive broadcast data
					case 0x1d:	// no visible difference
						rfu_polarity = 0;
						rfu_state = RFU_RECV;
						rfu_qrecv = 7;
						rfu_counter = 0;
						rfu_cmd |= 0x80;
						break;

					case 0x30:
						linkmem->rfu_request[vbaid] = 0;
						linkmem->rfu_q[vbaid] = 0;
						linkid = 0;
						numtransfers = 0;
						rfu_cmd |= 0x80;
						if (linkmem->numgbas == 2)
							ReleaseSemaphore(linksync[1-vbaid], 1, NULL);
						break;

					case 0x11:	// ? always receives 0xff - I suspect it's something for 3+ players
					case 0x13:	// unknown
					case 0x20:	// this has something to do with 0x1f
					case 0x21:	// this too
						rfu_cmd |= 0x80;
						rfu_polarity = 0;
						rfu_state = 3;
						rfu_qrecv = 1;
						break;

					case 0x26:
						if(linkid>0){
							rfu_qrecv = rfu_masterq;
						}
						if((rfu_qrecv=linkmem->rfu_q[1-vbaid])!=0){
							rfu_state = RFU_RECV;
							rfu_counter = 0;
						}
						rfu_cmd |= 0x80;
						break;

					case 0x24:	// send data
						if((numtransfers++)==0) linktime = 1;
						linkmem->rfu_linktime[vbaid] = linktime;
						if(linkmem->numgbas==2){
							ReleaseSemaphore(linksync[1-vbaid], 1, NULL);
							WaitForSingleObjectEx(linksync[vbaid], linktimeout, false);
						}
						rfu_cmd |= 0x80;
						linktime = 0;
						linkid = -1;
						break;

					case 0x25:	// send & wait for data
					case 0x1f:	// pick a server
					case 0x10:	// init
					case 0x16:	// send broadcast data
					case 0x17:	// setup or something ?
					case 0x27:	// wait for data ?
					case 0x3d:	// init
					default:
						rfu_cmd |= 0x80;
						break;

					case 0xa5:	//	2nd part of send&wait function 0x25
					case 0xa7:	//	2nd part of wait function 0x27
						if (linkid == -1) {
							linkid++;
							linkmem->rfu_linktime[vbaid] = 0;
						}
						if (linkid&&linkmem->rfu_request[1-vbaid] == 0) {
							linkmem->rfu_q[1-vbaid] = 0;
							rfu_transfer_end = 256;
							rfu_polarity = 1;
							rfu_cmd = 0x29;
							linktime = 0;
							break;
						}
						if ((numtransfers++) == 0)
							linktime = 0;
						linkmem->rfu_linktime[vbaid] = linktime;
						if (linkmem->numgbas == 2) {
							if (!linkid || (linkid && numtransfers))
								ReleaseSemaphore(linksync[1-vbaid], 1, NULL);
							WaitForSingleObjectEx(linksync[vbaid], linktimeout, false);
						}
						if ( linkid > 0) {
							memcpy(rfu_masterdata, linkmem->rfu_data[1-vbaid], 128);
							rfu_masterq = linkmem->rfu_q[1-vbaid];
						}
						rfu_transfer_end = linkmem->rfu_linktime[1-vbaid] - linktime + 256;

						if (rfu_transfer_end < 256)
							rfu_transfer_end = 256;

						linktime = -rfu_transfer_end;
						rfu_polarity = 1;
						rfu_cmd = 0x28;
						break;
					}
					UPDATE_REG(COMM_SIODATA32_H, 0x9966);
					UPDATE_REG(COMM_SIODATA32_L, (rfu_qrecv<<8) | rfu_cmd);

				} else {

					UPDATE_REG(COMM_SIODATA32_L, 0);
					UPDATE_REG(COMM_SIODATA32_H, 0x8000);
				}
				break;

			case RFU_SEND:
				if(--rfu_qsend == 0)
					rfu_state = RFU_COMM;

				switch (rfu_cmd) {
				case 0x16:
					linkmem->rfu_bdata[vbaid][rfu_counter++] = READ32LE(&ioMem[COMM_SIODATA32_L]);
					break;

				case 0x17:
					linkid = 1;
					break;

				case 0x1f:
					linkmem->rfu_request[1-vbaid] = 1;
					break;

				case 0x24:
				case 0x25:
					linkmem->rfu_data[vbaid][rfu_counter++] = READ32LE(&ioMem[COMM_SIODATA32_L]);
					break;
				}
				UPDATE_REG(COMM_SIODATA32_L, 0);
				UPDATE_REG(COMM_SIODATA32_H, 0x8000);
				break;

			case RFU_RECV:
				if (--rfu_qrecv == 0)
					rfu_state = RFU_COMM;

				switch (rfu_cmd) {
				case 0x9d:
				case 0x9e:
					if (rfu_counter == 0) {
						UPDATE_REG(COMM_SIODATA32_L, 0x61f1);
						UPDATE_REG(COMM_SIODATA32_H, 0);
						rfu_counter++;
						break;
					}
					UPDATE_REG(COMM_SIODATA32_L, linkmem->rfu_bdata[1-vbaid][rfu_counter-1]&0xffff);
					UPDATE_REG(COMM_SIODATA32_H, linkmem->rfu_bdata[1-vbaid][rfu_counter-1]>>16);
					rfu_counter++;
					break;

				case 0xa6:
					if (linkid>0) {
						UPDATE_REG(COMM_SIODATA32_L, rfu_masterdata[rfu_counter]&0xffff);
						UPDATE_REG(COMM_SIODATA32_H, rfu_masterdata[rfu_counter++]>>16);
					} else {
						UPDATE_REG(COMM_SIODATA32_L, linkmem->rfu_data[1-vbaid][rfu_counter]&0xffff);
						UPDATE_REG(COMM_SIODATA32_H, linkmem->rfu_data[1-vbaid][rfu_counter++]>>16);
					}
					break;

				case 0x93:	// it seems like the game doesn't care about this value
					UPDATE_REG(COMM_SIODATA32_L, 0x1234);	// put anything in here
					UPDATE_REG(COMM_SIODATA32_H, 0x0200);	// also here, but it should be 0200
					break;

				case 0xa0:
				case 0xa1:
					UPDATE_REG(COMM_SIODATA32_L, 0x641b);
					UPDATE_REG(COMM_SIODATA32_H, 0x0000);
					break;

				case 0x9a:
					UPDATE_REG(COMM_SIODATA32_L, 0x61f9);
					UPDATE_REG(COMM_SIODATA32_H, 0);
					break;

				case 0x91:
					UPDATE_REG(COMM_SIODATA32_L, 0x00ff);
					UPDATE_REG(COMM_SIODATA32_H, 0x0000);
					break;

				default:
					UPDATE_REG(COMM_SIODATA32_L, 0x0173);
					UPDATE_REG(COMM_SIODATA32_H, 0x0000);
					break;
				}
				break;
			}
			transfer = 1;
		}

		if (rfu_polarity)
			siocnt ^= 4;	// sometimes it's the other way around
		break;
	}

	UPDATE_REG(COMM_SIOCNT, siocnt);
}

static void StartCableIPC(u16 value)
{
	switch (GetSIOMode(value, READ16LE(&ioMem[COMM_RCNT]))) {
	case MULTIPLAYER: {
		bool start = (value & 0x80) && !linkid && !transfer;
		// clear start, seqno, si (RO on slave, start = pulse on master)
		value &= 0xff4b;
		// get current si.  This way, on slaves, it is low during xfer
		if(linkid) {
			if(!transfer)
				value |= 4;
			else
				value |= READ16LE(&ioMem[COMM_SIOCNT]) & 4;
		}
		if (start) {
			if (linkmem->numgbas > 1)
			{
				// find first active attached GBA
				// doing this first reduces the potential
				// race window size for new connections
				int n = linkmem->numgbas + 1;
				int f = linkmem->linkflags;
				int m;
				do {
					n--;
					m = (1 << n) - 1;
				} while((f & m) != m);
				linkmem->trgbas = n;

				// before starting xfer, make pathetic attempt
				// at clearing out any previous stuck xfer
				// this will fail if a slave was stuck for
				// too long
				for(int i = 0; i < 4; i++)
					while(WaitForSingleObjectEx(linksync[i], 0, false) != WAIT_TIMEOUT);

				// transmit first value
				linkmem->linkcmd = ('M' << 8) + (value & 3);
				linkmem->linkdata[0] = READ16LE(&ioMem[COMM_SIODATA8]);

				// start up slaves & sync clocks
				numtransfers = linkmem->numtransfers;
				if (numtransfers != 0)
					linkmem->lastlinktime = linktime;
				else
					linkmem->lastlinktime = 0;

				if ((++numtransfers) == 0)
					linkmem->numtransfers = 2;
				else
					linkmem->numtransfers = numtransfers;

				transfer = 1;
				linktime = 0;
				tspeed = value & 3;
				WRITE32LE(&ioMem[COMM_SIOMULTI0], 0xffffffff);
				WRITE32LE(&ioMem[COMM_SIOMULTI2], 0xffffffff);
				value &= ~0x40;
			} else {
				value |= 0x40; // comm error
			}
		}
		value |= (transfer != 0) << 7;
		value |= (linkid && !transfer ? 0xc : 8); // set SD (high), SI (low on master)
		value |= linkid << 4; // set seq
		UPDATE_REG(COMM_SIOCNT, value);
		if (linkid)
			// SC low -> transfer in progress
			// not sure why SO is low
			UPDATE_REG(COMM_RCNT, transfer ? 6 : 7);
		else
			// SI is always low on master
			// SO, SC always low during transfer
			// not sure why SO low otherwise
			UPDATE_REG(COMM_RCNT, transfer ? 2 : 3);
		break;
	}
	case NORMAL8:
	case NORMAL32:
	case UART:
	default:
		UPDATE_REG(COMM_SIOCNT, value);
		break;
	}
}

void StartCableSocket(u16 value)
{
	switch (GetSIOMode(value, READ16LE(&ioMem[COMM_RCNT]))) {
	case MULTIPLAYER: {
		bool start = (value & 0x80) && !linkid && !transfer;  //only master can have start == true, the 9th bit: 0 = inactive, 1=start/busy (gbaktek document is wrong on this?)
		// clear start, seqno, si (RO on slave, start = pulse on master)
		value &= 0xff4b;
		// get current si.  This way, on slaves, it is low during xfer
		if(linkid) 
		{
			if(!transfer)
				value |= 4;
			else
				value |= READ16LE(&ioMem[COMM_SIOCNT]) & 4;
		}
		if (start ) //&& !endLANthread) //this got called on the host after the player has started linking in the game (after save game in pokemon)
		{
			linkdata[0] = READ16LE(&ioMem[COMM_SIODATA8]);
			savedlinktime = linktime; //linktime is sent to salves later
			tspeed = value & 3;
			
			//SetEvent(startSendEvent);
			//newMasterCycle = true; //signal start of a new send-receive cycle

			ls.Send(); //server send data from all clients back to each client. when slave is busy, this keeps executing. Also ls.Recv() keep executing too
			transfer = 1;
			linktime = 0;
			UPDATE_REG(COMM_SIOMULTI0, linkdata[0]);
			UPDATE_REG(COMM_SIOMULTI1, 0xffff);
			WRITE32LE(&ioMem[COMM_SIOMULTI2], 0xffffffff);
			if (lanlink.speed && oncewait == false)
				ls.howmanytimes++;
			after = false;
			value &= ~0x40;
		}
		value |= (transfer != 0) << 7;
		value |= (linkid && !transfer ? 0xc : 8); // set SD (high), SI (low on master)
		value |= linkid << 4; // set seq
		UPDATE_REG(COMM_SIOCNT, value);
		if (linkid)
			// SC low -> transfer in progress
			// not sure why SO is low
			UPDATE_REG(COMM_RCNT, transfer ? 6 : 7);
		else
			// SI is always low on master
			// SO, SC always low during transfer
			// not sure why SO low otherwise
			UPDATE_REG(COMM_RCNT, transfer ? 2 : 3);
		break;
	}
	case NORMAL8:
	case NORMAL32:
	case UART:
	default:
		UPDATE_REG(COMM_SIOCNT, value);
		break;
	}
}

void StartLink(u16 siocnt)
{
	if (!linkDriver || !linkDriver->start) {
		return;
	}

	linkDriver->start(siocnt);
}

void StartGPLink(u16 value)
{
	UPDATE_REG(COMM_RCNT, value);

	if (!value)
		return;

	switch (GetSIOMode(READ16LE(&ioMem[COMM_SIOCNT]), value)) {
	case MULTIPLAYER:
		value &= 0xc0f0;
		value |= 3;
		if (linkid)
			value |= 4;
		UPDATE_REG(COMM_SIOCNT, ((READ16LE(&ioMem[COMM_SIOCNT])&0xff8b)|(linkid ? 0xc : 8)|(linkid<<4)));
		break;

	case GP:
		if (GetLinkMode() == LINK_RFU_IPC)
			rfu_state = RFU_INIT;
		break;
	}
}

static ConnectionState JoyBusConnect()
{
	delete dol;
	dol = NULL;

	dol = new GBASockClient();
	bool connected = dol->Connect(joybusHostAddr);

	if (connected) {
		return LINK_OK;
	} else {
		systemMessage(0, N_("Error, could not connect to Dolphin"));
		return LINK_ERROR;
	}
}

static void JoyBusShutdown()
{
	delete dol;
	dol = NULL;
}

static void JoyBusUpdate(int ticks)
{
	static int lastjoybusupdate = 0;

	// Kinda ugly hack to update joybus stuff intermittently
	if (linktime > lastjoybusupdate + 0x3000)
	{
		lastjoybusupdate = linktime;

		char data[5] = {0x10, 0, 0, 0, 0}; // init with invalid cmd
		std::vector<char> resp;

		if (!dol)
			JoyBusConnect();

		u8 cmd = dol->ReceiveCmd(data);
		switch (cmd) {
		case JOY_CMD_RESET:
			UPDATE_REG(COMM_JOYCNT, READ16LE(&ioMem[COMM_JOYCNT]) | JOYCNT_RESET);

		case JOY_CMD_STATUS:
			resp.push_back(0x00); // GBA device ID
			resp.push_back(0x04);
			break;
		
		case JOY_CMD_READ:
			resp.push_back((u8)(READ16LE(&ioMem[COMM_JOY_TRANS_L]) & 0xff));
			resp.push_back((u8)(READ16LE(&ioMem[COMM_JOY_TRANS_L]) >> 8));
			resp.push_back((u8)(READ16LE(&ioMem[COMM_JOY_TRANS_H]) & 0xff));
			resp.push_back((u8)(READ16LE(&ioMem[COMM_JOY_TRANS_H]) >> 8));
			UPDATE_REG(COMM_JOYSTAT, READ16LE(&ioMem[COMM_JOYSTAT]) & ~JOYSTAT_SEND);
			UPDATE_REG(COMM_JOYCNT, READ16LE(&ioMem[COMM_JOYCNT]) | JOYCNT_SEND_COMPLETE);
			break;

		case JOY_CMD_WRITE:
			UPDATE_REG(COMM_JOY_RECV_L, (u16)((u16)data[2] << 8) | (u8)data[1]);
			UPDATE_REG(COMM_JOY_RECV_H, (u16)((u16)data[4] << 8) | (u8)data[3]);
			UPDATE_REG(COMM_JOYSTAT, READ16LE(&ioMem[COMM_JOYSTAT]) | JOYSTAT_RECV);
			UPDATE_REG(COMM_JOYCNT, READ16LE(&ioMem[COMM_JOYCNT]) | JOYCNT_RECV_COMPLETE);
			break;

		default:
			return; // ignore
		}

		resp.push_back((u8)READ16LE(&ioMem[COMM_JOYSTAT]));
		dol->Send(resp);

		// Generate SIO interrupt if we can
		if ( ((cmd == JOY_CMD_RESET) || (cmd == JOY_CMD_READ) || (cmd == JOY_CMD_WRITE))
			&& (READ16LE(&ioMem[COMM_JOYCNT]) & JOYCNT_INT_ENABLE) )
		{
			IF |= 0x80;
			UPDATE_REG(0x202, IF);
		}
	}
}

static void ReInitLink();

static void UpdateCableIPC(int ticks)
{
	// slave startup depends on detecting change in numtransfers
	// and syncing clock with master (after first transfer)
	// this will fail if > ~2 minutes have passed since last transfer due
	// to integer overflow
	if(!transfer && numtransfers && linktime < 0) {
		linktime = 0;
		// there is a very, very, small chance that this will abort
		// a transfer that was just started
		linkmem->numtransfers = numtransfers = 0;
	}
	if (linkid && !transfer && linktime >= linkmem->lastlinktime &&
	    linkmem->numtransfers != numtransfers)
	{
		numtransfers = linkmem->numtransfers;
		if(!numtransfers)
			return;

		// if this or any previous machine was dropped, no transfer
		// can take place
		if(linkmem->trgbas <= linkid) {
			transfer = 0;
			numtransfers = 0;
			// if this is the one that was dropped, reconnect
			if(!(linkmem->linkflags & (1 << linkid)))
				ReInitLink();
			return;
		}

		// sync clock
		if (numtransfers == 1)
			linktime = 0;
		else
			linktime -= linkmem->lastlinktime;

		// there's really no point to this switch; 'M' is the only
		// possible command.
#if 0
		switch ((linkmem->linkcmd) >> 8)
		{
		case 'M':
#endif
			tspeed = linkmem->linkcmd & 3;
			transfer = 1;
			WRITE32LE(&ioMem[COMM_SIOMULTI0], 0xffffffff);
			WRITE32LE(&ioMem[COMM_SIOMULTI2], 0xffffffff);
			UPDATE_REG(COMM_SIOCNT, READ16LE(&ioMem[COMM_SIOCNT]) & ~0x40 | 0x80);
#if 0
			break;
		}
#endif
	}

	if (!transfer)
		return;

	if (transfer <= linkmem->trgbas && linktime >= trtimedata[transfer-1][tspeed])
	{
		// transfer #n -> wait for value n - 1
		if(transfer > 1 && linkid != transfer - 1) {
			if(WaitForSingleObjectEx(linksync[transfer - 1], linktimeout, false) == WAIT_TIMEOUT) {
				// assume slave has dropped off if timed out
				if(!linkid) {
					linkmem->trgbas = transfer - 1;
					int f = linkmem->linkflags;
					f &= ~(1 << (transfer - 1));
					linkmem->linkflags = f;
					if(f < (1 << transfer) - 1)
						linkmem->numgbas = transfer - 1;
					char message[30];
					sprintf(message, _("Player %d disconnected."), transfer - 1);
					systemScreenMessage(message);
				}
				transfer = linkmem->trgbas + 1;
				// next cycle, transfer will finish up
				return;
			}
		}
		// now that value is available, store it
		UPDATE_REG((COMM_SIOMULTI0 - 2) + (transfer<<1), linkmem->linkdata[transfer-1]);

		// transfer machine's value at start of its transfer cycle
		if(linkid == transfer) {
			// skip if dropped
			if(linkmem->trgbas <= linkid) {
				transfer = 0;
				numtransfers = 0;
				// if this is the one that was dropped, reconnect
				if(!(linkmem->linkflags & (1 << linkid)))
					ReInitLink();
				return;
			}
			// SI becomes low
			UPDATE_REG(COMM_SIOCNT, READ16LE(&ioMem[COMM_SIOCNT]) & ~4);
			UPDATE_REG(COMM_RCNT, 10);
			linkmem->linkdata[linkid] = READ16LE(&ioMem[COMM_SIODATA8]);
			ReleaseSemaphore(linksync[linkid], linkmem->numgbas-1, NULL);
		}
		if(linkid == transfer - 1) {
			// SO becomes low to begin next trasnfer
			// may need to set DDR as well
			UPDATE_REG(COMM_RCNT, 0x22);
		}

		// next cycle
		transfer++;
	}

	if (transfer > linkmem->trgbas && linktime >= trtimeend[transfer-3][tspeed])
	{
		// wait for slaves to finish
		// this keeps unfinished slaves from screwing up last xfer
		// not strictly necessary; may just slow things down
		if(!linkid) {
			for(int i = 2; i < transfer; i++)
				if(WaitForSingleObjectEx(linksync[0], linktimeout, false) == WAIT_TIMEOUT) {
					// impossible to determine which slave died
					// so leave them alone for now
					systemScreenMessage(_("Unknown slave timed out; resetting comm"));
					linkmem->numtransfers = numtransfers = 0;
					break;
				}
		} else if(linkmem->trgbas > linkid)
			// signal master that this slave is finished
			ReleaseSemaphore(linksync[0], 1, NULL);
		linktime -= trtimeend[transfer - 3][tspeed];
		transfer = 0;
		u16 value = READ16LE(&ioMem[COMM_SIOCNT]);
		if(!linkid)
			value |= 4; // SI becomes high on slaves after xfer
		UPDATE_REG(COMM_SIOCNT, (value & 0xff0f) | (linkid << 4));
		// SC/SI high after transfer
		UPDATE_REG(COMM_RCNT, linkid ? 15 : 11);
		if (value & 0x4000)
		{
			IF |= 0x80;
			UPDATE_REG(0x202, IF);
		}
	}
}

static void UpdateRFUIPC(int ticks)
{
	rfu_transfer_end -= ticks;

	if (transfer && rfu_transfer_end <= 0)
	{
		transfer = 0;
		if (READ16LE(&ioMem[COMM_SIOCNT]) & 0x4000)
		{
			IF |= 0x80;
			UPDATE_REG(0x202, IF);
		}
		UPDATE_REG(COMM_SIOCNT, READ16LE(&ioMem[COMM_SIOCNT]) & 0xff7f);
	}
}

static void UpdateSocket(int ticks)
{
	if (after)
	{
		if (linkid && linktime > 6044) {
			lc.Recv();
			oncewait = true;
		}
		else
			return;
	}

if (linkid && !transfer && lc.numtransfers > 0 && linktime >= savedlinktime)
	{
		linkdata[linkid] = READ16LE(&ioMem[COMM_SIODATA8]);

		//SetEvent(startSendEvent);
		lc.Send();

		UPDATE_REG(COMM_SIODATA32_L, linkdata[0]);
		UPDATE_REG(COMM_SIOCNT, READ16LE(&ioMem[COMM_SIOCNT]) | 0x80);
		transfer = 1;
		if (lc.numtransfers==1)
			linktime = 0;
		else
			linktime -= savedlinktime;
	}
	//else if (linkid == 0 && transfer && newMasterCycle ) //master
	//{
	//	WaitForSingleObjectEx(finishReceiveEvent, INFINITE, false);
	//	ResetEvent(finishReceiveEvent);
	//	newMasterCycle = false; //set to false so code does not go in here until master sends new data
	//}

	if (transfer && linktime >= trtimeend[lanlink.numslaves-1][tspeed]) //only start receive data after transfer is done
	{
		if (READ16LE(&ioMem[COMM_SIOCNT]) & 0x4000)
		{
			IF |= 0x80;
			UPDATE_REG(0x202, IF);
		}

		UPDATE_REG(COMM_SIOCNT, (READ16LE(&ioMem[COMM_SIOCNT]) & 0xff0f) | (linkid << 4));
		transfer = 0;
		linktime -= trtimeend[lanlink.numslaves-1][tspeed];
		oncewait = false;

		if (!lanlink.speed)
		{
			if (linkid)
			{
				//WaitForSingleObjectEx(finishReceiveEvent, INFINITE, false);
				//ResetEvent(finishReceiveEvent);
				lc.Recv(); //slave tries to receive data from master, have to wait for master to receive data and resent data, quite slow
			}
			else
			{
				ls.Recv(); // server tries receive data from slaves, should move this one up
			}
			UPDATE_REG(COMM_SIOMULTI1, linkdata[1]);
			UPDATE_REG(COMM_SIOMULTI2, linkdata[2]);
			UPDATE_REG(COMM_SIOMULTI3, linkdata[3]);
			oncewait = true;
			

		} else {

			after = true;
			if (lanlink.numslaves == 1)
			{
				UPDATE_REG(COMM_SIOMULTI1, linkdata[1]);
				UPDATE_REG(COMM_SIOMULTI2, linkdata[2]);
				UPDATE_REG(COMM_SIOMULTI3, linkdata[3]);
			}
		}
	}
}


void LinkUpdate(int ticks)
{
	if (!linkDriver) {
		return;
	}

	// this actually gets called every single instruction, so keep default
	// path as short as possible

	linktime += ticks;

	linkDriver->update(ticks);
}

inline static int GetSIOMode(u16 siocnt, u16 rcnt)
{
	if (!(rcnt & 0x8000))
	{
		switch (siocnt & 0x3000) {
		case 0x0000: return NORMAL8;
		case 0x1000: return NORMAL32;
		case 0x2000: return MULTIPLAYER;
		case 0x3000: return UART;
		}
	}

	if (rcnt & 0x4000)
		return JOYBUS;

	return GP;
}

static ConnectionState InitIPC() {
//	linkid = 0;
//
//#if (defined __WIN32__ || defined _WIN32)
//	if((mmf=CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(LINKDATA), LOCAL_LINK_NAME))==NULL){
//		systemMessage(0, N_("Error creating file mapping"));
//		return LINK_ERROR;
//	}
//
//	if(GetLastError() == ERROR_ALREADY_EXISTS)
//		vbaid = 1;
//	else
//		vbaid = 0;
//
//
//	if((linkmem=(LINKDATA *)MapViewOfFile(mmf, FILE_MAP_WRITE, 0, 0, sizeof(LINKDATA)))==NULL){
//		CloseHandle(mmf);
//		systemMessage(0, N_("Error mapping file"));
//		return LINK_ERROR;
//	}
//#else
//	if((mmf = shm_open("/" LOCAL_LINK_NAME, O_RDWR|O_CREAT|O_EXCL, 0777)) < 0) {
//		vbaid = 1;
//		mmf = shm_open("/" LOCAL_LINK_NAME, O_RDWR, 0);
//	} else
//		vbaid = 0;
//	if(mmf < 0 || ftruncate(mmf, sizeof(LINKDATA)) < 0 ||
//	   !(linkmem = (LINKDATA *)mmap(NULL, sizeof(LINKDATA),
//					PROT_READ|PROT_WRITE, MAP_SHARED,
//					mmf, 0))) {
//		systemMessage(0, N_("Error creating file mapping"));
//		if(mmf) {
//			if(!vbaid)
//				shm_unlink("/" LOCAL_LINK_NAME);
//			close(mmf);
//		}
//	}
//#endif
//
//	// get lowest-numbered available machine slot
//	bool firstone = !vbaid;
//	if(firstone) {
//		linkmem->linkflags = 1;
//		linkmem->numgbas = 1;
//		linkmem->numtransfers=0;
//		for(i=0;i<4;i++)
//			linkmem->linkdata[i] = 0xffff;
//	} else {
//		// FIXME: this should be done while linkmem is locked
//		// (no xfer in progress, no other vba trying to connect)
//		int n = linkmem->numgbas;
//		int f = linkmem->linkflags;
//		for(int i = 0; i <= n; i++)
//			if(!(f & (1 << i))) {
//				vbaid = i;
//				break;
//			}
//		if(vbaid == 4){
//#if (defined __WIN32__ || defined _WIN32)
//			UnmapViewOfFile(linkmem);
//			CloseHandle(mmf);
//#else
//			munmap(linkmem, sizeof(LINKDATA));
//			if(!vbaid)
//				shm_unlink("/" LOCAL_LINK_NAME);
//			close(mmf);
//#endif
//			systemMessage(0, N_("5 or more GBAs not supported."));
//			return LINK_ERROR;
//		}
//		if(vbaid == n)
//			linkmem->numgbas = n + 1;
//		linkmem->linkflags = f | (1 << vbaid);
//	}
//	linkid = vbaid;
//
//	for(i=0;i<4;i++){
//		linkevent[sizeof(linkevent)-2]=(char)i+'1';
//#if (defined __WIN32__ || defined _WIN32)
//		linksync[i] = firstone ?
//			CreateSemaphoreEx(NULL, 0, 4, linkevent, 0, SEMAPHORE_ALL_ACCESS ) :
//			OpenSemaphore(SEMAPHORE_ALL_ACCESS, false, linkevent);
//		if(linksync[i] == NULL) {
//			UnmapViewOfFile(linkmem);
//			CloseHandle(mmf);
//			for(j=0;j<i;j++){
//				CloseHandle(linksync[j]);
//			}
//			systemMessage(0, N_("Error opening event"));
//			return LINK_ERROR;
//		}
//#else
//		if((linksync[i] = sem_open(linkevent,
//					   firstone ? O_CREAT|O_EXCL : 0,
//					   0777, 0)) == SEM_FAILED) {
//			if(firstone)
//				shm_unlink("/" LOCAL_LINK_NAME);
//			munmap(linkmem, sizeof(LINKDATA));
//			close(mmf);
//			for(j=0;j<i;j++){
//				sem_close(linksync[i]);
//				if(firstone) {
//					linkevent[sizeof(linkevent)-2]=(char)i+'1';
//					sem_unlink(linkevent);
//				}
//			}
//			systemMessage(0, N_("Error opening event"));
//			return LINK_ERROR;
//		}
//#endif
//	}
//
//	return LINK_OK;
	return LINK_OK;
}

static ConnectionState InitSocket() {
	linkid = 0;

	for(int i = 0; i < 4; i++)
		linkdata[i] = 0xffff;

	if (lanlink.server) {
		lanlink.connectedSlaves = 0;
		// should probably use GetPublicAddress()
		//sid->ShowServerIP(sf::IPAddress::GetLocalAddress());

		// too bad Listen() doesn't take an address as well
		// then again, old code used INADDR_ANY anyway
		if (!lanlink.tcpsocket.Listen(IP_LINK_PORT))
			// Note: old code closed socket & retried once on bind failure
			return LINK_ERROR; // FIXME: error code?
		else
			return LINK_NEEDS_UPDATE;
	} else {
		lc.serverport = IP_LINK_PORT;

		if (!lc.serveraddr.IsValid()) {
			return  LINK_ERROR;
		} else {
			lanlink.tcpsocket.SetBlocking(false);
			sf::Socket::Status status = lanlink.tcpsocket.Connect(lc.serverport, lc.serveraddr);

			if (status == sf::Socket::Error || status == sf::Socket::Disconnected)
				return  LINK_ERROR;
			else
				return  LINK_NEEDS_UPDATE;
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// Probably from here down needs to be replaced with SFML goodness :)
// tjm: what SFML goodness?  SFML for network, yes, but not for IPC

ConnectionState InitLink(LinkMode mode)
{
	// Do nothing if we are already connected
	if (GetLinkMode() != LINK_DISCONNECTED) {
		systemMessage(0, N_("Link already connected"));
		return LINK_OK;
	}

	// Find the link driver
	linkDriver = NULL;
	for (u8 i = 0; i < sizeof(linkDrivers) / sizeof(linkDrivers[0]); i++) {
		if (linkDrivers[i].mode == mode) {
			linkDriver = &linkDrivers[i];
			break;
		}
	}

	if (linkDriver == NULL) {
		systemMessage(0, N_("Unable to find link driver"));
		return LINK_ERROR;
	}

	// Connect the link
	gba_connection_state = linkDriver->connect();

	//create event
	//startSendEvent = CreateEventEx(NULL, NULL, NULL, EVENT_ALL_ACCESS);
	//finishSendEvent = CreateEventEx(NULL, NULL, NULL, EVENT_ALL_ACCESS);
	//startReceiveEvent = CreateEventEx(NULL, NULL, NULL, EVENT_ALL_ACCESS);
	//finishReceiveEvent = CreateEventEx(NULL, NULL, NULL, EVENT_ALL_ACCESS);

	if (gba_connection_state == LINK_ERROR) {
		CloseLink();
	}
		
	return gba_connection_state;
}

static ConnectionState ConnectUpdateSocket(char * const message, size_t size) {
	ConnectionState newState = LINK_NEEDS_UPDATE;

	if (lanlink.server) {
		sf::Selector<sf::SocketTCP> fdset;
		fdset.Add(lanlink.tcpsocket);

		if (fdset.Wait(0.1) == 1) {
			int nextSlave = lanlink.connectedSlaves + 1;

			sf::Socket::Status st = lanlink.tcpsocket.Accept(ls.tcpsocket[nextSlave]);

			if (st == sf::Socket::Error) {
				for (int j = 1; j < nextSlave; j++)
					ls.tcpsocket[j].Close();

				snprintf(message, size, N_("Network error."));
				newState = LINK_ERROR;
			} else {
				sf::Packet packet;
				packet 	<< static_cast<sf::Uint16>(nextSlave)
						<< static_cast<sf::Uint16>(lanlink.numslaves);

				ls.tcpsocket[nextSlave].Send(packet);

				snprintf(message, size, N_("Player %d connected"), nextSlave);

				lanlink.connectedSlaves++;
			}
		}

		if (lanlink.numslaves == lanlink.connectedSlaves) {
			for (int i = 1; i <= lanlink.numslaves; i++) {
				sf::Packet packet;
				packet 	<< true;

				ls.tcpsocket[i].Send(packet);
			}

			snprintf(message, size, N_("All players connected"));
			newState = LINK_OK;

			//create thread
			//endLANThread = false;

			//lanlink.linkthread = ThreadPool::RunAsync(ref new WorkItemHandler([](Windows::Foundation::IAsyncAction ^action)
			//{
			//	LANLinkThread();
			//}), WorkItemPriority::Normal, WorkItemOptions::None);

		}
	} else {

		sf::Packet packet;
		
		sf::Socket::Status status = lanlink.tcpsocket.Receive(packet);

		if (status == sf::Socket::Error || status == sf::Socket::Disconnected) {
			snprintf(message, size, N_("Network error."));
			newState = LINK_ERROR;
		} else if (status == sf::Socket::Done) {

			if (linkid == 0) {
				sf::Uint16 receivedId, receivedSlaves;
				packet >> receivedId >> receivedSlaves;

				if (packet) {
					linkid = receivedId;
					lanlink.numslaves = receivedSlaves;

					snprintf(message, size, N_("Connected as #%d, Waiting for %d players to join"),
							linkid + 1, lanlink.numslaves - linkid);
				}
			} else {
				bool gameReady;
				packet >> gameReady;

				if (packet && gameReady) 
				{
					newState = LINK_OK;
					snprintf(message, size, N_("All players joined."));

					//create thread
					//endLANThread = false;

					//lanlink.linkthread = ThreadPool::RunAsync(ref new WorkItemHandler([](Windows::Foundation::IAsyncAction ^action)
					//{
					//	LANLinkThread();
					//}), WorkItemPriority::Normal, WorkItemOptions::None);
				}
			}

			sf::Selector<sf::SocketTCP> fdset;
			fdset.Add(lanlink.tcpsocket);
			fdset.Wait(0.1);
		}
	}

	return newState;
}

ConnectionState ConnectLinkUpdate(char * const message, size_t size)
{
	message[0] = '\0';

	if (!linkDriver || gba_connection_state != LINK_NEEDS_UPDATE) {
		gba_connection_state = LINK_ERROR;
		snprintf(message, size, N_("Link connection does not need updates."));

		return LINK_ERROR;
	}

	gba_connection_state = linkDriver->connectUpdate(message, size);

	return gba_connection_state;
}

void EnableLinkServer(bool enable, int numSlaves) {
	lanlink.server = enable;
	lanlink.numslaves = numSlaves;
}

void EnableSpeedHacks(bool enable) {
	lanlink.speed = enable;
}

bool SetLinkServerHost(const char *host) {
	sf::IPAddress addr = sf::IPAddress(host);

	lc.serveraddr = addr;
	joybusHostAddr = addr;

	return addr.IsValid();
}

void GetLinkServerHost(char * const host, size_t size) {
	if (host == NULL || size == 0)
		return;

	host[0] = '\0';

	if (linkDriver && linkDriver->mode == LINK_GAMECUBE_DOLPHIN)
		strncpy(host, joybusHostAddr.ToString().c_str(), size);
	else if (lanlink.server)
		strncpy(host, sf::IPAddress::GetLocalAddress().ToString().c_str(), size);
	else
		strncpy(host, lc.serveraddr.ToString().c_str(), size);
}

void SetLinkTimeout(int value) {
	linktimeout = value;
}

int GetLinkPlayerId() {
	if (GetLinkMode() == LINK_DISCONNECTED) {
		return -1;
	} else if (linkid > 0) {
		return linkid;
	} else {
		return vbaid;
	}
}

static void ReInitLink()
{
	int f = linkmem->linkflags;
	int n = linkmem->numgbas;
	if(f & (1 << linkid)) {
		systemMessage(0, N_("Lost link; reinitialize to reconnect"));
		return;
	}
	linkmem->linkflags |= 1 << linkid;
	if(n < linkid + 1)
		linkmem->numgbas = linkid + 1;
	numtransfers = linkmem->numtransfers;
	systemScreenMessage(_("Lost link; reconnected"));
}

static void CloseIPC() {
//	int f = linkmem->linkflags;
//	f &= ~(1 << linkid);
//	if(f & 0xf) {
//		linkmem->linkflags = f;
//		int n = linkmem->numgbas;
//		for(int i = 0; i < n; i--)
//			if(f <= (1 << (i + 1)) - 1) {
//				linkmem->numgbas = i + 1;
//				break;
//			}
//	}
//
//	for(i=0;i<4;i++){
//		if(linksync[i]!=NULL){
//#if (defined __WIN32__ || defined _WIN32)
//			ReleaseSemaphore(linksync[i], 1, NULL);
//			CloseHandle(linksync[i]);
//#else
//			sem_close(linksync[i]);
//			if(!(f & 0xf)) {
//				linkevent[sizeof(linkevent)-2]=(char)i+'1';
//				sem_unlink(linkevent);
//			}
//#endif
//		}
//	}
//#if (defined __WIN32__ || defined _WIN32)
//	CloseHandle(mmf);
//	UnmapViewOfFile(linkmem);
//
//	// FIXME: move to caller
//	// (but there are no callers, so why bother?)
//	//regSetDwordValue("LAN", lanlink.active);
//#else
//	if(!(f & 0xf))
//		shm_unlink("/" LOCAL_LINK_NAME);
//	munmap(linkmem, sizeof(LINKDATA));
//	close(mmf);
//#endif
}

static void CloseSocket() {
	if(linkid){
		char outbuffer[4];
		outbuffer[0] = 4;
		outbuffer[1] = -32;
		if(lanlink.type==0) lanlink.tcpsocket.Send(outbuffer, 4);
	} else {
		char outbuffer[12];
		int i;
		outbuffer[0] = 12;
		outbuffer[1] = -32;
		for(i=1;i<=lanlink.numslaves;i++){
			if(lanlink.type==0){
				ls.tcpsocket[i].Send(outbuffer, 12);
			}
			ls.tcpsocket[i].Close();
		}
	}
	lanlink.tcpsocket.Close();
}

void CloseLink(void){
	if (!linkDriver) {
		return; // Nothing to do
	}

	linkDriver->close();
	linkDriver = NULL;

	endLANThread = true; //so that LanLinkThread knows to terminate

	return;

	
}

// call this to clean up crashed program's shared state
// or to use TCP on same machine (for testing)
// this may be necessary under MSW as well, but I wouldn't know how
void CleanLocalLink()
{
#if !(defined __WIN32__ || defined _WIN32)
	shm_unlink("/" LOCAL_LINK_NAME);
	for(int i = 0; i < 4; i++) {
		linkevent[sizeof(linkevent) - 2] = '1' + i;
		sem_unlink(linkevent);
	}
#endif
}


void LANLinkThread()
{
	while (!endLANThread)
	{
		if (linkid == 0) //master
		{
			WaitForSingleObjectEx(startSendEvent, INFINITE, false);  
			ResetEvent(startSendEvent);
			ls.Send(); //send data to slaves

			ls.Recv(); //start receive data from slave right away

			SetEvent(finishReceiveEvent); //signal receive is complete


			//WaitForSingleObjectEx(startReceiveEvent, INFINITE, false); 
			//ResetEvent(startReceiveEvent);
			//ls.Recv();  //wait for data from slave

			//ls.Send(); //send data back to slaves right away

			//SetEvent(finishSendEvent); //signal sending is complete

		}
		else //slave
		{
			WaitForSingleObjectEx(startSendEvent, INFINITE, false);  
			ResetEvent(startSendEvent);
			lc.Send();  //send to master
			

			lc.Recv(); //start receive back from master right away

			SetEvent(finishReceiveEvent); //signal receive is complete
		}


	}

	//	//set all events to prevent deadlock
	//SetEvent(startSendEvent);
	//SetEvent(startReceiveEvent);
	//SetEvent(finishSendEvent);
	//SetEvent(finishReceiveEvent);

	CloseHandle(startSendEvent);
	CloseHandle(finishSendEvent);
	CloseHandle(startReceiveEvent);
	CloseHandle(finishReceiveEvent);


	return;


}

// Server
lserver::lserver(void){
	intinbuffer = (s32*)inbuffer;
	u16inbuffer = (u16*)inbuffer;
	intoutbuffer = (s32*)outbuffer;
	u16outbuffer = (u16*)outbuffer;
	oncewait = false;
}

void lserver::Send(void){
	if(lanlink.type==0){	// TCP
		if(savedlinktime==-1)
		{
			outbuffer[0] = 4;
			outbuffer[1] = -32;	//0xe0
			for(i=1;i<=lanlink.numslaves;i++){
				tcpsocket[i].Send(outbuffer, 4);
				size_t nr;
				tcpsocket[i].Receive(inbuffer, 4, nr);
			}
		}
		outbuffer[1] = tspeed;
		WRITE16LE(&u16outbuffer[1], linkdata[0]);
		WRITE32LE(&intoutbuffer[1], savedlinktime);
		if(lanlink.numslaves==1){
			if(lanlink.type==0){
				outbuffer[0] = 8;
				tcpsocket[1].Send(outbuffer, 8);
			}
		}
		else if(lanlink.numslaves==2){
			WRITE16LE(&u16outbuffer[4], linkdata[2]);
			if(lanlink.type==0){
				outbuffer[0] = 10;
				tcpsocket[1].Send(outbuffer, 10);
				WRITE16LE(&u16outbuffer[4], linkdata[1]);
				tcpsocket[2].Send(outbuffer, 10);
			}
		} else {
			if(lanlink.type==0){
				outbuffer[0] = 12;
				WRITE16LE(&u16outbuffer[4], linkdata[2]);
				WRITE16LE(&u16outbuffer[5], linkdata[3]);
				tcpsocket[1].Send(outbuffer, 12);
				WRITE16LE(&u16outbuffer[4], linkdata[1]);
				tcpsocket[2].Send(outbuffer, 12);
				WRITE16LE(&u16outbuffer[5], linkdata[2]);
				tcpsocket[3].Send(outbuffer, 12);
			}
		}
	}
	return;
}

void lserver::Recv(void){
	int numbytes;
	if(lanlink.type==0) // TCP
	{	
		fdset.Clear();
		for(i=0;i<lanlink.numslaves;i++) 
			fdset.Add(tcpsocket[i+1]);

		// was linktimeout/1000 (i.e., drop ms part), but that's wrong
		if (fdset.Wait((float)(linktimeout / 1000.)) == 0)  //this keeps executing even when slave is down
															//this function wait for data sent from sleep
															//when slave is slow, this slows down emulation a lot
		{
			return;
		}
		howmanytimes++; //this returns 1 without speed hack
		for(i=0;i<lanlink.numslaves;i++){
			numbytes = 0;
			inbuffer[0] = 1;

			sf::Socket::Status status;
			while(numbytes<howmanytimes*inbuffer[0]) {  //read all data from a client
				size_t nr;
				status = tcpsocket[i+1].Receive(inbuffer+numbytes, howmanytimes*inbuffer[0]-numbytes, nr);
				numbytes += nr;

				if (status == sf::Socket::Disconnected)
				{
					CloseLink();
					return;
				}
			}
			if(howmanytimes>1)
				memmove(inbuffer, inbuffer+inbuffer[0]*(howmanytimes-1), inbuffer[0]); //speed hack?

			if(inbuffer[1]==-32)
			{
				char message[30];
				sprintf(message, _("Player %d disconnected."), i+2);
				systemScreenMessage(message);
				outbuffer[0] = 4;
				outbuffer[1] = -32;
				for(i=1;i<lanlink.numslaves;i++)
				{
					tcpsocket[i].Send(outbuffer, 12);
					size_t nr;
					tcpsocket[i].Receive(inbuffer, 256, nr);
					tcpsocket[i].Close();
				}
				CloseLink();
				return;
			}
			linkdata[i+1] = READ16LE(&u16inbuffer[1]);
		}
		howmanytimes = 0;
	}
	after = false;
	return;
}

void CheckLinkConnection() {
	if (GetLinkMode() == LINK_CABLE_SOCKET) {
		if (linkid && lc.numtransfers == 0) {
			lc.CheckConn();
		}
	}
}

// Client
lclient::lclient(void){
	intinbuffer = (s32*)inbuffer;
	u16inbuffer = (u16*)inbuffer;
	intoutbuffer = (s32*)outbuffer;
	u16outbuffer = (u16*)outbuffer;
	numtransfers = 0;
	return;
}

//this function runs on slave to periodically check for available data from master
void lclient::CheckConn(void){
	size_t nr;
	lanlink.tcpsocket.Receive(inbuffer, 1, nr); //read 1 byte, whose content is number of bytes to read after that
	numbytes = nr;
	if(numbytes>0){
		sf::Socket::Status status;
		while(numbytes<inbuffer[0]) {
			status = lanlink.tcpsocket.Receive(inbuffer+numbytes, inbuffer[0] - numbytes, nr);
			numbytes += nr;

			if (status == sf::Socket::Disconnected)
			{
				CloseLink();
				return;
			}
		}
		if(inbuffer[1]==-32){
			outbuffer[0] = 4;
			lanlink.tcpsocket.Send(outbuffer, 4);
			systemScreenMessage(_("Server disconnected."));
			CloseLink();
			return;
		}
		numtransfers = 1;
		savedlinktime = 0;
		linkdata[0] = READ16LE(&u16inbuffer[1]);
		tspeed = inbuffer[1] & 3;
		for(i=1, numbytes=4;i<=lanlink.numslaves;i++)
			if(i!=linkid) {
				linkdata[i] = READ16LE(&u16inbuffer[numbytes]);
				numbytes++;
			}
		after = false;
		oncewait = true;
	}
	return;
}

void lclient::Recv(void){
	fdset.Clear();
	// old code used socket # instead of mask again
	fdset.Add(lanlink.tcpsocket);
	// old code stripped off ms again
	if (fdset.Wait((float)(linktimeout / 1000.)) == 0)  //afer a period of no data from master, set numtransfer to 0 to stop trying to receive data
														//then periodically check for data from master by CheckConn
														//when connection is slow, waiting for data slow down emulation a lot
	{
		numtransfers = 0;
		return;
	}
	numbytes = 0;
	inbuffer[0] = 1;
	size_t nr;

	//retrieve data from host after host has received data from all clients
	//first loop read only 1 byte, store in inbuffer[0], which is the total number of bytes to receive
	//subsequence loop read the remaining data

	sf::Socket::Status status;
	while(numbytes<inbuffer[0]) {
		status = lanlink.tcpsocket.Receive(inbuffer+numbytes, inbuffer[0] - numbytes, nr);
		if (status == sf::Socket::Disconnected)
		{
			CloseLink();
			return;
		}
		numbytes += nr;

	}

	//check inbuffer[1] for error code
	if(inbuffer[1]==-32){
		outbuffer[0] = 4;
		lanlink.tcpsocket.Send(outbuffer, 4);
		systemScreenMessage(_("Server disconnected."));
		CloseLink();
		return;
	}

	//adjust speed
	tspeed = inbuffer[1] & 3;
	linkdata[0] = READ16LE(&u16inbuffer[1]);  //u16inbuffer is mapped to inbuffer at the beginning
	savedlinktime = (s32)READ32LE(&intinbuffer[1]);
	for(i=1, numbytes=4;i<lanlink.numslaves+1;i++)
		if(i!=linkid) {
			linkdata[i] = READ16LE(&u16inbuffer[numbytes]);
			numbytes++;
		}
	numtransfers++;
	if(numtransfers==0) numtransfers = 2;
	after = false;
}

void lclient::Send(){
	outbuffer[0] = 4;
	outbuffer[1] = linkid<<2;
	WRITE16LE(&u16outbuffer[1], linkdata[linkid]);
	lanlink.tcpsocket.Send(outbuffer, 4);  //send data to host
	return;
}
#endif
