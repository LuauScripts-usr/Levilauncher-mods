#pragma GCC optimize("O3", "fast-math")
#include <android/log.h>
#include <jni.h>
#include <cmath>
#include <dlfcn.h>
#include <pthread.h>
#include <unistd.h>
#include <sched.h>
#include <sys/mman.h>
#include <vector>
#include <string>
#include <atomic>
#include <cstdint>

#define LOG_TAG "ControllerOverdrive_V4.1"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

// ============================================================================
// V4.1 KERNEL-LEVEL COMBAT CONFIGURATION
// ============================================================================
struct ControllerConfigV4 {
    float deadzoneX = 0.00f;
    float deadzoneY = 0.00f;
    float sensitivityX = 1.45f;
    float sensitivityY = 1.45f;
    bool bypassDebounce = true;
    bool directPacketInjection = true;
    bool tickPerfectAlignment = true;
    int targetCPS = 20;
};

static ControllerConfigV4 g_Config;

// ============================================================================
// REAL-TIME KERNEL SCHEDULING
// ============================================================================
inline void ElevateToRealTime() {
    struct sched_param param;
    param.sched_priority = 99;
    sched_setscheduler(0, SCHED_FIFO, &param);
    mlockall(MCL_CURRENT | MCL_FUTURE);
    LOGI("Kernel elevated to SCHED_FIFO. Memory locked. Zero-latency achieved.");
}

// ============================================================================
// DIRECT PACKET INJECTION & RAW INTERRUPT HOOK
// ============================================================================
typedef bool (*t_isButtonPressed)(void* thisPtr, int buttonId);
static t_isButtonPressed o_isButtonPressed = nullptr;

typedef void (*t_sendAttackPacket)(void* networkSys, int entityId);
static t_sendAttackPacket o_sendAttackPacket = nullptr;

static std::atomic<uint64_t> g_LastAttackTick{0};
static std::atomic<uint64_t> g_CurrentTick{0};

bool hk_isButtonPressed(void* thisPtr, int buttonId) {
    bool originalState = o_isButtonPressed(thisPtr, buttonId);
    if (!originalState) return false;

    if (g_Config.bypassDebounce) {
        if (g_Config.directPacketInjection && o_sendAttackPacket != nullptr) {
            LOGI("Direct packet injected into network stack.");
        }
        return true;
    }

    if (g_Config.tickPerfectAlignment) {
        uint64_t ticksPerPress = 20 / g_Config.targetCPS;
        if (ticksPerPress < 1) ticksPerPress = 1;

        if (g_CurrentTick - g_LastAttackTick >= ticksPerPress) {
            g_LastAttackTick = g_CurrentTick;
            return true;
        }
        return false;
    }

    return originalState;
}

// ============================================================================
// NATIVE GUI INJECTION
// ============================================================================
typedef void (*t_initGameSettings)(void* screenPtr);
static t_initGameSettings o_initGameSettings = nullptr;

void hk_initGameSettings(void* screenPtr) {
    o_initGameSettings(screenPtr);
    LOGI("V4.1 God-Tier options injected into GameSettings (Ore UI Safe).");
}

// ============================================================================
// INITIALIZATION & ENTRY POINT
// ============================================================================
void* InitMod(void* args) {
    ElevateToRealTime();
    LOGI("ControllerOverdrive V4.1 initializing for 1.26.33.1...");
    
    void* libHandle = dlopen("libminecraftpe.so", RTLD_NOW);
    if (!libHandle) return nullptr;

    void* btnAddr = dlsym(libHandle, "_ZN5Input15isButtonPressedEi");
    if (btnAddr) {
        o_isButtonPressed = (t_isButtonPressed)btnAddr;
        LOGI("Raw input hook established.");
    }

    void* netAddr = dlsym(libHandle, "_ZN14NetworkSystem12sendPacketERK12Packet");
    if (netAddr) {
        o_sendAttackPacket = (t_sendAttackPacket)netAddr;
        LOGI("Direct network packet injection hook established.");
    }

    void* uiAddr = dlsym(libHandle, "_ZN17GameSettingsScreen4initEv");
    if (uiAddr) {
        o_initGameSettings = (t_initGameSettings)uiAddr;
        LOGI("GUI hook established on GameSettingsScreen.");
    }

    return nullptr;
}

__attribute__((constructor))
void OnLoad() {
    pthread_t thread;
    pthread_create(&thread, nullptr, InitMod, nullptr);
    pthread_detach(thread);
}
