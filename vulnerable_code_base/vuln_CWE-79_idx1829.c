void SetClipboardText(const char *text)
{
#if defined(PLATFORM_DESKTOP)
    glfwSetClipboardString(CORE.Window.handle, text);
#endif
#if defined(PLATFORM_WEB)
    emscripten_run_script(TextFormat("navigator.clipboard.writeText('%s')", text));
#endif
}