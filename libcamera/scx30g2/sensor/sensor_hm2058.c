/*
* Copyright (C) 2012 The Android Open Source Project
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*      http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#include <utils/Log.h>
#include "sensor.h"
#include "jpeg_exif_header.h"
#include "sensor_drv_u.h"
#include "cmr_oem.h"

#define HM2058_I2C_ADDR_W		0x24
#define HM2058_I2C_ADDR_R		0x24
#define HM2058_I2C_ACK			0x0
#define SENSOR_WRITE_DELAY		0xffff
#define HM2058_SENSOR_ID			0x2056
#define HM2058_ID_H_VALUE		0x20
#define HM2058_ID_L_VALUE		0x56

static unsigned long HM2058_Power_On(unsigned long power_on);
static unsigned long HM2058_Identify(unsigned long param);
static unsigned long Set_HM2058_Brightness(unsigned long level);
static unsigned long Set_HM2058_Contrast(unsigned long level);
static unsigned long Set_HM2058_Image_Effect(unsigned long effect_type);
static unsigned long Set_HM2058_Ev(unsigned long level);
static unsigned long Set_HM2058_Anti_Flicker(unsigned long mode);
static unsigned long Set_HM2058_Preview_Mode(unsigned long preview_mode);
static unsigned long set_HM2058_work_mode(unsigned long mode);
static unsigned long Set_HM2058_AWB(unsigned long mode);
static unsigned long HM2058_Before_Snapshot(unsigned long param);
static unsigned long HM2058_After_Snapshot(unsigned long param);
static unsigned long Set_HM2058_Video_Mode(unsigned long mode);
static void HM2058_Write_Group_Regs( SENSOR_REG_T* sensor_reg_ptr );
static unsigned long HM2058_flash(unsigned long param);
static unsigned long set_HM2058_ae_enable(unsigned long enable);

typedef enum
{
	FLICKER_50HZ = 0,
	FLICKER_60HZ,
	FLICKER_MAX
}FLICKER_E;

SENSOR_REG_T HM2058_YUV_COMMON[] =
{
	{0x0022, 0x00},
	{0x0004, 0x10},
	{0x0006, 0x00},//bi0 V ;bit 1 H
	{0x000D, 0x11},
	{0x000E, 0x11},
	{0x000F, 0x10},
	{0x0011, 0x02},
	{0x0012, 0x1C},
	{0x0013, 0x01},
	{0x0015, 0x02},
	{0x0016, 0x80},
	{0x0018, 0x00},
	{0x001D, 0x40},
	{0x0020, 0x00},
	{0x0023, 0xCB},
	{0x0025, 0x00},
	{0x0026, 0x87},
	{0x0027, 0x30},
	{0x0040, 0x20},
	{0x0053, 0x0A},
	{0x0044, 0x06},
	{0x0046, 0xD8},
	{0x004A, 0x0A},
	{0x004B, 0x72},
	{0x0075, 0x01},
	{0x002A, 0x27},
	{0x0070, 0x5F},
	{0x0071, 0xFF},
	{0x0072, 0x55},
	{0x0073, 0x50},
	{0x0080, 0xC8},
	{0x0082, 0xE2},
	{0x0083, 0x70},
	{0x0085, 0x19},
	{0x0086, 0x02},
	{0x0087, 0x80},
	{0x0088, 0x6C},
	{0x0089, 0x2E},
	{0x008A, 0x7D},
	{0x008D, 0x20},
	{0x0090, 0x00},
	{0x0091, 0x10},
	{0x0092, 0x11},
	{0x0093, 0x12},
	{0x0094, 0x16},
	{0x0095, 0x08},
	{0x0096, 0x00},
	{0x0097, 0x10},
	{0x0098, 0x11},
	{0x0099, 0x12},
	{0x009A, 0x06},
	{0x009B, 0x34},
	{0x00A0, 0x00},
	{0x00A1, 0x04},
	{0x011F, 0xF7},
	{0x0120, 0x36},
	{0x0121, 0x83},
	{0x0122, 0x7B},
	{0x0123, 0xC2},
	{0x0124, 0xDE},
	{0x0125, 0xFF},
	{0x0126, 0x70},
	{0x0128, 0x1F},
	{0x0132, 0x10},
	{0x0131, 0xBD},
	{0x0140, 0x14},
	{0x0141, 0x0A},
	{0x0142, 0x14},
	{0x0143, 0x0A},
	{0x0144, 0x04},
	{0x0145, 0x00},
	{0x0146, 0x20},
	{0x0147, 0x0A},
	{0x0148, 0x10},
	{0x0149, 0x0C},
	{0x014A, 0x80},
	{0x014B, 0x80},
	{0x014C, 0x2E},
	{0x014D, 0x2E},
	{0x014E, 0x05},
	{0x014F, 0x05},
	{0x0150, 0x0D},
	{0x0155, 0x00},
	{0x0156, 0x10},
	{0x0157, 0x0A},
	{0x0158, 0x0A},
	{0x0159, 0x0A},
	{0x015A, 0x05},
	{0x015B, 0x05},
	{0x015C, 0x05},
	{0x015D, 0x05},
	{0x015E, 0x08},
	{0x015F, 0xFF},
	{0x0160, 0x50},
	{0x0161, 0x20},
	{0x0162, 0x14},
	{0x0163, 0x0A},
	{0x0164, 0x10},
	{0x0165, 0x0A},
	{0x0166, 0x0A},
	{0x018C, 0x24},
	{0x018D, 0x04},
	{0x018E, 0x00},
	{0x018F, 0x11},
	{0x0190, 0x80},
	{0x0191, 0x47},
	{0x0192, 0x48},
	{0x0193, 0x64},
	{0x0194, 0x32},
	{0x0195, 0xc8},
	{0x0196, 0x96},
	{0x0197, 0x64},
	{0x0198, 0x32},
	{0x0199, 0x14},
	{0x019A, 0x20},
	{0x019B, 0x14},
	{0x01B0, 0x55},
	{0x01B1, 0x0C},
	{0x01B2, 0x0A},
	{0x01B3, 0x10},
	{0x01B4, 0x0E},
	{0x01BA, 0x10},
	{0x01BB, 0x04},
	{0x01D8, 0x40},
	{0x01DE, 0x60},
	{0x01E4, 0x10},
	{0x01E5, 0x10},
	{0x01F2, 0x0C},
	{0x01F3, 0x14},
	{0x01F8, 0x04},
	{0x01F9, 0x0C},
	{0x01FE, 0x02},
	{0x01FF, 0x04},
	{0x0220, 0x00},
	{0x0221, 0xB0},
	{0x0222, 0x00},
	{0x0223, 0x80},
	{0x0224, 0x8E},
	{0x0225, 0x00},
	{0x0226, 0x88},
	{0x022A, 0x88},
	{0x022B, 0x00},
	{0x022C, 0x88},
	{0x022D, 0x13},
	{0x022E, 0x0B},
	{0x022F, 0x13},
	{0x0230, 0x0B},
	{0x0233, 0x13},
	{0x0234, 0x0B},
	{0x0235, 0x28},
	{0x0236, 0x03},
	{0x0237, 0x28},
	{0x0238, 0x03},
	{0x023B, 0x28},
	{0x023C, 0x03},
	{0x023D, 0x5C},
	{0x023E, 0x02},
	{0x023F, 0x5C},
	{0x0240, 0x02},
	{0x0243, 0x5C},
	{0x0244, 0x02},
	{0x0251, 0x0E},
	{0x0252, 0x00},
	{0x0280, 0x0A},
	{0x0282, 0x14},
	{0x0284, 0x2A},
	{0x0286, 0x50},
	{0x0288, 0x60},
	{0x028A, 0x6D},
	{0x028C, 0x79},
	{0x028E, 0x82},
	{0x0290, 0x8A},
	{0x0292, 0x91},
	{0x0294, 0x9C},
	{0x0296, 0xA7},
	{0x0298, 0xBA},
	{0x029A, 0xCD},
	{0x029C, 0xE0},
	{0x029E, 0x2D},
	{0x02A0, 0x06},
	{0x02E0, 0x04},
	{0x02C0, 0xB1},
	{0x02C1, 0x01},
	{0x02C2, 0x7D},
	{0x02C3, 0x07},
	{0x02C4, 0xD2},
	{0x02C5, 0x07},
	{0x02C6, 0xC4},
	{0x02C7, 0x07},
	{0x02C8, 0x79},
	{0x02C9, 0x01},
	{0x02CA, 0xC4},
	{0x02CB, 0x07},
	{0x02CC, 0xF7},
	{0x02CD, 0x07},
	{0x02CE, 0x3B},
	{0x02CF, 0x07},
	{0x02D0, 0xCF},
	{0x02D1, 0x01},
	{0x0302, 0x00},
	{0x0303, 0x00},
	{0x0304, 0x00},
	{0x02F0, 0x5E},
	{0x02F1, 0x07},
	{0x02F2, 0xA0},
	{0x02F3, 0x00},
	{0x02F4, 0x02},
	{0x02F5, 0x00},
	{0x02F6, 0xC4},
	{0x02F7, 0x07},
	{0x02F8, 0x11},
	{0x02F9, 0x00},
	{0x02FA, 0x2A},
	{0x02FB, 0x00},
	{0x02FC, 0xA1},
	{0x02FD, 0x07},
	{0x02FE, 0xB8},
	{0x02FF, 0x07},
	{0x0300, 0xA7},
	{0x0301, 0x00},
	{0x0305, 0x00},
	{0x0306, 0x00},
	{0x0307, 0x7A},
	{0x032D, 0x00},
	{0x032E, 0x01},
	{0x032F, 0x00},
	{0x0330, 0x01},
	{0x0331, 0x00},
	{0x0332, 0x01},
	{0x0333, 0x82},
	{0x0334, 0x00},
	{0x0335, 0x84},
	{0x0336, 0x00},
	{0x0337, 0x01},
	{0x0338, 0x00},
	{0x0339, 0x01},
	{0x033A, 0x00},
	{0x033B, 0x01},
	{0x033E, 0x04},
	{0x033F, 0x86},
	{0x0340, 0x30},
	{0x0341, 0x44},
	{0x0342, 0x4A},
	{0x0343, 0x42},
	{0x0344, 0x74},
	{0x0345, 0x4F},
	{0x0346, 0x67},
	{0x0347, 0x5C},
	{0x0348, 0x59},
	{0x0349, 0x67},
	{0x034A, 0x4D},
	{0x034B, 0x6E},
	{0x034C, 0x44},
	{0x0350, 0x80},
	{0x0351, 0x80},
	{0x0352, 0x18},
	{0x0353, 0x18},
	{0x0354, 0x6E},
	{0x0355, 0x4A},
	{0x0356, 0x7A},
	{0x0357, 0xC6},
	{0x0358, 0x06},
	{0x035A, 0x06},
	{0x035B, 0xA0},
	{0x035C, 0x73},
	{0x035D, 0x5A},
	{0x035E, 0xC6},
	{0x035F, 0xA0},
	{0x0360, 0x02},
	{0x0361, 0x18},
	{0x0362, 0x80},
	{0x0363, 0x6C},
	{0x0364, 0x00},
	{0x0365, 0xF0},
	{0x0366, 0x20},
	{0x0367, 0x0C},
	{0x0369, 0x00},
	{0x036A, 0x10},
	{0x036B, 0x10},
	{0x036E, 0x20},
	{0x036F, 0x00},
	{0x0370, 0x10},
	{0x0371, 0x18},
	{0x0372, 0x0C},
	{0x0373, 0x38},
	{0x0374, 0x3A},
	{0x0375, 0x13},
	{0x0376, 0x22},
	{0x0380, 0xFF},
	{0x0381, 0x4A},
	{0x0382, 0x36},
	{0x038A, 0x40},
	{0x038B, 0x08},
	{0x038C, 0xC1},
	{0x038E, 0x40},
	{0x038F, 0x04},
	{0x0390, 0xD8},
	{0x0391, 0x05},
	{0x0392, 0x03},
	{0x0393, 0x80},
	{0x0395, 0x21},
	{0x0398, 0x02},
	{0x0399, 0x74},
	{0x039A, 0x03},
	{0x039B, 0x11},
	{0x039C, 0x03},
	{0x039D, 0xAE},
	{0x039E, 0x04},
	{0x039F, 0xE8},
	{0x03A0, 0x06},
	{0x03A1, 0x22},
	{0x03A2, 0x07},
	{0x03A3, 0x5C},
	{0x03A4, 0x09},
	{0x03A5, 0xD0},
	{0x03A6, 0x0C},
	{0x03A7, 0x0E},
	{0x03A8, 0x10},
	{0x03A9, 0x18},
	{0x03AA, 0x20},
	{0x03AB, 0x28},
	{0x03AC, 0x1E},
	{0x03AD, 0x1A},
	{0x03AE, 0x13},
	{0x03AF, 0x0C},
	{0x03B0, 0x0B},
	{0x03B1, 0x09},
	{0x03B3, 0x10},
	{0x03B4, 0x00},
	{0x03B5, 0x10},
	{0x03B6, 0x00},
	{0x03B7, 0xEA},
	{0x03B8, 0x00},
	{0x03B9, 0x3A},
	{0x03BA, 0x01},
	{0x03BB, 0x9F},
	{0x03BC, 0xCF},
	{0x03BD, 0xE7},
	{0x03BE, 0xF3},
	{0x03BF, 0x01},
	{0x03D0, 0xF8},
	{0x03E0, 0x04},
	{0x03E1, 0x01},
	{0x03E2, 0x04},
	{0x03E4, 0x10},
	{0x03E5, 0x12},
	{0x03E6, 0x00},
	{0x03E8, 0x21},
	{0x03E9, 0x23},
	{0x03EA, 0x01},
	{0x03EC, 0x21},
	{0x03ED, 0x23},
	{0x03EE, 0x01},
	{0x03F0, 0x20},
	{0x03F1, 0x22},
	{0x03F2, 0x00},
	{0x0420, 0x84},
	{0x0421, 0x00},
	{0x0422, 0x00},
	{0x0423, 0x83},
	{0x0430, 0x08},
	{0x0431, 0x28},
	{0x0432, 0x10},
	{0x0433, 0x08},
	{0x0435, 0x0C},
	{0x0450, 0xFF},
	{0x0451, 0xE8},
	{0x0452, 0xC4},
	{0x0453, 0x88},
	{0x0454, 0x00},
	{0x0458, 0x70},
	{0x0459, 0x03},
	{0x045A, 0x00},
	{0x045B, 0x28},
	{0x045C, 0x00},
	{0x045D, 0x68},
	{0x0466, 0x14},
	{0x047A, 0x00},
	{0x047B, 0x00},
	{0x0480, 0x5C},
	{0x0481, 0x06},
	{0x0482, 0x0C},
	{0x04B0, 0x50},
	{0x04B6, 0x30},
	{0x04B9, 0x10},
	{0x04B3, 0x10},
	{0x04B1, 0x8e},
	{0x04B4, 0x20},
	{0x0540, 0x00},
	{0x0541, 0x7B},
	{0x0542, 0x00},
	{0x0543, 0x94},
	{0x0580, 0x01},
	{0x0581, 0x0F},
	{0x0582, 0x04},
	{0x0594, 0x00},
	{0x0595, 0x04},
	{0x05A9, 0x03},
	{0x05AA, 0x40},
	{0x05AB, 0x80},
	{0x05AC, 0x0A},
	{0x05AD, 0x10},
	{0x05AE, 0x0C},
	{0x05AF, 0x0C},
	{0x05B0, 0x03},
	{0x05B1, 0x03},
	{0x05B2, 0x1C},
	{0x05B3, 0x02},
	{0x05B4, 0x00},
	{0x05B5, 0x0C},
	{0x05B8, 0x80},
	{0x05B9, 0x32},
	{0x05BA, 0x00},
	{0x05BB, 0x80},
	{0x05BC, 0x03},
	{0x05BD, 0x00},
	{0x05BF, 0x05},
	{0x05C0, 0x10},
	{0x05C3, 0x00},
	{0x05C4, 0x0C},
	{0x05C5, 0x20},
	{0x05C7, 0x01},
	{0x05C8, 0x14},
	{0x05C9, 0x54},
	{0x05CA, 0x14},
	{0x05CB, 0xE0},
	{0x05CC, 0x20},
	{0x05CD, 0x00},
	{0x05CE, 0x08},
	{0x05CF, 0x60},
	{0x05D0, 0x10},
	{0x05D1, 0x05},
	{0x05D2, 0x03},
	{0x05D4, 0x00},
	{0x05D5, 0x05},
	{0x05D6, 0x05},
	{0x05D7, 0x05},
	{0x05D8, 0x08},
	{0x05DC, 0x0C},
	{0x05D9, 0x00},
	{0x05DB, 0x00},
	{0x05DD, 0x0F},
	{0x05DE, 0x00},
	{0x05DF, 0x0A},
	{0x05E0, 0xA0},
	{0x05E1, 0x00},
	{0x05E2, 0xA0},
	{0x05E3, 0x00},

	{0x05E4, 0x04},
	{0x05E5, 0x00},
	{0x05E6, 0x83},
	{0x05E7, 0x02},
	{0x05E8, 0x06},
	{0x05E9, 0x00},
	{0x05EA, 0xE5},
	{0x05EB, 0x01},

	{0x0660, 0x04},
	{0x0661, 0x16},
	{0x0662, 0x04},
	{0x0663, 0x28},
	{0x0664, 0x04},
	{0x0665, 0x18},
	{0x0666, 0x04},
	{0x0667, 0x21},
	{0x0668, 0x04},
	{0x0669, 0x0C},
	{0x066A, 0x04},
	{0x066B, 0x25},
	{0x066C, 0x00},
	{0x066D, 0x12},
	{0x066E, 0x00},
	{0x066F, 0x80},
	{0x0670, 0x00},
	{0x0671, 0x0A},
	{0x0672, 0x04},
	{0x0673, 0x1D},
	{0x0674, 0x04},
	{0x0675, 0x1D},
	{0x0676, 0x00},
	{0x0677, 0x7E},
	{0x0678, 0x01},
	{0x0679, 0x47},
	{0x067A, 0x00},
	{0x067B, 0x73},
	{0x067C, 0x04},
	{0x067D, 0x14},
	{0x067E, 0x04},
	{0x067F, 0x28},
	{0x0680, 0x00},
	{0x0681, 0x22},
	{0x0682, 0x00},
	{0x0683, 0xA5},
	{0x0684, 0x00},
	{0x0685, 0x1E},
	{0x0686, 0x04},
	{0x0687, 0x1D},
	{0x0688, 0x04},
	{0x0689, 0x19},
	{0x068A, 0x04},
	{0x068B, 0x21},
	{0x068C, 0x04},
	{0x068D, 0x0A},
	{0x068E, 0x04},
	{0x068F, 0x25},
	{0x0690, 0x04},
	{0x0691, 0x15},
	{0x0698, 0x20},
	{0x0699, 0x20},
	{0x069A, 0x01},
	{0x069C, 0x22},
	{0x069D, 0x10},
	{0x069E, 0x10},
	{0x069F, 0x08},
	{0x0000, 0x01},
	{0x0100, 0x01},
	{0x0101, 0x01},
	{0x0000, 0x01},
	{0x0100, 0x01},
	{0x0101, 0x01},
	{0x0005, 0x01},
	{0xFF, 0xFF},
};

SENSOR_REG_T HM2058_YUV_1600X1200[] =
{
	{0x000D,0x00},
	{0x000E,0x00},
	{0x011F ,0x88},
	{0x0125 ,0xDF},
	{0x0126 ,0x70},
	{0x0131 ,0xAC},
	{0x0366 ,0x20},
	{0x0433 ,0x40},
	{0x0435 ,0x50},
	{0x05E4 ,0x0A},
	{0x05E5 ,0x00},
	{0x05E6 ,0x49},
	{0x05E7 ,0x06},
	{0x05E8 ,0x0A},
	{0x05E9 ,0x00},
	{0x05EA ,0xB9},
	{0x05EB ,0x04},
	{0x0000, 0x01},
	{0x0100, 0x01},
	{0x0101, 0x01},
	{0x0000, 0x01},
	{0x0100, 0x01},
	{0x0101, 0x01},
	{0x0000, 0x01},
	{0x0100, 0x01},
	{0x0101, 0x01},
	{0x0000, 0x01},
	{0x0100, 0x01},
	{0x0101, 0x01},
	{0xFF, 0xFF},
};

SENSOR_REG_T HM2058_YUV_1280X960[] =
{
	{0x0006,0x0c},//0x0e

	{0x000D,0x00},
	{0x000E,0x00},
	{0x0027,0x30},

	{0x011F ,0x88},
	{0x0125 ,0xDF},
	{0x0126 ,0x70},
	{0x0131 ,0xAC},
	{0x0366 ,0x20},
	{0x0433 ,0x40},
	{0x0435 ,0x50},

	{0x05E4 ,0x0A},
	{0x05E5 ,0x00},
	{0x05E6 ,0x09},
	{0x05E7 ,0x05},
	{0x05E8 ,0x0A},
	{0x05E9 ,0x00},
	{0x05EA ,0xc9},
	{0x05EB ,0x03},

	{0x0000, 0x01},
	{0x0100, 0x01},
	{0x0101, 0x01},
	{0x0000, 0x01},
	{0x0100, 0x01},
	{0x0101, 0x01},
	{0x0000, 0x01},
	{0x0100, 0x01},
	{0x0101, 0x01},
	{0x0000, 0x01},
	{0x0100, 0x01},
	{0x0101, 0x01},

	{0xFF, 0xFF},
};

SENSOR_REG_T HM2058_YUV_640X480[] =
{
	{0x000D,0x11},
	{0x000E,0x11},
	{0x011F,0x80},
	{0x0125,0xFF},
	{0x0126,0x70},
	{0x0131,0xAD},
	{0x0366,0x08},
	{0x0433,0x10},
	{0x0435,0x14},
	{0x05E0,0xA0},
	{0x05E1,0x00},
	{0x05E2,0xA0},
	{0x05E3,0x00},
	{0x05E4,0x04},
	{0x05E5,0x00},
	{0x05E6,0x83},
	{0x05E7,0x02},
	{0x05E8,0x06},
	{0x05E9,0x00},
	{0x05EA,0xE5},
	{0x05EB,0x01},
	{0x0000,0x01},
	{0x0100,0x01},
	{0x0101,0x01},
	{0xFF, 0xFF},
};

static SENSOR_REG_TAB_INFO_T s_HM2058_resolution_Tab_YUV[] =
{
	{ ADDR_AND_LEN_OF_ARRAY(HM2058_YUV_COMMON), 0, 0, 24, SENSOR_IMAGE_FORMAT_YUV422 },
	// YUV422 PREVIEW 1
	{ ADDR_AND_LEN_OF_ARRAY(HM2058_YUV_640X480), 640, 480, 24, SENSOR_IMAGE_FORMAT_YUV422},
	{ ADDR_AND_LEN_OF_ARRAY(HM2058_YUV_1280X960), 1280, 960, 24, SENSOR_IMAGE_FORMAT_YUV422},
	{ ADDR_AND_LEN_OF_ARRAY(HM2058_YUV_1600X1200),1600, 1200, 24,SENSOR_IMAGE_FORMAT_YUV422},
	{ PNULL, 0, 0, 0 , 0, 0},

	// YUV422 PREVIEW 2
	{ PNULL, 0, 0, 0 , 0, 0},
	{ PNULL, 0, 0, 0 , 0, 0},
	{ PNULL, 0, 0, 0 , 0, 0},
	{ PNULL, 0, 0, 0 , 0, 0},
};

static SENSOR_TRIM_T s_HM2058_Resolution_Trim_Tab[] =
{
	// COMMON INIT
	{0, 0, 0, 0, 0, 0, 0, {0, 0, 0, 0}},

	// YUV422 PREVIEW 1
	{0, 0, 640, 480, 68, 56, 0, {0, 0, 640, 480}},
	{0, 0, 1280, 960, 122, 42, 0, {0, 0, 1280, 960}},
	{0, 0, 1600, 1200, 122, 42, 0, {0, 0, 1600, 1200}},
	{0, 0, 0, 0, 0, 0, 0, {0, 0, 0, 0}},

	// YUV422 PREVIEW 2
	{0, 0, 0, 0, 0, 0, 0, {0, 0, 0, 0}},
	{0, 0, 0, 0, 0, 0, 0, {0, 0, 0, 0}},
	{0, 0, 0, 0, 0, 0, 0, {0, 0, 0, 0}},
	{0, 0, 0, 0, 0, 0, 0, {0, 0, 0, 0}}
};

static unsigned long _HM2058_GetResolutionTrimTab(unsigned long param)
{
	return (unsigned long)s_HM2058_Resolution_Trim_Tab;
}

static SENSOR_IOCTL_FUNC_TAB_T s_HM2058_ioctl_func_tab =
{
	// Internal
	PNULL,
	HM2058_Power_On,
	PNULL,
	HM2058_Identify,

	PNULL,
	PNULL,
	PNULL,
	PNULL,

	// External
	set_HM2058_ae_enable,
	PNULL,
	PNULL,

	Set_HM2058_Brightness,
	Set_HM2058_Contrast,
	PNULL,
	PNULL,
	Set_HM2058_Preview_Mode,
	Set_HM2058_Image_Effect,
	HM2058_Before_Snapshot,
	HM2058_After_Snapshot,
	PNULL,
	PNULL,
	PNULL,
	PNULL,
	PNULL,
	PNULL,
	PNULL,
	PNULL,
	PNULL,
	Set_HM2058_AWB,
	PNULL,
	PNULL,
	Set_HM2058_Ev,
	PNULL,
	PNULL,
	PNULL,

	PNULL,
	PNULL,
	Set_HM2058_Anti_Flicker,
	Set_HM2058_Video_Mode,
	PNULL,
	PNULL,
	PNULL,
	PNULL,
	PNULL,
	PNULL,
};

SENSOR_INFO_T g_HM2058_yuv_info =
{
	HM2058_I2C_ADDR_W,			// salve i2c write address
	HM2058_I2C_ADDR_R,			// salve i2c read address

	SENSOR_I2C_REG_16BIT | SENSOR_I2C_REG_8BIT | SENSOR_I2C_FREQ_400,	// bit0: 0: i2c register value is 8 bit, 1: i2c register value is 16 bit
						// bit2: 0: i2c register addr  is 8 bit, 1: i2c register addr  is 16 bit
						// other bit: reseved
	SENSOR_HW_SIGNAL_PCLK_N|\
	SENSOR_HW_SIGNAL_VSYNC_P|\
	SENSOR_HW_SIGNAL_HSYNC_P,		// bit0: 0:negative; 1:positive -> polarily of pixel clock
						// bit2: 0:negative; 1:positive -> polarily of horizontal synchronization signal
						// bit4: 0:negative; 1:positive -> polarily of vertical synchronization signal
						// other bit: reseved

	// preview mode
	SENSOR_ENVIROMENT_NORMAL|\
	SENSOR_ENVIROMENT_NIGHT|\
	SENSOR_ENVIROMENT_SUNNY,

	// image effect
	SENSOR_IMAGE_EFFECT_NORMAL|\
	SENSOR_IMAGE_EFFECT_BLACKWHITE|\
	SENSOR_IMAGE_EFFECT_RED|\
	SENSOR_IMAGE_EFFECT_GREEN|\
	SENSOR_IMAGE_EFFECT_BLUE|\
	SENSOR_IMAGE_EFFECT_YELLOW|\
	SENSOR_IMAGE_EFFECT_NEGATIVE|\
	SENSOR_IMAGE_EFFECT_CANVAS,

	// while balance mode
	0,

	7,					// bit[0:7]: count of step in brightness, contrast, sharpness, saturation
						// bit[8:31] reseved

	SENSOR_LOW_PULSE_RESET,		// reset pulse level
	10,					// reset pulse width(ms)

	SENSOR_HIGH_LEVEL_PWDN,		//SENSOR_LOW_LEVEL_PWDN,// 1: high level valid; 0: low level valid

	2,					// count of identify code
	{{0x0001, 0x20},				// supply two code to identify sensor
	{0x0002, 0x56}},				// for Example: index = 0-> Device id, index = 1 -> version id

	SENSOR_AVDD_2800MV,			// voltage of avdd

	1600,					// max width of source image
	1200,					// max height of source image
	"HM2058",				// name of sensor

	SENSOR_IMAGE_FORMAT_YUV422,		// define in SENSOR_IMAGE_FORMAT_E enum
						// if set to SENSOR_IMAGE_FORMAT_MAX here, image format depent on SENSOR_REG_TAB_INFO_T
	SENSOR_IMAGE_PATTERN_YUV422_YUYV,	// pattern of input image form sensor

	s_HM2058_resolution_Tab_YUV,		// point to resolution table information structure
	&s_HM2058_ioctl_func_tab,			// point to ioctl function table

	PNULL,					// information and table about Rawrgb sensor
	PNULL,					//&ni99250_ext_info,//PNULL,// extend information about sensor
	SENSOR_AVDD_1800MV,			// iovdd
	SENSOR_AVDD_1800MV,			// dvdd
	1,					// kip frame num before preview
	3,					// skip frame num before capture
	0,					// deci frame num during preview
	0,					// deci frame num during video preview
	0,					// threshold enable
	0,					// threshold mode
	0,					// threshold start postion
	0,					// threshold end postion
	0,					// i2c_dev_handler
	{SENSOR_INTERFACE_TYPE_CCIR601, 8, 16, 1},
	PNULL,
	1,					// skip frame num while change setting
};

static unsigned long HM2058_Power_On(unsigned long power_on)
{
	SENSOR_AVDD_VAL_E		dvdd_val = g_HM2058_yuv_info.dvdd_val;
	SENSOR_AVDD_VAL_E		avdd_val = g_HM2058_yuv_info.avdd_val;
	SENSOR_AVDD_VAL_E		iovdd_val = g_HM2058_yuv_info.iovdd_val;
	BOOLEAN				power_down = g_HM2058_yuv_info.power_down_level;
	BOOLEAN				reset_level = g_HM2058_yuv_info.reset_pulse_level;
	uint32_t				reset_width = g_HM2058_yuv_info.reset_pulse_width;

	if (SENSOR_TRUE == power_on) {
		Sensor_PowerDown(power_down);
		Sensor_SetVoltage(dvdd_val, avdd_val, iovdd_val);
		SENSOR_Sleep(10);
		Sensor_SetMCLK(SENSOR_DEFALUT_MCLK);
		SENSOR_Sleep(5);
		Sensor_PowerDown(!power_down);
		Sensor_Reset(reset_level);
	} else {
		Sensor_PowerDown(power_down);
		Sensor_SetMCLK(SENSOR_DISABLE_MCLK);
		Sensor_SetVoltage(SENSOR_AVDD_CLOSED, SENSOR_AVDD_CLOSED, SENSOR_AVDD_CLOSED);
	}

	SENSOR_PRINT("set_HM2058_Power_On");
	return 0;
}

static unsigned long HM2058_Identify(unsigned long param)
{
	uint8_t id_h_value = 0;
	uint8_t id_l_value = 0;
	uint32_t ret_value = 0xFF;

	id_h_value = Sensor_ReadReg(0x0001);
	SENSOR_PRINT("HM2058_Identify-id_h_value %d", id_h_value);

	id_l_value = Sensor_ReadReg(0x0002);
	SENSOR_PRINT("HM2058_Identify-id_l_value %d", id_l_value);

	if ((HM2058_ID_H_VALUE == id_h_value) && (HM2058_ID_L_VALUE == id_l_value)) {
		ret_value = 0;
		SENSOR_PRINT("It Is HM2058 Sensor !");
	}

	return ret_value;
}

static unsigned long set_HM2058_ae_enable(unsigned long enable)
{
	uint32_t temp_AE_reg = 0;
	temp_AE_reg = Sensor_ReadReg(0x0380);

	if (0x00 == enable) {
		Sensor_WriteReg(0x0380, (temp_AE_reg | 0x01));
	} else if (0x01 == enable) {
		Sensor_WriteReg(0x0380, (temp_AE_reg & (~0x01)));
	}

	SENSOR_PRINT("set_ae_enable: enable = %d", enable);

	Sensor_WriteReg(0x0000,0x01);
	Sensor_WriteReg(0x0100,0x01);
	Sensor_WriteReg(0x0101,0x01);
	return 0;
}

static void HM2058_Write_Group_Regs(SENSOR_REG_T* sensor_reg_ptr)
{
	uint32_t i;

	for (i = 0; (0xFF != sensor_reg_ptr[i].reg_addr) || (0xFF != sensor_reg_ptr[i].reg_value) ; i++)
		Sensor_WriteReg(sensor_reg_ptr[i].reg_addr, sensor_reg_ptr[i].reg_value);
}

SENSOR_REG_T HM2058_work_mode_tab[][3] =
{
	{//normal
		{0x038F, 0x04},
		{0x0390, 0xD8},
		{0xff, 0xff}
	},
	{//night
		{0x038F,0x0a},
		{0x0390,0x00},
		{0xff, 0xff}
	}
};

static unsigned long set_HM2058_work_mode(unsigned long mode)
{
	if (mode > 1)
		return SENSOR_OP_PARAM_ERR;

	switch (mode) {
	case 0:// -3
		Sensor_WriteReg(0x038F, 0x0A);
		Sensor_WriteReg(0x0390,0x00);
		break;
	case 1://-2
		Sensor_WriteReg(0x038F, 0x04);
		Sensor_WriteReg(0x0390,0xD8);
		break;
	default:
		break;
	}

	Sensor_WriteReg(0x0000,0x01);
	Sensor_WriteReg(0x0100,0x01);
	Sensor_WriteReg(0x0101,0x01);

	SENSOR_PRINT("set_work_mode: mode = %d", mode);
	return 0;
}

static unsigned long Set_HM2058_Preview_Mode(unsigned long preview_mode)
{
	SENSOR_PRINT("set_preview_mode: preview_mode = %d", preview_mode);

	switch (preview_mode) {
	case DCAMERA_ENVIRONMENT_NORMAL:
		set_HM2058_work_mode(0);
		break;
	case DCAMERA_ENVIRONMENT_NIGHT:
		set_HM2058_work_mode(1);
		break;
	case DCAMERA_ENVIRONMENT_SUNNY:
		set_HM2058_work_mode(0);
		break;
	default:
		break;
	}

	usleep(20*1000);
	return 0;

}

SENSOR_REG_T HM2058_brightness_tab[][2]=
{
	{
		{0x04C0,0xB0}, //-3
		{0x0100,0x01}
	},
	{
		{0x04C0,0xA0}, //-2
		{0x0100,0x01}
	},
	{
		{0x04C0,0x90}, //-1
		{0x0100,0x01}
	},
	{
		{0x04C0,0x00}, //0
		{0x0100,0x01}
	},
	{
		{0x04C0,0x10}, //+1
		{0x0100,0x01}
	},
	{
		{0x04C0,0x20}, //+2
		{0x0100,0x01}
	},
	{
		{0x04C0,0x30}, //+3
		{0x0100,0x01}
	},

};

static unsigned long Set_HM2058_Brightness(unsigned long level)
{
	if(level > 6)
		return 0;

	switch (level) {
	case 0:// -3
		Sensor_WriteReg(0x04C0,0xB0);
		Sensor_WriteReg(0x0000,0x01);
		Sensor_WriteReg(0x0100,0x01);
		Sensor_WriteReg(0x0101,0x01);
		break;
	case 1://-2
		Sensor_WriteReg(0x04C0,0xA0);
		Sensor_WriteReg(0x0000,0x01);
		Sensor_WriteReg(0x0100,0x01);
		Sensor_WriteReg(0x0101,0x01);
		break;
	case 2:// -1
		Sensor_WriteReg(0x04C0,0x90);
		Sensor_WriteReg(0x0000,0x01);
		Sensor_WriteReg(0x0100,0x01);
		Sensor_WriteReg(0x0101,0x01);
		break;
	case 3:// 0
		Sensor_WriteReg(0x04C0,0x00);
		Sensor_WriteReg(0x0000,0x01);
		Sensor_WriteReg(0x0100,0x01);
		Sensor_WriteReg(0x0101,0x01);
		break;
	case 4:// 1
		Sensor_WriteReg(0x04C0,0x10);
		Sensor_WriteReg(0x0000,0x01);
		Sensor_WriteReg(0x0100,0x01);
		Sensor_WriteReg(0x0101,0x01);
		break;
	case 5://2
		Sensor_WriteReg(0x04C0,0x20);
		Sensor_WriteReg(0x0000,0x01);
		Sensor_WriteReg(0x0100,0x01);
		Sensor_WriteReg(0x0101,0x01);
		break;
	case 6:// 3
		Sensor_WriteReg(0x04C0,0x30);
		Sensor_WriteReg(0x0000,0x01);
		Sensor_WriteReg(0x0100,0x01);
		Sensor_WriteReg(0x0101,0x01);
		break;
	default:
		break;
	}

	SENSOR_PRINT("set_HM2058_brightness: level = %d", level);
	return 0;
}

SENSOR_REG_T HM2058_contrast_tab[][2] =
{
	{
		{0x04B0,0x20},//-3
		{0x0100,0x01}
	},
	{
		{0x04B0,0x30},///-2
		{0x0100,0x01}
	},
	{
		{0x04B0,0x40},///-1
		{0x0100,0x01}
	},
	{
		{0x04B0,0x50},///0
		{0x0100,0x01}// 80
	},
	{
		{0x04B0,0x60},///1
		{0x0100,0x01}
	},
	{
		{0x04B0,0x70},///2
		{0x0100,0x01}
	},
	{
		{0x04B0,0x80},///3
		{0x0100,0x01}
	},
};

static unsigned long Set_HM2058_Contrast(unsigned long level)
{

	if (level > 6)
		return 0;

	switch (level) {
	case 0:// -3
		Sensor_WriteReg(0x04B0,0x20);
		Sensor_WriteReg(0x0000,0x01);
		Sensor_WriteReg(0x0100,0x01);
		Sensor_WriteReg(0x0101,0x01);
		break;
	case 1:	//-2
		Sensor_WriteReg(0x04B0,0x30);
		Sensor_WriteReg(0x0000,0x01);
		Sensor_WriteReg(0x0100,0x01);
		Sensor_WriteReg(0x0101,0x01);
		break;
	case 2:	// -1
		Sensor_WriteReg(0x04B0,0x40);
		Sensor_WriteReg(0x0000,0x01);
		Sensor_WriteReg(0x0100,0x01);
		Sensor_WriteReg(0x0101,0x01);
		break;
	case 3:	// 0
		Sensor_WriteReg(0x04B0,0x50);
		Sensor_WriteReg(0x0000,0x01);
		Sensor_WriteReg(0x0100,0x01);
		Sensor_WriteReg(0x0101,0x01);
		break;
	case 4:	// 1
		Sensor_WriteReg(0x04B0,0x60);
		Sensor_WriteReg(0x0000,0x01);
		Sensor_WriteReg(0x0100,0x01);
		Sensor_WriteReg(0x0101,0x01);
		break;
	case 5://2
		Sensor_WriteReg(0x04B0,0x70);
		Sensor_WriteReg(0x0000,0x01);
		Sensor_WriteReg(0x0100,0x01);
		Sensor_WriteReg(0x0101,0x01);
		break;
	case 6:	// 3
		Sensor_WriteReg(0x04B0,0x80);
		Sensor_WriteReg(0x0000,0x01);
		Sensor_WriteReg(0x0100,0x01);
		Sensor_WriteReg(0x0101,0x01);
		break;
	default:
		break;
	}

	SENSOR_PRINT("set_HM2058_contrast: level = %d", level);
	return 0;
}

SENSOR_REG_T HM2058_image_effect_tab[][5] =
{
	// effect normal
	{
		{0x0488, 0x10},{0x0486, 0x00},{0x0487, 0xFF},{0x0101,0x01},{0xFF, 0xFF}
	},
	//effect BLACKWHITE
	{
		{0x0488, 0x11},{0x0486, 0x80},{0x0487, 0x80},{0x0101,0x01} ,{0xFF, 0xFF}
	},
	// effect RED
	{
		{0x0488, 0x11},{0x0486, 0x40},{0x0487, 0xb0},{0x0101,0x01},{0xFF, 0xFF}
	},

	// effect GREEN
	{
		{0x0488, 0x11},{0x0486, 0x60},{0x0487, 0x60},{0x0101,0x01},{0xFF, 0xFF}
	},

	// effect  BLUE
	{
		{0x0488, 0x11},{0x0486, 0xB0},{0x0487, 0x80},{0x0101,0x01},{0xFF, 0xFF}
	},

	//effect YELLOW
	{
		{0x0488, 0x10},{0x0486, 0x00},{0x0487, 0xFF},{0x0101,0x01},{0xFF, 0xFF}
	},

	// effect NEGATIVE
	{
		{0x0488, 0x10},{0x0486, 0x00},{0x0487, 0xFF},{0x0101,0x01},{0xFF, 0xFF}
	},

	// effect CANVAS ANTIQUE
	{
		{0x0488, 0x10},{0x0486, 0x00},{0x0487, 0xFF},{0x0101,0x01},{0xFF, 0xFF}
	},

};

static unsigned long Set_HM2058_Image_Effect(unsigned long effect_type)
{
	SENSOR_PRINT("set_HM2058_image_effect: effect_type = %d", effect_type);

	switch (effect_type) {
	case 0:	// effect normal
		Sensor_WriteReg(0x0488, 0x10);
		Sensor_WriteReg(0x0486, 0x00);
		Sensor_WriteReg(0x0487, 0xFF);
		Sensor_WriteReg(0x0000,0x01);
		Sensor_WriteReg(0x0100,0x01);
		Sensor_WriteReg(0x0101,0x01);
		break;
	case 1:	//effect BLACKWHITE
		Sensor_WriteReg(0x0488, 0x11);
		Sensor_WriteReg(0x0486, 0x80);
		Sensor_WriteReg(0x0487, 0x80);
		Sensor_WriteReg(0x0000,0x01);
		Sensor_WriteReg(0x0100,0x01);
		Sensor_WriteReg(0x0101,0x01);
		break;
	case 2:	// effect RED
		Sensor_WriteReg(0x0488, 0x11);
		Sensor_WriteReg(0x0486, 0x40);
		Sensor_WriteReg(0x0487, 0x90);
		Sensor_WriteReg(0x0000,0x01);
		Sensor_WriteReg(0x0100,0x01);
		Sensor_WriteReg(0x0101,0x01);
		break;
	case 3:	// effect GREEN
		Sensor_WriteReg(0x0488, 0x11);
		Sensor_WriteReg(0x0486, 0x60);
		Sensor_WriteReg(0x0487, 0x60);
		Sensor_WriteReg(0x0000,0x01);
		Sensor_WriteReg(0x0100,0x01);
		Sensor_WriteReg(0x0101,0x01);
		break;
	case 4:// effect  BLUE
		Sensor_WriteReg(0x0488, 0x11);
		Sensor_WriteReg(0x0486, 0xB0);
		Sensor_WriteReg(0x0487, 0x80);
		Sensor_WriteReg(0x0000,0x01);
		Sensor_WriteReg(0x0100,0x01);
		Sensor_WriteReg(0x0101,0x01);
		break;
	case 5://effect YELLOW
		Sensor_WriteReg(0x0488, 0x10);
		Sensor_WriteReg(0x0486, 0x00);
		Sensor_WriteReg(0x0487, 0xFF);
		Sensor_WriteReg(0x0000,0x01);
		Sensor_WriteReg(0x0100,0x01);
		Sensor_WriteReg(0x0101,0x01);
		break;
	case 6:// effect NEGATIVE
		Sensor_WriteReg(0x0488, 0x12);
		Sensor_WriteReg(0x0486, 0x00);
		Sensor_WriteReg(0x0487, 0x00);
		Sensor_WriteReg(0x0000,0x01);
		Sensor_WriteReg(0x0100,0x01);
		Sensor_WriteReg(0x0101,0x01);
		break;
	case 7:// effect CANVAS ANTIQUE
		Sensor_WriteReg(0x0488, 0x11);
		Sensor_WriteReg(0x0486, 0x00);
		Sensor_WriteReg(0x0487, 0xb0);
		Sensor_WriteReg(0x0000,0x01);
		Sensor_WriteReg(0x0100,0x01);
		Sensor_WriteReg(0x0101,0x01);
		break;
	default:
		break;
	}

	return 0;
}

SENSOR_REG_T HM2058_ev_tab[][3] =
{
	//{{0x038E,0x30},{0x0381,0x38},{0x0382,0x28}},
	//{{0x038E,0x38},{0x0381,0x40},{0x0382,0x30}},
	{{0x038E,0x34},{0x0381,0x40},{0x0382,0x28}},
	{{0x038E,0x40},{0x0381,0x4A},{0x0382,0x36}},
	{{0x038E,0x50},{0x0381,0x58},{0x0382,0x48}},
	{{0x038E,0x58},{0x0381,0x60},{0x0382,0x50}},
	{{0x038E,0x60},{0x0381,0x68},{0x0382,0x58}},
	{{0x038E,0x68},{0x0381,0x70},{0x0382,0x60}},
	{{0x038E,0x70},{0x0381,0x78},{0x0382,0x68}}
};

static unsigned long Set_HM2058_Ev(unsigned long level)
{
	if(level > 6)
		return 0;

	switch (level) {
	case 0:// -3
		Sensor_WriteReg(0x038E,0x30);
		Sensor_WriteReg(0x0381,0x38);
		Sensor_WriteReg(0x0382,0x28);
		Sensor_WriteReg(0x0000,0x01);
		Sensor_WriteReg(0x0100,0x01);
		Sensor_WriteReg(0x0101,0x01);
		break;
	case 1:	//-2
		Sensor_WriteReg(0x038E,0x38);
		Sensor_WriteReg(0x0381,0x40);
		Sensor_WriteReg(0x0382,0x30);
		Sensor_WriteReg(0x0000,0x01);
		Sensor_WriteReg(0x0100,0x01);
		Sensor_WriteReg(0x0101,0x01);
		break;
	case 2:	// -1
		Sensor_WriteReg(0x038E,0x30);
		Sensor_WriteReg(0x0381,0x38);
		Sensor_WriteReg(0x0382,0x28);
		Sensor_WriteReg(0x0000,0x01);
		Sensor_WriteReg(0x0100,0x01);
		Sensor_WriteReg(0x0101,0x01);
		break;
	case 3:	// 0
		Sensor_WriteReg(0x038E,0x40);
		Sensor_WriteReg(0x0381,0x4A);
		Sensor_WriteReg(0x0382,0x36);
		Sensor_WriteReg(0x0000,0x01);
		Sensor_WriteReg(0x0100,0x01);
		Sensor_WriteReg(0x0101,0x01);
		break;
	case 4:// 1
		Sensor_WriteReg(0x038E,0x50);
		Sensor_WriteReg(0x0381,0x58);
		Sensor_WriteReg(0x0382,0x48);
		Sensor_WriteReg(0x0000,0x01);
		Sensor_WriteReg(0x0100,0x01);
		Sensor_WriteReg(0x0101,0x01);
		break;
	case 5://2
		Sensor_WriteReg(0x038E,0x58);
		Sensor_WriteReg(0x0381,0x60);
		Sensor_WriteReg(0x0382,0x50);
		Sensor_WriteReg(0x0000,0x01);
		Sensor_WriteReg(0x0100,0x01);
		Sensor_WriteReg(0x0101,0x01);
		break;
	case 6:// 3
		Sensor_WriteReg(0x038E,0x60);
		Sensor_WriteReg(0x0381,0x68);
		Sensor_WriteReg(0x0382,0x58);
		Sensor_WriteReg(0x0000,0x01);
		Sensor_WriteReg(0x0100,0x01);
		Sensor_WriteReg(0x0101,0x01);
		break;
	default:
		break;
	}

	SENSOR_PRINT("set_HM2058_ev: level = %d", level);
	return 0;
}

static unsigned long Set_HM2058_Anti_Flicker(unsigned long mode)
{
	switch (mode) {
	case FLICKER_50HZ:
		Sensor_WriteReg(0x0120, 0x36);
		Sensor_WriteReg(0x0000,0x01);
		Sensor_WriteReg(0x0100,0x01);
		Sensor_WriteReg(0x0101,0x01);
		break;
	case FLICKER_60HZ:
		Sensor_WriteReg(0x0120, 0x37);
		Sensor_WriteReg(0x0000,0x01);
		Sensor_WriteReg(0x0100,0x01);
		Sensor_WriteReg(0x0101,0x01);
		break;
	default:
		return 0;
	}

	usleep(200*1000);

	return 0;
}

SENSOR_REG_T HM2058_awb_tab[][9] =
{
	//AUTO
	{
		{0x0380, 0xff},
		{0xff, 0xff},
		{0xff, 0xff},
		{0xff, 0xff},
		{0xff, 0xff},
		{0xff, 0xff},
		{0xff, 0xff},
		{0xff, 0xff},
		{0xff, 0xff}
	},
	//OFFICE:
	{
		{0x0380, 0xFD},
		{0x032D, 0x80},
		{0x032E, 0x01},
		{0x032F, 0x00},
		{0x0330, 0x01},
		{0x0331, 0x00},
		{0x0332, 0x01},
		{0x0101,0x01},
		{0xff, 0xff}
	},
	//U30
	{
		{0x0380, 0xFD},
		{0x032D, 0x10},
		{0x032E, 0x01},
		{0x032F, 0x00},
		{0x0330, 0x01},
		{0x0331, 0xA0},
		{0x0332, 0x01},
		{0x0101,0x01},
		{0xff, 0xff}
	},
	//CWF
	{
		{0x0380, 0xFD},
		{0x032D, 0x50},
		{0x032E, 0x01},
		{0x032F, 0x00},
		{0x0330, 0x01},
		{0x0331, 0x30},
		{0x0332, 0x01},
		{0x0101,0x01},
		{0xff, 0xff}
	},
	//HOME
	{
		{0x0380, 0xFD},
		{0x032D, 0x60},
		{0x032E, 0x01},
		{0x032F, 0x00},
		{0x0330, 0x01},
		{0x0331, 0x20},
		{0x0332, 0x01},
		{0x0101,0x01},
		{0xff, 0xff}
	},
	//SUN:
	{
		{0x0380, 0xFD},
		{0x032D, 0x60},
		{0x032E, 0x01},
		{0x032F, 0x00},
		{0x0330, 0x01},
		{0x0331, 0x20},
		{0x0332, 0x01},
		{0x0101,0x01},
		{0xff, 0xff}
	},
	//CLOUD:
	{
		{0x0380, 0xFD},
		{0x032D, 0x70},
		{0x032E, 0x01},
		{0x032F, 0x00},
		{0x0330, 0x01},
		{0x0331, 0x08},
		{0x0332, 0x01},
		{0x0101,0x01},
		{0xff, 0xff}
	}
};

static unsigned long Set_HM2058_AWB(unsigned long mode)
{
	if (mode > 6)
		return 0;

	switch (mode) {
	case 0:	// -3
		Sensor_WriteReg(0x0380, 0xff);
		Sensor_WriteReg(0x0000,0x01);
		Sensor_WriteReg(0x0100,0x01);
		Sensor_WriteReg(0x0101,0x01);
		break;
	case 1:	//-2
		Sensor_WriteReg(0x0380, 0xFD);
		Sensor_WriteReg(0x032D, 0x80);
		Sensor_WriteReg(0x032E, 0x01);
		Sensor_WriteReg(0x032F, 0x00);
		Sensor_WriteReg(0x0330, 0x01);
		Sensor_WriteReg(0x0331, 0x00);
		Sensor_WriteReg(0x0332, 0x01);
		Sensor_WriteReg(0x0000,0x01);
		Sensor_WriteReg(0x0100,0x01);
		Sensor_WriteReg(0x0101,0x01);
		break;
	case 2:	// -1
		Sensor_WriteReg(0x0380, 0xFD);
		Sensor_WriteReg(0x032D, 0x10);
		Sensor_WriteReg(0x032E, 0x01);
		Sensor_WriteReg(0x032F, 0x00);
		Sensor_WriteReg(0x0330, 0x01);
		Sensor_WriteReg(0x0331, 0xA0);
		Sensor_WriteReg(0x0332, 0x01);
		Sensor_WriteReg(0x0000,0x01);
		Sensor_WriteReg(0x0100,0x01);
		Sensor_WriteReg(0x0101,0x01);
		break;
	case 3:	// 0
		Sensor_WriteReg(0x0380, 0xFD);
		Sensor_WriteReg(0x032D, 0x50);
		Sensor_WriteReg(0x032E, 0x01);
		Sensor_WriteReg(0x032F, 0x00);
		Sensor_WriteReg(0x0330, 0x01);
		Sensor_WriteReg(0x0331, 0x30);
		Sensor_WriteReg(0x0332, 0x01);
		Sensor_WriteReg(0x0000,0x01);
		Sensor_WriteReg(0x0100,0x01);
		Sensor_WriteReg(0x0101,0x01);
		break;
	case 4:	// 1
		Sensor_WriteReg(0x0380, 0xFD);
		Sensor_WriteReg(0x032D, 0x60);
		Sensor_WriteReg(0x032E, 0x01);
		Sensor_WriteReg(0x032F, 0x00);
		Sensor_WriteReg(0x0330, 0x01);
		Sensor_WriteReg(0x0331, 0x20);
		Sensor_WriteReg(0x0332, 0x01);
		Sensor_WriteReg(0x0000,0x01);
		Sensor_WriteReg(0x0100,0x01);
		Sensor_WriteReg(0x0101,0x01);
		break;
	case 5://2
		Sensor_WriteReg(0x0380, 0xFD);
		Sensor_WriteReg(0x032D, 0x60);
		Sensor_WriteReg(0x032E, 0x01);
		Sensor_WriteReg(0x032F, 0x00);
		Sensor_WriteReg(0x0330, 0x01);
		Sensor_WriteReg(0x0331, 0x20);
		Sensor_WriteReg(0x0332, 0x01);
		Sensor_WriteReg(0x0000,0x01);
		Sensor_WriteReg(0x0100,0x01);
		Sensor_WriteReg(0x0101,0x01);
		break;
	case 6:// 3
		Sensor_WriteReg(0x0380, 0xFD);
		Sensor_WriteReg(0x032D, 0x70);
		Sensor_WriteReg(0x032E, 0x01);
		Sensor_WriteReg(0x032F, 0x00);
		Sensor_WriteReg(0x0330, 0x01);
		Sensor_WriteReg(0x0331, 0x08);
		Sensor_WriteReg(0x0332, 0x01);
		Sensor_WriteReg(0x0000,0x01);
		Sensor_WriteReg(0x0100,0x01);
		Sensor_WriteReg(0x0101,0x01);
		break;
	default:
		break;
	}

	SENSOR_PRINT("set_HM2058_awb_mode: mode = %d", mode);
	return 0;
}

static unsigned long HM2058_Before_Snapshot(unsigned long param)
{
	uint32_t cap_mode = (param >> 16);

	CMR_LOGI("SENSOR_HM2058: Before Snapshot 0x%lx", param);

	param = param&0xffff;

	Sensor_WriteReg(0x0300 , 0xc1);
	Sensor_SetMode((uint32_t)param);
	Sensor_SetMode_WaitDone();
	return 0;
}

static unsigned long HM2058_After_Snapshot(unsigned long param)
{
	Sensor_SetMode((uint32_t)param);
	return 0;
}

SENSOR_REG_T HM2058_video_mode_nor_tab[][4] =
{
	// normal mode
	{{0x038F,0x04},{0x0390,0xD8},{0x0000,0x01},{0x1000,0x01}},
	//video mode
	{{0x038F,0x03},{0x0390,0x00},{0x0000,0x01},{0x1000,0x01}},
	// UPCC  mod
	{{0x038F,0x04},{0x0390,0xD8},{0x0000,0x01},{0x1000,0x01}}
};

static unsigned long Set_HM2058_Video_Mode(unsigned long mode)
{

	if(mode == 0) {
		Sensor_WriteReg(0x038F, 0x03);  //20130204 0a
		Sensor_WriteReg(0x0390, 0x00);
	} else if(mode ==1) {
		Sensor_WriteReg(0x038F, 0x0a);  //20120204 03
		Sensor_WriteReg(0x0390, 0x00);
	}
	SENSOR_PRINT("set_HM2058_video_mode=%d",mode);

	return 0;
}