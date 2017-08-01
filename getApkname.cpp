#include <stdio.h>

#include <jni.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <string.h>
#include "invoke.h"
#include "getApkname.h"
#define LINE_MAX (128)

static char apkname[LINE_MAX] = {0};         //apkname

JavaVM* _jvm = NULL;
JNIEnv* env =  NULL;
JavaVM* JVM(){
	if (_jvm == NULL)
	{
	    dlerror();
		void* handle = dlopen("/system/lib/libandroid_runtime.so", RTLD_NOW);
		if (handle)
		{
			_jvm = *(JavaVM**)dlsym(handle, "_ZN7android14AndroidRuntime7mJavaVME");
			LOGD("JVM JavaVM = %p", _jvm);
			dlclose(handle);
		}
		else
			LOGE("JVM dlopen libandroid_runtime.so failed: %s", dlerror());
	}
	return _jvm;
}

JNIEnv* cacheEnv(JavaVM* jvm){
	JNIEnv* _env = NULL;
	// get jni environment
	jint ret = jvm->GetEnv((void**)&_env, JNI_VERSION_1_4);
	switch (ret) {
        case JNI_OK :
             //Success!
            //pthread_setspecific(g_key, _env);
            break;
        case JNI_EDETACHED :
            // Thread not attached
            if (jvm->AttachCurrentThread(&_env, NULL) < 0)
                LOGD("Failed to get the environment using AttachCurrentThread()");
            //else
                //pthread_setspecific(g_key, _env); // Success : Attached and obtained JNIEnv!
            break;
        case JNI_EVERSION :
            // Cannot recover from this error
            LOGD("JNI interface version 1.4 not supported");
            break;
        default :
            LOGD("Failed to get the environment using ENV()");
            break;
	}
	return _env;
}

JNIEnv* ENV(){
    JNIEnv* _env =  NULL;
	if (_env == NULL){
        _env = cacheEnv(JVM());
	}
	return _env;
}



static char* getApkname(JNIEnv *env) {
    jobject jCurrentActivityThread = NULL;
	int okey = 0;
	okey = invoke_func()->callStaticObjectMethod(env, &jCurrentActivityThread,
			"android/app/ActivityThread", "()Landroid/app/ActivityThread;",
			"currentActivityThread");
	if (!okey || !jCurrentActivityThread) {
		LOGE("getCurrentActivityThread null！");
		return 0;
	}
    jobject jboundApplication = NULL;
    invoke_func()->getObject(env, &jboundApplication, "android/app/ActivityThread",
			jCurrentActivityThread, "mBoundApplication", "Landroid/app/ActivityThread$AppBindData;");



	if (!jboundApplication) {
		LOGE("jboundApplication NULL");
		return 0;
	}

    LOGD("jboundApplication:%p",jboundApplication);
    jobject jLoadedApk = NULL;
    okey = invoke_func()->getObject(env, &jLoadedApk, "android/app/ActivityThread$AppBindData",
			jboundApplication, "info", "Landroid/app/LoadedApk;");
    if (!jLoadedApk) {
		LOGE("jLoadedApk NULL");
		return 0;
	}

    jobject mPackageName = NULL;
    okey = invoke_func()->getObject(env, &mPackageName, "android/app/LoadedApk",
			jLoadedApk, "mPackageName", "Ljava/lang/String;");
    if (!mPackageName) {
		LOGE("jLoadedApk NULL");
		return 0;
	}
	char* packagename = invoke_func()->tocstr(env,(jstring)mPackageName);
	LOGD("mPackageName:%s",packagename);
	return packagename;
}

int getPackagePath(char* ApkDataPaths){
    const char* datadir = "data/data/";
    char* jApkname = getApkname(ENV());
    if(jApkname==NULL){
        LOGE("jApkname is null");
        exit(-1);
    }
    //char* soPath = NULL;
    //soPath = (char*)malloc(sizeof(char)*LINE_MAX);
    //memset(soPath,0,sizeof(char)*LINE_MAX);
    sprintf(ApkDataPaths,"%s%s",datadir,jApkname);
    //soPath = strncpy(soPath, datadir, strlen(datadir)+1);
    //soPath = strncat(soPath, jApkname, strlen(jApkname)+1);
    return 0;
}

