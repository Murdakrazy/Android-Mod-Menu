#include <list>
#include <vector>
#include <string.h>
#include <pthread.h>
#include <thread>
#include <cstring>
#include <jni.h>
#include <unistd.h>
#include <fstream>
#include <iostream>
#include <dlfcn.h>
#include "Includes/Logger.h"
#include "Includes/obfuscate.h"
#include "Includes/Utils.h"
#include "KittyMemory/MemoryPatch.h"
#include "Menu/Setup.h"

#define targetLibName OBFUSCATE("libil2cpp.so")
#include "Includes/Macros.h"

// --- VARIABLES DE CONTROL ---
bool freePurchase = false;
bool godMode = false;
bool isFlying = false;      
bool controlNPC = false;    
float speedValue = 1.0f;

// --- SECCIÓN DE HOOKS ---

// Hook Compras
bool (*old_OnPurchased)(void *instance);
bool OnPurchased(void *instance) {
    if (instance != NULL && freePurchase) return true; 
    return old_OnPurchased(instance);
}

// Hook Volar
void (*old_FlightUpdate)(void *instance);
void FlightUpdate(void *instance) {
    if (instance != NULL && isFlying) {
        return; 
    }
    old_FlightUpdate(instance);
}

// Hook Control NPC
void (*old_NPCLogic)(void *instance);
void NPCLogic(void *instance) {
    if (instance != NULL && controlNPC) {
        return; 
    }
    old_NPCLogic(instance);
}

// Hilo de ejecución
void *hack_thread(void *) {
    do { sleep(1); } while (!isLibraryLoaded(targetLibName));

    // --- CORRECCIÓN AQUÍ ---
    // Se agregaron los offsets faltantes en las primeras líneas de cada bloque
#if defined(__aarch64__)
    // Compras (Offset: 0x1806A7C)
    A64HookFunction((void *)getAbsoluteAddress(targetLibName, 0x1806A7C), (void *)OnPurchased, (void **)&old_OnPurchased);
    // Volar (0x8C4EA8)
    A64HookFunction((void *)getAbsoluteAddress(targetLibName, 0x8C4EA8), (void *)FlightUpdate, (void **)&old_FlightUpdate);
    // Control NPC (0x9E23D4)
    A64HookFunction((void *)getAbsoluteAddress(targetLibName, 0x9E23D4), (void *)NPCLogic, (void **)&old_NPCLogic);
#else
    // Compras (Offset: 0x1806A7C)
    MSHookFunction((void *)getAbsoluteAddress(targetLibName, 0x1806A7C), (void *)OnPurchased, (void **)&old_OnPurchased);
    MSHookFunction((void *)getAbsoluteAddress(targetLibName, 0x8C4EA8), (void *)FlightUpdate, (void **)&old_FlightUpdate);
    MSHookFunction((void *)getAbsoluteAddress(targetLibName, 0x9E23D4), (void *)NPCLogic, (void **)&old_NPCLogic);
#endif

    return NULL;
}

// --- CONFIGURACIÓN DEL MENÚ ---

jobjectArray GetFeatureList(JNIEnv *env, jobject context) {
    jobjectArray ret;
    const char *features[] = {
            OBFUSCATE("Category_MOD MENU"),
            OBFUSCATE("10_Toggle_Free Purchase"),
            OBFUSCATE("11_Toggle_Volar (Fly Mode)"),
            OBFUSCATE("12_Toggle_Controlar NPCs"),
            OBFUSCATE("13_SeekBar_Velocidad_1_20"),
    };

    int Total_Feature = (sizeof features / sizeof features[0]);
    ret = (jobjectArray)env->NewObjectArray(Total_Feature, env->FindClass(OBFUSCATE("java/lang/String")), env->NewStringUTF(""));

    for (int i = 0; i < Total_Feature; i++)
        env->SetObjectArrayElement(ret, i, env->NewStringUTF(features[i]));

    return (ret);
}

// --- MANEJADOR DE CAMBIOS ---

void Changes(JNIEnv *env, jclass clazz, jobject obj, jint featNum, jstring featName, jint value, jboolean boolean, jstring str) {
    switch (featNum) {
        case 10: freePurchase = boolean; break;
        case 11: isFlying = boolean; break;
        case 12: controlNPC = boolean; break;
        case 13: speedValue = (float)value; break;
    }
}

// --- REGISTROS JNI ---

__attribute__((constructor))
void lib_main() {
    pthread_t ptid;
    pthread_create(&ptid, NULL, hack_thread, NULL);
}

int RegisterMenu(JNIEnv *env) {
    JNINativeMethod methods[] = {
            {OBFUSCATE("Icon"), OBFUSCATE("()Ljava/lang/String;"), reinterpret_cast<void *>(Icon)},
            {OBFUSCATE("IconWebViewData"),  OBFUSCATE("()Ljava/lang/String;"), reinterpret_cast<void *>(IconWebViewData)},
            {OBFUSCATE("IsGameLibLoaded"),  OBFUSCATE("()Z"), reinterpret_cast<void *>(isGameLibLoaded)},
            {OBFUSCATE("Init"),  OBFUSCATE("(Landroid/content/Context;Landroid/widget/TextView;Landroid/widget/TextView;)V"), reinterpret_cast<void *>(Init)},
            {OBFUSCATE("SettingsList"),  OBFUSCATE("()[Ljava/lang/String;"), reinterpret_cast<void *>(SettingsList)},
            {OBFUSCATE("GetFeatureList"),  OBFUSCATE("()[Ljava/lang/String;"), reinterpret_cast<void *>(GetFeatureList)},
    };
    jclass clazz = env->FindClass(OBFUSCATE("com/android/support/Menu"));
    if (!clazz) return JNI_ERR;
    env->RegisterNatives(clazz, methods, sizeof(methods) / sizeof(methods[0]));
    return JNI_OK;
}

int RegisterPreferences(JNIEnv *env) {
    JNINativeMethod methods[] = {
            {OBFUSCATE("Changes"), OBFUSCATE("(Landroid/content/Context;ILjava/lang/String;IZLjava/lang/String;)V"), reinterpret_cast<void *>(Changes)},
    };
    jclass clazz = env->FindClass(OBFUSCATE("com/android/support/Preferences"));
    if (!clazz) return JNI_ERR;
    env->RegisterNatives(clazz, methods, sizeof(methods) / sizeof(methods[0]));
    return JNI_OK;
}

int RegisterMain(JNIEnv *env) {
    JNINativeMethod methods[] = {
            {OBFUSCATE("CheckOverlayPermission"), OBFUSCATE("(Landroid/content/Context;)V"), reinterpret_cast<void *>(CheckOverlayPermission)},
    };
    jclass clazz = env->FindClass(OBFUSCATE("com/android/support/Main"));
    if (!clazz) return JNI_ERR;
    env->RegisterNatives(clazz, methods, sizeof(methods) / sizeof(methods[0]));
    return JNI_OK;
}

extern "C" JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved) {
    JNIEnv *env;
    vm->GetEnv((void **) &env, JNI_VERSION_1_6);
    RegisterMenu(env);
    RegisterPreferences(env);
    RegisterMain(env);
    return JNI_VERSION_1_6;
}
