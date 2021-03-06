/*
 * net/dsa/mv88e6xxx.h - Marvell 88e6xxx switch chip support
 * Copyright (c) 2008 Marvell Semiconductor
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef __MV88E6XXX_H
#define __MV88E6XXX_H

#define REG_PORT(p)		(0x10 + (p))
#define REG_GLOBAL		0x1b
#define REG_GLOBAL2		0x1c

/* ATU commands */

#define ATU_BUSY			0x8000

#define ATU_CMD_LOAD_FID		(ATU_BUSY | 0x3000)
#define ATU_CMD_GETNEXT_FID		(ATU_BUSY | 0x4000)
#define ATU_CMD_FLUSH_NONSTATIC_FID	(ATU_BUSY | 0x6000)

/* port states */

#define PSTATE_MASK		0x03
#define PSTATE_DISABLED		0x00
#define PSTATE_BLOCKING		0x01
#define PSTATE_LEARNING		0x02
#define PSTATE_FORWARDING	0x03

/* FDB states */

#define FDB_STATE_MASK			0x0f

#define FDB_STATE_UNUSED		0x00
#define FDB_STATE_MC_STATIC		0x07	/* static multicast */
#define FDB_STATE_STATIC		0x0e	/* static unicast */

struct mv88e6xxx_priv_state {
	/* When using multi-chip addressing, this mutex protects
	 * access to the indirect access registers.  (In single-chip
	 * mode, this mutex is effectively useless.)
	 */
	struct mutex	smi_mutex;

#ifdef CONFIG_NET_DSA_MV88E6XXX_NEED_PPU
	/* Handles automatic disabling and re-enabling of the PHY
	 * polling unit.
	 */
	struct mutex		ppu_mutex;
	int			ppu_disabled;
	struct work_struct	ppu_work;
	struct timer_list	ppu_timer;
#endif

	/* This mutex serialises access to the statistics unit.
	 * Hold this mutex over snapshot + dump sequences.
	 */
	struct mutex	stats_mutex;

	/* This mutex serializes phy access for chips with
	 * indirect phy addressing. It is unused for chips
	 * with direct phy access.
	 */
	struct mutex	phy_mutex;

	/* This mutex serializes eeprom access for chips with
	 * eeprom support.
	 */
	struct mutex eeprom_mutex;

	int		id; /* switch product id */

	/* hw bridging */

	u32 fid_mask;
	u8 fid[DSA_MAX_PORTS];
	u16 bridge_mask[DSA_MAX_PORTS];

	unsigned long port_state_update_mask;
	u8 port_state[DSA_MAX_PORTS];

	struct work_struct bridge_work;
};

struct mv88e6xxx_hw_stat {
	char string[ETH_GSTRING_LEN];
	int sizeof_stat;
	int reg;
};

int mv88e6xxx_setup_port_common(struct dsa_switch *ds, int port);
int mv88e6xxx_setup_common(struct dsa_switch *ds);
int __mv88e6xxx_reg_read(struct mii_bus *bus, int sw_addr, int addr, int reg);
int mv88e6xxx_reg_read(struct dsa_switch *ds, int addr, int reg);
int __mv88e6xxx_reg_write(struct mii_bus *bus, int sw_addr, int addr,
			  int reg, u16 val);
int mv88e6xxx_reg_write(struct dsa_switch *ds, int addr, int reg, u16 val);
int mv88e6xxx_config_prio(struct dsa_switch *ds);
int mv88e6xxx_set_addr_direct(struct dsa_switch *ds, u8 *addr);
int mv88e6xxx_set_addr_indirect(struct dsa_switch *ds, u8 *addr);
int mv88e6xxx_phy_read(struct dsa_switch *ds, int addr, int regnum);
int mv88e6xxx_phy_write(struct dsa_switch *ds, int addr, int regnum, u16 val);
void mv88e6xxx_ppu_state_init(struct dsa_switch *ds);
int mv88e6xxx_phy_read_ppu(struct dsa_switch *ds, int addr, int regnum);
int mv88e6xxx_phy_write_ppu(struct dsa_switch *ds, int addr,
			    int regnum, u16 val);
void mv88e6xxx_poll_link(struct dsa_switch *ds);
void mv88e6xxx_get_strings(struct dsa_switch *ds,
			   int nr_stats, struct mv88e6xxx_hw_stat *stats,
			   int port, uint8_t *data);
void mv88e6xxx_get_ethtool_stats(struct dsa_switch *ds,
				 int nr_stats, struct mv88e6xxx_hw_stat *stats,
				 int port, uint64_t *data);
int mv88e6xxx_get_regs_len(struct dsa_switch *ds, int port);
void mv88e6xxx_get_regs(struct dsa_switch *ds, int port,
			struct ethtool_regs *regs, void *_p);
int  mv88e6xxx_get_temp(struct dsa_switch *ds, int *temp);
int mv88e6xxx_phy_wait(struct dsa_switch *ds);
int mv88e6xxx_eeprom_load_wait(struct dsa_switch *ds);
int mv88e6xxx_eeprom_busy_wait(struct dsa_switch *ds);
int mv88e6xxx_phy_read_indirect(struct dsa_switch *ds, int addr, int regnum);
int mv88e6xxx_phy_write_indirect(struct dsa_switch *ds, int addr, int regnum,
				 u16 val);
int mv88e6xxx_get_eee(struct dsa_switch *ds, int port, struct ethtool_eee *e);
int mv88e6xxx_set_eee(struct dsa_switch *ds, int port,
		      struct phy_device *phydev, struct ethtool_eee *e);
int mv88e6xxx_join_bridge(struct dsa_switch *ds, int port, u32 br_port_mask);
int mv88e6xxx_leave_bridge(struct dsa_switch *ds, int port, u32 br_port_mask);
int mv88e6xxx_port_stp_update(struct dsa_switch *ds, int port, u8 state);
int mv88e6xxx_port_fdb_add(struct dsa_switch *ds, int port,
			   const unsigned char *addr, u16 vid);
int mv88e6xxx_port_fdb_del(struct dsa_switch *ds, int port,
			   const unsigned char *addr, u16 vid);
int mv88e6xxx_port_fdb_getnext(struct dsa_switch *ds, int port,
			       unsigned char *addr, bool *is_static);

extern struct dsa_switch_driver mv88e6131_switch_driver;
extern struct dsa_switch_driver mv88e6123_61_65_switch_driver;
extern struct dsa_switch_driver mv88e6352_switch_driver;
extern struct dsa_switch_driver mv88e6171_switch_driver;

#define REG_READ(addr, reg)						\
	({								\
		int __ret;						\
									\
		__ret = mv88e6xxx_reg_read(ds, addr, reg);		\
		if (__ret < 0)						\
			return __ret;					\
		__ret;							\
	})

#define REG_WRITE(addr, reg, val)					\
	({								\
		int __ret;						\
									\
		__ret = mv88e6xxx_reg_write(ds, addr, reg, val);	\
		if (__ret < 0)						\
			return __ret;					\
	})



#endif
