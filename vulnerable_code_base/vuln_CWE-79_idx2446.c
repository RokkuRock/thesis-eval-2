void OpenURL(const char *url)
{
    if (strchr(url, '\'') != NULL)
    {
        TRACELOG(LOG_WARNING, "SYSTEM: Provided URL is not valid");
    }
    else
    {
#if defined(PLATFORM_DESKTOP)
        char *cmd = (char *)RL_CALLOC(strlen(url) + 32, sizeof(char));
    #if defined(_WIN32)
        sprintf(cmd, "explorer \"%s\"", url);
    #endif
    #if defined(__linux__) || defined(__FreeBSD__) || defined(__OpenBSD__)
        sprintf(cmd, "xdg-open '%s'", url);  
    #endif
    #if defined(__APPLE__)
        sprintf(cmd, "open '%s'", url);
    #endif
        int result = system(cmd);
        if (result == -1) TRACELOG(LOG_WARNING, "OpenURL() child process could not be created");
        RL_FREE(cmd);
#endif
#if defined(PLATFORM_WEB)
        emscripten_run_script(TextFormat("window.open('%s', '_blank')", url));
#endif
#if defined(PLATFORM_ANDROID)
        JNIEnv *env = NULL;
        JavaVM *vm = CORE.Android.app->activity->vm;
        (*vm)->AttachCurrentThread(vm, &env, NULL);
        jstring urlString = (*env)->NewStringUTF(env, url);
        jclass uriClass = (*env)->FindClass(env, "android/net/Uri");
        jmethodID uriParse = (*env)->GetStaticMethodID(env, uriClass, "parse", "(Ljava/lang/String;)Landroid/net/Uri;");
        jobject uri = (*env)->CallStaticObjectMethod(env, uriClass, uriParse, urlString);
        jclass intentClass = (*env)->FindClass(env, "android/content/Intent");
        jfieldID actionViewId = (*env)->GetStaticFieldID(env, intentClass, "ACTION_VIEW", "Ljava/lang/String;");
        jobject actionView = (*env)->GetStaticObjectField(env, intentClass, actionViewId);
        jmethodID newIntent = (*env)->GetMethodID(env, intentClass, "<init>", "(Ljava/lang/String;Landroid/net/Uri;)V");
        jobject intent = (*env)->AllocObject(env, intentClass);
        (*env)->CallVoidMethod(env, intent, newIntent, actionView, uri);
        jclass activityClass = (*env)->FindClass(env, "android/app/Activity");
        jmethodID startActivity = (*env)->GetMethodID(env, activityClass, "startActivity", "(Landroid/content/Intent;)V");
        (*env)->CallVoidMethod(env, CORE.Android.app->activity->clazz, startActivity, intent);
        (*vm)->DetachCurrentThread(vm);
#endif
    }
}