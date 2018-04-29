// Based on MAME sources by Bryan McPhail, Nicola Salmoria

#include "burnint.h"
#include "taito_ic.h"
#include "taito.h"

static UINT8 CurrentBank;

/////////////////////////////////////////////////////////////////////////////////////////

// Operation Wolf C-Chip

#define OPWOLF_REGION_JAPAN		1
#define OPWOLF_REGION_US		2
#define OPWOLF_REGION_WORLD		3
#define OPWOLF_REGION_OTHER		4

static INT32 CChipRegion;

static UINT8 CurrentCmd=0;
static UINT8* CChipRam=0;
static UINT8 CChipLast_7a=0;
static UINT8 CChipLast_04=0;
static UINT8 CChipLast_05=0;
static UINT8 CChipCoinsForCredit[2]={1,1};
static UINT8 CChipCreditsForCoin[2]={1,1};
static UINT8 CChipCoins[2]={0,0};
static UINT8 c588=0, c589=0, c58a=0;
static UINT8 triggeredLevel1b; // These variables derived from comparison to unprotection version
static UINT8 triggeredLevel2;
static UINT8 triggeredLevel2b;
static UINT8 triggeredLevel2c;
static UINT8 triggeredLevel3b;
static UINT8 triggeredLevel13b;
static UINT8 triggeredLevel4;
static UINT8 triggeredLevel5;
static UINT8 triggeredLevel7;
static UINT8 triggeredLevel8;
static UINT8 triggeredLevel9;

static const UINT16 level_data_00[] = {
	0x0480, 0x1008, 0x0300,   0x5701, 0x0001, 0x0010,
	0x0480, 0x1008, 0x0300,   0x5701, 0x0001, 0x002b,
	0x0780, 0x0009, 0x0300,   0x4a01, 0x0004, 0x0020,
	0x0780, 0x1208, 0x0300,   0x5d01, 0x0004, 0x0030,
	0x0780, 0x0209, 0x0300,   0x4c01, 0x0004, 0x0038,
	0x0780, 0x0309, 0x0300,   0x4d01, 0x0004, 0x0048,
	0x0980, 0x1108, 0x0300,   0x5a01, 0xc005, 0x0018,
	0x0980, 0x0109, 0x0300,   0x4b01, 0xc005, 0x0028,
	0x0b80, 0x020a, 0x0000,   0x6401, 0x8006, 0x0004,
	0x0c80, 0x010b, 0x0000,   0xf201, 0x8006, 0x8002,
	0x0b80, 0x020a, 0x0000,   0x6401, 0x8006, 0x0017,
	0x0c80, 0x010b, 0x0000,   0xf201, 0x8006, 0x8015,
	0x0b80, 0x020a, 0x0000,   0x6401, 0x0007, 0x0034,
	0x0c80, 0x010b, 0x0000,   0xf201, 0x0007, 0x8032,
	0x0b80, 0x020a, 0x0000,   0x6401, 0x8006, 0x803e,
	0x0c80, 0x010b, 0x0000,   0xf201, 0x8006, 0x803d,
	0x0b80, 0x100a, 0x0000,   0x6001, 0x0007, 0x0008,
	0x0b80, 0x100a, 0x0000,   0x6001, 0x0007, 0x000b,
	0x0b80, 0x100a, 0x0000,   0x6001, 0x0007, 0x001b,
	0x0b80, 0x100a, 0x0000,   0x6001, 0x0007, 0x001e,
	0x0b80, 0x100a, 0x0000,   0x6001, 0x8007, 0x0038,
	0x0b80, 0x100a, 0x0000,   0x6001, 0x8007, 0x003b,
	0x0b80, 0x100a, 0x0000,   0x6001, 0x0007, 0x8042,
	0x0b80, 0x100a, 0x0000,   0x6001, 0x0007, 0x8045,
	0x0c80, 0x000b, 0x0000,   0xf101, 0x800b, 0x8007,
	0x0c80, 0x000b, 0x0000,   0xf101, 0x800b, 0x801a,
	0x0c80, 0x000b, 0x0000,   0xf101, 0x000c, 0x8037,
	0x0c80, 0x000b, 0x0000,   0xf101, 0x800b, 0x0042,
	0x0c80, 0xd04b, 0x0000,   0xf301, 0x8006, 0x8009,
	0x0c80, 0xd04b, 0x0000,   0xf301, 0x8006, 0x801c,
	0x0c80, 0xd04b, 0x0000,   0xf301, 0x8006, 0x0044,
	0x0c80, 0x030b, 0x0000,   0xf401, 0x0008, 0x0028,
	0x0c80, 0x030b, 0x0000,   0xf401, 0x0008, 0x804b,
	0x0c00, 0x040b, 0x0000,   0xf501, 0x0008, 0x8026,
	0xffff
};

static const UINT16 level_data_01[] = {
	0x0780, 0x0209, 0x0300,   0x4c01, 0x0004, 0x0010,
	0x0780, 0x0209, 0x0300,   0x4c01, 0x4004, 0x0020,
	0x0780, 0x0309, 0x0300,   0x4d01, 0xe003, 0x0030,
	0x0780, 0x0309, 0x0300,   0x4d01, 0x8003, 0x0040,
	0x0780, 0x0209, 0x0300,   0x4c01, 0x8004, 0x0018,
	0x0780, 0x0309, 0x0300,   0x4d01, 0xc003, 0x0028,
	0x0b80, 0x000b, 0x0000,   0x0b02, 0x8009, 0x0029,
	0x0b80, 0x0409, 0x0000,   0x0f02, 0x8008, 0x8028,
	0x0b80, 0x040a, 0x0000,   0x3502, 0x000a, 0x8028,
	0x0b80, 0x050a, 0x0000,   0x1002, 0x8006, 0x8028,
	0x0b80, 0x120a, 0x0000,   0x3602, 0x0008, 0x004d,
	0x0b80, 0x120a, 0x0000,   0x3602, 0x0008, 0x004f,
	0x0b80, 0x120a, 0x0000,   0x3602, 0x0008, 0x0001,
	0x0b80, 0x120a, 0x0000,   0x3602, 0x0008, 0x0003,
	0x0b80, 0x130a, 0x0000,   0x3a02, 0x0007, 0x0023,
	0x0b80, 0x130a, 0x0000,   0x3a02, 0x0007, 0x8025,
	0x0b80, 0x130a, 0x0000,   0x3a02, 0x8009, 0x0023,
	0x0b80, 0x130a, 0x0000,   0x3a02, 0x8009, 0x8025,
	0x0b80, 0x140a, 0x0000,   0x3e02, 0x0007, 0x000d,
	0x0b80, 0x140a, 0x0000,   0x3e02, 0x0007, 0x800f,
	0x0b80, 0x000b, 0x0000,   0x0102, 0x0007, 0x804e,
	0x0b80, 0xd24b, 0x0000,   0x0302, 0x0007, 0x000e,
	0x0b80, 0x000b, 0x0000,   0x0402, 0x8006, 0x0020,
	0x0b80, 0xd34b, 0x0000,   0x0502, 0x8006, 0x0024,
	0x0b80, 0x000b, 0x0000,   0x0602, 0x8009, 0x0001,
	0x0b80, 0xd44b, 0x0000,   0x0702, 0x800b, 0x800b,
	0x0b80, 0xd54b, 0x0000,   0x0802, 0x800b, 0x000e,
	0x0b80, 0x000b, 0x0000,   0x0902, 0x800b, 0x0010,
	0x0b80, 0x000b, 0x0000,   0x0a02, 0x0009, 0x0024,
	0x0b80, 0xd64b, 0x0000,   0x0c02, 0x000c, 0x8021,
	0x0b80, 0x000b, 0x0000,   0x0d02, 0x000c, 0x0025,
	0x0b80, 0x000b, 0x0000,   0x0e02, 0x8009, 0x004e,
	0x0b80, 0x000b, 0x0300,   0x4e01, 0x8006, 0x8012,
	0x0b80, 0x000b, 0x0300,   0x4e01, 0x0007, 0x8007,
	0xffff
};

static const UINT16 level_data_02[] = {
	0x0480, 0x000b, 0x0300,   0x4501, 0x0001, 0x0018,
	0x0480, 0x000b, 0x0300,   0x4501, 0x2001, 0x0030,
	0x0780, 0x1208, 0x0300,   0x5d01, 0x0004, 0x0010,
	0x0780, 0x1208, 0x0300,   0x5d01, 0x2004, 0x001c,
	0x0780, 0x1208, 0x0300,   0x5d01, 0xe003, 0x0026,
	0x0780, 0x1208, 0x0300,   0x5d01, 0x8003, 0x0034,
	0x0780, 0x1208, 0x0300,   0x5d01, 0x3004, 0x0040,
	0x0780, 0x010c, 0x0300,   0x4601, 0x4004, 0x0022,
	0x0780, 0x010c, 0x0300,   0x4601, 0x6004, 0x0042,
	0x0780, 0x000c, 0x0500,   0x7b01, 0x800b, 0x0008,
	0x0780, 0x010c, 0x0300,   0x4601, 0x2004, 0x0008,
	0x0000, 0x0000, 0x0000,   0xf001, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000,   0xf001, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000,   0xf001, 0x0000, 0x0000,
	0x0b80, 0x000b, 0x0000,   0x1902, 0x000b, 0x0004,
	0x0b80, 0x000b, 0x0000,   0x1a02, 0x0009, 0x8003,
	0x0b80, 0x000b, 0x0000,   0x1902, 0x000b, 0x000c,
	0x0b80, 0x000b, 0x0000,   0x1a02, 0x0009, 0x800b,
	0x0b80, 0x000b, 0x0000,   0x1902, 0x000b, 0x001c,
	0x0b80, 0x000b, 0x0000,   0x1a02, 0x0009, 0x801b,
	0x0b80, 0x000b, 0x0000,   0x1902, 0x000b, 0x002c,
	0x0b80, 0x000b, 0x0000,   0x1a02, 0x0009, 0x802b,
	0x0b80, 0x000b, 0x0000,   0x1902, 0x000b, 0x0044,
	0x0b80, 0x000b, 0x0000,   0x1a02, 0x0009, 0x8043,
	0x0b80, 0x000b, 0x0000,   0x1902, 0x000b, 0x004c,
	0x0b80, 0x000b, 0x0000,   0x1a02, 0x0009, 0x804b,
	0x0b80, 0x020c, 0x0300,   0x4801, 0xa009, 0x0010,
	0x0b80, 0x020c, 0x0300,   0x4801, 0xa009, 0x0028,
	0x0b80, 0x020c, 0x0300,   0x4801, 0xa009, 0x0036,
	0x0000, 0x0000, 0x0000,   0xf001, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000,   0xf001, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000,   0xf001, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000,   0xf001, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000,   0xf001, 0x0000, 0x0000,
	0xffff
};

static const UINT16 level_data_03[] = {
	0x0480, 0x000b, 0x0300,   0x4501, 0x0001, 0x0018,
	0x0480, 0x000b, 0x0300,   0x4501, 0x2001, 0x002b,
	0x0780, 0x010c, 0x0300,   0x4601, 0x0004, 0x000d,
	0x0780, 0x000c, 0x0500,   0x7b01, 0x800b, 0x0020,
	0x0780, 0x010c, 0x0300,   0x4601, 0x2004, 0x0020,
	0x0780, 0x010c, 0x0300,   0x4601, 0x8003, 0x0033,
	0x0780, 0x010c, 0x0300,   0x4601, 0x0004, 0x003c,
	0x0780, 0x010c, 0x0300,   0x4601, 0xd003, 0x0045,
	0x0780, 0x000c, 0x0500,   0x7b01, 0x900b, 0x0041,
	0x0780, 0x010c, 0x0300,   0x4601, 0x3004, 0x0041,
	0x0b80, 0x020c, 0x0300,   0x4801, 0x0007, 0x0000,
	0x0b80, 0x410a, 0x0000,   0x2b02, 0xe006, 0x4049,
	0x0b80, 0x020c, 0x0300,   0x4801, 0x8007, 0x000b,
	0x0b80, 0x000b, 0x0000,   0x2702, 0x800a, 0x8005,
	0x0b80, 0x000b, 0x0000,   0x1e02, 0x0008, 0x800e,
	0x0b80, 0x000b, 0x0000,   0x1f02, 0x8007, 0x0011,
	0x0b80, 0x000b, 0x0000,   0x2802, 0x000b, 0x0012,
	0x0b80, 0x000b, 0x0000,   0x2002, 0x0007, 0x8015,
	0x0b80, 0x000b, 0x0000,   0x2102, 0x0007, 0x801b,
	0x0b80, 0x000b, 0x0000,   0x2902, 0x800a, 0x001a,
	0x0b80, 0x000b, 0x0000,   0x2202, 0x8007, 0x001e,
	0x0b80, 0x000b, 0x0000,   0x1e02, 0x0008, 0x0025,
	0x0b80, 0x000b, 0x0000,   0x2302, 0x8007, 0x802c,
	0x0b80, 0x000b, 0x0000,   0x2802, 0x000b, 0x8028,
	0x0b80, 0x020c, 0x0300,   0x4801, 0x0007, 0x0030,
	0x0b80, 0x400a, 0x0000,   0x2e02, 0x4007, 0x002d,
	0x0b80, 0x000b, 0x0000,   0x2702, 0x800a, 0x8035,
	0x0b80, 0x020c, 0x0300,   0x4801, 0x8007, 0x0022,
	0x0b80, 0x000b, 0x0000,   0x2402, 0x8007, 0x0047,
	0x0b80, 0x000b, 0x0000,   0x2a02, 0x800a, 0x004b,
	0x0b80, 0x000b, 0x0000,   0x2502, 0x0007, 0x804b,
	0x0b80, 0x000b, 0x0000,   0x2602, 0x0007, 0x004e,
	0x0b80, 0x020c, 0x0300,   0x4801, 0x0007, 0x8043,
	0x0b80, 0x020c, 0x0300,   0x4801, 0x8007, 0x803d,
	0xffff
};

static const UINT16 level_data_04[] = {
	0x0780, 0x0209, 0x0300,   0x4c01, 0x0004, 0x0010,
	0x0780, 0x0209, 0x0300,   0x4c01, 0x4004, 0x0020,
	0x0780, 0x0309, 0x0300,   0x4d01, 0xe003, 0x0030,
	0x0780, 0x0309, 0x0300,   0x4d01, 0x8003, 0x0040,
	0x0780, 0x0209, 0x0300,   0x4c01, 0x8004, 0x0018,
	0x0780, 0x0309, 0x0300,   0x4d01, 0xc003, 0x0028,
	0x0780, 0x000b, 0x0300,   0x5601, 0x8004, 0x0008,
	0x0780, 0x000b, 0x0300,   0x5601, 0x8004, 0x0038,
	0x0780, 0x000b, 0x0300,   0x5501, 0x8004, 0x0048,
	0x0980, 0x0509, 0x0f00,   0x0f01, 0x4005, 0x4007,
	0x0980, 0x0509, 0x0f00,   0x0f01, 0x4005, 0x4037,
	0x0b80, 0x030a, 0x0000,   0x1302, 0x8006, 0x0040,
	0x0b80, 0x110a, 0x0000,   0x1502, 0x8008, 0x8048,
	0x0b80, 0x110a, 0x0000,   0x1502, 0x8008, 0x8049,
	0x0b80, 0x000b, 0x0000,   0xf601, 0x0007, 0x8003,
	0x0b80, 0x000b, 0x0000,   0xf701, 0x0007, 0x0005,
	0x0b80, 0x000b, 0x0000,   0xf901, 0x0007, 0x8008,
	0x0b80, 0x000b, 0x0000,   0xf901, 0x0007, 0x0010,
	0x0b80, 0x000b, 0x0000,   0xfa01, 0x0007, 0x8013,
	0x0b80, 0x000b, 0x0000,   0xf801, 0x800b, 0x800b,
	0x0b80, 0x000b, 0x0000,   0x0002, 0x800b, 0x801a,
	0x0b80, 0x000b, 0x0000,   0xf901, 0x0007, 0x8017,
	0x0b80, 0x000b, 0x0000,   0xfa01, 0x0007, 0x001b,
	0x0b80, 0x000b, 0x0000,   0xf801, 0x800b, 0x0013,
	0x0b80, 0x000b, 0x0000,   0x4202, 0x800b, 0x0016,
	0x0b80, 0x000b, 0x0000,   0xfb01, 0x8007, 0x8020,
	0x0b80, 0x000b, 0x0000,   0xf601, 0x0007, 0x8023,
	0x0b80, 0x000b, 0x0000,   0x4202, 0x800b, 0x800e,
	0x0b80, 0x000b, 0x0000,   0x4302, 0x800b, 0x801d,
	0x0b80, 0x000b, 0x0000,   0xf701, 0x0007, 0x0025,
	0x0b80, 0x000b, 0x0000,   0xfd01, 0x8006, 0x003f,
	0x0b80, 0x000b, 0x0000,   0xfe01, 0x0007, 0x0046,
	0x0b80, 0x000b, 0x0000,   0xff01, 0x8007, 0x8049,
	0x0b80, 0x000b, 0x0000,   0xfc01, 0x8009, 0x0042,
	0xffff
};

static const UINT16 level_data_05[] = {
	0x0480, 0x1008, 0x0300,   0x5701, 0x0001, 0x0010,
	0x0480, 0x1008, 0x0300,   0x5701, 0x0001, 0x002b,
	0x0780, 0x0009, 0x0300,   0x4a01, 0x0004, 0x0020,
	0x0780, 0x1208, 0x0300,   0x5d01, 0x0004, 0x0030,
	0x0780, 0x0209, 0x0300,   0x4c01, 0x0004, 0x0038,
	0x0780, 0x0309, 0x0300,   0x4d01, 0x0004, 0x0048,
	0x0980, 0x1108, 0x0300,   0x5a01, 0xc005, 0x0018,
	0x0980, 0x0109, 0x0300,   0x4b01, 0xc005, 0x0028,
	0x0b80, 0x020a, 0x0000,   0x6401, 0x8006, 0x0004,
	0x0c80, 0x010b, 0x0000,   0xf201, 0x8006, 0x8002,
	0x0b80, 0x020a, 0x0000,   0x6401, 0x8006, 0x0017,
	0x0c80, 0x010b, 0x0000,   0xf201, 0x8006, 0x8015,
	0x0b80, 0x020a, 0x0000,   0x6401, 0x0007, 0x0034,
	0x0c80, 0x010b, 0x0000,   0xf201, 0x0007, 0x8032,
	0x0b80, 0x020a, 0x0000,   0x6401, 0x8006, 0x803e,
	0x0c80, 0x010b, 0x0000,   0xf201, 0x8006, 0x803d,
	0x0b80, 0x100a, 0x0000,   0x6001, 0x0007, 0x0008,
	0x0b80, 0x100a, 0x0000,   0x6001, 0x0007, 0x000b,
	0x0b80, 0x100a, 0x0000,   0x6001, 0x0007, 0x001b,
	0x0b80, 0x100a, 0x0000,   0x6001, 0x0007, 0x001e,
	0x0b80, 0x100a, 0x0000,   0x6001, 0x8007, 0x0038,
	0x0b80, 0x100a, 0x0000,   0x6001, 0x8007, 0x003b,
	0x0b80, 0x100a, 0x0000,   0x6001, 0x0007, 0x8042,
	0x0b80, 0x100a, 0x0000,   0x6001, 0x0007, 0x8045,
	0x0c80, 0x000b, 0x0000,   0xf101, 0x800b, 0x8007,
	0x0c80, 0x000b, 0x0000,   0xf101, 0x800b, 0x801a,
	0x0c80, 0x000b, 0x0000,   0xf101, 0x000c, 0x8037,
	0x0c80, 0x000b, 0x0000,   0xf101, 0x800b, 0x0042,
	0x0c80, 0xd04b, 0x0000,   0xf301, 0x8006, 0x8009,
	0x0c80, 0xd04b, 0x0000,   0xf301, 0x8006, 0x801c,
	0x0c80, 0xd04b, 0x0000,   0xf301, 0x8006, 0x0044,
	0x0c80, 0x030b, 0x0000,   0xf401, 0x0008, 0x0028,
	0x0c80, 0x030b, 0x0000,   0xf401, 0x0008, 0x804b,
	0x0c00, 0x040b, 0x0000,   0xf501, 0x0008, 0x8026,
	0xffff
};

static const UINT16 level_data_06[] = {
	0x0000, 0x1008, 0x0300,   0x5701, 0x0001, 0x0010,
	0x0000, 0x1008, 0x0300,   0x5701, 0x0001, 0x002b,
	0x0000, 0x0000, 0x0000,   0x0000, 0x0000, 0x0000,
	0x0700, 0x0009, 0x0300,   0x4a01, 0x0004, 0x0020,
	0x0700, 0x1208, 0x0300,   0x5d01, 0x0004, 0x0030,
	0x0700, 0x0209, 0x0300,   0x4c01, 0x0004, 0x0038,
	0x0700, 0x0309, 0x0300,   0x4d01, 0x0004, 0x0048,
	0x0900, 0x1108, 0x0300,   0x5a01, 0xc005, 0x0018,
	0x0900, 0x0109, 0x0300,   0x4b01, 0xc005, 0x0028,
	0x0000, 0x000b, 0x0000,   0x0000, 0x0018, 0x0000,
	0x0000, 0x000b, 0x0000,   0x0000, 0x0018, 0x0000,
	0x0000, 0x000b, 0x0000,   0x0000, 0x0018, 0x0000,
	0x0000, 0x000b, 0x0000,   0x0000, 0x0018, 0x0000,
	0x0000, 0x000b, 0x0000,   0x0000, 0x0018, 0x0000,
	0x0000, 0x000b, 0x0000,   0x0000, 0x0018, 0x0000,
	0x0000, 0x000b, 0x0000,   0x0000, 0x0018, 0x0000,
	0x0000, 0x000b, 0x0000,   0x0000, 0x0018, 0x0000,
	0x0000, 0x000b, 0x0000,   0x0000, 0x0018, 0x0000,
	0x0980, 0xdb4c, 0x0000,   0x3202, 0x0006, 0x0004,
	0x0000, 0x000b, 0x0000,   0x0000, 0x0018, 0x0000,
	0x0000, 0x000b, 0x0000,   0x0000, 0x0018, 0x0000,
	0x0000, 0x000b, 0x0000,   0x0000, 0x0018, 0x0000,
	0x0000, 0x000b, 0x0000,   0x0000, 0x0018, 0x0000,
	0x0000, 0x000b, 0x0000,   0x0000, 0x0018, 0x0000,
	0x0000, 0x000b, 0x0000,   0x0000, 0x0018, 0x0000,
	0x0000, 0x000b, 0x0000,   0x0000, 0x0018, 0x0000,
	0x0000, 0x000b, 0x0000,   0x0000, 0x0018, 0x0000,
	0x0000, 0x000b, 0x0000,   0x0000, 0x0018, 0x0000,
	0x0000, 0x000b, 0x0000,   0x0000, 0x0018, 0x0000,
	0x0000, 0x000b, 0x0000,   0x0000, 0x0018, 0x0000,
	0x0000, 0x000b, 0x0000,   0x0000, 0x0018, 0x0000,
	0x0000, 0x000b, 0x0000,   0x0000, 0x0018, 0x0000,
	0x0000, 0x000b, 0x0000,   0x0000, 0x0018, 0x0000,
	0x0000, 0x000b, 0x0000,   0x0000, 0x0018, 0x0000,
	0xffff
};

static const UINT16 level_data_07[] = {
	0x0480, 0x000b, 0x0300,   0x4501, 0x0001, 0x0001,
	0x0000, 0x0000, 0x0000,   0xf001, 0x0000, 0x0000,
	0x0780, 0x0109, 0x0300,   0x4a01, 0x0004, 0x0004,
	0x0780, 0x0009, 0x0300,   0x4a01, 0x0004, 0x000d,
	0x0780, 0x000c, 0x0500,   0x7b01, 0x000c, 0x0005,
	0x0780, 0x000c, 0x0540,   0x7b01, 0x000c, 0x0005,
	0x0780, 0x010c, 0x0300,   0x4601, 0x0005, 0x0005,
	0x0780, 0x000c, 0x0500,   0x7b01, 0x800b, 0xc00d,
	0x0780, 0x000c, 0x0540,   0x7b01, 0x800b, 0xc00d,
	0x0780, 0x010c, 0x0300,   0x4601, 0x8004, 0xc00d,
	0x0900, 0x0109, 0x0340,   0x4b01, 0x2006, 0x400c,
	0x0780, 0x020c, 0x0300,   0x4801, 0x8007, 0x0008,
	0x0780, 0x020c, 0x0300,   0x4801, 0x4007, 0xc00b,
	0x0980, 0x0109, 0x0300,   0x4b01, 0xc006, 0x8007,
	0x0980, 0x0109, 0x0300,   0x4b01, 0x8007, 0x8008,
	0x0980, 0x0109, 0x0300,   0x4b01, 0xc006, 0x800c,
	0x0000, 0x0000, 0x0000,   0xf001, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000,   0xf001, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000,   0xf001, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000,   0xf001, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000,   0xf001, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000,   0xf001, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000,   0xf001, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000,   0xf001, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000,   0xf001, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000,   0xf001, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000,   0xf001, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000,   0xf001, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000,   0xf001, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000,   0xf001, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000,   0xf001, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000,   0xf001, 0x0000, 0x0000,
	0xffff
};

static const UINT16 level_data_08[] = {
	0xffff
};

static const UINT16 level_data_09[] = {
	0x0000, 0x0000, 0x0000,   0xf001, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000,   0xf001, 0x0000, 0x0000,
	0x0780, 0x0109, 0x0300,   0x4a01, 0x8003, 0x8003,
	0x0780, 0x0009, 0x0300,   0x4a01, 0x0004, 0x800e,
	0x0780, 0x000c, 0x0500,   0x7b01, 0x000c, 0x0005,
	0x0780, 0x000c, 0x0540,   0x7b01, 0x000c, 0x0005,
	0x0780, 0x010c, 0x0300,   0x4601, 0x0005, 0x0005,
	0x0780, 0x000c, 0x0500,   0x7b01, 0x800b, 0xc00d,
	0x0780, 0x000c, 0x0540,   0x7b01, 0x800b, 0xc00d,
	0x0780, 0x010c, 0x0300,   0x4601, 0x8004, 0xc00d,
	0x0900, 0x0109, 0x0340,   0x4b01, 0x2006, 0x400c,
	0x0780, 0x020c, 0x0300,   0x4801, 0x8007, 0x0008,
	0x0780, 0x020c, 0x0300,   0x4801, 0x4007, 0xc00b,
	0x0980, 0x0109, 0x0300,   0x4b01, 0xc006, 0x8007,
	0x0980, 0x0109, 0x0300,   0x4b01, 0x8007, 0x8008,
	0x0980, 0x0109, 0x0300,   0x4b01, 0xc006, 0x800c,
	0x0000, 0x0000, 0x0000,   0xf001, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000,   0xf001, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000,   0xf001, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000,   0xf001, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000,   0xf001, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000,   0xf001, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000,   0xf001, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000,   0xf001, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000,   0xf001, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000,   0xf001, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000,   0xf001, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000,   0xf001, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000,   0xf001, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000,   0xf001, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000,   0xf001, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000,   0xf001, 0x0000, 0x0000,
	0xffff
};

static const UINT16 *const level_data_lookup[] =
{
	level_data_00,
	level_data_01,
	level_data_02,
	level_data_03,
	level_data_04,
	level_data_05,
	level_data_06,
	level_data_07,
	level_data_08,
	level_data_09
};

static void AccessLevelDataCommand()
{
	// Level data command
	if (CurrentCmd == 0xf5)
	{
		int const level = CChipRam[0x1b] % 10;
		UINT16 const *const level_data = level_data_lookup[level];

		// The c-chip data is stored as a series of 3 word sets, delimited by 0xffff
		memset(CChipRam + 0x200, 0, 0x200);
		for (unsigned i = 0; (i < 0x200) && (level_data[i] != 0xffff); i += 3)
		{
			CChipRam[0x200 + i*2 + 0] = level_data[i]>>8;
			CChipRam[0x200 + i*2 + 1] = level_data[i]&0xff;
			CChipRam[0x200 + i*2 + 2] = level_data[i+1]>>8;
			CChipRam[0x200 + i*2 + 3] = level_data[i+1]&0xff;
			CChipRam[0x200 + i*2 + 4] = level_data[i+2]>>8;
			CChipRam[0x200 + i*2 + 5] = level_data[i+2]&0xff;
		}

		// The bootleg cchip writes 0 to these locations - we can probably assume the real one
		// does similar as this is just zeroing out work variables used in the level.
		CChipRam[0x0] = 0;
		CChipRam[0x76] = 0;
		CChipRam[0x75] = 0;
		CChipRam[0x74] = 0;
		CChipRam[0x72] = 0;
		CChipRam[0x71] = 0;
		// CChipRam[0x70] = 0; // The bootleg writes this to disable mini-levels.  The real c-chip does not do this.
		CChipRam[0x66] = 0;
		CChipRam[0x2b] = 0;
		CChipRam[0x30] = 0;
		CChipRam[0x31] = 0;
		CChipRam[0x32] = 0;
		CChipRam[0x27] = 0;
		c588 = 0;
		c589 = 0;
		c58a = 0;
		triggeredLevel1b = 0;
		triggeredLevel13b = 0;
		triggeredLevel2 = 0;
		triggeredLevel2b = 0;
		triggeredLevel2c = 0;
		triggeredLevel3b = 0;
		triggeredLevel4 = 0;
		triggeredLevel5 = 0;
		triggeredLevel7 = 0;
		triggeredLevel8 = 0;
		triggeredLevel9 = 0;

		CChipRam[0x1a] = 0;
		CChipRam[0x7a] = 1; // Signal command complete

		bprintf(0, _T("Signal level command complete\n"));
	}

	CurrentCmd = 0;
}

static void updateDifficulty(INT32 Mode)
{
	// The game is made up of 6 rounds, when you complete the
	// sixth you return to the start but with harder difficulty.
	if (Mode == 0)
	{
		switch (CChipRam[0x15]&3) // Dipswitch B
		{
		case 3:
			CChipRam[0x2c]=0x31;
			CChipRam[0x77]=0x05;
			CChipRam[0x25]=0x0f;
			CChipRam[0x26]=0x0b;
			break;
		case 0:
			CChipRam[0x2c]=0x20;
			CChipRam[0x77]=0x06;
			CChipRam[0x25]=0x07;
			CChipRam[0x26]=0x03;
			break;
		case 1:
			CChipRam[0x2c]=0x31;
			CChipRam[0x77]=0x05;
			CChipRam[0x25]=0x0f;
			CChipRam[0x26]=0x0b;
			break;
		case 2:
			CChipRam[0x2c]=0x3c;
			CChipRam[0x77]=0x04;
			CChipRam[0x25]=0x13;
			CChipRam[0x26]=0x0f;
			break;
		}
	}
	else
	{
		switch (CChipRam[0x15]&3) // Dipswitch B
		{
		case 3:
			CChipRam[0x2c]=0x46;
			CChipRam[0x77]=0x05;
			CChipRam[0x25]=0x11;
			CChipRam[0x26]=0x0e;
			break;
		case 0:
			CChipRam[0x2c]=0x30;
			CChipRam[0x77]=0x06;
			CChipRam[0x25]=0x0b;
			CChipRam[0x26]=0x03;
			break;
		case 1:
			CChipRam[0x2c]=0x3a;
			CChipRam[0x77]=0x05;
			CChipRam[0x25]=0x0f;
			CChipRam[0x26]=0x09;
			break;
		case 2:
			CChipRam[0x2c]=0x4c;
			CChipRam[0x77]=0x04;
			CChipRam[0x25]=0x19;
			CChipRam[0x26]=0x11;
			break;
		};
	}
}

void OpwolfCChipUpdate(UINT8 Input1, UINT8 Input2)
{
	// Update input ports, these are used by both the 68k directly and by the c-chip
	CChipRam[0x4] = Input1;
	CChipRam[0x5] = Input2;

	// Coin slots
	if (CChipRam[0x4]!=CChipLast_04)
	{
		INT32 slot=-1;

		if (CChipRam[0x4]&1) slot=0;
		if (CChipRam[0x4]&2) slot=1;

		if (slot != -1)
		{
			CChipCoins[slot]++;
			if (CChipCoins[slot] >= CChipCoinsForCredit[slot])
			{
				CChipRam[0x53]+=CChipCreditsForCoin[slot];
				CChipRam[0x51]=0x55;
				CChipRam[0x52]=0x55;
				CChipCoins[slot]-=CChipCoinsForCredit[slot];
			}
		}

		if (CChipRam[0x53]>9)
			CChipRam[0x53]=9;
	}
	CChipLast_04=CChipRam[0x4];

	// Service switch
	if (CChipRam[0x5]!=CChipLast_05)
	{
		if ((CChipRam[0x5]&4)==0)
		{
			CChipRam[0x53]++;
			CChipRam[0x51]=0x55;
			CChipRam[0x52]=0x55;
		}
	}
	CChipLast_05=CChipRam[0x5];

	// These variables are cleared every frame during attract mode and the intro.
	if (CChipRam[0x34] < 2)
	{
		updateDifficulty(0);
		CChipRam[0x76]=0;
		CChipRam[0x75]=0;
		CChipRam[0x74]=0;
		CChipRam[0x72]=0;
		CChipRam[0x71]=0;
		CChipRam[0x70]=0;
		CChipRam[0x66]=0;
		CChipRam[0x2b]=0;
		CChipRam[0x30]=0;
		CChipRam[0x31]=0;
		CChipRam[0x32]=0;
		CChipRam[0x27]=0;
		c588=0;
		c589=0;
		c58a=0;
	}

	// The unprotected Operation Wolf (prototype) shows the game sets up a special thread function specific to each level of the game.
	// This includes the end of level check as different levels have different rules.  In the protected version this logic is moved
	// to the c-chip, so we simulate it here.
	if (CChipRam[0x1c] == 0 && CChipRam[0x1d] == 0 && CChipRam[0x1e] == 0 && CChipRam[0x1f] == 0 && CChipRam[0x20] == 0)
	{
		// Special handling for end of level 6
		if (CChipRam[0x1b] == 0x6)
		{
			// Don't signal end of level until final boss is destroyed
			if (CChipRam[0x27] == 0x1)
				CChipRam[0x32] = 1;
		}
		// Level 2 - Boss check - cross-referenced from logic at 0x91CE in OpWolfP
		// When all enemies are destroyed c-chip signals function 4 in the level function table, which
		// starts the 'WARNING' sequences for the boss.
		else if (CChipRam[0x1b] == 0x2)
		{
			if (triggeredLevel2==0 && CChipRam[0x5f]==0)
			{
				CChipRam[0x5f] = 4; // 0xBE at 68K side
				triggeredLevel2=1;
			}

			// When the level 2 boss has been defeated the 68K will write 0xff to $ff0ba.l - this should signal
			// the c-chip to start the end of level routine.  See code at 0xC370 in OpWolf and 0x933e in OpWolfP
			if (triggeredLevel2 && CChipRam[0x5d]!=0)
			{
				// Signal end of level
				CChipRam[0x32] = 1;
				CChipRam[0x5d] = 0; // acknowledge 68K command
			}
		}
		else if (CChipRam[0x1b] == 0x4)
		{
			CChipRam[0x32] = 1;

			// When level 4 (powder magazine) is complete the c-chip triggers an explosion animation.
			if (triggeredLevel4==0 && CChipRam[0x5f]==0)
			{
				CChipRam[0x5f]=10;
				triggeredLevel4=1;
			}
		}
		else
		{
			// Signal end of level
			CChipRam[0x32] = 1;
		}
	}

	// When all men are destroyed (not necessarily vehicles) the enemy look up table changes
	// Reference functions around 0x96A4 in the unprotected prototype.
	// Level 1 has a specific table.
	// Level 3 has an additional flag set
	if (CChipRam[0x1c] == 0 && CChipRam[0x1d] == 0)
	{
		// Compare code at 0x96DC in prototype with 0xC3A2 in protected version
		if (CChipRam[0x1b] == 0x1 && triggeredLevel1b==0 && CChipRam[0x5f]==0) // Don't write unless 68K is ready (0 at 0x5f))
		{
			CChipRam[0x5f]=7;
			triggeredLevel1b=1;
		}

		// Compare code at 0x96BC in prototype with 0xC3B2 in protected version
		if (CChipRam[0x1b] == 0x3 && triggeredLevel3b==0 && CChipRam[0x5f]==0) // Don't write unless 68K is ready (0 at 0x5f))
		{
			CChipRam[0x5f]=8;
			triggeredLevel3b=1;
		}

		// Compare code at 0x96BC in prototype with 0xC3C8 in protected version
		if ((CChipRam[0x1b] != 0x1 && CChipRam[0x1b] != 0x3) && triggeredLevel13b==0 && CChipRam[0x5f]==0) // Don't write unless 68K is ready (0 at 0x5f))
		{
			CChipRam[0x5f]=9;
			triggeredLevel13b=1;
		}
	}

	//-------------------------------------------------------------------------------------------------
	// Level 2.  On level 2 specifically when there are less than 45 men left the enemy lookup table is
	// switched.  This drops down a wave of paratroopers.  When there are less than 25 men left the lookup
	// table is switched again.
	// See code at 0xC37A and 0xc390 in protected version against 0x9648 in prototype.
	if (CChipRam[0x1b] == 0x2)
	{
		// (Note:  it's correct that 25 decimal is represented as 0x25 in hex here).
		int numMen=(CChipRam[0x1d]<<8) + CChipRam[0x1c];
		if (numMen<0x25 && triggeredLevel2b==1 && triggeredLevel2c==0 && CChipRam[0x5f]==0) // Don't write unless 68K is ready (0 at 0x5f))
		{
			CChipRam[0x5f]=6;
			triggeredLevel2c=1;
		}

		// (Note:  it's correct that 45 decimal is represented as 0x45 in hex here).
		if (numMen<0x45 && triggeredLevel2b==0 && CChipRam[0x5f]==0) // Don't write unless 68K is ready (0 at 0x5f))
		{
			CChipRam[0x5f]=5;
			triggeredLevel2b=1;
		}
	}

	//-------------------------------------------------------------------------------------------------
	// Level 5
	if (CChipRam[0x1b] == 0x5)
	{
		// When all men are destroyed (not necessarily vehicles), the c-chip writes 1 to location
		// 0x2f to spawn a thread that scrolls the screen upwards to focus on the helicopter
		// enemies.  The 68K acknowledges this by writing 2 to 0x2f.
		// See code at 0x4ED6 in prototype and 0x687e in original.
		if (CChipRam[0x1c] == 0 && CChipRam[0x1d] == 0 && triggeredLevel5==0)
		{
			CChipRam[0x2f] = 1;
			triggeredLevel5 = 1;
		}
	}

	//-------------------------------------------------------------------------------------------------
	// Level 6
	if (CChipRam[0x1b] == 0x6)
	{
		// Check for triggering final helicopter (end boss)
		if (c58a == 0)
		{
			if ((CChipRam[0x72] & 0x7f) >= 8 && CChipRam[0x74] == 0 && CChipRam[0x1c] == 0 && CChipRam[0x1d] == 0 && CChipRam[0x1f] == 0)
			{
				CChipRam[0x30] = 1;
				CChipRam[0x74] = 1;
				c58a = 1;
			}
		}

		if (CChipRam[0x1a] == 0x90)
			CChipRam[0x74] = 0;

		if (c58a != 0)
		{
			if (c589 == 0 && CChipRam[0x27] == 0 && CChipRam[0x75] == 0 && CChipRam[0x1c] == 0 && CChipRam[0x1d] == 0 && CChipRam[0x1e] == 0 && CChipRam[0x1f] == 0)
			{
				CChipRam[0x31] = 1;
				CChipRam[0x75] = 1;
				c589 = 1;
			}
		}

		if (CChipRam[0x2b] == 0x1)
		{
			CChipRam[0x2b] = 0;

			if (CChipRam[0x30] == 0x1)
			{
				if (CChipRam[0x1a] != 0x90)
					CChipRam[0x1a]--;
			}

			if (CChipRam[0x72] == 0x9)
			{
				if (CChipRam[0x76] != 0x4)
				{
					CChipRam[0x76] = 3;
				}
			}
			else
			{
				// This timer is derived from the bootleg rather than the real board, I'm not 100% sure about it
				c588 |= 0x80;

				CChipRam[0x72] = c588;
				c588++;

				CChipRam[0x1a]--;
				CChipRam[0x1a]--;
				CChipRam[0x1a]--;
			}
		}

		// Update difficulty settings
		if (CChipRam[0x76] == 0)
		{
			CChipRam[0x76] = 1;
			updateDifficulty(1);
		}
	}

	//-------------------------------------------------------------------------------------------------
	// Start of level 7 - should trigger '1' in level thread table (compare 0xC164 in protected to 0x9468 in unprotected)
	if (CChipRam[0x1b] == 0x7 && triggeredLevel7==0 && CChipRam[0x5f]==0) // Don't write unless 68K is ready (0 at 0x5f))
	{
		triggeredLevel7 = 1;
		CChipRam[0x5f] = 1;
	}

	//-------------------------------------------------------------------------------------------------
	// Start of level 8 - should trigger '2' in level thread table (compare 0xC18E in protected to 0x9358 in unprotected)
	// This controls the 'zoom in helicopters' enemy
	if (CChipRam[0x1b] == 0x8 && triggeredLevel8==0 && CChipRam[0x5f]==0) // Don't write unless 68K is ready (0 at 0x5f))
	{
		triggeredLevel8 = 1;
		CChipRam[0x5f] = 2;
	}

	//-------------------------------------------------------------------------------------------------
	// Start of level 9 - should trigger '3' in level thread table (compare 0xC1B0 in protected to 0x9500 in unprotected)
	// This controls the 'zoom in helicopters' enemy
	if (CChipRam[0x1b] == 0x9 && triggeredLevel9==0 && CChipRam[0x5f]==0) // Don't write unless 68K is ready (0 at 0x5f))
	{
		triggeredLevel9 = 1;
		CChipRam[0x5f] = 3;
	}

	if (CChipRam[0xe] == 1)
	{
		CChipRam[0xe] = 0xfd;
		CChipRam[0x61] = 0x04;
	}

	// Access level data command (address 0xf5 goes from 1 -> 0)
	if (CChipRam[0x7a]==0 && CChipLast_7a!=0 && CurrentCmd!=0xf5)
	{
		// Simulate time for command to execute (exact timing unknown, this is close)
		CurrentCmd=0xf5;
		bprintf(PRINT_NORMAL, _T("Accessing Level Data Command\n"));
		AccessLevelDataCommand();
//		timer_set(ATTOTIME_IN_CYCLES(80000,0), NULL, 0, opwolf_timer_callback);
	}
	CChipLast_7a=CChipRam[0x7a];

	// This seems to some kind of periodic counter - results are expected
	// by the 68k when the counter reaches 0xa
	if (CChipRam[0x7f]==0xa)
	{
		CChipRam[0xfe]=0xf7;
		CChipRam[0xff]=0x6e;
	}

	// These are set every frame
	CChipRam[0x64]=0;
	CChipRam[0x66]=0;
}

UINT16 OpwolfCChipStatusRead()
{
	return 0x01;
}

UINT16 OpwolfCChipDataRead(UINT32 Offset)
{
	return CChipRam[(CurrentBank * 0x400) + Offset];
}

void OpwolfCChipStatusWrite()
{
	CChipRam[0x3d] = 1;
	CChipRam[0x7a] = 1;
	updateDifficulty(0);
}

void OpwolfCChipBankWrite(UINT16 Data)
{
	CurrentBank = Data & 7;
}

void OpwolfCChipDataWrite(UINT8 *p68kRom, UINT32 Offset, UINT16 Data )
{
	CChipRam[(CurrentBank * 0x400) + Offset] = Data & 0xff;

	if (CurrentBank == 0) {
		if (Offset == 0x14)
		{
			UINT16* rom = (UINT16*)p68kRom;
			UINT32 CoinTable[2]= { 0, 0};
			UINT8 CoinOffset[2];
			INT32 Slot;

			if ((CChipRegion == OPWOLF_REGION_JAPAN) || (CChipRegion == OPWOLF_REGION_US)) {
				CoinTable[0] = 0x03ffce;
				CoinTable[1] = 0x03ffce;
			}
			if ((CChipRegion == OPWOLF_REGION_WORLD) || (CChipRegion == OPWOLF_REGION_OTHER)) {
				CoinTable[0] = 0x03ffde;
				CoinTable[1] = 0x03ffee;
			}
			
			CoinOffset[0] = 12 - (4 * ((Data & 0x30) >> 4));
			CoinOffset[1] = 12 - (4 * ((Data & 0xc0) >> 6));

			for (Slot = 0; Slot < 2; Slot++) {
				if (CoinTable[Slot]) {
					CChipCoinsForCredit[Slot] = rom[(CoinTable[Slot] + CoinOffset[Slot] + 0) / 2] & 0xff;
					CChipCreditsForCoin[Slot] = rom[(CoinTable[Slot] + CoinOffset[Slot] + 2) / 2] & 0xff;
				}
			}
		}

		// Dip switch B
		if (Offset == 0x15) {
			updateDifficulty(0);
		}
	}
}

void OpwolfCChipReset()
{
	memset(CChipRam, 0, 0x400 * 8);
	
	CChipLast_7a = 0;
	CChipLast_04 = 0xfc;
	CChipLast_05 = 0xff;
	CChipCoins[0] = 0;
	CChipCoins[1] = 0;
	CChipCoinsForCredit[0] = 1;
	CChipCreditsForCoin[0] = 1;
	CChipCoinsForCredit[1] = 1;
	CChipCreditsForCoin[1] = 1;
	CurrentBank = 0;
	CurrentCmd = 0;
	c588 = 0;
	c589 = 0;
	c58a = 0;
	triggeredLevel1b = 0;
	triggeredLevel13b = 0;
	triggeredLevel2 = 0;
	triggeredLevel2b = 0;
	triggeredLevel2c = 0;
	triggeredLevel3b = 0;
	triggeredLevel4 = 0;
	triggeredLevel5 = 0;
	triggeredLevel7 = 0;
	triggeredLevel8 = 0;
	triggeredLevel9 = 0;
}

void OpwolfCChipInit(INT32 Region)
{
	CChipRegion = Region;
	
	CChipRam = (UINT8*)BurnMalloc(0x400 * 8);
	memset(CChipRam, 0, 0x400 * 8);
	
	CChipLast_7a = 0;
	CChipLast_04 = 0xfc;
	CChipLast_05 = 0xff;
	CChipCoins[0] = 0;
	CChipCoins[1] = 0;
	CChipCoinsForCredit[0] = 1;
	CChipCreditsForCoin[0] = 1;
	CChipCoinsForCredit[1] = 1;
	CChipCreditsForCoin[1] = 1;
	
	TaitoIC_OpwolfCChipInUse = 1;
}

void OpwolfCChipExit()
{
	BurnFree(CChipRam);
	
	CChipRegion = 0;
	
	CChipLast_7a = 0;
	CChipLast_04 = 0;
	CChipLast_05 = 0;
	CChipCoins[0] = 0;
	CChipCoins[1] = 0;
	CChipCoinsForCredit[0] = 0;
	CChipCreditsForCoin[0] = 0;
	CChipCoinsForCredit[1] = 0;
	CChipCreditsForCoin[1] = 0;
	CurrentBank = 0;
	CurrentCmd = 0;
	c588 = 0;
	c589 = 0;
	c58a = 0;
	
}

void OpwolfCChipScan(INT32 nAction)
{
	struct BurnArea ba;
	
	if (nAction & ACB_MEMORY_RAM) {
		memset(&ba, 0, sizeof(ba));
		ba.Data	  = CChipRam;
		ba.nLen	  = 0x400 * 8;
		ba.szName = "C-Chip Ram";
		BurnAcb(&ba);
	}
	
	if (nAction & ACB_DRIVER_DATA) {
		SCAN_VAR(CurrentBank);
		SCAN_VAR(CurrentCmd);
		SCAN_VAR(CChipLast_7a);
		SCAN_VAR(CChipLast_04);
		SCAN_VAR(CChipLast_05);
		SCAN_VAR(CChipCoinsForCredit);
		SCAN_VAR(CChipCreditsForCoin);
		SCAN_VAR(CChipCoins);
		SCAN_VAR(c588);
		SCAN_VAR(c589);
		SCAN_VAR(c58a);
		SCAN_VAR(triggeredLevel1b);
		SCAN_VAR(triggeredLevel13b);
		SCAN_VAR(triggeredLevel2);
		SCAN_VAR(triggeredLevel2b);
		SCAN_VAR(triggeredLevel2c);
		SCAN_VAR(triggeredLevel3b);
		SCAN_VAR(triggeredLevel4);
		SCAN_VAR(triggeredLevel5);
		SCAN_VAR(triggeredLevel7);
		SCAN_VAR(triggeredLevel8);
		SCAN_VAR(triggeredLevel9);
	}
}

#undef OPWOLF_REGION_JAPAN
#undef OPWOLF_REGION_US
#undef OPWOLF_REGION_WORLD
#undef OPWOLF_REGION_OTHER
