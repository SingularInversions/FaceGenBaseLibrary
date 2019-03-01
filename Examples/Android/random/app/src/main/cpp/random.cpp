#include <jni.h>

#include "FgRandom.hpp"

extern "C"
JNIEXPORT jdouble JNICALL
Java_com_example_random_Random_getRandom( JNIEnv* env,
                                          jobject thiz )
{
    return fgRand();
}
