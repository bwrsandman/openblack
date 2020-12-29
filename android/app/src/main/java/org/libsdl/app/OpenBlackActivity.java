package org.libsdl.app;

import org.libsdl.app.SDLActivity;

public class OpenBlackActivity extends SDLActivity {

    /**
     * This method returns the name of the shared object with the application entry point
     * It can be overridden by derived classes.
     */
    protected String getMainSharedObject() {
        String library;
        String[] libraries = SDLActivity.mSingleton.getLibraries();
        if (libraries.length > 0) {
            library = "lib" + libraries[libraries.length - 1] + ".so";
        } else {
            library = "libopenblack.so";
        }
        return getContext().getApplicationInfo().nativeLibraryDir + "/" + library;
    }

    /**
     * This method returns the name of the application entry point
     * It can be overridden by derived classes.
     */
    protected String getMainFunction() {
        // Will call main()
        return "SDL_main";
    }

    /**
     * This method is called by SDL before starting the native application thread.
     * It can be overridden to provide the arguments after the application name.
     * The default implementation returns an empty array. It never returns null.
     *
     * @return arguments for the native application.
     */
    protected String[] getArguments() {
        // TODO(bwrsandman): Add arguments such as renderer and path to game resources
        return new String[]{
                "--log-file", "logcat",
                //"--backend-type", "Vulkan",
                "--backend-type", "OpenGLES",
                "-g", "butts",
        };
    }

    /**
     * This method is called by SDL before loading the native shared libraries.
     * It can be overridden to provide names of shared libraries to be loaded.
     * The default implementation returns the defaults. It never returns null.
     * An array returned by a new implementation must at least contain "SDL2".
     * Also keep in mind that the order the libraries are loaded may matter.
     *
     * @return names of shared libraries to be loaded (e.g. "SDL2", "main").
     */
    protected String[] getLibraries() {
        return new String[]{
                "hidapi",
                "SDL2",
                // "SDL2_image",
                // "SDL2_mixer",
                // "SDL2_net",
                // "SDL2_ttf",
                "openblack"
        };
    }
}
