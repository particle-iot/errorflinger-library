#include <stdio.h>
#include <stdarg.h>
#include <functional>
#include "ErrorFlinger.h"
#include "deviceid_hal.h"

//What does this library do?
// - registers for all error handlers from Device OS
// - records the errors in battery backup SRAM
// - on a device connection, publishes any device crashes to the particle platform

#define SYSTEM_VERSION_GET_MAJOR(version) (((version) >> 24) & 0xFF)

#if SYSTEM_VERSION_GET_MAJOR(SYSTEM_VERSION) < 3
    #error "ErrorFlinger only supports 3.0 and above"
#endif

using namespace std::placeholders;

static void ErrorFlinger_HardFault_Handler(void);
static void ErrorFlinger_MemoryManagement_Handler(void);
static void ErrorFlinger_BusFault_Handler(void);
static void ErrorFlinger_UsageFault_Handler(void);
static void ErrorFlinger_NMI_Handler(void);

//a buffer that is retained over a crash
#define ERRORFLINGER_CRASH_BUFFER_SIZE 32
static retained uint8_t crashBuffer[ERRORFLINGER_CRASH_BUFFER_SIZE];

//a list of pending crash reports to send to the cloud
#define ERRORFLINGER_MAX_CRASHES_TO_BUFFER 4
static retained ERRORFLINGER_PENDING_CRASH_REPORT_T pendingCrashReports[ERRORFLINGER_MAX_CRASHES_TO_BUFFER];

ErrorFlinger::ErrorFlinger() :
    m_connected(false)
{
    System.enableFeature(FEATURE_RESET_INFO);

    //register for system events
    System.on(cloud_status, &ErrorFlinger::cloudConnectivityEvent, this);
    System.on(out_of_memory, &ErrorFlinger::outOfMemoryEvent, this);

    //hook into all of the ISRs
    bool isrAttachOK = true;
    isrAttachOK &= attachInterruptDirect(HardFault_IRQn, ErrorFlinger_HardFault_Handler);
    isrAttachOK &= attachInterruptDirect(MemoryManagement_IRQn, ErrorFlinger_MemoryManagement_Handler);
    isrAttachOK &= attachInterruptDirect(BusFault_IRQn, ErrorFlinger_BusFault_Handler);
    isrAttachOK &= attachInterruptDirect(UsageFault_IRQn, ErrorFlinger_UsageFault_Handler);
    isrAttachOK &= attachInterruptDirect(NonMaskableInt_IRQn, ErrorFlinger_NMI_Handler);
    SPARK_ASSERT(isrAttachOK);

    //store why we booted
    const ERRORFLINGER_REBOOT_REASON_T resetReason = readRebootReason();

    //inspect and store a crash (if it exists)
    //we might also store a crash report if the reboot reason is something bad (without an error trace)
    processCrashBuffer(resetReason);

    //reset the crash buffer and get ready for the next one!
    memset(crashBuffer, 0, sizeof(crashBuffer));
}

void ErrorFlinger::loop()
{
    static uint32_t s_last_check_time = 0;

    if ((millis() - s_last_check_time) >= 1000) {
        s_last_check_time = millis();

        //only send data is we are connected (and there is data)
        if (m_connected && (getPendingCrashReportsCount() > 0) {
            //get a crash report from the pile
            ERRORFLINGER_CRASH_T *pendingCrash = ERRORFLINGER_CRASH_T()

            //convert crash to a JSON blob
            void *crashJSONBuffer = malloc(32);

            Particle.publish("crash-report", crashJSONBuffer, WITH_ACK);

            free(crashJSONBuffer);
        }
    }
}

/****************************************************************************************************
 * Private
 ****************************************************************************************************/

void ErrorFlinger::cloudConnectivityEvent(system_event_t event, int param)
{
    m_connected = (param == cloud_status_connected);
}

void ErrorFlinger::outOfMemoryEvent(system_event_t event, int param)
{
    LOG(ERROR, "Out of memory");
}

uint32_t ErrorFlinger::getPendingCrashReportsCount( void )
{
    //walk through the records from 0 to N and find if any of them are valid
    return 0;
}

void ErrorFlinger::processCrashBuffer( const ERRORFLINGER_REBOOT_REASON_T resetReason )
{
    
}

static ERRORFLINGER_REBOOT_REASON_T ErrorFlinger::readRebootReason(void)
{
  const uint32_t s_last_mcu_reset_reason = System.resetReason();
  ERRORFLINGER_REBOOT_REASON_T reset_reason = ERRORFLINGER_REBOOT_REASON_UNKNOWN;

  switch( s_last_mcu_reset_reason ) {
    case RESET_REASON_PIN_RESET:
      reset_reason = ERRORFLINGER_REBOOT_REASON_PIN_RESET;
    break;
    case RESET_REASON_WATCHDOG:
      reset_reason = ERRORFLINGER_REBOOT_REASON_WATCHDOG;
    break;
    case RESET_REASON_USER:
      reset_reason = ERRORFLINGER_REBOOT_REASON_USER_RESET;
    break;
    case RESET_REASON_POWER_DOWN:
    case RESET_REASON_POWER_MANAGEMENT:
      reset_reason = ERRORFLINGER_REBOOT_REASON_POWER_DOWN;
    break;
    case RESET_REASON_POWER_BROWNOUT:
      reset_reason = ERRORFLINGER_REBOOT_REASON_POWER_BROWNOUT;
    break;
    case RESET_REASON_UPDATE:
    case RESET_REASON_UPDATE_TIMEOUT:
      reset_reason = ERRORFLINGER_REBOOT_REASON_OTA_UPDATE;
    break;
    case RESET_REASON_PANIC:
      reset_reason = ERRORFLINGER_REBOOT_REASON_PANIC;
    break;
    default:
      reset_reason = kMfltRebootReason_Unknown;
    break;
  }

  return reset_reason;
}

static void ErrorFlinger_Fault_Handler(const ERRORFLINGER_FAULT_REASON_T reason)
{
    //reason
}

static void ErrorFlinger_HardFault_Handler(void)
{
    ErrorFlinger_Fault_Handler(ERRORFLINGER_FAULT_REASON_HARD);
}

static void ErrorFlinger_MemoryManagement_Handler(void)
{
    ErrorFlinger_Fault_Handler(ERRORFLINGER_FAULT_REASON_MEMORYMANAGEMENT);
}

static void ErrorFlinger_BusFault_Handler(void)
{
    ErrorFlinger_Fault_Handler(ERRORFLINGER_FAULT_REASON_BUS);
}

static void ErrorFlinger_UsageFault_Handler(void)
{
    ErrorFlinger_Fault_Handler(ERRORFLINGER_FAULT_REASON_USAGE);
}

static void ErrorFlinger_NMI_Handler(void)
{
    ErrorFlinger_Fault_Handler(ERRORFLINGER_FAULT_REASON_NMI);
}
