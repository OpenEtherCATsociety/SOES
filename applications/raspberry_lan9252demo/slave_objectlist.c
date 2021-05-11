#include "esc_coe.h"
#include "utypes.h"
#include <stddef.h>

#ifdef BYTE_NUM // for BYTE_NUM we accept only 16  32  64  128 --
  #if ((BYTE_NUM !=16) && (BYTE_NUM !=32) && (BYTE_NUM !=64)  && (BYTE_NUM !=128))
    #error "BYTE_NUM must be 16, 32, 64 or 128 !!! define it correctly in file utypes.h"
  #endif 
#else
  #define BYTE_NUM 32
#endif

#ifndef HW_REV
#define HW_REV "1.0"
#endif

#ifndef SW_REV
#define SW_REV "1.0"
#endif

static const char acName1000[] = "Device Type";
static const char acName1008[] = "Device Name";
static const char acName1009[] = "Hardware Version";
static const char acName100A[] = "Software Version";
static const char acName1018[] = "Identity Object";
static const char acName1018_00[] = "Max SubIndex";
static const char acName1018_01[] = "Vendor ID";
static const char acName1018_02[] = "Product Code";
static const char acName1018_03[] = "Revision Number";
static const char acName1018_04[] = "Serial Number";
static const char acName1600[] = "Outputs";
static const char acName1600_00[] = "Max SubIndex";
static const char acName1600_01[] = "Byte0";
static const char acName1600_02[] = "Byte1";
static const char acName1600_03[] = "Byte2";
static const char acName1600_04[] = "Byte3";
static const char acName1600_05[] = "Byte4";
static const char acName1600_06[] = "Byte5";
static const char acName1600_07[] = "Byte6";
static const char acName1600_08[] = "Byte7";
static const char acName1600_09[] = "Byte8";
static const char acName1600_10[] = "Byte9";
static const char acName1600_11[] = "Byte10";
static const char acName1600_12[] = "Byte11";
static const char acName1600_13[] = "Byte12";
static const char acName1600_14[] = "Byte13";
static const char acName1600_15[] = "Byte14";
static const char acName1600_16[] = "Byte15";
#if (BYTE_NUM > 16)
static const char acName1600_17[] = "Byte16";
static const char acName1600_18[] = "Byte17";
static const char acName1600_19[] = "Byte18";
static const char acName1600_20[] = "Byte19";
static const char acName1600_21[] = "Byte20";
static const char acName1600_22[] = "Byte21";
static const char acName1600_23[] = "Byte22";
static const char acName1600_24[] = "Byte23";
static const char acName1600_25[] = "Byte24";
static const char acName1600_26[] = "Byte25";
static const char acName1600_27[] = "Byte26";
static const char acName1600_28[] = "Byte27";
static const char acName1600_29[] = "Byte28";
static const char acName1600_30[] = "Byte29";
static const char acName1600_31[] = "Byte30";
static const char acName1600_32[] = "Byte31";
#endif
#if (BYTE_NUM > 32)
static const char acName1600_33[] = "Byte32";
static const char acName1600_34[] = "Byte33";
static const char acName1600_35[] = "Byte34";
static const char acName1600_36[] = "Byte35";
static const char acName1600_37[] = "Byte36";
static const char acName1600_38[] = "Byte37";
static const char acName1600_39[] = "Byte38";
static const char acName1600_40[] = "Byte39";
static const char acName1600_41[] = "Byte40";
static const char acName1600_42[] = "Byte41";
static const char acName1600_43[] = "Byte42";
static const char acName1600_44[] = "Byte43";
static const char acName1600_45[] = "Byte44";
static const char acName1600_46[] = "Byte45";
static const char acName1600_47[] = "Byte46";
static const char acName1600_48[] = "Byte47";
static const char acName1600_49[] = "Byte48";
static const char acName1600_50[] = "Byte49";
static const char acName1600_51[] = "Byte50";
static const char acName1600_52[] = "Byte51";
static const char acName1600_53[] = "Byte52";
static const char acName1600_54[] = "Byte53";
static const char acName1600_55[] = "Byte54";
static const char acName1600_56[] = "Byte55";
static const char acName1600_57[] = "Byte56";
static const char acName1600_58[] = "Byte57";
static const char acName1600_59[] = "Byte58";
static const char acName1600_60[] = "Byte59";
static const char acName1600_61[] = "Byte60";
static const char acName1600_62[] = "Byte61";
static const char acName1600_63[] = "Byte62";
static const char acName1600_64[] = "Byte63";
#endif
#if (BYTE_NUM > 64)
static const char acName1600_65[] = "Byte64";
static const char acName1600_66[] = "Byte65";
static const char acName1600_67[] = "Byte66";
static const char acName1600_68[] = "Byte67";
static const char acName1600_69[] = "Byte68";
static const char acName1600_70[] = "Byte69";
static const char acName1600_71[] = "Byte70";
static const char acName1600_72[] = "Byte71";
static const char acName1600_73[] = "Byte72";
static const char acName1600_74[] = "Byte73";
static const char acName1600_75[] = "Byte74";
static const char acName1600_76[] = "Byte75";
static const char acName1600_77[] = "Byte76";
static const char acName1600_78[] = "Byte77";
static const char acName1600_79[] = "Byte78";
static const char acName1600_80[] = "Byte79";
static const char acName1600_81[] = "Byte80";
static const char acName1600_82[] = "Byte81";
static const char acName1600_83[] = "Byte82";
static const char acName1600_84[] = "Byte83";
static const char acName1600_85[] = "Byte84";
static const char acName1600_86[] = "Byte85";
static const char acName1600_87[] = "Byte86";
static const char acName1600_88[] = "Byte87";
static const char acName1600_89[] = "Byte88";
static const char acName1600_90[] = "Byte89";
static const char acName1600_91[] = "Byte90";
static const char acName1600_92[] = "Byte91";
static const char acName1600_93[] = "Byte92";
static const char acName1600_94[] = "Byte93";
static const char acName1600_95[] = "Byte94";
static const char acName1600_96[] = "Byte95";
static const char acName1600_97[] = "Byte96";
static const char acName1600_98[] = "Byte97";
static const char acName1600_99[] = "Byte98";
static const char acName1600_100[] = "Byte99";
static const char acName1600_101[] = "Byte100";
static const char acName1600_102[] = "Byte101";
static const char acName1600_103[] = "Byte102";
static const char acName1600_104[] = "Byte103";
static const char acName1600_105[] = "Byte104";
static const char acName1600_106[] = "Byte105";
static const char acName1600_107[] = "Byte106";
static const char acName1600_108[] = "Byte107";
static const char acName1600_109[] = "Byte108";
static const char acName1600_110[] = "Byte109";
static const char acName1600_111[] = "Byte110";
static const char acName1600_112[] = "Byte111";
static const char acName1600_113[] = "Byte112";
static const char acName1600_114[] = "Byte113";
static const char acName1600_115[] = "Byte114";
static const char acName1600_116[] = "Byte115";
static const char acName1600_117[] = "Byte116";
static const char acName1600_118[] = "Byte117";
static const char acName1600_119[] = "Byte118";
static const char acName1600_120[] = "Byte119";
static const char acName1600_121[] = "Byte120";
static const char acName1600_122[] = "Byte121";
static const char acName1600_123[] = "Byte122";
static const char acName1600_124[] = "Byte123";
static const char acName1600_125[] = "Byte124";
static const char acName1600_126[] = "Byte125";
static const char acName1600_127[] = "Byte126";
static const char acName1600_128[] = "Byte127";
#endif
static const char acName1A00[] = "Inputs";
static const char acName1A00_00[] = "Max SubIndex";
static const char acName1A00_01[] = "Byte0";
static const char acName1A00_02[] = "Byte1";
static const char acName1A00_03[] = "Byte2";
static const char acName1A00_04[] = "Byte3";
static const char acName1A00_05[] = "Byte4";
static const char acName1A00_06[] = "Byte5";
static const char acName1A00_07[] = "Byte6";
static const char acName1A00_08[] = "Byte7";
static const char acName1A00_09[] = "Byte8";
static const char acName1A00_10[] = "Byte9";
static const char acName1A00_11[] = "Byte10";
static const char acName1A00_12[] = "Byte11";
static const char acName1A00_13[] = "Byte12";
static const char acName1A00_14[] = "Byte13";
static const char acName1A00_15[] = "Byte14";
static const char acName1A00_16[] = "Byte15";
#if (BYTE_NUM > 16)
static const char acName1A00_17[] = "Byte16";
static const char acName1A00_18[] = "Byte17";
static const char acName1A00_19[] = "Byte18";
static const char acName1A00_20[] = "Byte19";
static const char acName1A00_21[] = "Byte20";
static const char acName1A00_22[] = "Byte21";
static const char acName1A00_23[] = "Byte22";
static const char acName1A00_24[] = "Byte23";
static const char acName1A00_25[] = "Byte24";
static const char acName1A00_26[] = "Byte25";
static const char acName1A00_27[] = "Byte26";
static const char acName1A00_28[] = "Byte27";
static const char acName1A00_29[] = "Byte28";
static const char acName1A00_30[] = "Byte29";
static const char acName1A00_31[] = "Byte30";
static const char acName1A00_32[] = "Byte31";
#endif
#if (BYTE_NUM > 32)
static const char acName1A00_33[] = "Byte32";
static const char acName1A00_34[] = "Byte33";
static const char acName1A00_35[] = "Byte34";
static const char acName1A00_36[] = "Byte35";
static const char acName1A00_37[] = "Byte36";
static const char acName1A00_38[] = "Byte37";
static const char acName1A00_39[] = "Byte38";
static const char acName1A00_40[] = "Byte39";
static const char acName1A00_41[] = "Byte40";
static const char acName1A00_42[] = "Byte41";
static const char acName1A00_43[] = "Byte42";
static const char acName1A00_44[] = "Byte43";
static const char acName1A00_45[] = "Byte44";
static const char acName1A00_46[] = "Byte45";
static const char acName1A00_47[] = "Byte46";
static const char acName1A00_48[] = "Byte47";
static const char acName1A00_49[] = "Byte48";
static const char acName1A00_50[] = "Byte49";
static const char acName1A00_51[] = "Byte50";
static const char acName1A00_52[] = "Byte51";
static const char acName1A00_53[] = "Byte52";
static const char acName1A00_54[] = "Byte53";
static const char acName1A00_55[] = "Byte54";
static const char acName1A00_56[] = "Byte55";
static const char acName1A00_57[] = "Byte56";
static const char acName1A00_58[] = "Byte57";
static const char acName1A00_59[] = "Byte58";
static const char acName1A00_60[] = "Byte59";
static const char acName1A00_61[] = "Byte60";
static const char acName1A00_62[] = "Byte61";
static const char acName1A00_63[] = "Byte62";
static const char acName1A00_64[] = "Byte63";
#endif
#if (BYTE_NUM > 64)
static const char acName1A00_65[] = "Byte64";
static const char acName1A00_66[] = "Byte65";
static const char acName1A00_67[] = "Byte66";
static const char acName1A00_68[] = "Byte67";
static const char acName1A00_69[] = "Byte68";
static const char acName1A00_70[] = "Byte69";
static const char acName1A00_71[] = "Byte70";
static const char acName1A00_72[] = "Byte71";
static const char acName1A00_73[] = "Byte72";
static const char acName1A00_74[] = "Byte73";
static const char acName1A00_75[] = "Byte74";
static const char acName1A00_76[] = "Byte75";
static const char acName1A00_77[] = "Byte76";
static const char acName1A00_78[] = "Byte77";
static const char acName1A00_79[] = "Byte78";
static const char acName1A00_80[] = "Byte79";
static const char acName1A00_81[] = "Byte80";
static const char acName1A00_82[] = "Byte81";
static const char acName1A00_83[] = "Byte82";
static const char acName1A00_84[] = "Byte83";
static const char acName1A00_85[] = "Byte84";
static const char acName1A00_86[] = "Byte85";
static const char acName1A00_87[] = "Byte86";
static const char acName1A00_88[] = "Byte87";
static const char acName1A00_89[] = "Byte88";
static const char acName1A00_90[] = "Byte89";
static const char acName1A00_91[] = "Byte90";
static const char acName1A00_92[] = "Byte91";
static const char acName1A00_93[] = "Byte92";
static const char acName1A00_94[] = "Byte93";
static const char acName1A00_95[] = "Byte94";
static const char acName1A00_96[] = "Byte95";
static const char acName1A00_97[] = "Byte96";
static const char acName1A00_98[] = "Byte97";
static const char acName1A00_99[] = "Byte98";
static const char acName1A00_100[] = "Byte99";
static const char acName1A00_101[] = "Byte100";
static const char acName1A00_102[] = "Byte101";
static const char acName1A00_103[] = "Byte102";
static const char acName1A00_104[] = "Byte103";
static const char acName1A00_105[] = "Byte104";
static const char acName1A00_106[] = "Byte105";
static const char acName1A00_107[] = "Byte106";
static const char acName1A00_108[] = "Byte107";
static const char acName1A00_109[] = "Byte108";
static const char acName1A00_110[] = "Byte109";
static const char acName1A00_111[] = "Byte110";
static const char acName1A00_112[] = "Byte111";
static const char acName1A00_113[] = "Byte112";
static const char acName1A00_114[] = "Byte113";
static const char acName1A00_115[] = "Byte114";
static const char acName1A00_116[] = "Byte115";
static const char acName1A00_117[] = "Byte116";
static const char acName1A00_118[] = "Byte117";
static const char acName1A00_119[] = "Byte118";
static const char acName1A00_120[] = "Byte119";
static const char acName1A00_121[] = "Byte120";
static const char acName1A00_122[] = "Byte121";
static const char acName1A00_123[] = "Byte122";
static const char acName1A00_124[] = "Byte123";
static const char acName1A00_125[] = "Byte124";
static const char acName1A00_126[] = "Byte125";
static const char acName1A00_127[] = "Byte126";
static const char acName1A00_128[] = "Byte127";
#endif
static const char acName1C00[] = "Sync Manager Communication type";
static const char acName1C00_00[] = "Max SubIndex";
static const char acName1C00_01[] = "Communications type SM0";
static const char acName1C00_02[] = "Communications type SM1";
static const char acName1C00_03[] = "Communications type SM2";
static const char acName1C00_04[] = "Communications type SM3";
static const char acName1C10[] = "Sync Manager 0 PDO Assignment";
static const char acName1C10_00[] = "Max SubIndex";
static const char acName1C10_01[] = "PDO Mapping";
static const char acName1C11[] = "Sync Manager 1 PDO Assignment";
static const char acName1C11_00[] = "Max SubIndex";
static const char acName1C11_01[] = "PDO Mapping";
static const char acName6000[] = "Inputs";
static const char acName6000_00[] = "Max SubIndex";
static const char acName7000[] = "Outputs";
static const char acName7000_00[] = "Max SubIndex";




const _objd SDO1000[] =
{
  {0x0, DTYPE_UNSIGNED32, 32, ATYPE_RO, acName1000, 0x00000000, NULL},
};
const _objd SDO1008[] =
{
  {0x0, DTYPE_VISIBLE_STRING, 88, ATYPE_RO, acName1008, 0, DEVICE_NAME},
};
const _objd SDO1009[] =
{
  {0x0, DTYPE_VISIBLE_STRING, 0, ATYPE_RO, acName1009, 0, HW_REV},
};
const _objd SDO100A[] =
{
  {0x0, DTYPE_VISIBLE_STRING, 0, ATYPE_RO, acName100A, 0, SW_REV},
};
const _objd SDO1018[] =
{
  {0x00, DTYPE_UNSIGNED8,   8, ATYPE_RO, acName1018_00, 4, NULL},
#ifdef DEVICE_EASYCAT
  {0x01, DTYPE_UNSIGNED32, 32, ATYPE_RO, acName1018_01, 0x0000079A, NULL},
  {0x02, DTYPE_UNSIGNED32, 32, ATYPE_RO, acName1018_02, 0x00DEFEDE, NULL},
  {0x03, DTYPE_UNSIGNED32, 32, ATYPE_RO, acName1018_03, 0x00005A01, NULL},
  {0x04, DTYPE_UNSIGNED32, 32, ATYPE_RO, acName1018_04, 0x00000000, NULL},
#else
#ifdef DEVICE_ETHERC
  {0x01, DTYPE_UNSIGNED32, 32, ATYPE_RO, acName1018_01, 0x00534745, NULL},
  {0x02, DTYPE_UNSIGNED32, 32, ATYPE_RO, acName1018_02, 0x00001010, NULL},
  {0x03, DTYPE_UNSIGNED32, 32, ATYPE_RO, acName1018_03, 0x00000008, NULL},
  {0x04, DTYPE_UNSIGNED32, 32, ATYPE_RO, acName1018_04, 0x00000000, NULL},
#else
  {0x01, DTYPE_UNSIGNED32, 32, ATYPE_RO, acName1018_01, 0x00000000, NULL},
  {0x02, DTYPE_UNSIGNED32, 32, ATYPE_RO, acName1018_02, 0x00000000, NULL},
  {0x03, DTYPE_UNSIGNED32, 32, ATYPE_RO, acName1018_03, 0x00000000, NULL},
  {0x04, DTYPE_UNSIGNED32, 32, ATYPE_RO, acName1018_04, 0x00000000, NULL},
#endif
#endif
};
const _objd SDO1600[] =
{
  {0x00, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1600_00, BYTE_NUM, NULL},
  {0x01, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1600_01, 0x70000108, NULL},
  {0x02, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1600_02, 0x70000208, NULL},
  {0x03, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1600_03, 0x70000308, NULL},
  {0x04, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1600_04, 0x70000408, NULL},
  {0x05, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1600_05, 0x70000508, NULL},
  {0x06, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1600_06, 0x70000608, NULL},
  {0x07, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1600_07, 0x70000708, NULL},
  {0x08, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1600_08, 0x70000808, NULL},
  {0x09, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1600_09, 0x70000908, NULL},
  {0x0A, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1600_10, 0x70000A08, NULL},
  {0x0B, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1600_11, 0x70000B08, NULL},
  {0x0C, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1600_12, 0x70000C08, NULL},
  {0x0D, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1600_13, 0x70000D08, NULL},
  {0x0E, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1600_14, 0x70000E08, NULL},
  {0x0F, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1600_15, 0x70000D08, NULL},
  {0x10, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1600_16, 0x70001008, NULL},
#if (BYTE_NUM > 16)
  {0x11, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1600_17, 0x70001108, NULL},
  {0x12, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1600_18, 0x70001208, NULL},
  {0x13, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1600_19, 0x70001308, NULL},
  {0x14, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1600_20, 0x70001408, NULL},
  {0x15, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1600_21, 0x70001508, NULL},
  {0x16, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1600_22, 0x70001608, NULL},
  {0x17, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1600_23, 0x70001708, NULL},
  {0x18, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1600_24, 0x70001808, NULL},
  {0x19, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1600_25, 0x70001908, NULL},
  {0x1A, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1600_26, 0x70001A08, NULL},
  {0x1B, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1600_27, 0x70001B08, NULL},
  {0x1C, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1600_28, 0x70001C08, NULL},
  {0x1D, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1600_29, 0x70001D08, NULL},
  {0x1E, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1600_30, 0x70001E08, NULL},
  {0x1F, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1600_31, 0x70001F08, NULL},
  {0x20, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1600_32, 0x70002008, NULL},
#endif
#if (BYTE_NUM > 32)
  {0x21, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1600_33, 0x70002108, NULL},
  {0x22, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1600_34, 0x70002208, NULL},
  {0x23, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1600_35, 0x70002308, NULL},
  {0x24, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1600_36, 0x70002408, NULL},
  {0x25, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1600_37, 0x70002508, NULL},
  {0x26, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1600_38, 0x70002608, NULL},
  {0x27, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1600_39, 0x70002708, NULL},
  {0x28, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1600_40, 0x70002808, NULL},
  {0x29, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1600_41, 0x70002908, NULL},
  {0x2A, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1600_42, 0x70002A08, NULL},
  {0x2B, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1600_43, 0x70002B08, NULL},
  {0x2C, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1600_44, 0x70002C08, NULL},
  {0x2D, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1600_45, 0x70002D08, NULL},
  {0x2E, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1600_46, 0x70002E08, NULL},
  {0x2F, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1600_47, 0x70002F08, NULL},
  {0x30, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1600_48, 0x70003008, NULL},
  {0x31, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1600_49, 0x70003108, NULL},
  {0x32, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1600_50, 0x70003208, NULL},
  {0x33, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1600_51, 0x70003308, NULL},
  {0x34, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1600_52, 0x70003408, NULL},
  {0x35, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1600_53, 0x70003508, NULL},
  {0x36, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1600_54, 0x70003608, NULL},
  {0x37, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1600_55, 0x70003708, NULL},
  {0x38, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1600_56, 0x70003808, NULL},
  {0x39, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1600_57, 0x70003908, NULL},
  {0x3A, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1600_58, 0x70003A08, NULL},
  {0x3B, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1600_59, 0x70003B08, NULL},
  {0x3C, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1600_60, 0x70003C08, NULL},
  {0x3D, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1600_61, 0x70003D08, NULL},
  {0x3E, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1600_62, 0x70003E08, NULL},
  {0x3F, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1600_63, 0x70003F08, NULL},
  {0x40, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1600_64, 0x70004008, NULL},
#endif
#if (BYTE_NUM > 64)
  {0x41, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1600_65, 0x70004108, NULL},
  {0x42, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1600_66, 0x70004208, NULL},
  {0x43, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1600_67, 0x70004308, NULL},
  {0x44, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1600_68, 0x70004408, NULL},
  {0x45, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1600_69, 0x70004508, NULL},
  {0x46, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1600_70, 0x70004608, NULL},
  {0x47, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1600_71, 0x70004708, NULL},
  {0x48, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1600_72, 0x70004808, NULL},
  {0x49, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1600_73, 0x70004908, NULL},
  {0x4A, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1600_74, 0x70004A08, NULL},
  {0x4B, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1600_75, 0x70004B08, NULL},
  {0x4C, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1600_76, 0x70004C08, NULL},
  {0x4D, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1600_77, 0x70004D08, NULL},
  {0x4E, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1600_78, 0x70004E08, NULL},
  {0x4F, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1600_79, 0x70004F08, NULL},
  {0x50, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1600_80, 0x70005008, NULL},
  {0x51, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1600_81, 0x70005108, NULL},
  {0x52, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1600_82, 0x70005208, NULL},
  {0x53, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1600_83, 0x70005308, NULL},
  {0x54, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1600_84, 0x70005408, NULL},
  {0x55, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1600_85, 0x70005508, NULL},
  {0x56, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1600_86, 0x70005608, NULL},
  {0x57, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1600_87, 0x70005708, NULL},
  {0x58, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1600_88, 0x70005808, NULL},
  {0x59, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1600_89, 0x70005908, NULL},
  {0x5A, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1600_90, 0x70005A08, NULL},
  {0x5B, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1600_91, 0x70005B08, NULL},
  {0x5C, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1600_92, 0x70005C08, NULL},
  {0x5D, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1600_93, 0x70005D08, NULL},
  {0x5E, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1600_94, 0x70005E08, NULL},
  {0x5F, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1600_95, 0x70005F08, NULL},
  {0x60, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1600_96, 0x70006008, NULL},
  {0x61, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1600_97, 0x70006108, NULL},
  {0x62, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1600_98, 0x70006208, NULL},
  {0x63, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1600_99, 0x70006308, NULL},
  {0x64, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1600_100, 0x70006408, NULL},
  {0x65, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1600_101, 0x70006508, NULL},
  {0x66, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1600_102, 0x70006608, NULL},
  {0x67, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1600_103, 0x70006708, NULL},
  {0x68, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1600_104, 0x70006808, NULL},
  {0x69, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1600_105, 0x70006908, NULL},
  {0x6A, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1600_106, 0x70006A08, NULL},
  {0x6B, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1600_107, 0x70006B08, NULL},
  {0x6C, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1600_108, 0x70006C08, NULL},
  {0x6D, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1600_109, 0x70006D08, NULL},
  {0x6E, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1600_110, 0x70006E08, NULL},
  {0x6F, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1600_111, 0x70006F08, NULL},
  {0x70, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1600_112, 0x70007008, NULL},
  {0x71, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1600_113, 0x70007108, NULL},
  {0x72, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1600_114, 0x70007208, NULL},
  {0x73, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1600_115, 0x70007308, NULL},
  {0x74, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1600_116, 0x70007408, NULL},
  {0x75, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1600_117, 0x70007508, NULL},
  {0x76, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1600_118, 0x70007608, NULL},
  {0x77, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1600_119, 0x70007708, NULL},
  {0x78, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1600_120, 0x70007808, NULL},
  {0x79, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1600_121, 0x70007908, NULL},
  {0x7A, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1600_122, 0x70007A08, NULL},
  {0x7B, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1600_123, 0x70007B08, NULL},
  {0x7C, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1600_124, 0x70007C08, NULL},
  {0x7D, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1600_125, 0x70007D08, NULL},
  {0x7E, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1600_126, 0x70007E08, NULL},
  {0x7F, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1600_127, 0x70007F08, NULL},
  {0x80, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1600_128, 0x70008008, NULL},
#endif
};
const _objd SDO1A00[] =
{
  {0x00, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_00, BYTE_NUM, NULL},
  {0x01, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_01, 0x60000108, NULL},
  {0x02, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_02, 0x60000208, NULL},
  {0x03, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_03, 0x60000308, NULL},
  {0x04, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_04, 0x60000408, NULL},
  {0x05, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_05, 0x60000508, NULL},
  {0x06, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_06, 0x60000608, NULL},
  {0x07, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_07, 0x60000708, NULL},
  {0x08, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_08, 0x60000808, NULL},
  {0x09, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_09, 0x60000908, NULL},
  {0x0A, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_10, 0x60000A08, NULL},
  {0x0B, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_11, 0x60000B08, NULL},
  {0x0C, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_12, 0x60000C08, NULL},
  {0x0D, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_13, 0x60000D08, NULL},
  {0x0E, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_14, 0x60000E08, NULL},
  {0x0F, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_15, 0x60000D08, NULL},
  {0x10, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_16, 0x60001008, NULL},
#if (BYTE_NUM > 16)
  {0x11, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_17, 0x60001108, NULL},
  {0x12, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_18, 0x60001208, NULL},
  {0x13, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_19, 0x60001308, NULL},
  {0x14, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_20, 0x60001408, NULL},
  {0x15, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_21, 0x60001508, NULL},
  {0x16, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_22, 0x60001608, NULL},
  {0x17, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_23, 0x60001708, NULL},
  {0x18, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_24, 0x60001808, NULL},
  {0x19, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_25, 0x60001908, NULL},
  {0x1A, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_26, 0x60001A08, NULL},
  {0x1B, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_27, 0x60001B08, NULL},
  {0x1C, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_28, 0x60001C08, NULL},
  {0x1D, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_29, 0x60001D08, NULL},
  {0x1E, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_30, 0x60001E08, NULL},
  {0x1F, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_31, 0x60001F08, NULL},
  {0x20, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_32, 0x60002008, NULL},
#endif
#if (BYTE_NUM > 32)
  {0x21, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_33, 0x60002108, NULL},
  {0x22, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_34, 0x60002208, NULL},
  {0x23, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_35, 0x60002308, NULL},
  {0x24, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_36, 0x60002408, NULL},
  {0x25, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_37, 0x60002508, NULL},
  {0x26, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_38, 0x60002608, NULL},
  {0x27, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_39, 0x60002708, NULL},
  {0x28, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_40, 0x60002808, NULL},
  {0x29, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_41, 0x60002908, NULL},
  {0x2A, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_42, 0x60002A08, NULL},
  {0x2B, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_43, 0x60002B08, NULL},
  {0x2C, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_44, 0x60002C08, NULL},
  {0x2D, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_45, 0x60002D08, NULL},
  {0x2E, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_46, 0x60002E08, NULL},
  {0x2F, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_47, 0x60002F08, NULL},
  {0x30, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_48, 0x60003008, NULL},
  {0x31, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_49, 0x60003108, NULL},
  {0x32, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_50, 0x60003208, NULL},
  {0x33, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_51, 0x60003308, NULL},
  {0x34, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_52, 0x60003408, NULL},
  {0x35, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_53, 0x60003508, NULL},
  {0x36, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_54, 0x60003608, NULL},
  {0x37, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_55, 0x60003708, NULL},
  {0x38, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_56, 0x60003808, NULL},
  {0x39, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_57, 0x60003908, NULL},
  {0x3A, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_58, 0x60003A08, NULL},
  {0x3B, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_59, 0x60003B08, NULL},
  {0x3C, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_60, 0x60003C08, NULL},
  {0x3D, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_61, 0x60003D08, NULL},
  {0x3E, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_62, 0x60003E08, NULL},
  {0x3F, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_63, 0x60003F08, NULL},
  {0x40, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_64, 0x60004008, NULL},
#endif
#if (BYTE_NUM > 64)
  {0x41, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_65, 0x60004108, NULL},
  {0x42, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_66, 0x60004208, NULL},
  {0x43, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_67, 0x60004308, NULL},
  {0x44, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_68, 0x60004408, NULL},
  {0x45, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_69, 0x60004508, NULL},
  {0x46, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_70, 0x60004608, NULL},
  {0x47, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_71, 0x60004708, NULL},
  {0x48, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_72, 0x60004808, NULL},
  {0x49, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_73, 0x60004908, NULL},
  {0x4A, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_74, 0x60004A08, NULL},
  {0x4B, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_75, 0x60004B08, NULL},
  {0x4C, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_76, 0x60004C08, NULL},
  {0x4D, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_77, 0x60004D08, NULL},
  {0x4E, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_78, 0x60004E08, NULL},
  {0x4F, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_79, 0x60004F08, NULL},
  {0x50, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_80, 0x60005008, NULL},
  {0x51, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_81, 0x60005108, NULL},
  {0x52, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_82, 0x60005208, NULL},
  {0x53, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_83, 0x60005308, NULL},
  {0x54, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_84, 0x60005408, NULL},
  {0x55, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_85, 0x60005508, NULL},
  {0x56, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_86, 0x60005608, NULL},
  {0x57, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_87, 0x60005708, NULL},
  {0x58, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_88, 0x60005808, NULL},
  {0x59, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_89, 0x60005908, NULL},
  {0x5A, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_90, 0x60005A08, NULL},
  {0x5B, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_91, 0x60005B08, NULL},
  {0x5C, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_92, 0x60005C08, NULL},
  {0x5D, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_93, 0x60005D08, NULL},
  {0x5E, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_94, 0x60005E08, NULL},
  {0x5F, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_95, 0x60005F08, NULL},
  {0x60, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_96, 0x60006008, NULL},
  {0x61, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_97, 0x60006108, NULL},
  {0x62, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_98, 0x60006208, NULL},
  {0x63, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_99, 0x60006308, NULL},
  {0x64, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_100, 0x60006408, NULL},
  {0x65, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_101, 0x60006508, NULL},
  {0x66, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_102, 0x60006608, NULL},
  {0x67, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_103, 0x60006708, NULL},
  {0x68, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_104, 0x60006808, NULL},
  {0x69, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_105, 0x60006908, NULL},
  {0x6A, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_106, 0x60006A08, NULL},
  {0x6B, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_107, 0x60006B08, NULL},
  {0x6C, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_108, 0x60006C08, NULL},
  {0x6D, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_109, 0x60006D08, NULL},
  {0x6E, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_110, 0x60006E08, NULL},
  {0x6F, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_111, 0x60006F08, NULL},
  {0x70, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_112, 0x60007008, NULL},
  {0x71, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_113, 0x60007108, NULL},
  {0x72, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_114, 0x60007208, NULL},
  {0x73, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_115, 0x60007308, NULL},
  {0x74, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_116, 0x60007408, NULL},
  {0x75, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_117, 0x60007508, NULL},
  {0x76, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_118, 0x60007608, NULL},
  {0x77, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_119, 0x60007708, NULL},
  {0x78, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_120, 0x60007808, NULL},
  {0x79, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_121, 0x60007908, NULL},
  {0x7A, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_122, 0x60007A08, NULL},
  {0x7B, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_123, 0x60007B08, NULL},
  {0x7C, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_124, 0x60007C08, NULL},
  {0x7D, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_125, 0x60007D08, NULL},
  {0x7E, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_126, 0x60007E08, NULL},
  {0x7F, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_127, 0x60007F08, NULL},
  {0x80, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_128, 0x60008008, NULL},
#endif
};
const _objd SDO1C00[] =
{
  {0x00, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1C00_00, 4, NULL},
  {0x01, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1C00_01, 3, NULL},
  {0x02, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1C00_02, 4, NULL},
  {0x03, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1C00_03, 0, NULL},
  {0x04, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1C00_04, 0, NULL},
};
const _objd SDO1C10[] =
{
  {0x00, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1C10_00, 1, NULL},
  {0x01, DTYPE_UNSIGNED16, 16, ATYPE_RO, acName1C10_01, 0x1600, NULL},
};
const _objd SDO1C11[] =
{
  {0x00, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1C11_00, 1, NULL},
  {0x01, DTYPE_UNSIGNED16, 16, ATYPE_RO, acName1C11_01, 0x1A00, NULL},
};
const _objd SDO6000[] =
{
  {0x00, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName6000_00, BYTE_NUM, NULL},
  {0x01, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_01, 0x00 , &(Obj.txpdo[0])},
  {0x02, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_02, 0x00 , &(Obj.txpdo[1])},
  {0x03, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_03, 0x00 , &(Obj.txpdo[2])},
  {0x04, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_04, 0x00 , &(Obj.txpdo[3])},
  {0x05, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_05, 0x00 , &(Obj.txpdo[4])},
  {0x06, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_06, 0x00 , &(Obj.txpdo[5])},
  {0x07, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_07, 0x00 , &(Obj.txpdo[6])},
  {0x08, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_08, 0x00 , &(Obj.txpdo[7])},
  {0x09, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_09, 0x00 , &(Obj.txpdo[8])},
  {0x0A, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_10, 0x00 , &(Obj.txpdo[9])},
  {0x0B, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_11, 0x00 , &(Obj.txpdo[10])},
  {0x0C, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_12, 0x00 , &(Obj.txpdo[11])},
  {0x0D, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_13, 0x00 , &(Obj.txpdo[12])},
  {0x0E, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_14, 0x00 , &(Obj.txpdo[13])},
  {0x0F, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_15, 0x00 , &(Obj.txpdo[14])},
  {0x10, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_16, 0x00 , &(Obj.txpdo[15])},
#if (BYTE_NUM > 16)
  {0x11, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_17, 0x00 , &(Obj.txpdo[16])},
  {0x12, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_18, 0x00 , &(Obj.txpdo[17])},
  {0x13, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_19, 0x00 , &(Obj.txpdo[18])},
  {0x14, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_20, 0x00 , &(Obj.txpdo[19])},
  {0x15, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_21, 0x00 , &(Obj.txpdo[20])},
  {0x16, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_22, 0x00 , &(Obj.txpdo[21])},
  {0x17, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_23, 0x00 , &(Obj.txpdo[22])},
  {0x18, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_24, 0x00 , &(Obj.txpdo[23])},
  {0x19, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_25, 0x00 , &(Obj.txpdo[24])},
  {0x1A, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_26, 0x00 , &(Obj.txpdo[25])},
  {0x1B, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_27, 0x00 , &(Obj.txpdo[26])},
  {0x1C, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_28, 0x00 , &(Obj.txpdo[27])},
  {0x1D, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_29, 0x00 , &(Obj.txpdo[28])},
  {0x1E, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_30, 0x00 , &(Obj.txpdo[29])},
  {0x1F, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_31, 0x00 , &(Obj.txpdo[30])},
  {0x20, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_32, 0x00 , &(Obj.txpdo[31])},
#endif
#if (BYTE_NUM > 32)
  {0x21, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_33, 0x00 , &(Obj.txpdo[32])},
  {0x22, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_34, 0x00 , &(Obj.txpdo[33])},
  {0x23, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_35, 0x00 , &(Obj.txpdo[34])},
  {0x24, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_36, 0x00 , &(Obj.txpdo[35])},
  {0x25, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_37, 0x00 , &(Obj.txpdo[36])},
  {0x26, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_38, 0x00 , &(Obj.txpdo[37])},
  {0x27, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_39, 0x00 , &(Obj.txpdo[38])},
  {0x28, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_40, 0x00 , &(Obj.txpdo[39])},
  {0x29, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_41, 0x00 , &(Obj.txpdo[40])},
  {0x2A, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_42, 0x00 , &(Obj.txpdo[41])},
  {0x2B, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_43, 0x00 , &(Obj.txpdo[42])},
  {0x2C, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_44, 0x00 , &(Obj.txpdo[43])},
  {0x2D, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_45, 0x00 , &(Obj.txpdo[44])},
  {0x2E, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_46, 0x00 , &(Obj.txpdo[45])},
  {0x2F, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_47, 0x00 , &(Obj.txpdo[46])},
  {0x30, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_48, 0x00 , &(Obj.txpdo[47])},
  {0x31, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_49, 0x00 , &(Obj.txpdo[48])},
  {0x32, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_50, 0x00 , &(Obj.txpdo[49])},
  {0x33, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_51, 0x00 , &(Obj.txpdo[50])},
  {0x34, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_52, 0x00 , &(Obj.txpdo[51])},
  {0x35, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_53, 0x00 , &(Obj.txpdo[52])},
  {0x36, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_54, 0x00 , &(Obj.txpdo[53])},
  {0x37, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_55, 0x00 , &(Obj.txpdo[54])},
  {0x38, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_56, 0x00 , &(Obj.txpdo[55])},
  {0x39, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_57, 0x00 , &(Obj.txpdo[56])},
  {0x3A, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_58, 0x00 , &(Obj.txpdo[57])},
  {0x3B, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_59, 0x00 , &(Obj.txpdo[58])},
  {0x3C, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_60, 0x00 , &(Obj.txpdo[59])},
  {0x3D, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_61, 0x00 , &(Obj.txpdo[60])},
  {0x3E, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_62, 0x00 , &(Obj.txpdo[61])},
  {0x3F, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_63, 0x00 , &(Obj.txpdo[62])},
  {0x40, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_64, 0x00 , &(Obj.txpdo[63])},
#endif
#if (BYTE_NUM > 64)
  {0x41, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_65, 0x00 , &(Obj.txpdo[64])},
  {0x42, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_66, 0x00 , &(Obj.txpdo[65])},
  {0x43, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_67, 0x00 , &(Obj.txpdo[66])},
  {0x44, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_68, 0x00 , &(Obj.txpdo[67])},
  {0x45, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_69, 0x00 , &(Obj.txpdo[68])},
  {0x46, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_70, 0x00 , &(Obj.txpdo[69])},
  {0x47, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_71, 0x00 , &(Obj.txpdo[70])},
  {0x48, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_72, 0x00 , &(Obj.txpdo[71])},
  {0x49, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_73, 0x00 , &(Obj.txpdo[72])},
  {0x4A, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_74, 0x00 , &(Obj.txpdo[73])},
  {0x4B, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_75, 0x00 , &(Obj.txpdo[74])},
  {0x4C, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_76, 0x00 , &(Obj.txpdo[75])},
  {0x4D, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_77, 0x00 , &(Obj.txpdo[76])},
  {0x4E, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_78, 0x00 , &(Obj.txpdo[77])},
  {0x4F, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_79, 0x00 , &(Obj.txpdo[78])},
  {0x50, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_80, 0x00 , &(Obj.txpdo[79])},
  {0x51, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_81, 0x00 , &(Obj.txpdo[80])},
  {0x52, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_82, 0x00 , &(Obj.txpdo[81])},
  {0x53, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_83, 0x00 , &(Obj.txpdo[82])},
  {0x54, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_84, 0x00 , &(Obj.txpdo[83])},
  {0x55, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_85, 0x00 , &(Obj.txpdo[84])},
  {0x56, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_86, 0x00 , &(Obj.txpdo[85])},
  {0x57, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_87, 0x00 , &(Obj.txpdo[86])},
  {0x58, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_88, 0x00 , &(Obj.txpdo[87])},
  {0x59, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_89, 0x00 , &(Obj.txpdo[88])},
  {0x5A, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_90, 0x00 , &(Obj.txpdo[89])},
  {0x5B, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_91, 0x00 , &(Obj.txpdo[90])},
  {0x5C, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_92, 0x00 , &(Obj.txpdo[91])},
  {0x5D, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_93, 0x00 , &(Obj.txpdo[92])},
  {0x5E, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_94, 0x00 , &(Obj.txpdo[93])},
  {0x5F, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_95, 0x00 , &(Obj.txpdo[94])},
  {0x60, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_96, 0x00 , &(Obj.txpdo[95])},
  {0x61, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_97, 0x00 , &(Obj.txpdo[96])},
  {0x62, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_98, 0x00 , &(Obj.txpdo[97])},
  {0x63, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_99, 0x00 , &(Obj.txpdo[98])},
  {0x64, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_100, 0x00 , &(Obj.txpdo[99])},
  {0x65, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_101, 0x00 , &(Obj.txpdo[100])},
  {0x66, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_102, 0x00 , &(Obj.txpdo[101])},
  {0x67, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_103, 0x00 , &(Obj.txpdo[102])},
  {0x68, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_104, 0x00 , &(Obj.txpdo[103])},
  {0x69, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_105, 0x00 , &(Obj.txpdo[104])},
  {0x6A, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_106, 0x00 , &(Obj.txpdo[105])},
  {0x6B, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_107, 0x00 , &(Obj.txpdo[106])},
  {0x6C, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_108, 0x00 , &(Obj.txpdo[107])},
  {0x6D, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_109, 0x00 , &(Obj.txpdo[108])},
  {0x6E, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_110, 0x00 , &(Obj.txpdo[109])},
  {0x6F, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_111, 0x00 , &(Obj.txpdo[110])},
  {0x70, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_112, 0x00 , &(Obj.txpdo[111])},
  {0x71, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_113, 0x00 , &(Obj.txpdo[112])},
  {0x72, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_114, 0x00 , &(Obj.txpdo[113])},
  {0x73, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_115, 0x00 , &(Obj.txpdo[114])},
  {0x74, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_116, 0x00 , &(Obj.txpdo[115])},
  {0x75, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_117, 0x00 , &(Obj.txpdo[116])},
  {0x76, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_118, 0x00 , &(Obj.txpdo[117])},
  {0x77, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_119, 0x00 , &(Obj.txpdo[118])},
  {0x78, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_120, 0x00 , &(Obj.txpdo[119])},
  {0x79, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_121, 0x00 , &(Obj.txpdo[120])},
  {0x7A, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_122, 0x00 , &(Obj.txpdo[121])},
  {0x7B, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_123, 0x00 , &(Obj.txpdo[122])},
  {0x7C, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_124, 0x00 , &(Obj.txpdo[123])},
  {0x7D, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_125, 0x00 , &(Obj.txpdo[124])},
  {0x7E, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_126, 0x00 , &(Obj.txpdo[125])},
  {0x7F, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_127, 0x00 , &(Obj.txpdo[126])},
  {0x80, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_128, 0x00 , &(Obj.txpdo[127])},
#endif
};
const _objd SDO7000[] =
{
  {0x00, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName7000_00, BYTE_NUM, NULL},
  {0x01, DTYPE_UNSIGNED8, 8, ATYPE_RW, acName1600_01, 0x00, &(Obj.rxpdo[0])},
  {0x02, DTYPE_UNSIGNED8, 8, ATYPE_RW, acName1600_02, 0x00, &(Obj.rxpdo[1])},
  {0x03, DTYPE_UNSIGNED8, 8, ATYPE_RW, acName1600_03, 0x00, &(Obj.rxpdo[2])},
  {0x04, DTYPE_UNSIGNED8, 8, ATYPE_RW, acName1600_04, 0x00, &(Obj.rxpdo[3])},
  {0x05, DTYPE_UNSIGNED8, 8, ATYPE_RW, acName1600_05, 0x00, &(Obj.rxpdo[4])},
  {0x06, DTYPE_UNSIGNED8, 8, ATYPE_RW, acName1600_06, 0x00, &(Obj.rxpdo[5])},
  {0x07, DTYPE_UNSIGNED8, 8, ATYPE_RW, acName1600_07, 0x00, &(Obj.rxpdo[6])},
  {0x08, DTYPE_UNSIGNED8, 8, ATYPE_RW, acName1600_08, 0x00, &(Obj.rxpdo[7])},
  {0x09, DTYPE_UNSIGNED8, 8, ATYPE_RW, acName1600_09, 0x00, &(Obj.rxpdo[8])},
  {0x0A, DTYPE_UNSIGNED8, 8, ATYPE_RW, acName1600_10, 0x00, &(Obj.rxpdo[9])},
  {0x0B, DTYPE_UNSIGNED8, 8, ATYPE_RW, acName1600_11, 0x00, &(Obj.rxpdo[10])},
  {0x0C, DTYPE_UNSIGNED8, 8, ATYPE_RW, acName1600_12, 0x00, &(Obj.rxpdo[11])},
  {0x0D, DTYPE_UNSIGNED8, 8, ATYPE_RW, acName1600_13, 0x00, &(Obj.rxpdo[12])},
  {0x0E, DTYPE_UNSIGNED8, 8, ATYPE_RW, acName1600_14, 0x00, &(Obj.rxpdo[13])},
  {0x0F, DTYPE_UNSIGNED8, 8, ATYPE_RW, acName1600_15, 0x00, &(Obj.rxpdo[14])},
  {0x10, DTYPE_UNSIGNED8, 8, ATYPE_RW, acName1600_16, 0x00, &(Obj.rxpdo[15])},
#if (BYTE_NUM > 16)
  {0x11, DTYPE_UNSIGNED8, 8, ATYPE_RW, acName1600_17, 0x00, &(Obj.rxpdo[16])},
  {0x12, DTYPE_UNSIGNED8, 8, ATYPE_RW, acName1600_18, 0x00, &(Obj.rxpdo[17])},
  {0x13, DTYPE_UNSIGNED8, 8, ATYPE_RW, acName1600_19, 0x00, &(Obj.rxpdo[18])},
  {0x14, DTYPE_UNSIGNED8, 8, ATYPE_RW, acName1600_20, 0x00, &(Obj.rxpdo[19])},
  {0x15, DTYPE_UNSIGNED8, 8, ATYPE_RW, acName1600_21, 0x00, &(Obj.rxpdo[20])},
  {0x16, DTYPE_UNSIGNED8, 8, ATYPE_RW, acName1600_22, 0x00, &(Obj.rxpdo[21])},
  {0x17, DTYPE_UNSIGNED8, 8, ATYPE_RW, acName1600_23, 0x00, &(Obj.rxpdo[22])},
  {0x18, DTYPE_UNSIGNED8, 8, ATYPE_RW, acName1600_24, 0x00, &(Obj.rxpdo[23])},
  {0x19, DTYPE_UNSIGNED8, 8, ATYPE_RW, acName1600_25, 0x00, &(Obj.rxpdo[24])},
  {0x1A, DTYPE_UNSIGNED8, 8, ATYPE_RW, acName1600_26, 0x00, &(Obj.rxpdo[25])},
  {0x1B, DTYPE_UNSIGNED8, 8, ATYPE_RW, acName1600_27, 0x00, &(Obj.rxpdo[26])},
  {0x1C, DTYPE_UNSIGNED8, 8, ATYPE_RW, acName1600_28, 0x00, &(Obj.rxpdo[27])},
  {0x1D, DTYPE_UNSIGNED8, 8, ATYPE_RW, acName1600_29, 0x00, &(Obj.rxpdo[28])},
  {0x1E, DTYPE_UNSIGNED8, 8, ATYPE_RW, acName1600_30, 0x00, &(Obj.rxpdo[29])},
  {0x1F, DTYPE_UNSIGNED8, 8, ATYPE_RW, acName1600_31, 0x00, &(Obj.rxpdo[30])},
  {0x20, DTYPE_UNSIGNED8, 8, ATYPE_RW, acName1600_32, 0x00, &(Obj.rxpdo[31])},
#endif
#if (BYTE_NUM > 32)
  {0x21, DTYPE_UNSIGNED8, 8, ATYPE_RW, acName1600_33, 0x00, &(Obj.rxpdo[32])},
  {0x22, DTYPE_UNSIGNED8, 8, ATYPE_RW, acName1600_34, 0x00, &(Obj.rxpdo[33])},
  {0x23, DTYPE_UNSIGNED8, 8, ATYPE_RW, acName1600_35, 0x00, &(Obj.rxpdo[34])},
  {0x24, DTYPE_UNSIGNED8, 8, ATYPE_RW, acName1600_36, 0x00, &(Obj.rxpdo[35])},
  {0x25, DTYPE_UNSIGNED8, 8, ATYPE_RW, acName1600_37, 0x00, &(Obj.rxpdo[36])},
  {0x26, DTYPE_UNSIGNED8, 8, ATYPE_RW, acName1600_38, 0x00, &(Obj.rxpdo[37])},
  {0x27, DTYPE_UNSIGNED8, 8, ATYPE_RW, acName1600_39, 0x00, &(Obj.rxpdo[38])},
  {0x28, DTYPE_UNSIGNED8, 8, ATYPE_RW, acName1600_40, 0x00, &(Obj.rxpdo[39])},
  {0x29, DTYPE_UNSIGNED8, 8, ATYPE_RW, acName1600_41, 0x00, &(Obj.rxpdo[40])},
  {0x2A, DTYPE_UNSIGNED8, 8, ATYPE_RW, acName1600_42, 0x00, &(Obj.rxpdo[41])},
  {0x2B, DTYPE_UNSIGNED8, 8, ATYPE_RW, acName1600_43, 0x00, &(Obj.rxpdo[42])},
  {0x2C, DTYPE_UNSIGNED8, 8, ATYPE_RW, acName1600_44, 0x00, &(Obj.rxpdo[43])},
  {0x2D, DTYPE_UNSIGNED8, 8, ATYPE_RW, acName1600_45, 0x00, &(Obj.rxpdo[44])},
  {0x2E, DTYPE_UNSIGNED8, 8, ATYPE_RW, acName1600_46, 0x00, &(Obj.rxpdo[45])},
  {0x2F, DTYPE_UNSIGNED8, 8, ATYPE_RW, acName1600_47, 0x00, &(Obj.rxpdo[46])},
  {0x30, DTYPE_UNSIGNED8, 8, ATYPE_RW, acName1600_48, 0x00, &(Obj.rxpdo[47])},
  {0x31, DTYPE_UNSIGNED8, 8, ATYPE_RW, acName1600_49, 0x00, &(Obj.rxpdo[48])},
  {0x32, DTYPE_UNSIGNED8, 8, ATYPE_RW, acName1600_50, 0x00, &(Obj.rxpdo[49])},
  {0x33, DTYPE_UNSIGNED8, 8, ATYPE_RW, acName1600_51, 0x00, &(Obj.rxpdo[50])},
  {0x34, DTYPE_UNSIGNED8, 8, ATYPE_RW, acName1600_52, 0x00, &(Obj.rxpdo[51])},
  {0x35, DTYPE_UNSIGNED8, 8, ATYPE_RW, acName1600_53, 0x00, &(Obj.rxpdo[52])},
  {0x36, DTYPE_UNSIGNED8, 8, ATYPE_RW, acName1600_54, 0x00, &(Obj.rxpdo[53])},
  {0x37, DTYPE_UNSIGNED8, 8, ATYPE_RW, acName1600_55, 0x00, &(Obj.rxpdo[54])},
  {0x38, DTYPE_UNSIGNED8, 8, ATYPE_RW, acName1600_56, 0x00, &(Obj.rxpdo[55])},
  {0x39, DTYPE_UNSIGNED8, 8, ATYPE_RW, acName1600_57, 0x00, &(Obj.rxpdo[56])},
  {0x3A, DTYPE_UNSIGNED8, 8, ATYPE_RW, acName1600_58, 0x00, &(Obj.rxpdo[57])},
  {0x3B, DTYPE_UNSIGNED8, 8, ATYPE_RW, acName1600_59, 0x00, &(Obj.rxpdo[58])},
  {0x3C, DTYPE_UNSIGNED8, 8, ATYPE_RW, acName1600_60, 0x00, &(Obj.rxpdo[59])},
  {0x3D, DTYPE_UNSIGNED8, 8, ATYPE_RW, acName1600_61, 0x00, &(Obj.rxpdo[60])},
  {0x3E, DTYPE_UNSIGNED8, 8, ATYPE_RW, acName1600_62, 0x00, &(Obj.rxpdo[61])},
  {0x3F, DTYPE_UNSIGNED8, 8, ATYPE_RW, acName1600_63, 0x00, &(Obj.rxpdo[62])},
  {0x40, DTYPE_UNSIGNED8, 8, ATYPE_RW, acName1600_64, 0x00, &(Obj.rxpdo[63])},
#endif
#if (BYTE_NUM > 64)
  {0x41, DTYPE_UNSIGNED8, 8, ATYPE_RW, acName1600_65, 0x00, &(Obj.rxpdo[64])},
  {0x42, DTYPE_UNSIGNED8, 8, ATYPE_RW, acName1600_66, 0x00, &(Obj.rxpdo[65])},
  {0x43, DTYPE_UNSIGNED8, 8, ATYPE_RW, acName1600_67, 0x00, &(Obj.rxpdo[66])},
  {0x44, DTYPE_UNSIGNED8, 8, ATYPE_RW, acName1600_68, 0x00, &(Obj.rxpdo[67])},
  {0x45, DTYPE_UNSIGNED8, 8, ATYPE_RW, acName1600_69, 0x00, &(Obj.rxpdo[68])},
  {0x46, DTYPE_UNSIGNED8, 8, ATYPE_RW, acName1600_70, 0x00, &(Obj.rxpdo[69])},
  {0x47, DTYPE_UNSIGNED8, 8, ATYPE_RW, acName1600_71, 0x00, &(Obj.rxpdo[70])},
  {0x48, DTYPE_UNSIGNED8, 8, ATYPE_RW, acName1600_72, 0x00, &(Obj.rxpdo[71])},
  {0x49, DTYPE_UNSIGNED8, 8, ATYPE_RW, acName1600_73, 0x00, &(Obj.rxpdo[72])},
  {0x4A, DTYPE_UNSIGNED8, 8, ATYPE_RW, acName1600_74, 0x00, &(Obj.rxpdo[73])},
  {0x4B, DTYPE_UNSIGNED8, 8, ATYPE_RW, acName1600_75, 0x00, &(Obj.rxpdo[74])},
  {0x4C, DTYPE_UNSIGNED8, 8, ATYPE_RW, acName1600_76, 0x00, &(Obj.rxpdo[75])},
  {0x4D, DTYPE_UNSIGNED8, 8, ATYPE_RW, acName1600_77, 0x00, &(Obj.rxpdo[76])},
  {0x4E, DTYPE_UNSIGNED8, 8, ATYPE_RW, acName1600_78, 0x00, &(Obj.rxpdo[77])},
  {0x4F, DTYPE_UNSIGNED8, 8, ATYPE_RW, acName1600_79, 0x00, &(Obj.rxpdo[78])},
  {0x50, DTYPE_UNSIGNED8, 8, ATYPE_RW, acName1600_80, 0x00, &(Obj.rxpdo[79])},
  {0x51, DTYPE_UNSIGNED8, 8, ATYPE_RW, acName1600_81, 0x00, &(Obj.rxpdo[80])},
  {0x52, DTYPE_UNSIGNED8, 8, ATYPE_RW, acName1600_82, 0x00, &(Obj.rxpdo[81])},
  {0x53, DTYPE_UNSIGNED8, 8, ATYPE_RW, acName1600_83, 0x00, &(Obj.rxpdo[82])},
  {0x54, DTYPE_UNSIGNED8, 8, ATYPE_RW, acName1600_84, 0x00, &(Obj.rxpdo[83])},
  {0x55, DTYPE_UNSIGNED8, 8, ATYPE_RW, acName1600_85, 0x00, &(Obj.rxpdo[84])},
  {0x56, DTYPE_UNSIGNED8, 8, ATYPE_RW, acName1600_86, 0x00, &(Obj.rxpdo[85])},
  {0x57, DTYPE_UNSIGNED8, 8, ATYPE_RW, acName1600_87, 0x00, &(Obj.rxpdo[86])},
  {0x58, DTYPE_UNSIGNED8, 8, ATYPE_RW, acName1600_88, 0x00, &(Obj.rxpdo[87])},
  {0x59, DTYPE_UNSIGNED8, 8, ATYPE_RW, acName1600_89, 0x00, &(Obj.rxpdo[88])},
  {0x5A, DTYPE_UNSIGNED8, 8, ATYPE_RW, acName1600_90, 0x00, &(Obj.rxpdo[89])},
  {0x5B, DTYPE_UNSIGNED8, 8, ATYPE_RW, acName1600_91, 0x00, &(Obj.rxpdo[90])},
  {0x5C, DTYPE_UNSIGNED8, 8, ATYPE_RW, acName1600_92, 0x00, &(Obj.rxpdo[91])},
  {0x5D, DTYPE_UNSIGNED8, 8, ATYPE_RW, acName1600_93, 0x00, &(Obj.rxpdo[92])},
  {0x5E, DTYPE_UNSIGNED8, 8, ATYPE_RW, acName1600_94, 0x00, &(Obj.rxpdo[93])},
  {0x5F, DTYPE_UNSIGNED8, 8, ATYPE_RW, acName1600_95, 0x00, &(Obj.rxpdo[94])},
  {0x60, DTYPE_UNSIGNED8, 8, ATYPE_RW, acName1600_96, 0x00, &(Obj.rxpdo[95])},
  {0x61, DTYPE_UNSIGNED8, 8, ATYPE_RW, acName1600_97, 0x00, &(Obj.rxpdo[96])},
  {0x62, DTYPE_UNSIGNED8, 8, ATYPE_RW, acName1600_98, 0x00, &(Obj.rxpdo[97])},
  {0x63, DTYPE_UNSIGNED8, 8, ATYPE_RW, acName1600_99, 0x00, &(Obj.rxpdo[98])},
  {0x64, DTYPE_UNSIGNED8, 8, ATYPE_RW, acName1600_100, 0x00, &(Obj.rxpdo[99])},
  {0x65, DTYPE_UNSIGNED8, 8, ATYPE_RW, acName1600_101, 0x00, &(Obj.rxpdo[100])},
  {0x66, DTYPE_UNSIGNED8, 8, ATYPE_RW, acName1600_102, 0x00, &(Obj.rxpdo[101])},
  {0x67, DTYPE_UNSIGNED8, 8, ATYPE_RW, acName1600_103, 0x00, &(Obj.rxpdo[102])},
  {0x68, DTYPE_UNSIGNED8, 8, ATYPE_RW, acName1600_104, 0x00, &(Obj.rxpdo[103])},
  {0x69, DTYPE_UNSIGNED8, 8, ATYPE_RW, acName1600_105, 0x00, &(Obj.rxpdo[104])},
  {0x6A, DTYPE_UNSIGNED8, 8, ATYPE_RW, acName1600_106, 0x00, &(Obj.rxpdo[105])},
  {0x6B, DTYPE_UNSIGNED8, 8, ATYPE_RW, acName1600_107, 0x00, &(Obj.rxpdo[106])},
  {0x6C, DTYPE_UNSIGNED8, 8, ATYPE_RW, acName1600_108, 0x00, &(Obj.rxpdo[107])},
  {0x6D, DTYPE_UNSIGNED8, 8, ATYPE_RW, acName1600_109, 0x00, &(Obj.rxpdo[108])},
  {0x6E, DTYPE_UNSIGNED8, 8, ATYPE_RW, acName1600_110, 0x00, &(Obj.rxpdo[109])},
  {0x6F, DTYPE_UNSIGNED8, 8, ATYPE_RW, acName1600_111, 0x00, &(Obj.rxpdo[110])},
  {0x70, DTYPE_UNSIGNED8, 8, ATYPE_RW, acName1600_112, 0x00, &(Obj.rxpdo[111])},
  {0x71, DTYPE_UNSIGNED8, 8, ATYPE_RW, acName1600_113, 0x00, &(Obj.rxpdo[112])},
  {0x72, DTYPE_UNSIGNED8, 8, ATYPE_RW, acName1600_114, 0x00, &(Obj.rxpdo[113])},
  {0x73, DTYPE_UNSIGNED8, 8, ATYPE_RW, acName1600_115, 0x00, &(Obj.rxpdo[114])},
  {0x74, DTYPE_UNSIGNED8, 8, ATYPE_RW, acName1600_116, 0x00, &(Obj.rxpdo[115])},
  {0x75, DTYPE_UNSIGNED8, 8, ATYPE_RW, acName1600_117, 0x00, &(Obj.rxpdo[116])},
  {0x76, DTYPE_UNSIGNED8, 8, ATYPE_RW, acName1600_118, 0x00, &(Obj.rxpdo[117])},
  {0x77, DTYPE_UNSIGNED8, 8, ATYPE_RW, acName1600_119, 0x00, &(Obj.rxpdo[118])},
  {0x78, DTYPE_UNSIGNED8, 8, ATYPE_RW, acName1600_120, 0x00, &(Obj.rxpdo[119])},
  {0x79, DTYPE_UNSIGNED8, 8, ATYPE_RW, acName1600_121, 0x00, &(Obj.rxpdo[120])},
  {0x7A, DTYPE_UNSIGNED8, 8, ATYPE_RW, acName1600_122, 0x00, &(Obj.rxpdo[121])},
  {0x7B, DTYPE_UNSIGNED8, 8, ATYPE_RW, acName1600_123, 0x00, &(Obj.rxpdo[122])},
  {0x7C, DTYPE_UNSIGNED8, 8, ATYPE_RW, acName1600_124, 0x00, &(Obj.rxpdo[123])},
  {0x7D, DTYPE_UNSIGNED8, 8, ATYPE_RW, acName1600_125, 0x00, &(Obj.rxpdo[124])},
  {0x7E, DTYPE_UNSIGNED8, 8, ATYPE_RW, acName1600_126, 0x00, &(Obj.rxpdo[125])},
  {0x7F, DTYPE_UNSIGNED8, 8, ATYPE_RW, acName1600_127, 0x00, &(Obj.rxpdo[126])},
  {0x80, DTYPE_UNSIGNED8, 8, ATYPE_RW, acName1600_128, 0x00, &(Obj.rxpdo[127])},
#endif
};


const _objectlist SDOobjects[] =
{
  {0x1000, OTYPE_VAR, 0, 0, acName1000, SDO1000},
  {0x1008, OTYPE_VAR, 0, 0, acName1008, SDO1008},
  {0x1009, OTYPE_VAR, 0, 0, acName1009, SDO1009},
  {0x100A, OTYPE_VAR, 0, 0, acName100A, SDO100A},
  {0x1018, OTYPE_RECORD, 4, 0, acName1018, SDO1018},
  {0x1600, OTYPE_RECORD, BYTE_NUM, 0, acName1600, SDO1600},
  {0x1A00, OTYPE_RECORD, BYTE_NUM, 0, acName1A00, SDO1A00},
  {0x1C00, OTYPE_ARRAY, 4, 0, acName1C00, SDO1C00},
  {0x1C10, OTYPE_ARRAY, 1, 0, acName1C10, SDO1C10},
  {0x1C11, OTYPE_ARRAY, 1, 0, acName1C11, SDO1C11},
  {0x6000, OTYPE_RECORD, BYTE_NUM, 0, acName6000, SDO6000},
  {0x7000, OTYPE_RECORD, BYTE_NUM, 0, acName7000, SDO7000},
  {0xffff, 0xff, 0xff, 0xff, NULL, NULL}
};
