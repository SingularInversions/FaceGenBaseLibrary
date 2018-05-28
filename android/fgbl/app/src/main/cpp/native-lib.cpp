#include <jni.h>
#include <string>
#include <FgCommand.hpp>

void fgGeometryTest(const FgArgs &);

extern "C" JNIEXPORT jstring

JNICALL
Java_com_facegen_fgbl_MainActivity_stringFromJNI(JNIEnv *env,jobject /* this */) {

    std::string         hello;
    try {
        // We run this test because it doesn't create any temp files, which isn't yet set up to work with Android:
        fgGeometryTest(FgArgs());
    }
    catch (...) {
        hello = "Fgbl test failed.";
    }
    hello = "Fgbl test passed.";
    return env->NewStringUTF(hello.c_str());
}
