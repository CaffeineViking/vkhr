#ifndef VKHR_DEFINITION_HH
#define VKHR_DEFINITION_HH

#ifndef SHARE_PATH
#define SHARE_PATH "share/"
#endif

// e.g shared path could be /usr/share/vkhr/
// need to supply SHARE_PATH at compile time

#define SHARED(PATH) SHARE_PATH        PATH

#define IMAGE(PATH)  SHARED("images/"  PATH)
#define MODEL(PATH)  SHARED("models/"  PATH)
#define SCENE(PATH)  SHARED("scenes/"  PATH)
#define SHADER(PATH) SHARED("shaders/" PATH)
#define STYLE(PATH)  SHARED("styles/"  PATH)

#endif
