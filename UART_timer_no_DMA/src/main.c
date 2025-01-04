#include "stm32f3xx.h"

#define PA0_HIGH *(volatile uint32_t*)&(GPIOA->BSRRL) = (1 << (0));
#define PA0_LOW *(volatile uint32_t*)&(GPIOA->BSRRL) = (1 << (0 + 16));

void USART1_IRQHandler()
{
    //дрыг ножкой вверх
	PA0_HIGH

	//если прерывание по пустому
	//регистру отправляемых
	//данных
	if (USART1->ISR & USART_ISR_TXE)
	{
		//сброс флага прерывания
		//при помощи записи новых
		//данных
		USART1->TDR = 0;
	}
    //дрыг ножкой вниз
	PA0_LOW
}

void set_72MHz()
{
	RCC->CR |= ((uint32_t)RCC_CR_HSEON);

	/* Ждем пока HSE не выставит бит готовности */
	while(!(RCC->CR & RCC_CR_HSERDY)) {}

	/* Конфигурируем Flash на 2 цикла ожидания */
	/* Flash не может работать на высокой частоте */
	FLASH->ACR &= ~FLASH_ACR_LATENCY;
	FLASH->ACR |= FLASH_ACR_LATENCY_2;

	/* HCLK = SYSCLK */
	RCC->CFGR |= RCC_CFGR_HPRE_DIV1;

	/* PCLK2 = HCLK */
	RCC->CFGR |= RCC_CFGR_PPRE2_DIV1;

	/* PCLK1 = HCLK / 2 */
	RCC->CFGR |= RCC_CFGR_PPRE1_DIV2;

	/* Конфигурируем множитель PLL configuration: PLLCLK = HSE * 9 = 72 MHz */
	/* При условии, что кварц на 8МГц! */
	RCC->CFGR &= ~(RCC_CFGR_PLLSRC | RCC_CFGR_PLLXTPRE | RCC_CFGR_PLLMUL);
	RCC->CFGR |= RCC_CFGR_PLLSRC_HSE_PREDIV | RCC_CFGR_PLLMUL9;

	/* Включаем PLL */
	RCC->CR |= RCC_CR_PLLON;

	/* Ожидаем, пока PLL выставит бит готовности */
	while((RCC->CR & RCC_CR_PLLRDY) == 0) { asm("nop"); }

	/* Выбираем PLL как источник системной частоты */
	RCC->CFGR &= ~RCC_CFGR_SW;
	RCC->CFGR |= RCC_CFGR_SW_PLL;

	/* Ожидаем, пока PLL выберется как источник системной частоты */
	while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL) { asm("nop"); }

	SystemCoreClockUpdate();
}

void init_clocks()
{
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
	RCC->APB2ENR |= RCC_APB2ENR_USART1EN;
}

int main(void)
{
	init_clocks();
	set_72MHz();

	//настройка вывода PA0 на выход
	GPIOA->MODER &= ~GPIO_MODER_MODER0;
	GPIOA->MODER |= GPIO_MODER_MODER0_0;

	//устанавливаем низкий логический уровень на выводе PA0
	PA0_LOW

	//настройка UART

	//прерывание по пустому регистру отправляемых данных,
	//9 бит данных в кадре (максимум)
	USART1->CR1 |= USART_CR1_TXEIE | USART_CR1_M0;
	//2 стоповых бита в кадре (максимум)
	USART1->CR2 |= USART_CR2_STOP_1;
	//включаем UART (но не включаем передачу)
	USART1->CR1 |= USART_CR1_UE;

	//срабатывание
	//6000000 / 1000 = 6000 раз в секунду
	//период 166.(6) мкс
	USART1->BRR = 1000;

	//записываем отправляемые данные
	USART1->TDR = 0;

	//включаем передачу
	USART1->CR1 |= USART_CR1_TE;

	//включаем прерывания от USART1
	//(включение таймера)
	NVIC_EnableIRQ(USART1_IRQn);

	//ждём
	for(int j = 0; j <= 60000; j++) { asm("nop"); }

	for(int i = 1000; i >= 100; i -= 1)
	{
		//увеличиваем частоту срабатывания таймера
		USART1->BRR = i;

		//ждём
		for(int j = 0; j <= 600; j++) { asm("nop"); }
	}

	for(int i = 100; i <= 1000; i += 1)
	{
		//уменьшаем частоту срабатывания таймера
		USART1->BRR = i;

		//ждём
		for(int j = 0; j <= 600; j++) { asm("nop"); }
	}

	//выключаем прерывания от USART1
	//(выключение таймера)
	NVIC_DisableIRQ(USART1_IRQn);

	//выключаем передачу
	USART1->CR1 &= ~USART_CR1_TE;

	while (1) { asm("nop"); }

	return 0;
}
