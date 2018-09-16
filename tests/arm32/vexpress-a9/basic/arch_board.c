/**
 * Copyright (c) 2012 Jean-Christophe Dubois.
 * All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * @file arch_board.c
 * @author Jean-Christophe Dubois (jcd@tribudubois.net)
 * @brief various platform specific functions
 */

#include <arch_types.h>
#include <arch_io.h>
#include <arch_math.h>
#include <arch_board.h>
#include <arm_plat.h>
#include <basic_stdio.h>
#include <basic_string.h>
#include <libfdt/libfdt.h>
#include <libfdt/fdt_support.h>
#include <pic/gic.h>
#include <timer/sp804.h>
#include <serial/pl01x.h>
#include <sys/vminfo.h>

void arch_board_reset(void)
{
	arch_writel(~0x0, (void *)(V2M_SYS_FLAGSCLR));
	arch_writel(0x0, (void *)(V2M_SYS_FLAGSSET));
	arch_writel(0xc0900000, (void *)(V2M_SYS_CFGCTRL));
}

void arch_board_init(void)
{
	/* Nothing to do */
}

char *arch_board_name(void)
{
	return "ARM VExpress-A9";
}

physical_addr_t arch_board_ram_start(void)
{
	return (physical_addr_t)vminfo_ram_base(V2M_VMINFO_BASE, 0);
}

physical_size_t arch_board_ram_size(void)
{
	return (physical_size_t)vminfo_ram_size(V2M_VMINFO_BASE, 0);
}

void arch_board_linux_default_cmdline(char *cmdline, u32 cmdline_sz)
{
	basic_strcpy(cmdline, "root=/dev/ram rw earlyprintk console=ttyAMA0");
}

void arch_board_fdt_fixup(void *fdt_addr)
{
	u32 vals[5];
	char str[64];
	int rc, poff, noff;

	poff = fdt_path_offset(fdt_addr, "/");
	if (poff < 0) {
		basic_printf("%s: failed to find nodeoffset of / node\n",
			   __func__);
		return;
	}

	poff = fdt_add_subnode(fdt_addr, poff, "virt");
	if (poff < 0) {
		basic_printf("%s: failed to add %s subnode in %s node\n",
			   __func__, "virt", "/");
		return;
	}

	basic_strcpy(str, "simple-bus");
	rc = fdt_setprop(fdt_addr, poff, "compatible",
			 str, basic_strlen(str)+1);
	if (rc < 0) {
		basic_printf("%s: failed to setprop %s in %s node\n",
			   __func__, "compatible", "virt");
		return;
	}

	vals[0] = cpu_to_fdt32(1);
	rc = fdt_setprop(fdt_addr, poff, "#address-cells",
			 vals, sizeof(u32));
	if (rc < 0) {
		basic_printf("%s: failed to setprop %s in %s node\n",
			   __func__, "#address-cells", "virt");
		return;
	}

	vals[0] = cpu_to_fdt32(1);
	rc = fdt_setprop(fdt_addr, poff, "#size-cells",
			 vals, sizeof(u32));
	if (rc < 0) {
		basic_printf("%s: failed to setprop %s in %s node\n",
			   __func__, "#size-cells", "virt");
		return;
	}

	rc = fdt_setprop(fdt_addr, poff, "ranges", NULL, 0);
	if (rc < 0) {
		basic_printf("%s: failed to setprop %s in %s node\n",
			   __func__, "ranges", "virt");
		return;
	}

	noff = fdt_add_subnode(fdt_addr, poff, "virtio_net");
	if (poff < 0) {
		basic_printf("%s: failed to add %s subnode in %s node\n",
			   __func__, "virtio_net", "virt");
		return;
	}

	basic_strcpy(str, "virtio,mmio");
	rc = fdt_setprop(fdt_addr, noff, "compatible",
			 str, basic_strlen(str)+1);
	if (rc < 0) {
		basic_printf("%s: failed to setprop %s in %s node\n",
			   __func__, "compatible", "virtio_net");
		return;
	}

	vals[0] = cpu_to_fdt32(0x20100000);
	vals[1] = cpu_to_fdt32(0x1000);
	rc = fdt_setprop(fdt_addr, noff, "reg",
			 vals, sizeof(u32)*2);
	if (rc < 0) {
		basic_printf("%s: failed to setprop %s in %s node\n",
			   __func__, "reg", "virtio_net");
		return;
	}

	vals[0] = cpu_to_fdt32(0);
	vals[1] = cpu_to_fdt32(50);
	vals[2] = cpu_to_fdt32(4);
	rc = fdt_setprop(fdt_addr, noff, "interrupts",
			 vals, sizeof(u32)*3);
	if (rc < 0) {
		basic_printf("%s: failed to setprop %s in %s node\n",
			   __func__, "interrupts", "virtio_net");
		return;
	}

	rc = fdt_setprop(fdt_addr, noff, "dma-coherent", NULL, 0);
	if (rc < 0) {
		basic_printf("%s: failed to setprop %s in %s node\n",
			   __func__, "dma-coherent", "virtio_net");
		return;
	}

	noff = fdt_add_subnode(fdt_addr, poff, "virtio_block");
	if (poff < 0) {
		basic_printf("%s: failed to add %s subnode in %s node\n",
			   __func__, "virtio_block", "virt");
		return;
	}

	basic_strcpy(str, "virtio,mmio");
	rc = fdt_setprop(fdt_addr, noff, "compatible",
			 str, basic_strlen(str)+1);
	if (rc < 0) {
		basic_printf("%s: failed to setprop %s in %s node\n",
			   __func__, "compatible", "virtio_block");
		return;
	}

	vals[0] = cpu_to_fdt32(0x20200000);
	vals[1] = cpu_to_fdt32(0x1000);
	rc = fdt_setprop(fdt_addr, noff, "reg",
			 vals, sizeof(u32)*2);
	if (rc < 0) {
		basic_printf("%s: failed to setprop %s in %s node\n",
			   __func__, "reg", "virtio_block");
		return;
	}

	vals[0] = cpu_to_fdt32(0);
	vals[1] = cpu_to_fdt32(53);
	vals[2] = cpu_to_fdt32(4);
	rc = fdt_setprop(fdt_addr, noff, "interrupts",
			 vals, sizeof(u32)*3);
	if (rc < 0) {
		basic_printf("%s: failed to setprop %s in %s node\n",
			   __func__, "interrupts", "virtio_block");
		return;
	}

	rc = fdt_setprop(fdt_addr, noff, "dma-coherent", NULL, 0);
	if (rc < 0) {
		basic_printf("%s: failed to setprop %s in %s node\n",
			   __func__, "dma-coherent", "virtio_block");
		return;
	}

	noff = fdt_add_subnode(fdt_addr, poff, "virtio_console");
	if (poff < 0) {
		basic_printf("%s: failed to add %s subnode in %s node\n",
			   __func__, "virtio_console", "virt");
		return;
	}

	basic_strcpy(str, "virtio,mmio");
	rc = fdt_setprop(fdt_addr, noff, "compatible",
			 str, basic_strlen(str)+1);
	if (rc < 0) {
		basic_printf("%s: failed to setprop %s in %s node\n",
			   __func__, "compatible", "virtio_console");
		return;
	}

	vals[0] = cpu_to_fdt32(0x20300000);
	vals[1] = cpu_to_fdt32(0x1000);
	rc = fdt_setprop(fdt_addr, noff, "reg",
			 vals, sizeof(u32)*2);
	if (rc < 0) {
		basic_printf("%s: failed to setprop %s in %s node\n",
			   __func__, "reg", "virtio_console");
		return;
	}

	vals[0] = cpu_to_fdt32(0);
	vals[1] = cpu_to_fdt32(54);
	vals[2] = cpu_to_fdt32(4);
	rc = fdt_setprop(fdt_addr, noff, "interrupts",
			 vals, sizeof(u32)*3);
	if (rc < 0) {
		basic_printf("%s: failed to setprop %s in %s node\n",
			   __func__, "interrupts", "virtio_console");
		return;
	}

	rc = fdt_setprop(fdt_addr, noff, "dma-coherent", NULL, 0);
	if (rc < 0) {
		basic_printf("%s: failed to setprop %s in %s node\n",
			   __func__, "dma-coherent", "virtio_console");
		return;
	}

	noff = fdt_add_subnode(fdt_addr, poff, "virtio_rpmsg");
	if (poff < 0) {
		basic_printf("%s: failed to add %s subnode in %s node\n",
			   __func__, "virtio_rpmsg", "virt");
		return;
	}

	basic_strcpy(str, "virtio,mmio");
	rc = fdt_setprop(fdt_addr, noff, "compatible",
			 str, basic_strlen(str)+1);
	if (rc < 0) {
		basic_printf("%s: failed to setprop %s in %s node\n",
			   __func__, "compatible", "virtio_rpmsg");
		return;
	}

	vals[0] = cpu_to_fdt32(0x20400000);
	vals[1] = cpu_to_fdt32(0x1000);
	rc = fdt_setprop(fdt_addr, noff, "reg",
			 vals, sizeof(u32)*2);
	if (rc < 0) {
		basic_printf("%s: failed to setprop %s in %s node\n",
			   __func__, "reg", "virtio_rpmsg");
		return;
	}

	vals[0] = cpu_to_fdt32(0);
	vals[1] = cpu_to_fdt32(55);
	vals[2] = cpu_to_fdt32(4);
	rc = fdt_setprop(fdt_addr, noff, "interrupts",
			 vals, sizeof(u32)*3);
	if (rc < 0) {
		basic_printf("%s: failed to setprop %s in %s node\n",
			   __func__, "interrupts", "virtio_rpmsg");
		return;
	}

	rc = fdt_setprop(fdt_addr, noff, "dma-coherent", NULL, 0);
	if (rc < 0) {
		basic_printf("%s: failed to setprop %s in %s node\n",
			   __func__, "dma-coherent", "virtio_rpmsg");
		return;
	}
}

physical_addr_t arch_board_autoexec_addr(void)
{
	return (V2M_NOR0 + 0xFF000);
}

u32 arch_board_boot_delay(void)
{
	return vminfo_boot_delay(V2M_VMINFO_BASE);
}

u32 arch_board_iosection_count(void)
{
	return 19;
}

physical_addr_t arch_board_iosection_addr(int num)
{
	physical_addr_t ret = 0;

	switch (num) {
	case 0:
		ret = V2M_PA_CS7;
		break;
	case 1:
		ret = CT_CA9X4_MPIC;
		break;
	case 2:
		ret = V2M_VMINFO_BASE;
		break;
	case 3:
	case 4:
	case 5:
	case 6:
	case 7:
	case 8:
	case 9:
	case 10:
	case 11:
	case 12:
	case 13:
	case 14:
	case 15:
	case 16:
	case 17:
	case 18:
		ret = V2M_NOR0 + (num - 3) * 0x100000;
		break;
	default:
		while (1);
		break;
	}

	return ret;
}

u32 arch_board_pic_nr_irqs(void)
{
	return NR_IRQS_CA9X4;
}

int arch_board_pic_init(void)
{
	int rc;

	/*
	 * Initialize Generic Interrupt Controller
	 */
	rc = gic_dist_init(0, A9_MPCORE_GIC_DIST, IRQ_CA9X4_GIC_START);
	if (rc) {
		return rc;
	}
	rc = gic_cpu_init(0, A9_MPCORE_GIC_CPU);
	if (rc) {
		return rc;
	}

	return 0;
}

u32 arch_board_pic_active_irq(void)
{
	return gic_active_irq(0);
}

int arch_board_pic_ack_irq(u32 irq)
{
	return 0;
}

int arch_board_pic_eoi_irq(u32 irq)
{
	return gic_eoi_irq(0, irq);
}

int arch_board_pic_mask(u32 irq)
{
	return gic_mask(0, irq);
}

int arch_board_pic_unmask(u32 irq)
{
	return gic_unmask(0, irq);
}

void arch_board_timer_enable(void)
{
	return sp804_enable();
}

void arch_board_timer_disable(void)
{
	return sp804_disable();
}

u64 arch_board_timer_irqcount(void)
{
	return sp804_irqcount();
}

u64 arch_board_timer_irqdelay(void)
{
	return sp804_irqdelay();
}

u64 arch_board_timer_timestamp(void)
{
	return sp804_timestamp();
}

void arch_board_timer_change_period(u32 usecs)
{
	return sp804_change_period(usecs);
}

int arch_board_timer_init(u32 usecs)
{
	u32 val, irq;
	u64 counter_mult, counter_shift, counter_mask;

	counter_mask = 0xFFFFFFFFULL;
	counter_shift = 20;
	counter_mult = ((u64)1000000) << counter_shift;
	counter_mult += (((u64)1000) >> 1);
	counter_mult = arch_udiv64(counter_mult, ((u64)1000));

	irq = IRQ_V2M_TIMER0;

	val = arch_readl((void *)V2M_SYSCTL) | SCCTRL_TIMEREN0SEL_TIMCLK;
	arch_writel(val, (void *)V2M_SYSCTL);

	return sp804_init(usecs, V2M_TIMER0, irq, 
			  counter_mask, counter_mult, counter_shift);
}

#define	CA9X4_UART_BASE			V2M_UART0
#define	CA9X4_UART_TYPE			PL01X_TYPE_1
#define	CA9X4_UART_INCLK		24000000
#define	CA9X4_UART_BAUD			115200

int arch_board_serial_init(void)
{
	pl01x_init(CA9X4_UART_BASE, 
			CA9X4_UART_TYPE, 
			CA9X4_UART_BAUD, 
			CA9X4_UART_INCLK);

	return 0;
}

void arch_board_serial_putc(char ch)
{
	if (ch == '\n') {
		pl01x_putc(CA9X4_UART_BASE, CA9X4_UART_TYPE, '\r');
	}
	pl01x_putc(CA9X4_UART_BASE, CA9X4_UART_TYPE, ch);
}

bool arch_board_serial_can_getc(void)
{
	return pl01x_can_getc(CA9X4_UART_BASE, CA9X4_UART_TYPE);
}

char arch_board_serial_getc(void)
{
	char ch = pl01x_getc(CA9X4_UART_BASE, CA9X4_UART_TYPE);
	if (ch == '\r') {
		ch = '\n';
	}
	arch_board_serial_putc(ch);
	return ch;
}
