#include "jl_types.h"
#include "jl_error.h"

extern int sunxi_mdio_read(void *iobase, int phyaddr, int phyreg);
extern int sunxi_mdio_write(void *iobase, int phyaddr, int phyreg, unsigned short data);
extern void __iomem *mdio_base;

jl_ret_t port_mdio_init(void)
{
	return JL_ERR_OK;
}

jl_ret_t port_mdio_deinit(void)
{
	return JL_ERR_OK;
}

void port_mdio_write(jl_uint8 phy, jl_uint8 reg, jl_uint16 val)
{
	sunxi_mdio_write(mdio_base, 0, reg, val);
}

jl_uint16 port_mdio_read(jl_uint8 phy, jl_uint8 reg)
{
	return sunxi_mdio_read(mdio_base, 0, reg);
}
