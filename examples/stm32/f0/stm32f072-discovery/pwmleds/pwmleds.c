/*
 * This file is part of the libopencm3 project.
 *
 * Copyright (C) 2014 Kuldeep Singh Dhaka <kuldeepdhaka9@gmail.com>
 *
 * This library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/timer.h>
#include <libopencm3/stm32/flash.h>

void rcc_clock_setup_in_hsi_out_48mhz_corrected(void);

void rcc_clock_setup_in_hsi_out_48mhz_corrected(void)
{
	rcc_osc_on(HSI);
	rcc_wait_for_osc_ready(HSI);
	rcc_set_sysclk_source(HSI);

	// correction (f072 has PREDIV after clock multiplexer (near PLL)
	//Figure 12. Clock tree (STM32F07x devices)  P96 	RM0091
	//applies to rcc_clock_setup_in_hsi_out_*mhz()
	rcc_set_prediv(RCC_CFGR2_PREDIV_DIV2);

	rcc_set_hpre(RCC_CFGR_HPRE_NODIV);
	rcc_set_ppre(RCC_CFGR_PPRE_NODIV);

	flash_set_ws(FLASH_ACR_LATENCY_024_048MHZ);

	// 8MHz * 12 / 2 = 48MHz
	rcc_set_pll_multiplication_factor(RCC_CFGR_PLLMUL_MUL12);

	RCC_CFGR &= ~RCC_CFGR_PLLSRC;

	rcc_osc_on(PLL);
	rcc_wait_for_osc_ready(PLL);
	rcc_set_sysclk_source(PLL);

	rcc_ppre_frequency = 48000000;
	rcc_core_frequency = 48000000;
}

static void clock_setup(void)
{
	rcc_clock_setup_in_hsi_out_48mhz_corrected();
	
	/* Enable TIM3 clock. */
	rcc_periph_clock_enable(RCC_TIM3);

	/* Enable GPIOC */
	rcc_periph_clock_enable(RCC_GPIOC);
}

static void gpio_setup(void)
{
	gpio_mode_setup(GPIOC, GPIO_MODE_AF, GPIO_PUPD_PULLDOWN, GPIO6);
	gpio_set_output_options(GPIOC, GPIO_OTYPE_PP, GPIO_OSPEED_100MHZ, GPIO6); // 100mhz means highspeed actually
	gpio_set_af(GPIOC, GPIO_AF0, GPIO6);
}

static void tim_setup(void)
{
	//timer_reset(TIM3);
	timer_set_prescaler(TIM3, 0xFF);
	//timer_set_clock_division(TIM3, 0xFF);
	timer_set_period(TIM3, 0xFFF);
	
	timer_continuous_mode(TIM3);
	timer_direction_up(TIM3);
	
	timer_disable_oc_output(TIM3, TIM_OC1);
	timer_set_oc_mode(TIM3, TIM_OC1, TIM_OCM_PWM1);
	timer_set_oc_value(TIM3, TIM_OC1, 0xFF);
	timer_enable_oc_output(TIM3, TIM_OC1);
	timer_enable_preload(TIM3);
	timer_enable_counter(TIM3);
}

void main(void)
{

	clock_setup();
	gpio_setup();
	tim_setup();

	while(1);
}
