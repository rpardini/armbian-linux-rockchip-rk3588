// SPDX-License-Identifier: GPL-2.0-only
/*
 * Based on arch/arm/kernel/time.c
 *
 * Copyright (C) 1991, 1992, 1995  Linus Torvalds
 * Modifications for ARM (C) 1994-2001 Russell King
 * Copyright (C) 2012 ARM Ltd.
 */

#include <linux/clockchips.h>
#include <linux/export.h>
#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/time.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/smp.h>
#include <linux/timex.h>
#include <linux/errno.h>
#include <linux/profile.h>
#include <linux/stacktrace.h>
#include <linux/syscore_ops.h>
#include <linux/timer.h>
#include <linux/irq.h>
#include <linux/delay.h>
#include <linux/clocksource.h>
#include <linux/of_clk.h>
#include <linux/acpi.h>

#include <clocksource/arm_arch_timer.h>

#include <asm/thread_info.h>
#include <asm/paravirt.h>

static bool profile_pc_cb(void *arg, unsigned long pc)
{
	unsigned long *prof_pc = arg;

	if (in_lock_functions(pc))
		return true;
	*prof_pc = pc;
	return false;
}

unsigned long profile_pc(struct pt_regs *regs)
{
	unsigned long prof_pc = 0;

	arch_stack_walk(profile_pc_cb, &prof_pc, current, regs);

	return prof_pc;
}
EXPORT_SYMBOL(profile_pc);

void __init time_init(void)
{
	u32 arch_timer_rate;

	pr_emerg("RKDBG> time_init: enter\n");
	of_clk_init(NULL);
	pr_emerg("RKDBG> time_init: of_clk_init done\n");
	timer_probe();
	pr_emerg("RKDBG> time_init: timer_probe done\n");

	tick_setup_hrtimer_broadcast();

	arch_timer_rate = arch_timer_get_rate();
	pr_emerg("RKDBG> time_init: arch_timer_rate=%u\n", arch_timer_rate);
	if (!arch_timer_rate)
		panic("Unable to initialise architected timer.\n");

	/* Calibrate the delay loop directly */
	lpj_fine = arch_timer_rate / HZ;

	pv_time_init();
}
