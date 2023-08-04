#include "jl_types.h"
#include "jl_error.h"
#include "jl_device.h"

extern void sunxi_jl_mdio_write(int phy, int reg, unsigned short val);
extern int sunxi_jl_mdio_read(int phy, int reg);

#ifdef CONFIG_JL51XX
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
	sunxi_jl_mdio_write(phy, reg, val);
}

jl_uint16 port_mdio_read(jl_uint8 phy, jl_uint8 reg)
{
	return sunxi_jl_mdio_read(phy, reg);
}

#else

jl_ret_t port_mdio_init(jl_io_desc_t *io_desc)
{
	(void) io_desc;

	return JL_ERR_OK;
}

jl_ret_t port_mdio_deinit(jl_io_desc_t *io_desc)
{
	(void) io_desc;

	return JL_ERR_OK;
}

void port_mdio_write(jl_io_desc_t *io_desc,
			jl_uint8 phy, jl_uint8 reg, jl_uint16 val)
{
	(void) io_desc;

	sunxi_jl_mdio_write(phy, reg, val);
}

jl_uint16 port_mdio_read(jl_io_desc_t *io_desc,
			jl_uint8 phy, jl_uint8 reg)
{
	(void) io_desc;

	return sunxi_jl_mdio_read(phy, reg);
}
#endif
