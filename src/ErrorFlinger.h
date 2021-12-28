#pragma once

#include "Particle.h"

typedef enum
{
    ERRORFLINGER_REBOOT_REASON_PIN_RESET,
    ERRORFLINGER_REBOOT_REASON_WATCHDOG,
    ERRORFLINGER_REBOOT_REASON_POWER_BROWNOUT,
    ERRORFLINGER_REBOOT_REASON_PANIC,
    ERRORFLINGER_REBOOT_REASON_OTA_UPDATE,
    ERRORFLINGER_REBOOT_REASON_USER_RESET,
    ERRORFLINGER_REBOOT_REASON_POWER_DOWN,
    ERRORFLINGER_REBOOT_REASON_UNKNOWN

} ERRORFLINGER_REBOOT_REASON_T;

typedef enum
{
    ERRORFLINGER_CRASH_REASON_PANIC,
    ERRORFLINGER_CRASH_REASON_FAULT,
    ERRORFLINGER_CRASH_REASON_OUT_OF_MEMORY,
    ERRORFLINGER_CRASH_REASON_UNUSUAL_REBOOT

} ERRORFLINGER_CRASH_REASON_T;

typedef enum
{
    ERRORFLINGER_FAULT_REASON_NONE,
    ERRORFLINGER_FAULT_REASON_HARD,
    ERRORFLINGER_FAULT_REASON_MEMORYMANAGEMENT,
    ERRORFLINGER_FAULT_REASON_BUS,
    ERRORFLINGER_FAULT_REASON_USAGE,
    ERRORFLINGER_FAULT_REASON_NMI

} ERRORFLINGER_FAULT_REASON_T;

#define ERRORFLINGER_PENDING_CRASH_REPORT_MAGIC 0x29592fed
typedef struct
{
    //magic "is valid" flag
    uint32_t magicIsValid;

    //Crash reason
    ERRORFLINGER_CRASH_REASON_T creashReason;

    //time of crash (if known)
    uint32_t time;

    //union of crash reasons and things
    union 
    {
        ERRORFLINGER_FAULT_REASON_T faultReason;
    };

} ERRORFLINGER_PENDING_CRASH_REPORT_T;


class ErrorFlinger
{
  public:
    explicit ErrorFlinger();

    void loop();
  private:
    void cloudConnectivityEvent(system_event_t event, int param);
    void outOfMemoryEvent(system_event_t event, int param);
    uint32_t getPendingCrashReportsCount( void );
    void processCrashBuffer( const ERRORFLINGER_REBOOT_REASON_T resetReason );
    static ERRORFLINGER_REBOOT_REASON_T readRebootReason(void);
    bool m_connected;
};
