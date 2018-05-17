#include <jni.h>
#include <string>

using namespace std;

extern "C" JNIEXPORT jstring

JNICALL
Java_com_facegen_empty_MainActivity_stringFromJNI(JNIEnv *env,jobject /* this */)
{
    string          hello = "fgbl - Hello World";
    return env->NewStringUTF(hello.c_str());
}
