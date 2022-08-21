/**
 * @file KeyPad.h
 * @author Ali Mirghasemi (ali.mirghasemi1376@gmail.com)
 * @brief this library use for drive keypad, button and input signals
 * @version 0.2.0
 * @date 2021-06-28
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#ifndef _KEYPAD_H_
#define _KEYPAD_H_

#ifdef __cplusplus
extern "C" {
#endif 

#include <stdint.h>

/******************************************************************************/
/*                                Configuration                               */
/******************************************************************************/

/**
 * @brief define KEYPAD_MULTI_CALLBACK if u want have sperate callback functions
 * for each state such as KeyPad_onPressed, KeyPad_onHold, KeyPad_onReleased
 */
#define KEYPAD_MULTI_CALLBACK              1

/**
 * @brief user can have different KeyPad Active State for each keypad (pin)
 */
#define KEYPAD_ACTIVE_STATE                0

/**
 * @brief if you enable this option you can disable or enable keypad
 */
#define KEYPAD_ENABLE_FLAG                 0

/**
 * @brief give user KeyPad_State_None callback
 * None callback fire periodically
 */
#define KEYPAD_NONE_CALLBACK	            0

/**
 * @brief user must define deinitPin function in KeyPad_Driver
 */
#define KEYPAD_USE_DEINIT	                0
/**
 * @brief if KeyPad is based on pair of GPIO and Pin num must enable it
 * for arduino must disable it
 */
#define KEYPAD_CONFIG_IO	                1

#if KEYPAD_CONFIG_IO
/**
 * @brief hold keypad io
 * user can change it to GPIO_TypeDef or anything else that system want
 */
typedef void* KeyPad_IO;
#endif // KEYPAD_CONFIG_IO

/**
 * @brief hold keypad pin num or pin bit
 * user can change it to uint8_t for 8-bit systems like AVR
 */
typedef uint16_t KeyPad_Pin;

/**
 * @brief maximum number of keypads
 * -1 for unlimited, lib use linked list 
 * x for limited keypads, lib use pointer array
 */
#define KEYPAD_MAX_NUM                     -1

/**
 * @brief user can store some args in keypad struct and retrive them in callbacks
 */
#define KEYPAD_ARGS                        0

#define KEYPAD_MODE_ROW_INPUT              1
#define KEYPAD_MODE_COLUMN_INPUT           2
/**
 * @brief user can choose between row and column input mode
 */
#define KEYPAD_MODE                         KEYPAD_MODE_ROW_INPUT

/**
 * @brief hold keypad value
 */
typedef int8_t KeyPad_KeyValue;
/**
 * @brief hold keypad len type
 */
typedef int8_t KeyPad_LenType;
/**
 * @brief KeyPad key none value
 */
#define KEYPAD_KEY_NONE                    ((KeyPad_KeyValue) -1)
/******************************************************************************/

#define KEYPAD_NULL                         ((KeyPad*) 0)
#define KEYPAD_CONFIG_NULL                  ((KeyPad_PinConfig*) 0)

/**
 * @brief hold pin configuration that use for handle keypad
 */
typedef struct {
#if KEYPAD_CONFIG_IO
    KeyPad_IO     	IO;
#endif
    KeyPad_Pin     Pin;
} KeyPad_PinConfig;

/**
 * @brief KeyPad state
 *        _____                __(None)__
 * (Presed)  <-|____(Hold)____|-> (Released)
 */
typedef enum {
    KeyPad_State_None          = 0x00,
    KeyPad_State_Pressed       = 0x01,
    KeyPad_State_Hold          = 0x02,
    KeyPad_State_Released      = 0x03,
} KeyPad_State;

/**
 * @brief logic of keypad
 * Active Low for Pull-Up keypads
 * Active High for Pull-Down keypads
 * remember if you have keypads with different logic must enable KEYPAD_ACTIVE_STATE
 */
typedef enum {
    KeyPad_ActiveState_Low     = 0,
    KeyPad_ActiveState_High    = 1
} KeyPad_ActiveState;
/**
 * @brief tell KeyPad_initPin function to init pin as input or output
 * you can ignore pullup/pulldown if you have external pullup/pulldown and just configure pin as input float
 */
typedef enum {
    KeyPad_PinMode_InputPullUp      = 0,
    KeyPad_PinMode_InputPullDown    = 1,  
    KeyPad_PinMode_Output           = 2,
} KeyPad_PinMode;

/**
 * @brief show next callbacks can fire or ignore incoming callbacks
 */
typedef enum {
    KeyPad_NotHandled          = 0,
    KeyPad_Handled	           = 1
} KeyPad_HandleStatus;
/**
 * @brief hold keypad configuration
 */
typedef struct {
    const KeyPad_KeyValue*      Map;
    const KeyPad_PinConfig*     Columns;
    const KeyPad_PinConfig*     Rows;
    KeyPad_LenType              RowLen;
    KeyPad_LenType              ColumnsLen;
} KeyPad_Config;

#define KEYPAD_CONFIG_INIT(MAP, COL, ROW)       {MAP, COL, ROW, sizeof(COL)/sizeof(KeyPad_PinConfig), sizeof(ROW)/sizeof(KeyPad_PinConfig)}

/* Pre-Defined data types*/
struct _KeyPad;
typedef struct _KeyPad KeyPad;

/**
 * @brief initialize pin in input mode, remember if your pin is pull-up, or pull-down 
 * must configured in init function
 * this function call when new keypad add to queue
 */
typedef void (*KeyPad_InitPinFn)(const KeyPad_PinConfig* config, KeyPad_PinMode mode);
/**
 * @brief de-initialize pin and change pin to reset mode
 * this function call on remove keypad
 */
typedef void (*KeyPad_DeInitPinFn)(const KeyPad_PinConfig* config);
/**
 * @brief this function must return value of a pin
 * 0 -> LOW, 1 -> HIGH
 */
typedef uint8_t (*KeyPad_ReadPinFn)(const KeyPad_PinConfig* config);
/**
 * @brief this function must set pin state
 * 0 -> LOW, !0 -> HIGH
 */
typedef void (*KeyPad_WritePinFn)(const KeyPad_PinConfig* config, uint8_t value);
/**
 * @brief this callback call when keypad state change
 * 
 * @param keypad show which keypad changed
 * @param value key value hold in map
 * @param state show current state of keypad
 * @return user can return KeyPad_NotHandled (0) if wanna get callback on other events 
 *                  otherwise can return KeyPad_Handled (1) that mean keypad handled nad next event is onPressed
 */
typedef KeyPad_HandleStatus (*KeyPad_Callback)(KeyPad* keypad, KeyPad_KeyValue value, KeyPad_State state);
/**
 * @brief hold minimum function for KeyPad lib to work
 * user must pass atleast init and read functions to keypad library
 */
typedef struct {
    KeyPad_InitPinFn     initPin;
    KeyPad_ReadPinFn     readPin;
    KeyPad_WritePinFn    writePin;
    #if KEYPAD_USE_DEINIT
        KeyPad_DeInitPinFn   deinitPin;
    #endif
} KeyPad_Driver;


// determine how many callbacks need
#if KEYPAD_MULTI_CALLBACK && KEYPAD_NONE_CALLBACK
    #define KEYPAD_CALLBACKS_NUM 4
#elif KEYPAD_MULTI_CALLBACK
    #define KEYPAD_CALLBACKS_NUM 3
#else
    #define KEYPAD_CALLBACKS_NUM 1
#endif

typedef union {
    KeyPad_Callback            callbacks[KEYPAD_CALLBACKS_NUM];
    struct {
    #if KEYPAD_MULTI_CALLBACK
        KeyPad_Callback        onHold;
        KeyPad_Callback        onReleased;
        KeyPad_Callback        onPressed;
    #if KEYPAD_NONE_CALLBACK
        KeyPad_Callback        onNone;
    #endif // KEYPAD_NONE_CALLBACK
    #else
        KeyPad_Callback        onChange;
    #endif // KEYPAD_MULTI_CALLBACK
    };
} KeyPad_Callbacks;

/**
 * @brief this struct hold KeyPad parameters
 * do not change the parameters or something else with it
 */
struct _KeyPad {
#if KEYPAD_MAX_NUM == -1
    struct _KeyPad*             Previous;               	/**< point to previous keypad, if it's null show they keypad is end of linked list */
#endif // KEYPAD_MAX_NUM == -1
#if KEYPAD_ARGS
    void*                       Args;                       /**< hold user arguments */
#endif
    const KeyPad_Config*        Config;                 	/**< hold pointer to keypad configuration */
    KeyPad_Callbacks            Callbacks;                  /**< hold user separate callbacks for each keypad state */
    KeyPad_LenType              RowIndex;                   /**< hold current row index */
    KeyPad_LenType              ColIndex;                   /**< hold current col index */
    uint8_t                     State           : 2;    	/**< show current state of keypad*/
    uint8_t                     NotActive       : 1;    	/**< show other states will be ignore or not */
    uint8_t                     ActiveState     : 1;    	/**< this parameters use only when Activestate Enabled */
    uint8_t                     Configured      : 1;        /**< this flag shows KeyPad is configured or not, just useful fo fixed keypad num */
    uint8_t                     Enabled         : 1;        /**< check this flag in irq */
    uint8_t                     Reserved        : 2;
};

void KeyPad_init(const KeyPad_Driver* driver);
void KeyPad_handle(void);

void KeyPad_setConfig(KeyPad* keypad, const KeyPad_Config* config);
const KeyPad_Config* KeyPad_getConfig(KeyPad* keypad);

#if KEYPAD_MAX_NUM > 0
    KeyPad* KeyPad_new(void);
#endif // KEYPAD_MAX_NUM

uint8_t KeyPad_add(KeyPad* keypad, const KeyPad_Config* config);
uint8_t KeyPad_remove(KeyPad* remove);
KeyPad* KeyPad_find(const KeyPad_Config* config);

#if KEYPAD_MULTI_CALLBACK
    void KeyPad_onHold(KeyPad* keypad, KeyPad_Callback cb);
    void KeyPad_onReleased(KeyPad* keypad, KeyPad_Callback cb);
    void KeyPad_onPressed(KeyPad* keypad, KeyPad_Callback cb);
#if KEYPAD_NONE_CALLBACK
    void KeyPad_onNone(KeyPad* keypad, KeyPad_Callback cb);
#endif // KEYPAD_NONE_CALLBACK
#else
    void KeyPad_onChange(KeyPad* keypad, KeyPad_Callback cb);
#endif // KEYPAD_MULTI_CALLBACK


#if KEYPAD_ACTIVE_STATE
    void KeyPad_setActiveState(KeyPad* keypad, KeyPad_ActiveState state);
    KeyPad_ActiveState KeyPad_getActiveState(KeyPad* keypad);
#endif

#if KEYPAD_ENABLE_FLAG
    void KeyPad_setEnabled(KeyPad* keypad, uint8_t enabled);
    uint8_t KeyPad_isEnabled(KeyPad* keypad);
#endif

#if KEYPAD_ARGS
    void KeyPad_setArgs(KeyPad*, void* args);
    void* KeyPad_getArgs(KeyPad* keypad);
#endif

#ifdef __cplusplus
};
#endif

#endif /* _KEYPAD_H_ */
