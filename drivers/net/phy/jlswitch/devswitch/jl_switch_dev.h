/*
 * Copyright (c) 2014-2023 JLSemi Limited
 * All Rights Reserved
 *
 * THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE of JLSemi Limited
 * The copyright notice above does not evidence any actual or intended
 * publication of such source code.
 *
 * No part of this code may be reproduced, stored in a retrieval system,
 * or transmitted, in any form or by any means, electronic, mechanical,
 * photocopying, recording, or otherwise, without the prior written
 * permission of JLSemi Limited
 */
#ifndef __JL_SWITCH_DEV_H__
#define __JL_SWITCH_DEV_H__
#include <linux/netdevice.h>

typedef enum {
	JL_MODE_MII,
	JL_MODE_RMII,
	JL_MODE_RGMII,
	JL_MODE_SGMII,
	JL_MODE_HSGMII
}jl_mode_t;

/**
 * jl_switch_open - initialize jlsemi switch.
 * @ndev: net device.
 * @duplex: duplex mode, 0: half-duplex, 1: full-duplex.
 * @speed: working speed, eg: 10, 100, 1000.
 * @mode: interface mode, eg: JL_MODE_MII, JL_MODE_RMII,
 * 		JL_MODE_RGMII, JL_MODE_SGMII, JL_MODE_HSGMII.
 *
 * return 0 if success, otherwise failed.
 */
int jl_switch_open(struct net_device *ndev, int duplex, int speed, jl_mode_t mode);
int jl_switch_stop(void);

#endif /* __JL_SWITCH_DEV_H__ */
