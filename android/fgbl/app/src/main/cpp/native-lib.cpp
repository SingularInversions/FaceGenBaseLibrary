#include <android/asset_manager_jni.h>
#include <jni.h>
#include <string>
#include <FgCommand.hpp>

#define BUFSIZE 30

void fgGeometryTest(const FgArgs &);

extern "C" JNIEXPORT jstring

JNICALL
Java_com_facegen_fgbl_MainActivity_stringFromJNI(JNIEnv *env, jobject /* this */, jobject _assetManager) {

    std::string         hello;
    char                buf[BUFSIZE];

    AAssetManager *assetManager = AAssetManager_fromJava(env, _assetManager);
    AAsset *asset = AAssetManager_open(assetManager, "hello.txt", AASSET_MODE_BUFFER);
    buf[AAsset_read(asset, buf, BUFSIZE)] = 0;
    AAsset_close(asset);

    hello = buf;
    try {
        // We run this test because it doesn't create any temp files, which isn't yet set up to work with Android:
        fgGeometryTest(FgArgs());
    }
    catch (...) {
        hello += ". Fgbl test failed.";
    }
    hello += ". Fgbl test passed.";
    return env->NewStringUTF(hello.c_str());
}
