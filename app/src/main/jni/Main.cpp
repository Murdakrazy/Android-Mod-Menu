#include <jni.h>
#include <pthread.h>
#include <cstring>
#include <unistd.h>
#include "Includes/Logger.h"
#include "Includes/obfuscate.h"
#include "Includes/Utils.h"

#define targetLibName OBFUSCATE("libil2cpp.so")

// --- VARIABLE DEL BOTÓN ---
bool freePurchase = false;

// --- HOOK ---
bool (*old_OnPurchased)(void *instance);
bool OnPurchased(void *instance) {
    if (instance != NULL && freePurchase) {
        return true; 
    }
    return old_OnPurchased(instance);
}

void *hack_thread(void *) {
    do { sleep(1); } while (!isLibraryLoaded(targetLibName));
    
    // El offset que me pasaste: 0x1806A7C
    #if defined(__aarch64__)
        A64HookFunction((void *)getAbsoluteAddress(targetLibName, 0x1806A7C), (void *)OnPurchased, (void **)&old_OnPurchased);
    #else
        MSHookFunction((void *)getAbsoluteAddress(targetLibName, 0x1806A7C), (void *)OnPurchased, (void **)&old_OnPurchased);
    #endif
    return NULL;
}

// --- MENÚ LGL ---
extern "C" JNIEXPORT jobjectArray JNICALL Java_com_android_support_Menu_GetFeatureList(JNIEnv *env, jobject context) {
    jobjectArray ret;
    const char *features[] = {
            OBFUSCATE("Category_Free Shopping"),
            OBFUSCATE("10_Toggle_Free Purchase"), // ID 10
    };
    int Total_Feature = (sizeof features / sizeof features[0]);
    ret = (jobjectArray)env->NewObjectArray(Total_Feature, env->FindClass("java/lang/String"), env->NewStringUTF(""));
    for (int i = 0; i < Total_Feature; i++) env->SetObjectArrayElement(ret, i, env->NewStringUTF(features[i]));
    return ret;
}

extern "C" JNIEXPORT void JNICALL Java_com_android_support_Preferences_Changes(JNIEnv *env, jclass clazz, jobject obj, jint featNum, jstring featName, jint value, jboolean boolean, jstring str) {
    if (featNum == 10) {
        freePurchase = boolean;
    }
}

__attribute__((constructor)) void lib_main() {
    pthread_t ptid;
    pthread_create(&ptid, NULL, hack_thread, NULL);
}
