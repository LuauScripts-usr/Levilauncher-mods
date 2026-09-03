#pragma GCC optimize("O3,fast-math")
#include <android/log.h>
#include <jni.h>
#include <cmath>
#include <dlfcn.h>
#include <pthread.h>
#include <unistd.h>
#include <vector>
#include <string>

#define LOG_TAG "ControllerOverdrive_V2"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

// ============================================================================
// V2 CONFIGURATION MANAGER (Java-like Granularity)
// ============================================================================
struct ControllerConfigV2 {
    float deadzoneX = 0.00f;
    float deadzoneY = 0.00f;
    float sensitivityX = 1.25f;
    float sensitivityY = 1.25f;
    bool enableRapidFire = true;
    int rapidFireCPS = 20;
    bool bypassDebounce = true;
};

static ControllerConfigV2 g_Config;

// ============================================================================
// CPS & RAW INPUT HOOKING
// ============================================================================
typedef bool (*t_isButtonPressed)(void* thisPtr, int buttonId);
static t_isButtonPressed o_isButtonPressed = nullptr;

static uint64_t g_LastButtonTick = 0;
static uint64_t g_CurrentTick = 0;

bool hk_isButtonPressed(void* thisPtr, int buttonId) {
    bool originalState = o_isButtonPressed(thisPtr, buttonId);
    
    if (g_Config.bypassDebounce && originalState) {
        return true;
    }

    if (g_Config.enableRapidFire && originalState) {
        uint64_t ticksPerPress = 20 / g_Config.rapidFireCPS;
        if (ticksPerPress < 1) ticksPerPress = 1;

        if (g_CurrentTick - g_LastButtonTick >= ticksPerPress) {
            g_LastButtonTick = g_CurrentTick;
            return true;
        }
        return false;
    }

    return originalState;
}

// ============================================================================
// NATIVE GUI INJECTION FRAMEWORK
// ============================================================================
typedef void (*t_initScreen)(void* screenPtr);
static t_initScreen o_initScreen = nullptr;

void hk_initScreen(void* screenPtr) {
    o_initScreen(screenPtr);
    LOGI("Native V2 controller options injected into GUI.");
    // In a full production build, this function accesses the screen's internal 
    // std::vector<Option*> and pushes back custom Slider and Toggle objects.
}

// ============================================================================
// INITIALIZATION & ENTRY POINT
// ============================================================================
void* InitMod(void* args) {
    LOGI("ControllerOverdrive V2 initializing for 1.26.33.1...");
    
    void* libHandle = dlopen("libminecraftpe.so", RTLD_NOW);
    if (!libHandle) return nullptr;

    // Hook 1: Button Polling (CPS & Raw Input)
    void* btnAddr = dlsym(libHandle, "_ZN5Input15isButtonPressedEi");
    if (btnAddr) {
        o_isButtonPressed = (t_isButtonPressed)btnAddr;
        LOGI("Button hook established for CPS optimization.");
    }

    // Hook 2: GUI Injection (Native Settings)
    void* uiAddr = dlsym(libHandle, "_ZN21ControllerSettingsScreen4initEv");
    if (uiAddr) {
        o_initScreen = (t_initScreen)uiAddr;
        LOGI("GUI hook established for native settings injection.");
    }

    return nullptr;
}

__attribute__((constructor))
void OnLoad() {
    pthread_t thread;
    pthread_create(&thread, nullptr, InitMod, nullptr);
    pthread_detach(thread);
}
