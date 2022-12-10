# KeyPad Manager Library

## Abstract

This library used for handle input keypads

it's based on HAL layer
user can port library to every hardware 

## Port

for port library user must implement following functions

initPin - configure pin based on input PinConfig
```C
void KeyPad_initPin(const Key_PinConfig* config);
```
deinitPin - reset pin to register reset value
```C
void KeyPad_deInitPin(const Key_PinConfig* config);
```
readPin - read pin and return 1 or 0
```C
uint8_t KeyPad_readPin(const Key_PinConfig* config);
```

and in the final step give Key_Driver to Key_init function
remember deinitPin function it's optional and can be disable in Configuration 
```C
const KeyPad_Driver keypadDriver = {
    KeyPad_initPin,
    KeyPad_readPin,
#if KEYPAD_USE_DEINIT
    KeyPad_deInitPin,
#endif
};
//...

KeyPad_init(&keyDriver);

```

## Configuration

in the configuration part in top of Key.h user can change and customize library based on what need in project

#### KEYPAD_MULTI_CALLBACK
define KEY_MULTI_CALLBACK if u want have sperate callback functions for each state such as Key_onPressed, Key_onHold, Key_onReleased

#### KEYPAD_ACTIVE_STATE
user can have different Key Active State for each key (pin)

#### KEYPAD_NONE_CALLBACK
give user Key_State_None callback, must enable KEY_MULTI_CALLBACK None callback fire periodically

#### KEYPAD_USE_DEINIT
user must define deinitPin function in Key_Driver

#### KEYPAD_CONFIG_IO
if Key is based on pair of GPIO and Pin num must enable it for arduino must disable it

#### KEYPAD_IO
hold key io
user can change it to GPIO_TypeDef or anything else that system want

#### KEYPAD_PIN
hold key pin num or pin bit
user can change it to uint8_t for 8-bit systems like AVR

#### KEYPAD_MAX_NUM
maximum number of keys
-1 for unlimited, lib use linked list 
x for limited keys, lib use pointer array

