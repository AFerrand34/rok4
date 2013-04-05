/* DO NOT EDIT THIS FILE - it is machine generated */
#include <jni.h>
/* Header for class org_proj4_Projections */

#ifndef _Included_org_proj4_Projections
#define _Included_org_proj4_Projections
#ifdef __cplusplus
extern "C" {
#endif
    /*
     * Class:     org_proj4_Projections
     * Method:    getProjInfo
     * Signature: (Ljava/lang/String;)Ljava/lang/String;
     */
    JNIEXPORT jstring JNICALL Java_org_proj4_Projections_getProjInfo
    ( JNIEnv *, jobject, jstring );

    /*
     * Class:     org_proj4_Projections
     * Method:    getEllipsInfo
     * Signature: (Ljava/lang/String;)Ljava/lang/String;
     */
    JNIEXPORT jstring JNICALL Java_org_proj4_Projections_getEllipsInfo
    ( JNIEnv *, jobject, jstring );

    /*
     * Class:     org_proj4_Projections
     * Method:    transform
     * Signature: ([D[D[DLjava/lang/String;Ljava/lang/String;JI)V
     */
    JNIEXPORT void JNICALL Java_org_proj4_Projections_transform
    ( JNIEnv *, jobject, jdoubleArray, jdoubleArray, jdoubleArray, jstring, jstring, jlong, jint );

#ifdef __cplusplus
}
#endif
#endif
