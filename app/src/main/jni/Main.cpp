#include <jni.h>
#include <string>

// Código ultra básico para probar si el compilador funciona
extern "C" JNIEXPORT jobjectArray JNICALL 
Java_com_android_support_Menu_GetFeatureList(JNIEnv *env, jobject context) {
    jobjectArray ret;
    const char *features[] = {"0_Toggle_Prueba de Mod", "1_Button_Si ves esto funciona"};
    int Total_Feature = 2;
    ret = (jobjectArray)env->NewObjectArray(Total_Feature, env->FindClass("java/lang/String"), env->NewStringUTF(""));
    for (int i = 0; i < Total_Feature; i++) env->SetObjectArrayElement(ret, i, env->NewStringUTF(features[i]));
    return ret;
}

extern "C" JNIEXPORT void JNICALL 
Java_com_android_support_Preferences_Changes(JNIEnv *env, jclass clazz, jobject obj, jint featNum, jstring featName, jint value, jboolean boolean, jstring str) {
    // Aquí iría la lógica después
}
