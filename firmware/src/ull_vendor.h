#if defined(CONFIG_BT_CTLR_USER_EXT)

static inline uint16_t ull_conn_interval_min_get(struct ll_conn *conn)
{
	ARG_UNUSED(conn);
	return 4U;
}

static inline int ull_user_init(void)
{
	return 0;
}

static inline int rx_demux_rx_proprietary(memq_link_t *link,
					  struct node_rx_hdr *rx,
					  memq_link_t *tail,
					  memq_link_t **head)
{
	ARG_UNUSED(link);
	ARG_UNUSED(rx);
	ARG_UNUSED(tail);
	ARG_UNUSED(head);
	return 0;
}

static inline void ull_proprietary_done(struct node_rx_event_done *evdone)
{
	ARG_UNUSED(evdone);
}

#endif
