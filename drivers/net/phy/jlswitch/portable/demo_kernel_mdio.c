#include "jl_types.h"
#include "jl_error.h"

extern jl_uint16 jl_mdio_read(jl_uint8 phy_adr, jl_uint8 reg);
extern void jl_mdio_write(jl_uint8 phy_adr, jl_uint8 reg, jl_uint16 data);

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
	jl_mdio_write(phy, reg, val);
}

jl_uint16 port_mdio_read(jl_uint8 phy, jl_uint8 reg)
{
	return jl_mdio_read(phy, reg);
}
