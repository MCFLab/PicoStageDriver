
#define MAX_NUM_MOTORS                      4 // maximum number of motors (at most 4)
#define COMMON_ADC_RESOLUTION               10 // bit depth of the ADC (Joystick and SensAdjust)
#define ADC_AVERAGING_BASE                  6 // average 2^6=64 samples, ADC_RES + BASE must be <= 16
#define ADC_UPDATE_INTERVAL_MS              4 // ADC sampling interval in ms for averaging

#define INPUT_MODE_CHECK_INTERVAL_MS        10 // check interval for joystick/encoder switchover in ms
#define INPUT_MODE_DEBOUNCE_TIMEOUT_MS      300 // dbounce timeout for switching input mode in ms

#define ENCODER_CH0_PIN_ENCA                D14   // pin number (GPxx) for signal A (-1 for unused); B is the next pin
#define ENCODER_CH1_PIN_ENCA                D10
#define ENCODER_CH2_PIN_ENCA                D8
#define ENCODER_CH3_PIN_ENCA                D2
#define ENCODER_CH0_PIN_BUTTON              D13 // pin number (GPxx) for the button of the encoder (-1 for unused)
#define ENCODER_CH1_PIN_BUTTON              D12
#define ENCODER_CH2_PIN_BUTTON              D7
#define ENCODER_CH3_PIN_BUTTON              D6

#define JOYSTICK_CH0_PIN_ADC                A0 // Analog ADC0 (set to -1 if not used)
#define JOYSTICK_CH1_PIN_ADC                A1 // Analog ADC1 (set to -1 if not used)
#define JOYSTICK_CH2_PIN_ADC                -1 // not used
#define JOYSTICK_CH3_PIN_ADC                -1 // not used
#define JOYSTICK_CH0_PIN_BUTTON             D22 // GPIO 22 (set to -1 if not used)
#define JOYSTICK_CH1_PIN_BUTTON             D22 // same as Ch0
#define JOYSTICK_CH2_PIN_BUTTON             -1 // not used
#define JOYSTICK_CH3_PIN_BUTTON             -1 // not used

#define SENSADJUST_PIN_ADC                  A2 // Analog ADC2 (set to -1 if not used)

#define DISPLAY_PIN_SDA                     PIN_WIRE0_SDA // SDA pin, default is D4 (board pin 6) for the Pico 2
#define DISPLAY_PIN_SCL                     PIN_WIRE0_SCL // SCL pin, default is D5 (board pin 7) for the Pico 2
#define DISPLAY_I2C_ADDRESS                 0x3C
#define DISPLAY_SCREEN_WIDTH                128
#define DISPLAY_SCREEN_HEIGHT               64

#define UART_BAUDRATE                       921600
#define UART_PIN_TX                         PIN_SERIAL1_TX
#define UART_PIN_RX                         PIN_SERIAL1_RX

#define UART_BUFFER_SIZE                    1024
#define UART_SEND_INTERVAL_MS               20 // interval in ms for sending updates to the controller
#define UART_RECEIVE_INTERVAL_MS            10 // interval in ms for receiving updates from the controller
