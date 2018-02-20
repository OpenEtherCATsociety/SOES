#ifndef __ECAT_DEF_H__
#define __ECAT_DEF_H__

#define ESC_DC_SYNC0_CYCLETIME_OFFSET           0x09A0
#define ESC_SYSTEMTIME_OFFSET                   0x0910
#define ESC_DC_SYNC_ACTIVATION_OFFSET           0x0981

#define STATE_INIT                              ((uint8_t)0x01)

#ifndef SOES
#define  SOES 1
#endif

#ifndef VARVOLATILE
#define  VARVOLATILE volatile
#endif

#ifndef AL_EVENT_ENABLED
#define AL_EVENT_ENABLED 1
#endif


int MainInit(void);
void MainLoop(void);

extern const unsigned char * tiesc_eeprom;
extern int bRunApplication;

#endif /* __ECAT_DEF_H__ */
