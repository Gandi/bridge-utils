/*
 * Copyright (C) 2000 Lennert Buytenhek
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef _LIBBRIDGE_H
#define _LIBBRIDGE_H

#include <net/if.h>
#include <linux/if_bridge.h>

struct bridge;
struct bridge_info;
struct fdb_entry;
struct port;
struct port_info;

struct bridge_id
{
	unsigned char prio[2];
	unsigned char addr[6];
};

struct bridge_info
{
	struct bridge_id designated_root;
	struct bridge_id bridge_id;
	unsigned root_path_cost;
	struct timeval max_age;
	struct timeval hello_time;
	struct timeval forward_delay;
	struct timeval bridge_max_age;
	struct timeval bridge_hello_time;
	struct timeval bridge_forward_delay;
	u_int16_t root_port;
	unsigned char stp_enabled;
	unsigned char topology_change;
	unsigned char topology_change_detected;
	struct timeval ageing_time;
	struct timeval hello_timer_value;
	struct timeval tcn_timer_value;
	struct timeval topology_change_timer_value;
	struct timeval gc_timer_value;
};

struct bridge
{
	struct bridge *next;

	int ifindex;
	char ifname[IFNAMSIZ];
	struct port *firstport;
};

struct fdb_entry
{
	u_int8_t mac_addr[6];
	u_int16_t port_no;
	unsigned char is_local;
	struct timeval ageing_timer_value;
};

struct port_info
{
	struct bridge_id designated_root;
	struct bridge_id designated_bridge;
	u_int16_t port_id;
	u_int16_t designated_port;
	u_int8_t priority;
	unsigned char top_change_ack;
	unsigned char config_pending;
	unsigned char state;
	unsigned path_cost;
	unsigned designated_cost;
	struct timeval message_age_timer_value;
	struct timeval forward_delay_timer_value;
	struct timeval hold_timer_value;
};

struct port
{
	struct port *next;
	int index;
	int ifindex;
	struct bridge *parent;
};

extern struct bridge *bridge_list;

int br_init(void);
int br_refresh(void);
struct bridge *br_find_bridge(const char *brname);
struct port *br_find_port(struct bridge *br, const char *portname);
const char *br_get_state_name(int state);

int br_get_bridge_info(const struct bridge *br, struct bridge_info *);
int br_get_port_info(const struct port *port, struct port_info *);
int br_get_version(void);
int br_add_bridge(const char *brname);
int br_del_bridge(const char *brname);
int br_add_interface(struct bridge *br, int ifindex);
int br_del_interface(struct bridge *br, int ifindex);
int br_set_bridge_forward_delay(struct bridge *br, struct timeval *tv);
int br_set_bridge_hello_time(struct bridge *br, struct timeval *tv);
int br_set_bridge_max_age(struct bridge *br, struct timeval *tv);
int br_set_ageing_time(struct bridge *br, struct timeval *tv);
int br_set_gc_interval(struct bridge *br, struct timeval *tv);
int br_set_stp_state(struct bridge *br, int stp_state);
int br_set_bridge_priority(struct bridge *br, int bridge_priority);
int br_set_port_priority(struct port *p, int port_priority);
int br_set_path_cost(struct port *p, int path_cost);
int br_read_fdb(struct bridge *br, struct fdb_entry *fdbs, int offset, int num);
#endif
