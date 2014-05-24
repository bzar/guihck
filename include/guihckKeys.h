#ifndef GUIHCK_KEYS
#define GUIHCK_KEYS


/* NOTICE: Keyboard codes correspond to GLFW. GLFW key events should be
 * directly forwardable to guihck. */


/*! @name Key and button actions
 *  @{ */
/*! @brief The key or button was released.
 *  @ingroup input
 */
#define GUIHCK_RELEASE                0
/*! @brief The key or button was pressed.
 *  @ingroup input
 */
#define GUIHCK_PRESS                  1
/*! @brief The key was held down until it repeated.
 *  @ingroup input
 */
#define GUIHCK_REPEAT                 2
/*! @} */

/*! @defgroup keys Keyboard keys
 *
 * These key codes are inspired by the *USB HID Usage Tables v1.12* (p. 53-60),
 * but re-arranged to map to 7-bit ASCII for printable keys (function keys are
 * put in the 256+ range).
 *
 * The naming of the key codes follow these rules:
 *  - The US keyboard layout is used
 *  - Names of printable alpha-numeric characters are used (e.g. "A", "R",
 *    "3", etc.)
 *  - For non-alphanumeric characters, Unicode:ish names are used (e.g.
 *    "COMMA", "LEFT_SQUARE_BRACKET", etc.). Note that some names do not
 *    correspond to the Unicode standard (usually for brevity)
 *  - Keys that lack a clear US mapping are named "WORLD_x"
 *  - For non-printable keys, custom names are used (e.g. "F4",
 *    "BACKSPACE", etc.)
 *
 *  @ingroup input
 *  @{
 */

/* The unknown key */
#define GUIHCK_KEY_UNKNOWN            -1

/* Printable keys */
#define GUIHCK_KEY_SPACE              32
#define GUIHCK_KEY_APOSTROPHE         39  /* ' */
#define GUIHCK_KEY_COMMA              44  /* , */
#define GUIHCK_KEY_MINUS              45  /* - */
#define GUIHCK_KEY_PERIOD             46  /* . */
#define GUIHCK_KEY_SLASH              47  /* / */
#define GUIHCK_KEY_0                  48
#define GUIHCK_KEY_1                  49
#define GUIHCK_KEY_2                  50
#define GUIHCK_KEY_3                  51
#define GUIHCK_KEY_4                  52
#define GUIHCK_KEY_5                  53
#define GUIHCK_KEY_6                  54
#define GUIHCK_KEY_7                  55
#define GUIHCK_KEY_8                  56
#define GUIHCK_KEY_9                  57
#define GUIHCK_KEY_SEMICOLON          59  /* ; */
#define GUIHCK_KEY_EQUAL              61  /* = */
#define GUIHCK_KEY_A                  65
#define GUIHCK_KEY_B                  66
#define GUIHCK_KEY_C                  67
#define GUIHCK_KEY_D                  68
#define GUIHCK_KEY_E                  69
#define GUIHCK_KEY_F                  70
#define GUIHCK_KEY_G                  71
#define GUIHCK_KEY_H                  72
#define GUIHCK_KEY_I                  73
#define GUIHCK_KEY_J                  74
#define GUIHCK_KEY_K                  75
#define GUIHCK_KEY_L                  76
#define GUIHCK_KEY_M                  77
#define GUIHCK_KEY_N                  78
#define GUIHCK_KEY_O                  79
#define GUIHCK_KEY_P                  80
#define GUIHCK_KEY_Q                  81
#define GUIHCK_KEY_R                  82
#define GUIHCK_KEY_S                  83
#define GUIHCK_KEY_T                  84
#define GUIHCK_KEY_U                  85
#define GUIHCK_KEY_V                  86
#define GUIHCK_KEY_W                  87
#define GUIHCK_KEY_X                  88
#define GUIHCK_KEY_Y                  89
#define GUIHCK_KEY_Z                  90
#define GUIHCK_KEY_LEFT_BRACKET       91  /* [ */
#define GUIHCK_KEY_BACKSLASH          92  /* \ */
#define GUIHCK_KEY_RIGHT_BRACKET      93  /* ] */
#define GUIHCK_KEY_GRAVE_ACCENT       96  /* ` */
#define GUIHCK_KEY_WORLD_1            161 /* non-US #1 */
#define GUIHCK_KEY_WORLD_2            162 /* non-US #2 */

/* Function keys */
#define GUIHCK_KEY_ESCAPE             256
#define GUIHCK_KEY_ENTER              257
#define GUIHCK_KEY_TAB                258
#define GUIHCK_KEY_BACKSPACE          259
#define GUIHCK_KEY_INSERT             260
#define GUIHCK_KEY_DELETE             261
#define GUIHCK_KEY_RIGHT              262
#define GUIHCK_KEY_LEFT               263
#define GUIHCK_KEY_DOWN               264
#define GUIHCK_KEY_UP                 265
#define GUIHCK_KEY_PAGE_UP            266
#define GUIHCK_KEY_PAGE_DOWN          267
#define GUIHCK_KEY_HOME               268
#define GUIHCK_KEY_END                269
#define GUIHCK_KEY_CAPS_LOCK          280
#define GUIHCK_KEY_SCROLL_LOCK        281
#define GUIHCK_KEY_NUM_LOCK           282
#define GUIHCK_KEY_PRINT_SCREEN       283
#define GUIHCK_KEY_PAUSE              284
#define GUIHCK_KEY_F1                 290
#define GUIHCK_KEY_F2                 291
#define GUIHCK_KEY_F3                 292
#define GUIHCK_KEY_F4                 293
#define GUIHCK_KEY_F5                 294
#define GUIHCK_KEY_F6                 295
#define GUIHCK_KEY_F7                 296
#define GUIHCK_KEY_F8                 297
#define GUIHCK_KEY_F9                 298
#define GUIHCK_KEY_F10                299
#define GUIHCK_KEY_F11                300
#define GUIHCK_KEY_F12                301
#define GUIHCK_KEY_F13                302
#define GUIHCK_KEY_F14                303
#define GUIHCK_KEY_F15                304
#define GUIHCK_KEY_F16                305
#define GUIHCK_KEY_F17                306
#define GUIHCK_KEY_F18                307
#define GUIHCK_KEY_F19                308
#define GUIHCK_KEY_F20                309
#define GUIHCK_KEY_F21                310
#define GUIHCK_KEY_F22                311
#define GUIHCK_KEY_F23                312
#define GUIHCK_KEY_F24                313
#define GUIHCK_KEY_F25                314
#define GUIHCK_KEY_KP_0               320
#define GUIHCK_KEY_KP_1               321
#define GUIHCK_KEY_KP_2               322
#define GUIHCK_KEY_KP_3               323
#define GUIHCK_KEY_KP_4               324
#define GUIHCK_KEY_KP_5               325
#define GUIHCK_KEY_KP_6               326
#define GUIHCK_KEY_KP_7               327
#define GUIHCK_KEY_KP_8               328
#define GUIHCK_KEY_KP_9               329
#define GUIHCK_KEY_KP_DECIMAL         330
#define GUIHCK_KEY_KP_DIVIDE          331
#define GUIHCK_KEY_KP_MULTIPLY        332
#define GUIHCK_KEY_KP_SUBTRACT        333
#define GUIHCK_KEY_KP_ADD             334
#define GUIHCK_KEY_KP_ENTER           335
#define GUIHCK_KEY_KP_EQUAL           336
#define GUIHCK_KEY_LEFT_SHIFT         340
#define GUIHCK_KEY_LEFT_CONTROL       341
#define GUIHCK_KEY_LEFT_ALT           342
#define GUIHCK_KEY_LEFT_SUPER         343
#define GUIHCK_KEY_RIGHT_SHIFT        344
#define GUIHCK_KEY_RIGHT_CONTROL      345
#define GUIHCK_KEY_RIGHT_ALT          346
#define GUIHCK_KEY_RIGHT_SUPER        347
#define GUIHCK_KEY_MENU               348
#define GUIHCK_KEY_LAST               GUIHCK_KEY_MENU

/*! @} */

/*! @defgroup mods Modifier key flags
 *  @ingroup input
 *  @{ */

/*! @brief If this bit is set one or more Shift keys were held down.
 */
#define GUIHCK_MOD_SHIFT           0x0001
/*! @brief If this bit is set one or more Control keys were held down.
 */
#define GUIHCK_MOD_CONTROL         0x0002
/*! @brief If this bit is set one or more Alt keys were held down.
 */
#define GUIHCK_MOD_ALT             0x0004
/*! @brief If this bit is set one or more Super keys were held down.
 */
#define GUIHCK_MOD_SUPER           0x0008

#endif
