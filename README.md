# STM32_UART_as_timer
 Using UART as timer for periodic interrupt generation on STM32F303. Two examples are given - with and without DMA. DMA allows to lower the frequency of the interrupt below ~91 Hz (if using USART1 with 72 MHz clock)
