#include <linux/delay.h>
#include <linux/slab.h>

#include "jl_types.h"

void *port_mutex_init(void)
{
	return NULL;
	/* return mutex; */
}

void port_mutex_deinit(void *arg)
{
	(void) arg;
}

void port_mutex_lock(void *arg)
{
	(void) arg;
}

void port_mutex_unlock(void *arg)
{
	(void) arg;
}

void port_ndelay(jl_uint32 ns)
{
	ndelay(ns);
}

void port_udelay(jl_uint32 us)
{
	udelay(us);
}

void *port_mem_malloc(jl_uint32 size)
{
	return kmalloc(size, GFP_KERNEL);
}

void port_mem_free(void *ptr)
{
	kfree(ptr);
}

