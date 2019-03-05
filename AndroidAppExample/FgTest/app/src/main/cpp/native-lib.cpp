#include <jni.h>
#include <string>

#include <FgRandom.hpp>

extern "C" JNIEXPORT jstring JNICALL
Java_com_example_fgtest_MainActivity_stringFromJNI(
        JNIEnv *env,
        jobject /* this */) {
    std::string hello = "Hello from C++" + std::to_string( fgRand() );;
    return env->NewStringUTF(hello.c_str());
}
