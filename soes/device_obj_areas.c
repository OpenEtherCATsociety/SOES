//
// Created by Mikhail Gladyshev on 28/12/25.
//
#include "esc_coe.h"
#include "utypes.h"
#include <stddef.h>

static const char acNameARR[]    = "Number of Entries";

static const char acNameF000[]    = "Module Device Profile";
static const char acNameF000_00[] = "Max SubIndex";
static const char acNameF000_01[] = "Index Distance";
static const char acNameF000_02[] = "Maximum Number of Modules";
static const char acNameF000_03[] = "General configuration";
static const char acNameF000_04[] = "General information";
static const char acNameF000_05[] = "Module PDO Group of device";

static const char acNameF002[]    = "Detect Modules Command";

static const char acNameF00E[]    = "Module PDO Group Mapping Alignment PDO Number";
static const char acNameF00E_0x[]    = "Alignment PDO";

static const char acNameF00F[]    = "Module PDO Group Mapping Alignment";
static const char acNameF00F_0x[]    = "Alignment";

static const char acNameF010[]    = "Module Profile List"; // Profile information of the modules
static const char acNameF010_0x[]    = "Profile";

static const char acNameF020[]    = "Address list of the configured Modules";
static const char acNameF020_0x[]    = "Module address";

static const char acNameF030[]    = "Module Ident list of the configured Modules";
static const char acNameF030_0x[]    = "Module ident";

static const char acNameF040[]    = "Address list of detected Modules";
static const char acNameF040_0x[]    = "Module address";

static const char acNameF050[]    = "Module Ident list of detected Modules";
static const char acNameF050_0x[]    = "Module ident";

static const char acNameF100[]    = "Device Status";      // TxPDO mappable
static const char acNameF110[]    = "Device Diagnosis";
static const char acNameF120[]    = "Device Statistics";

static const char acNameF200[]    = "Device Control";     // RxPDO mappable

static const char acNameF600[]    = "Input Area of device itself";  // TxPDO mappable
static const char acNameF700[]    = "Output Area of device itself"; // RxPDO mappable
static const char acNameF800[]    = "Device Configuration Parameter Area";
static const char acNameFB00[]    = "Device Command";

/* --- Структуры подобъектов (_objd) --- */

// 0xF000: Modular Device Profile (Обязателен для всех типов)
const _objd SDOF000[] =
{
  {0x00, DTYPE_UNSIGNED8, 8, ATYPE_RO, acNameF000_00, 5, NULL},//TODO maybe need to delete
  {0x01, DTYPE_UNSIGNED16,  16, ATYPE_RO, acNameF000_01, 16, NULL},
  {0x02, DTYPE_UNSIGNED16,  32, ATYPE_RO, acNameF000_02, 0, NULL},
  {0x03, DTYPE_UNSIGNED32,  32, ATYPE_RO, acNameF000_03, 0, NULL},
  {0x04, DTYPE_UNSIGNED32,  16, ATYPE_RO, acNameF000_04, 0, NULL},
  {0x05, DTYPE_UNSIGNED16,  16, ATYPE_RO, acNameF000_05, 0, NULL},
};
const _objd SDOF002[] = {
  //TODO check what i need here
};

const _objd SDOF00E[] = {
  {0x00, DTYPE_UNSIGNED8, 8, ATYPE_RO, acNameARR, 255, NULL},
  {0x01, DTYPE_UNSIGNED16, 16, ATYPE_RO, acNameF00E_0x + " num", 0x1A00, NULL},
  //TODO add 254 more elements
};
const _objd SDOF00F[] = {
  {0x00, DTYPE_UNSIGNED8, 8, ATYPE_RO, acNameARR, 255, NULL},
  {0x01, DTYPE_UNSIGNED16, 16, ATYPE_RO, acNameF00F_0x + " num", 0x1A00, NULL},
  //TODO add 254 more elements
};
const _objd SDOF010[] = {
  {0x00, DTYPE_UNSIGNED8, 8, ATYPE_RO, acNameARR, 255, NULL},
  {0x01, DTYPE_UNSIGNED16, 16, ATYPE_RO, acNameF010_0x + " num", 0x1A00, NULL},
  //TODO add 254 more elements
};
const _objd SDOF020[] = {
  {0x00, DTYPE_UNSIGNED8, 8, ATYPE_RO, acNameARR, 255, NULL},
  {0x01, DTYPE_UNSIGNED16, 16, ATYPE_RO, acNameF020_0x + " num",0x1A00, NULL},
  //TODO add 254 more elements
};
const _objd SDOF030[] =
{
  {0x00, DTYPE_UNSIGNED8,   8,  ATYPE_RO, acNameARR, 255, &Obj.Mdp.nConfiguredModules},
  {0x01, DTYPE_UNSIGNED32,  32, ATYPE_RW, acNameF030_0x + " num", 0, &Obj.Mdp.ConfigIdent[0]},
  //TODO add 254 more elements
};
const _objd SDOF040[] = {
  {0x00, DTYPE_UNSIGNED8, 8, ATYPE_RO, acNameARR, 255, NULL},
  {0x01, DTYPE_UNSIGNED16, 16, ATYPE_RO, acNameF040_0x + " num", 0x1A00, NULL},
  //TODO add 254 more elements
};
const _objd SDOF050[] =
{
  {0x00, DTYPE_UNSIGNED8,   8,  ATYPE_RO, acNameARR, 0, &Obj.Mdp.nDetectedModules},
  {0x01, DTYPE_UNSIGNED32,  32, ATYPE_RO, acNameF050_0x + " num", 0, &Obj.Mdp.DetectIdent[0]},
  //TODO add 254 more elements
};

const _objd SDOF100[] =
{
  //TODO check what i need here. 4.2.3.11 Device Status (0xF100…0xF10F)
};

const _objd SDOF110[] = {
  //TODO check what i need here. 4.2.3.12 Device Diagnosis (0xF110…0xF11F)
};
const _objd SDOF120[] = {
  //TODO check what i need here. 4.2.3.13 Device Statistics (0xF120…0xF12F)
};
const _objd SDOF200[] =
{
  //TODO check what i need here. 4.2.3.14 Device Control (0xF200…0xF20F)
};
const _objd SDOF600[] = {
  //TODO check what i need here. 4.2.3.15 Device Input Data (0xF600…0xF6FF)
};
const _objd SDOF700[] = {
  //TODO check what i need here. 4.2.3.16 Device Output Data (0xF700…0xF7FF)
};
const _objd SDOF800[] = {
  //TODO check what i need here. 4.2.3.17 Device Configuration Data
};
const _objd SDOFB00[] =
{
  //TODO check what i need here. 4.2.3.18 Device Command (0xFB00…0xFBFF)
};

const _objectlist SDOobjects[] =
{
  {0xF000, OTYPE_RECORD, 4, 0, acNameF000, SDOF000}, //Modular Device Profile
  {0xF002, OTYPE_RECORD,    0, 0, acNameF002, SDOF002}, //Detect Modules Command
  {0xF00E, OTYPE_ARRAY,    0, 0, acNameF00E, SDOF00E}, //Module PDO Group Mapping Alignment PDO Number
  {0xF00F, OTYPE_ARRAY,    0, 0, acNameF00F, SDOF00F}, //Module PDO Group Mapping Alignment
  {0xF010, OTYPE_ARRAY,  255, 0, acNameF010, SDOF010}, //Module Profile List Profile information of the modules
  //TODO add here 15 more elements. Last index 0xF01F (16 objects)
  {0xF020, OTYPE_ARRAY,  255, 0, acNameF020, SDOF020}, //Address list of the configured Modules
  //TODO add here 15 more elements. Last index 0xF02F (16 objects)
  {0xF030, OTYPE_ARRAY,  255, 0, acNameF030, SDOF030}, //Module Ident list of the configured Modules
  //TODO add here 15 more elements.Last index 0xF03F (16 objects)
  {0xF040, OTYPE_ARRAY,  255, 0, acNameF040, SDOF040}, //Address list of detected Modules
  //TODO add here 15 more elements. Last index 0xF04F (16 objects)
  {0xF050, OTYPE_ARRAY,  255, 0, acNameF050, SDOF050}, //Module Ident list of detected Modules
  //TODO add here 15 more elements. Last index 0xF05F (16 objects)
  {0xF100, OTYPE_RECORD, 0, 0, acNameF100, SDOF100}, //Device Status (TxPDO mappable)
  //TODO add here 15 more elements. Last index 0xF10F (16 objects)
  {0xF110, OTYPE_RECORD, 0, 0, acNameF110, SDOF110}, //Device Diagnosis
  //TODO add here 15 more elements. Last index 0xF11F (16 objects)
  {0xF120, OTYPE_RECORD, 0, 0, acNameF120, SDOF120}, //Device Statistics
  //TODO add here 15 more elements.Last index 0xF12F (16 objects)
  {0xF200, OTYPE_RECORD, 0, 0, acNameF200, SDOF200}, //Device Control (RxPDO mappable)
  //TODO add here 15 more elements. Last index 0xF20F (16 objects)
  {0xF600, OTYPE_RECORD, 0, 0, acNameF600, SDOF600}, //Input Area of device itself (TxPDO mappable)
  //TODO add here 255 more elements. Last index 0xF6FF (255 objects)
  {0xF700, OTYPE_RECORD, 0, 0, acNameF700, SDOF700}, //Output Area of device itself (RxPDO mappable)
  //TODO add here 255 more elements. Last index 0xF7FF (255 objects)
  {0xF800, OTYPE_RECORD, 0, 0, acNameF800, SDOF800}, //Device Configuration Parameter Area
  //TODO add here 255 more elements. Last index 0xF8FF (255 objects)
  {0xFB00, OTYPE_RECORD,    0, 0, acNameFB00, SDOFB00}, //Device Command
  //TODO add here 255 more elements. Last index 0xFBFF (255 objects)
};

