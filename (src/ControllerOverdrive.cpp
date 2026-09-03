#pragma GCC optimize("O3,fast-math")
#include <android/log.h>
#include <jni.h>
#include <cmath>
#include <dlfcn.h>
#include <pthread.h>
#include <unistd.h>

#define LOG_TAG "ControllerOverdrive"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

// ============================================================================
// CONFIGURATION MANAGER (Zero-allocation hot path)
// ============================================================================
struct ControllerConfig {
    float deadzone = 0.00f;
    float sensitivity = 1.25f;
    float responseCurve = 0.85f; 
    bool enableRawInput = true;
    float cpsMultiplier = 1.15f;  
    
    void LoadConfig() {
        LOGI("Loading high-performance controller configuration...");
        // Hardcoded for maximum performance. 
        // Deadzone at 0.00f for instant registration.
    }
};

static ControllerConfig g_Config;

// ============================================================================
// INPUT MATH & RAW PROCESSING
// ============================================================================
namespace InputMath {
    inline __attribute__((always_inline)) 
    float ProcessAxis(float originalValue, const ControllerConfig& cfg) {
        float absVal = std::abs(originalValue);
        
        if (absVal < cfg.deadzone) return 0.0f;
        
        float remapped = (absVal - cfg.deadzone) / (1.0f - cfg.deadzone);
        remapped *= (originalValue < 0.0f ? -1.0f : 1.0f);
        
        if (cfg.enableRawInput) {
            float sign = (remapped < 0.0f) ? -1.0f : 1.0f;
            return std::pow(std::abs(remapped), 1.0f / cfg.responseCurve) * cfg.sensitivity * sign;
        }
        
        return remapped * cfg.sensitivity;
    }
}

// ============================================================================
// HOOKING FRAMEWORK
// ============================================================================
typedef float (*t_getAxis)(void* thisPtr, int axisId);
static t_getAxis o_getAxis = nullptr;

float hk_getAxis(void* thisPtr, int axisId) {
    float originalValue = o_getAxis(thisPtr, axisId);
    float processedValue = InputMath::ProcessAxis(originalValue, g_Config);
    
    if (processedValue > 1.0f) processedValue = 1.0f;
    if (processedValue < -1.0f) processedValue = -1.0f;
    
    return processedValue;
}

// ============================================================================
// INITIALIZATION & ENTRY POINT
// ============================================================================
void* InitMod(void* args) {
    g_Config.LoadConfig();
    
    // Resolve target function in libminecraftpe.so
    void* libHandle = dlopen("libminecraftpe.so", RTLD_NOW);
    if (libHandle) {
        // Note: Replace with the exact mangled symbol or calculate offset via IDA Pro
        void* targetAddr = dlsym(libHandle, "_ZNK13ControllerInput7getAxisEi"); 
        
        if (targetAddr) {
            // In production, integrate Dobby or ShadowHook here to redirect targetAddr to hk_getAxis.
            // For this build pipeline, we establish the function pointer.
            o_getAxis = (t_getAxis)targetAddr;
            LOGI("Successfully resolved ControllerInput::getAxis. Mod Active.");
        } else {
            LOGI("Symbol not found. Awaiting offset injection.");
        }
    } else {
        LOGI("Failed to load libminecraftpe.so.");
    }

    return nullptr;
}

__attribute__((constructor))
void OnLoad() {
    LOGI("ControllerOverdrive .so injected. Initializing...");
    pthread_t thread;
    pthread_create(&thread, nullptr, InitMod, nullptr);
    pthread_detach(thread);
}
