/*
 * This file is part of the coreboot project.
 *
 * Copyright 2016 Google Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
 * GNU General Public License for more details.
 */

#include <commonlib/helpers.h>
#include <console/console.h>
#include <stdint.h>
#include <soc/iosf.h>
#include <soc/itss.h>

#define IOSF_ITSS_PORT_ID	0xd0
#define ITSS_MAX_IRQ		119
#define IPC0			0x3200
#define IRQS_PER_IPC		32
#define NUM_IPC_REGS		((ITSS_MAX_IRQ + IRQS_PER_IPC - 1)/IRQS_PER_IPC)

void itss_set_irq_polarity(int irq, int active_low)
{
	uint32_t mask;
	uint32_t val;
	uint16_t reg;
	const uint16_t port = IOSF_ITSS_PORT_ID;

	if (irq < 0 || irq > ITSS_MAX_IRQ)
		return;

	reg = IPC0 + sizeof(uint32_t) * (irq / IRQS_PER_IPC);
	mask = 1 << (irq % IRQS_PER_IPC);

	val = iosf_read(port, reg);
	val &= ~mask;
	/* Setting the bit makes the IRQ active low. */
	val |= active_low ? mask : 0;
	iosf_write(port, reg, val);
}

static uint32_t irq_snapshot[NUM_IPC_REGS];

void itss_snapshot_irq_polarities(int start, int end)
{
	int i;
	int reg_start;
	int reg_end;
	const uint16_t port = IOSF_ITSS_PORT_ID;

	if (start < 0 || start > ITSS_MAX_IRQ ||
	    end < 0 || end > ITSS_MAX_IRQ || end < start)
		return;

	reg_start = start / IRQS_PER_IPC;
	reg_end = (end + IRQS_PER_IPC - 1) / IRQS_PER_IPC;

	for (i = reg_start; i < reg_end; i++) {
		uint16_t reg = IPC0 + sizeof(uint32_t) * i;
		irq_snapshot[i] = iosf_read(port, reg);
	}
}

static void show_irq_polarities(const char *msg)
{
	int i;
	const uint16_t port = IOSF_ITSS_PORT_ID;

	printk(BIOS_INFO, "ITSS IRQ Polarities %s:\n", msg);
	for (i = 0; i < NUM_IPC_REGS; i++) {
		uint16_t reg = IPC0 + sizeof(uint32_t) * i;
		printk(BIOS_INFO, "IPC%d: 0x%08x\n", i, iosf_read(port, reg));
	}
}

void itss_restore_irq_polarities(int start, int end)
{
	int i;
	int reg_start;
	int reg_end;
	const uint16_t port = IOSF_ITSS_PORT_ID;

	if (start < 0 || start > ITSS_MAX_IRQ ||
	    end < 0 || end > ITSS_MAX_IRQ || end < start)
		return;

	show_irq_polarities("Before");

	reg_start = start / IRQS_PER_IPC;
	reg_end = (end + IRQS_PER_IPC - 1) / IRQS_PER_IPC;

	for (i = reg_start; i < reg_end; i++) {
		uint32_t mask;
		uint32_t val;
		uint16_t reg;
		int irq_start;
		int irq_end;

		irq_start = i * IRQS_PER_IPC;
		irq_end = MIN(irq_start + IRQS_PER_IPC - 1, ITSS_MAX_IRQ);

		if (start > irq_end)
			continue;
		if (end < irq_start)
			break;

		/* Track bits within the bounds of of the register. */
		irq_start = MAX(start, irq_start) % IRQS_PER_IPC;
		irq_end = MIN(end, irq_end) % IRQS_PER_IPC;

		/* Create bitmask of the inclusive range of start and end. */
		mask = (((1U << irq_end) - 1) | (1U << irq_end));
		mask &= ~((1U << irq_start) - 1);

		reg = IPC0 + sizeof(uint32_t) * i;
		val = iosf_read(port, reg);
		val &= ~mask;
		val |= mask & irq_snapshot[i];
		iosf_write(port, reg, val);
	}

	show_irq_polarities("After");
}
