const char *GetClipboardText(void)
{
#if defined(PLATFORM_DESKTOP)
    return glfwGetClipboardString(CORE.Window.handle);
#endif
#if defined(PLATFORM_WEB)
    emscripten_run_script_string("navigator.clipboard.readText() \
        .then(text => { document.getElementById('clipboard').innerText = text; console.log('Pasted content: ', text); }) \
        .catch(err => { console.error('Failed to read clipboard contents: ', err); });"
    );
    return NULL;
#endif
    return NULL;
}