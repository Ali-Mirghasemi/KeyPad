#include "KeyPad.h"

/* private variables */
static const KeyPad_Driver* keypadDriver;
#if KEYPAD_MAX_NUM == -1
    static KeyPad* lastKeyPad = KEYPAD_NULL;

    #define __keypads()         lastKeyPad
    #define __next(K)           K = (K)->Previous
#else
    static KeyPad keypads[KEYPAD_MAX_NUM] = {0};

    #define __keypads()         keypads
    #define __next(K)           K++
#endif // KEYPAD_MAX_NUM == -1

#if KEYPAD_ACTIVE_STATE
    #define __activeState(K)            (K)->ActiveState
#else
    #define __activeState(K)            KeyPad_ActiveState_Low
#endif

#if KEYPAD_MODE == KEYPAD_MODE_ROW_INPUT
    #define __inPinMode(K)              ((KeyPad_PinMode) __activeState(K))
    #define __outPinMode(K)             KeyPad_PinMode_Output
    #define __outNum(K)                 (K)->Config.ColumnsLen
    #define __inNum(K)                  (K)->Config.RowsLen
    #define __outPin(K,I)               (K)->Config.Columns[I]
    #define __inPin(K,I)                (K)->Config.Rows[I]
    #define __value(OUT, IN)            *((K)->Config.Map * (OUT) + (IN))
    #define __setOutIndex(K, IDX)       (K)->ColIndex = (IDX)
    #define __setInIndex(K, IDX)        (K)->RowIndex = (IDX)
    #define __getOutIndex(K)            (K)->ColIndex
    #define __getInIndex(K)             (K)->RowIndex
#else
    #define __inPinMode(K)              KeyPad_PinMode_Output
    #define __outPinMode(K)             ((KeyPad_PinMode) __activeState(K))
    #define __outNum(K)                 (K)->Config.RowsLen
    #define __inNum(K)                  (K)->Config.ColumnsLen
    #define __outPin(K,I)               (K)->Config.Rows[I]
    #define __inPin(K,I)                (K)->Config.Columns[I]
    #define __value(OUT, IN)            *((K)->Config.Map * (IN) + (OUT))
    #define __setOutIndex(K, IDX)       (K)->RowIndex = (IDX)
    #define __setInIndex(K, IDX)        (K)->ColIndex = (IDX)
    #define __getOutIndex(K)            (K)->RowIndex
    #define __getInIndex(K)             (K)->ColIndex
#endif

#if KEYPAD_MULTI_CALLBACK
    #define __fireCallback(TYPE, K, VALUE)      if ((K)->Callbacks.on ##TYPE) (K)->Callbacks.on ##TYPE ((K), (VALUE), KeyPad_State_ ##TYPE)
#if KEYPAD_NONE_CALLBACK
    #define __fireNoneCallback(K)               if ((K)->State == KeyPad_State_None && (K)->Callbacks.onNone) (K)->Callbacks.onNone((K), KEYPAD_KEY_NONE, KeyPad_State_None)
#else
    #define __fireNoneCallback(K)            
#endif
#else
    #define __fireCallback(TYPE, K, VALUE)      if ((K)->Callbacks.onChange) (K)->Callbacks.onChange((K), (VALUE), KeyPad_State_ ##TYPE)
#if KEYPAD_NONE_CALLBACK
    #define __fireNoneCallback(K)               if ((K)->State == KeyPad_State_None && (K)->Callbacks.onChange) (K)->Callbacks.onNone((K), KEYPAD_KEY_NONE, KeyPad_State_None)
#else
    #define __fireNoneCallback(K)            
#endif
#endif // KEYPAD_MULTI_CALLBACK

static void KeyPad_initOut(KeyPad* keypad);
static void KeyPad_initIn(KeyPad* keypad);


/**
 * @brief use for initialize
 * 
 * @param driver 
 */
void KeyPad_init(const KeyPad_Driver* driver) {
    keypadDriver = driver;
}
/**
 * @brief user must place it in timer with 20ms ~ 50ms 
 * all of callbacks handle and fire in this function
 */
void KeyPad_handle(void) {
    KeyPad_LenType outIndex;
    KeyPad_LenType inIndex;
    KeyPad_State state;
    KeyPad* pKeyPad = __keypads();

#if KEYPAD_MAX_NUM == -1
    while (KEYPAD_NULL != pKeyPad) {
#else
    uint8_t len = KEYPAD_MAX_NUM;
    while (len-- > 0) {
        if (pKeyPad->Configured) {
#endif
    #if KEYPAD_ENABLE_FLAG
        if (pKeyPad->Enabled) {
    #endif // KEYPAD_ENABLE_FLAG

        if (pKeyPad->State == KeyPad_State_None) {
            // scan all keys
            uint8_t state = __activeState(keypad);
            for (outIndex = 0; outIndex < __outNum(keypad); outIndex++) {
                keypadDriver->writePin(__outPin(keypad, outIndex), state);
                for (inIndex = 0; inIndex < __inNum(keypad); inIndex++) {
                    if (keypadDriver->readPin(__inPin(keypad, inIndex)) == state) {
                        // key pressed
                        keypad->State = KeyPad_State_Pressed;
                        __setOutIndex(keypad, outIndex);
                        __setInIndex(keypad, inIndex);
                        __fireCallback(Pressed, keypad, __value(outIndex, inIndex));
                        break;
                    }
                }
                keypadDriver->writePin(__outPin(keypad, outIndex), !state);
            }
            // fire callback if state is none
            __fireNoneCallback(keypad);
        }
        else {
            // scan only pressed key
            KeyPad_LenType outIndex = __getOutIndex(keypad);
            KeyPad_LenType inIndex = __getInIndex(keypad);
            uint8_t state = __activeState(keypad);
            // check pressed key
            if (keypadDriver->readPin(__inPin(keypad, inIndex)) == state) {
                // key holding
                keypad->State = KeyPad_State_Hold;
                __fireCallback(Hold, keypad, __value(outIndex, inIndex));
            }
            else {
                // key released
                keypad->State = KeyPad_State_Released;
                __fireCallback(Released, keypad, __value(outIndex, inIndex));
                keypad->State = KeyPad_State_None;
                keypadDriver->writePin(__outPin(keypad, outIndex), !state);
            }
        }

    #if KEYPAD_ENABLE_FLAG
        }
    #endif // KEYPAD_ENABLE_FLAG
    #if KEYPAD_MAX_NUM > 0
        }
    #endif // KEYPAD_MAX_NUM
        __next(pKeyPad);
    }
}

/**
 * @brief set new pin configuration for keypad
 * 
 * @param keypad address of keypad instance
 * @param config new pin configuration
 */
void KeyPad_setConfig(KeyPad* keypad, const KeyPad_Config* config) {
    keypad->Config = config;
}
/**
 * @brief get keypad pin config
 * 
 * @param keypad 
 * @return const KeyPad_PinConfig* 
 */
const KeyPad_Config* KeyPad_getConfig(KeyPad* keypad) {
    return keypad->Config;
}
#if KEYPAD_MAX_NUM > 0
/**
 * @brief finding a empty space for new keypad in array
 * 
 * @return KeyPad* return null if not found empty space
 */
KeyPad* KeyPad_new(void) {
    uint8_t len = KEYPAD_MAX_NUM;
    KeyPad* pKeyPad = keypads;
    while (len--) {
        if (pKeyPad->Configured) {
            return pKeyPad;
        }
        pKeyPad++;
    }
    return KEYPAD_NULL;
}
#endif // KEYPAD_MAX_NUM
/**
 * @brief add keypad into list for process
 * 
 * @param keypad address of keypad
 * @param config keypad pin configuration
 */
uint8_t KeyPad_add(KeyPad* keypad, const KeyPad_Config* config) {
    // check for null
    if (KEYPAD_NULL == keypad) {
        return 0;
    }
    // add new keypad to list
    keypad->State = KeyPad_State_None;
    keypad->NotActive = KeyPad_NotHandled;
    KeyPad_setConfig(keypad, config);
    // init IOs
    KeyPad_initOut(keypad);
    KeyPad_initIn(keypad);
#if KEYPAD_MAX_NUM == -1
    // add keypad to linked list
    keypad->Previous = lastKeyPad;
    lastKeyPad = keypad;
#endif // KEYPAD_MAX_NUM == -1
    keypad->Configured = 1;
    keypad->Enabled = 1;
    return 1;
}
/**
 * @brief remove keypad from list
 * 
 * @param remove address of keypad
 * @return uint8_t return 1 if keypad found, 0 if not found
 */
uint8_t KeyPad_remove(KeyPad* remove) {
#if KEYPAD_MAX_NUM == -1
    KeyPad* pKeyPad = lastKeyPad;
    // check last keypad first
    if (remove == pKeyPad) {
        // deinit IO
    #if KEYPAD_USE_DEINIT
        if (keypadDriver->deinitPin) {
            keypadDriver->deinitPin(remove->Config);
        }
    #endif
        // remove keypad dropped from link list
        pKeyPad->Previous = remove->Previous;
        remove->Previous = KEYPAD_NULL;
        remove->Configured = 0;
        remove->Enabled = 0;
        return 1;
    }
    while (KEYPAD_NULL != pKeyPad) {
        if (remove == pKeyPad->Previous) {
            // deinit IO
		#if KEYPAD_USE_DEINIT
            if (keypadDriver->deinitPin) {
                keypadDriver->deinitPin(remove->Config);
            }
        #endif
            // remove keypad dropped from link list
            pKeyPad->Previous = remove->Previous;
            remove->Previous = KEYPAD_NULL;
            remove->Configured = 0;
            remove->Enabled = 0;
            return 1;
        }
        pKeyPad = pKeyPad->Previous;
    }
#else
    uint8_t len = KEYPAD_MAX_NUM;
    KeyPad* pKeyPad = keypads;
    while (len--) {
        if (remove == pKeyPad && pKeyPad->Configured) {
            pKeyPad->Configured = 0;
            pKeyPad->Enabled = 0;
            return 1;
        }
        pKeyPad++;
    }
#endif // KEYPAD_MAX_NUM == -1
    return 0;
}
/**
 * @brief finding keypad based on PinConfig in list
 * 
 * @param config 
 * @return KeyPad* 
 */
KeyPad* KeyPad_find(const KeyPad_Config* config) {
#if KEYPAD_MAX_NUM == -1
    KeyPad* pKeyPad = lastKeyPad;
    while (KEYPAD_NULL != pKeyPad) {
        if (config == pKeyPad->Config) {
            return pKeyPad;
        }
        pKeyPad = pKeyPad->Previous;
    }
#else
    uint8_t len = KEYPAD_MAX_NUM;
    KeyPad* pKeyPad = keypads;
    while (len--) {
        if (config == pKeyPad->Config) {
            return pKeyPad;
        }
        pKeyPad++;
    }
#endif // KEYPAD_MAX_NUM == -1
    return KEYPAD_NULL;
}

#if KEYPAD_MULTI_CALLBACK
void KeyPad_onHold(KeyPad* keypad, KeyPad_Callback cb) {
    keypad->Callbacks.onHold = cb;
}
void KeyPad_onReleased(KeyPad* keypad, KeyPad_Callback cb) {
    keypad->Callbacks.onReleased = cb;
}
void KeyPad_onPressed(KeyPad* keypad, KeyPad_Callback cb) {
    keypad->Callbacks.onPressed = cb;
}
#if KEYPAD_NONE_CALLBACK
void KeyPad_onNone(KeyPad* keypad, KeyPad_Callback cb) {
    keypad->Callbacks.onNone = cb;
}
#endif // KEYPAD_NONE_CALLBACK
#else
void KeyPad_onChange(KeyPad* keypad, KeyPad_Callback cb) {
    keypad->Callbacks.onChange = cb;
}
#endif // KEYPAD_MULTI_CALLBACK

#if KEYPAD_ACTIVE_STATE
void KeyPad_setActiveState(KeyPad* keypad, KeyPad_ActiveState state) {
    keypad->ActiveState = (uint8_t) state;
}
KeyPad_ActiveState KeyPad_getActiveState(KeyPad* keypad) {
    return (KeyPad_ActiveState) keypad->ActiveState;
}
#endif /* KEYPAD_ACTIVE_STATE_ENABLE */

#if KEYPAD_ENABLE_FLAG
void KeyPad_setEnabled(KeyPad* keypad, uint8_t enabled) {
    keypad->Enabled = enabled;
}
uint8_t KeyPad_isEnabled(KeyPad* keypad) {
    return keypad->Enabled;
}
#endif

#if KEYPAD_ARGS
void KeyPad_setArgs(KeyPad*, void* args) {
    keypad->Args = args;
}
void* KeyPad_getArgs(KeyPad* keypad) {
    return keypad->Args;
}
#endif
/**
 * @brief init output pins
 * 
 * @param keypad 
 */
static void KeyPad_initOut(KeyPad* keypad) {
    KeyPad_PinMode mode = __outPinMode(keypad);
    KeyPad_PinConfig* config = &__outPin(keypad, 0);
    uint8_t len = __outNum(keypad);
    uint8_t state = !__activeState(keypad);
    while (len-- > 0) {
        keypadDriver->writePin(config, state);
        keypadDriver->initPin(config++, mode);
    }
}
/**
 * @brief init input pins
 * 
 * @param keypad 
 */
static void KeyPad_initIn(KeyPad* keypad) {
    KeyPad_PinMode mode = __inPinMode(keypad);
    KeyPad_PinConfig* config = &__inPin(keypad, 0);
    uint8_t len = __inNum(keypad);
    while (len-- > 0) {
        keypadDriver->initPin(config++, mode);
    }
}

