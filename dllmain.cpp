#include <windows.h>
#include <jni.h>

DWORD WINAPI InitHile(LPVOID lpParam) {
    JavaVM* jvm;
    JNIEnv* env;
    jsize nVMs;

    // Çalışan Minecraft JVM'ini bul
    if (JNI_GetCreatedJavaVMs(&jvm, 1, &nVMs) != JNI_OK || nVMs == 0) return 1;

    jvm->AttachCurrentThread((void**)&env, NULL);

    // Java sınıfını bul ve main metodunu çağır
    jclass cls = env->FindClass("HileModul");
    if (cls != nullptr) {
        jmethodID mid = env->GetStaticMethodID(cls, "main", "([Ljava/lang/String;)V");
        if (mid != nullptr) {
            env->CallStaticVoidMethod(cls, mid, NULL);
        }
    }
    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    if (ul_reason_for_call == DLL_PROCESS_ATTACH) {
        CreateThread(NULL, 0, InitHile, NULL, 0, NULL);
    }
    return TRUE;
}

