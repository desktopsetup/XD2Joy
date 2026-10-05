#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/settings/settings.h>
#include <zephyr/init.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/bluetooth/controller.h>
#include <zephyr/bluetooth/crypto.h>
#include <zephyr/sys/byteorder.h>
#include <zephyr/sys/crc.h>
#include <zephyr/drivers/uart.h>
#include <soc.h>
#include <string.h>
#include <stdlib.h>

#if defined(JC_LEFT)
#include "XData_left.h"
#else
#include "XData_right.h"
#endif

#include <host/keys.h>
#include <host/hci_core.h>

#if defined(JC_LEFT)
#include "usb_dev.h"
#endif

#define BUILD_TAG "ease1"

#define UUID_SVC_CFG \
	BT_UUID_DECLARE_128(BT_UUID_128_ENCODE(0x00c5af5d, 0x1964, 0x4e30, 0x8f51, 0x1956f96bd280))
#define UUID_CFG1 \
	BT_UUID_DECLARE_128(BT_UUID_128_ENCODE(0x00c5af5d, 0x1964, 0x4e30, 0x8f51, 0x1956f96bd281))
#define UUID_CFG2 \
	BT_UUID_DECLARE_128(BT_UUID_128_ENCODE(0x00c5af5d, 0x1964, 0x4e30, 0x8f51, 0x1956f96bd282))
#define UUID_CFG3 \
	BT_UUID_DECLARE_128(BT_UUID_128_ENCODE(0x00c5af5d, 0x1964, 0x4e30, 0x8f51, 0x1956f96bd283))

#define UUID_SVC_MAIN \
	BT_UUID_DECLARE_128(BT_UUID_128_ENCODE(0xab7de9be, 0x89fe, 0x49ad, 0x828f, 0x118f09df7fd0))
#define UUID_N000A \
	BT_UUID_DECLARE_128(BT_UUID_128_ENCODE(0xab7de9be, 0x89fe, 0x49ad, 0x828f, 0x118f09df7fd2))
#if defined(JC_LEFT)
#define UUID_RPTHID \
	BT_UUID_DECLARE_128(BT_UUID_128_ENCODE(0xcc1bbbb5, 0x7354, 0x4d32, 0xa716, 0xa81cb241a32a))
#else
#define UUID_RPTHID \
	BT_UUID_DECLARE_128(BT_UUID_128_ENCODE(0xd5a9e01e, 0x2ffc, 0x4cca, 0xb20c, 0x8b67142bf442))
#endif
#if defined(JC_LEFT)
#define UUID_W0012 \
	BT_UUID_DECLARE_128(BT_UUID_128_ENCODE(0x289326cb, 0xa471, 0x485d, 0xa8f4, 0x240c14f18241))
#else
#define UUID_W0012 \
	BT_UUID_DECLARE_128(BT_UUID_128_ENCODE(0xfa19b0fb, 0xcd1f, 0x46a7, 0x84a1, 0xbbb09e00c149))
#endif
#define UUID_W0014 \
	BT_UUID_DECLARE_128(BT_UUID_128_ENCODE(0x649d4ac9, 0x8eb7, 0x4e6c, 0xaf44, 0x1ea54fe5f005))
#if defined(JC_LEFT)
#define UUID_W0016 \
	BT_UUID_DECLARE_128(BT_UUID_128_ENCODE(0xce49a830, 0xdced, 0x48ae, 0x931e, 0xc8cf88aadbea))
#else
#define UUID_W0016 \
	BT_UUID_DECLARE_128(BT_UUID_128_ENCODE(0x65a724b3, 0xf1e7, 0x4a61, 0x8078, 0xa342376b27ff))
#endif
#define UUID_W0018 \
	BT_UUID_DECLARE_128(BT_UUID_128_ENCODE(0x4147423d, 0xfdae, 0x4df7, 0xa4f7, 0xd23e5df59f8d))
#define UUID_CMDRSP \
	BT_UUID_DECLARE_128(BT_UUID_128_ENCODE(0xc765a961, 0xd9d8, 0x4d36, 0xa20a, 0x5315b111836a))
#if defined(JC_LEFT)
#define UUID_N001E \
	BT_UUID_DECLARE_128(BT_UUID_128_ENCODE(0x63a3810f, 0xaec7, 0x474b, 0x9010, 0x3d52403cb996))
#else
#define UUID_N001E \
	BT_UUID_DECLARE_128(BT_UUID_128_ENCODE(0x640ca58e, 0x0e88, 0x410c, 0xa7f3, 0x426faf2b690b))
#endif
#define UUID_N0022 \
	BT_UUID_DECLARE_128(BT_UUID_128_ENCODE(0xd3bd69d2, 0x841c, 0x4241, 0xab15, 0xf86f406d2a80))
#define UUID_DESC_VENDOR \
	BT_UUID_DECLARE_128(BT_UUID_128_ENCODE(0x679d5510, 0x5a24, 0x4dee, 0x9557, 0x95df80486ecb))

#define UUID_DESC_VENDOR2 \
	BT_UUID_DECLARE_128(BT_UUID_128_ENCODE(0xb746df8c, 0xf358, 0x495b, 0x9cd2, 0xe3bbeda4f979))

#define UUID_N0026 \
	BT_UUID_DECLARE_128(BT_UUID_128_ENCODE(0xab7de9be, 0x89fe, 0x49ad, 0x828f, 0x118f09df7fde))
#define UUID_W002A \
	BT_UUID_DECLARE_128(BT_UUID_128_ENCODE(0xab7de9be, 0x89fe, 0x49ad, 0x828f, 0x118f09df7fdf))

#if defined(JC_LEFT)
static uint8_t FAKE_ADDR[6] = { 0x46, 0xBD, 0x91, 0x70, 0x68, 0xB8 };
#else
static uint8_t FAKE_ADDR[6] = { 0x47, 0xBD, 0x91, 0x70, 0x68, 0xB8 };
#endif

#define ADV_OFF_WAKE 11
#define ADV_WAKE_FLAG 0x81
#define ADV_OFF_HOST 12
static uint8_t mfg_data[] = {
#if defined(JC_LEFT)
	0x53, 0x05, 0x01, 0x00, 0x03, 0x7E, 0x05, 0x67, 0x20, 0x00,
#else
	0x53, 0x05, 0x01, 0x00, 0x03, 0x7E, 0x05, 0x66, 0x20, 0x00,
#endif
	0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

static const struct bt_data ad[] = {
	BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
	BT_DATA(BT_DATA_MANUFACTURER_DATA, mfg_data, sizeof(mfg_data)),
};

static const uint8_t CONTROLLER_PUBKEY[16] = {
	0x5C, 0xF6, 0xEE, 0x79, 0x2C, 0xDF, 0x05, 0xE1,
	0xBA, 0x2B, 0x63, 0x25, 0xC4, 0x1A, 0x5F, 0x10,
};
static uint8_t pair_ltk[16];
static bool have_ltk;

#define BTN_DOWN 0x0001
#define BTN_RIGHT 0x0002
#define BTN_LEFT 0x0004
#define BTN_UP 0x0008
#define BTN_L 0x0010
#define BTN_ZL 0x0020
#define BTN_MINUS 0x0040
#define BTN_LS 0x0080
#define BTN_CAPTURE 0x0100
#define STICK_CX 2077
#define STICK_CY 1984
#define STICK_THROW 1200
#define STICK_Y_UP_POSITIVE 1

#define BTN_SR 0x4000
#define BTN_SL 0x8000

#define RBTN_B 0x0001
#define RBTN_A 0x0002
#define RBTN_Y 0x0004
#define RBTN_X 0x0008
#define RBTN_R 0x0010
#define RBTN_ZR 0x0020
#define RBTN_PLUS 0x0040
#define RBTN_HOME 0x0100
#define RBTN_C 0x1000
#define RBTN_SL 0x8000
#define RBTN_SR 0x4000

#define BTN_GL 0x0400

static int16_t grip_byte = 0x01;
static int16_t grip_wait = -1;
#define GRIP_WAIT_REPORTS 6

static const uint8_t RSP_08_01[] = {
	0x00, 0x00, 0x00, 0x00, 0x01, 0x00,
	'H', 'D', 'L', '5', '0', '0', '0', '8', '8', '9', '5', '8', '2', '7',
	0x00, 0x00, 0x7E, 0x05, 0x68, 0x20, 0x01, 0x03, 0x01,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
};

#define STICK_WHEEL_GAIN 500
#define STICK_DECAY_NUM 7
#define STICK_DECAY_DEN 8

#define JC_CFG_MAGIC0 0x32434A55UL
#define JC_CFG_MAGIC1 0x1A5F4247UL
#define JC_CFG_BTNS 32
#define JC_CFG_VER 3

#define JC_SRC_NONE 0
#define JC_SRC_DX 1
#define JC_SRC_DY 2
#define JC_SRC_WHEEL 3

#define JC_BTN_MOUSE 0
#define JC_BTN_KEY 1
#define JC_BTN_MOD 2
#define JC_BTN_PEER 3
#define JC_BTN_STICK 4
#define JC_BTN_PEERMOD 5
#define JC_BTN_MPEER 6
#define JC_BTN_MODE 7
#define JC_BTN_SET 8
#define JC_BTN_WHEEL 9
#define JC_BTN_WPEER 10
#define WHEEL_PULSE_MS 90
#define JC_SET_STICK_GAIN 1
#define JC_SET_MOUSE_GAIN 2
#define JC_SET_ACCEL 3
#define JC_SET_STICK_DZ 4
#define JC_SET_STICK_VG 5

#define JC_STICK(dx, dy) ((uint16_t)((((uint16_t)(uint8_t)(int8_t)(dy)) << 8) | ((uint16_t)(uint8_t)(int8_t)(dx))))

struct jc_cfg_btn {
	uint8_t code;
	uint8_t kind;
	uint16_t mask;
};

struct jc_cfg {
	uint32_t magic0;
	uint32_t magic1;
	uint16_t version;
	uint8_t side;
	uint8_t n_btn;
	uint16_t stick_gain;
	uint8_t stick_src_x;
	uint8_t stick_src_y;
	uint8_t col_body[3];
	uint8_t col_buttons[3];
	uint8_t col_rail[3];
	uint8_t col_stick[3];
	struct jc_cfg_btn btn[JC_CFG_BTNS];
	uint32_t magic_end;
};

static const volatile struct jc_cfg CFG_FLASH = {
	.magic0 = JC_CFG_MAGIC0,
	.magic1 = JC_CFG_MAGIC1,
	.version = JC_CFG_VER,
	.stick_gain = STICK_WHEEL_GAIN,
#if defined(JC_LEFT)
	.side = 0x67,
	.n_btn = 27,
	.stick_src_x = JC_SRC_NONE,
	.stick_src_y = JC_SRC_NONE,
	.col_body = { 0x4B, 0xA1, 0x6B },
	.col_buttons = { 0xD5, 0x4C, 0x58 },
	.col_rail = { 0x54, 0x3E, 0x38 },
	.col_stick = { 0xF7, 0x82, 0x6F },
	.btn = {
		{ 0x3E, JC_BTN_KEY, BTN_CAPTURE },
		{ 0x51, JC_BTN_KEY, BTN_DOWN },
		{ 0x14, JC_BTN_KEY, BTN_GL },
		{ 0x0F, JC_BTN_KEY, BTN_L },
		{ 0x50, JC_BTN_KEY, BTN_LEFT },
		{ 0x02, JC_BTN_MOD, BTN_LS },
		{ 0x2B, JC_BTN_KEY, BTN_MINUS },
		{ 0x09, JC_BTN_PEER, RBTN_A },
		{ 0x2C, JC_BTN_PEER, RBTN_B },
		{ 0x12, JC_BTN_PEER, RBTN_C },
		{ 0x29, JC_BTN_PEER, RBTN_HOME },
		{ 0x28, JC_BTN_PEER, RBTN_PLUS },
		{ 0x20, JC_BTN_PEER, RBTN_SL },
		{ 0x21, JC_BTN_PEER, RBTN_SR },
		{ 0x15, JC_BTN_PEER, RBTN_X },
		{ 0x17, JC_BTN_PEER, RBTN_Y },
		{ 0x4F, JC_BTN_KEY, BTN_RIGHT },
		{ 0x1E, JC_BTN_KEY, BTN_SL },
		{ 0x1F, JC_BTN_KEY, BTN_SR },
		{ 0x16, JC_BTN_STICK, JC_STICK(0, -1) },
		{ 0x04, JC_BTN_STICK, JC_STICK(-1, 0) },
		{ 0x07, JC_BTN_STICK, JC_STICK(1, 0) },
		{ 0x1A, JC_BTN_STICK, JC_STICK(0, 1) },
		{ 0x52, JC_BTN_KEY, BTN_UP },
		{ 0x08, JC_BTN_KEY, BTN_ZL },
		{ 0x0E, JC_BTN_MODE, 0x0101 },
		{ 0x0D, JC_BTN_MODE, 0x0201 },
	},
#else
	.side = 0x66,
	.n_btn = 7,
	.stick_src_x = JC_SRC_NONE,
	.stick_src_y = JC_SRC_WHEEL,
	.col_body = { 0x4B, 0xA1, 0x6B },
	.col_buttons = { 0xD5, 0x4C, 0x58 },
	.col_rail = { 0x54, 0x3E, 0x38 },
	.col_stick = { 0xF7, 0x82, 0x6F },
	.btn = {
		{ 0x01, JC_BTN_MOUSE, RBTN_R },
		{ 0x02, JC_BTN_MOUSE, RBTN_ZR },
		{ JC_SET_STICK_GAIN, JC_BTN_SET, 10 },
		{ JC_SET_MOUSE_GAIN, JC_BTN_SET, 420 },
		{ JC_SET_ACCEL, JC_BTN_SET, 1 },
		{ JC_SET_STICK_DZ, JC_BTN_SET, 290 },
		{ JC_SET_STICK_VG, JC_BTN_SET, 5 },
	},
#endif
	.magic_end = JC_CFG_MAGIC0,
};

static struct jc_cfg CFG;

static void cfg_settings_apply(void);

static int cfg_load(void)
{
	memcpy(&CFG, (const void *)&CFG_FLASH, sizeof(CFG));
	cfg_settings_apply();
	return 0;
}
SYS_INIT(cfg_load, PRE_KERNEL_1, 0);

static bool cfg_record_ok(const struct jc_cfg *c)
{
	if (c->magic0 != CFG_FLASH.magic0 || c->magic1 != CFG_FLASH.magic1) {
		printk("[CFG] stored record rejected: bad magic\n");
		return false;
	}
	if (c->magic_end != CFG_FLASH.magic_end) {
		printk("[CFG] stored record rejected: bad end marker\n");
		return false;
	}
	if (c->version != JC_CFG_VER) {
		printk("[CFG] stored record rejected: version %u, this build "
		       "speaks %u\n", c->version, (unsigned)JC_CFG_VER);
		return false;
	}
	if (c->side != CFG_FLASH.side) {
		printk("[CFG] stored record rejected: side 0x%02X, this is a "
		       "0x%02X board\n", c->side, CFG_FLASH.side);
		return false;
	}
	if (c->n_btn > JC_CFG_BTNS) {
		printk("[CFG] stored record rejected: n_btn %u > %u\n",
		       c->n_btn, (unsigned)JC_CFG_BTNS);
		return false;
	}
	return true;
}

#define JC_MODE_JOYCON 0
#define JC_MODE_KEYBOARD 1
#define JC_MODE_STICK 2
static uint8_t jc_mode;

static inline bool jc_live(void)
{
	return jc_mode != JC_MODE_KEYBOARD;
}

static const struct gpio_dt_spec mode_led =
	GPIO_DT_SPEC_GET_OR(DT_ALIAS(led0), gpios, {0});

static void mode_led_set(bool on)
{
	if (mode_led.port != NULL) {
		gpio_pin_set_dt(&mode_led, on ? 1 : 0);
	}
}

static int mode_led_init(void)
{
	if (mode_led.port == NULL || !device_is_ready(mode_led.port)) {
		return 0;
	}
	gpio_pin_configure_dt(&mode_led, GPIO_OUTPUT_INACTIVE);
	return 0;
}
SYS_INIT(mode_led_init, APPLICATION, 0);

static const char *jc_mode_id_name(uint8_t m)
{
	if (m == JC_MODE_KEYBOARD) {
		return "KEYBOARD";
	}
	return m == JC_MODE_STICK ? "STICK" : "JOYCON";
}

static const char *jc_mode_name(void)
{
	return jc_mode_id_name(jc_mode);
}

#define JC_NAME_MAX 22

struct jc_peer {
	uint8_t have;
	bt_addr_le_t addr;
	char name[JC_NAME_MAX];
};

static struct jc_peer PEER;

#define JC_PROFILES 4
#define JC_PROF_NAME 52
static struct jc_cfg prof_slot[JC_PROFILES];
static uint8_t prof_have;
static uint8_t prof_active = 1;
static char prof_name[JC_PROFILES][JC_PROF_NAME];
struct jc_prof_key {
	uint8_t code;
	uint8_t mods;
};
static struct jc_prof_key prof_key[JC_PROFILES];

#define SYNC_KEY 0x45
struct jc_sync_key {
	uint8_t code;
	uint8_t mods;
	uint8_t on;
};
static struct jc_sync_key sync_key = { SYNC_KEY, 0, 1 };

static void cfg_apply(const struct jc_cfg *c);
static bool prof_filled(uint8_t n);

static int cfg_set(const char *name, size_t len, settings_read_cb read_cb,
		   void *cb_arg)
{
	struct jc_cfg tmp;
	const char *next;
	ssize_t n;

	if (settings_name_steq(name, "peer", &next) && !next) {
		struct jc_peer p;

		if (len != sizeof(p) ||
		    read_cb(cb_arg, &p, sizeof(p)) != (ssize_t)sizeof(p)) {
			return 0;
		}
		p.name[sizeof(p.name) - 1] = '\0';
		PEER = p;
		return 0;
	}
	{
		uint8_t i;

		for (i = 0; i < JC_PROFILES; i++) {
			char k[3] = { 'p', (char)('1' + i), '\0' };
			struct jc_cfg t;

			if (!settings_name_steq(name, k, &next) || next) {
				continue;
			}
			if (len == sizeof(t) &&
			    read_cb(cb_arg, &t, sizeof(t)) == (ssize_t)sizeof(t) &&
			    cfg_record_ok(&t)) {
				memcpy(&prof_slot[i], &t, sizeof(t));
				prof_have |= (uint8_t)(1U << i);
			}
			return 0;
		}
	}
	if (settings_name_steq(name, "pact", &next) && !next) {
		uint8_t n;

		if (len == sizeof(n) &&
		    read_cb(cb_arg, &n, sizeof(n)) == (ssize_t)sizeof(n) &&
		    n >= 1 && n <= JC_PROFILES) {
			prof_active = n;
		}
		return 0;
	}
	if (settings_name_steq(name, "pname", &next) && !next) {
		if (len == sizeof(prof_name)) {
			read_cb(cb_arg, prof_name, sizeof(prof_name));
			for (uint8_t i = 0; i < JC_PROFILES; i++) {
				prof_name[i][JC_PROF_NAME - 1] = '\0';
			}
		}
		return 0;
	}
	if (settings_name_steq(name, "pkey", &next) && !next) {
		if (len == sizeof(prof_key)) {
			read_cb(cb_arg, prof_key, sizeof(prof_key));
		}
		return 0;
	}
	if (settings_name_steq(name, "sync", &next) && !next) {
		if (len == sizeof(sync_key)) {
			read_cb(cb_arg, &sync_key, sizeof(sync_key));
		}
		return 0;
	}
	if (settings_name_steq(name, "grip", &next) && !next) {
		uint8_t on;

		if (len == sizeof(on) &&
		    read_cb(cb_arg, &on, sizeof(on)) == (ssize_t)sizeof(on)) {
			grip_byte = on ? 0x01 : -1;
		}
		return 0;
	}
	if (!settings_name_steq(name, "cfg", &next) || next) {
		return -ENOENT;
	}
	if (len != sizeof(tmp)) {
		printk("[CFG] stored record rejected: %u bytes, expected %u\n",
		       (unsigned)len, (unsigned)sizeof(tmp));
		return 0;
	}

	n = read_cb(cb_arg, &tmp, sizeof(tmp));
	if (n != (ssize_t)sizeof(tmp)) {
		return 0;
	}
	if (!cfg_record_ok(&tmp)) {
		return 0;
	}

	memcpy(&CFG, &tmp, sizeof(CFG));
	cfg_settings_apply();
	return 0;
}

SETTINGS_STATIC_HANDLER_DEFINE(jc_cfg, "jc", NULL, cfg_set, NULL, NULL);

static int cfg_storage_load(void)
{
	int rc = settings_subsys_init();

	if (rc) {
		return 0;
	}
	rc = settings_load_subtree("jc");
	if (prof_filled(prof_active)) {
		cfg_apply(&prof_slot[prof_active - 1]);
	}
	return 0;
}
SYS_INIT(cfg_storage_load, APPLICATION, 0);

#define CFG_CMD_MAX 400

static void scan_print(void);
static void scan_restart(void);
static void peer_print(void);
static void peer_set(const char *arg);
static void peer_clear(void);
static void peer_commit(const bt_addr_le_t *addr);
static void mode_set(uint8_t m);
static void sync_cmd(const char *a);

static char cfg_cmd[CFG_CMD_MAX];
static volatile size_t cfg_cmd_len;
static volatile bool cfg_cmd_ready;

static struct jc_cfg cfg_tmp;

static int hex_nib(char c)
{
	if (c >= '0' && c <= '9') return c - '0';
	if (c >= 'a' && c <= 'f') return c - 'a' + 10;
	if (c >= 'A' && c <= 'F') return c - 'A' + 10;
	return -1;
}

static void cfg_print(void)
{
	const uint8_t *p = (const uint8_t *)&CFG;

	printk("[CFG] blob ");
	for (size_t i = 0; i < sizeof(CFG); i++) {
		printk("%02X", p[i]);
	}
	printk("\n");
}

static void cfg_apply(const struct jc_cfg *c)
{
	k_sched_lock();
	memcpy(&CFG, c, sizeof(CFG));
	k_sched_unlock();
	cfg_settings_apply();
}

static void cfg_cmd_write(const char *hex)
{
	uint8_t *out = (uint8_t *)&cfg_tmp;
	size_t n = 0;
	int rc;

	while (hex[n * 2] && n < sizeof(cfg_tmp)) {
		int hi = hex_nib(hex[n * 2]);
		int lo = hex_nib(hex[n * 2 + 1]);

		if (hi < 0 || lo < 0) {
			printk("[CFG] write rejected: bad hex at byte %u\n",
			       (unsigned)n);
			return;
		}
		out[n] = (uint8_t)((hi << 4) | lo);
		n++;
	}
	if (n != sizeof(cfg_tmp) || hex[n * 2]) {
		printk("[CFG] write rejected: %u bytes, expected %u\n",
		       (unsigned)n, (unsigned)sizeof(cfg_tmp));
		return;
	}
	if (!cfg_record_ok(&cfg_tmp)) {
		return;
	}

	rc = settings_save_one("jc/cfg", &cfg_tmp, sizeof(cfg_tmp));
	if (rc) {
		printk("[CFG] write FAILED: settings_save_one (%d)\n", rc);
		return;
	}
	cfg_apply(&cfg_tmp);
	printk("[CFG] write ok, %u bytes stored and live\n",
	       (unsigned)sizeof(cfg_tmp));
}

static void sync_key_print(void);

static void cfg_cmd_default(void)
{
	int rc = settings_delete("jc/cfg");

	if (rc) {
		printk("[CFG] delete FAILED (%d)\n", rc);
		return;
	}
	memcpy(&cfg_tmp, (const void *)&CFG_FLASH, sizeof(cfg_tmp));
	cfg_apply(&cfg_tmp);
	printk("[CFG] stored config deleted, back to the built-in blob\n");

	rc = settings_delete("jc/sync");
	sync_key.code = SYNC_KEY;
	sync_key.mods = 0;
	sync_key.on = 1;
	sync_key_print();
}

#define LINK_SYNC1_CFG 0x5B
#define LINK_CFG_GRIP 0x01
#define LINK_CFG_PROFILE 0x02

static volatile uint8_t link_cfg_pending, link_cfg_id, link_cfg_val;
static volatile uint8_t link_rx_cfg_pending, link_rx_cfg_id, link_rx_cfg_val;

static void link_send_cfg(uint8_t id, uint8_t val)
{
	link_cfg_id = id;
	link_cfg_val = val;
	link_cfg_pending = 1;
}

static bool prof_filled(uint8_t n)
{
	return n >= 1 && n <= JC_PROFILES &&
	       (prof_have & (uint8_t)(1U << (n - 1))) != 0;
}

static void prof_store_u8(const char *key, uint8_t v)
{
	settings_save_one(key, &v, sizeof(v));
}

static int prof_save(uint8_t n)
{
	char key[8];
	int rc;

	if (n < 1 || n > JC_PROFILES) {
		return -EINVAL;
	}
	memcpy(&prof_slot[n - 1], &CFG, sizeof(CFG));
	prof_have |= (uint8_t)(1U << (n - 1));
	snprintf(key, sizeof(key), "jc/p%u", n);
	rc = settings_save_one(key, &prof_slot[n - 1], sizeof(struct jc_cfg));
	return rc;
}

static int prof_load(uint8_t n, bool tell_peer)
{
	if (!prof_filled(n)) {
		return -ENOENT;
	}
	cfg_apply(&prof_slot[n - 1]);
	if (prof_active != n) {
		prof_active = n;
		prof_store_u8("jc/pact", n);
	}
	printk("[PROF] now on %u%s%s\n", n,
	       prof_name[n - 1][0] ? ": " : "", prof_name[n - 1]);
	if (tell_peer) {
		link_send_cfg(LINK_CFG_PROFILE, n);
	}
	return 0;
}

static void cfg_cmd_fn(struct k_work *w)
{
	const char *c = cfg_cmd;

	ARG_UNUSED(w);

	while (*c == ' ') {
		c++;
	}
	if (!strncmp(c, "cfg", 3)) {
		c += 3;
		if (*c == '?') {
			cfg_print();
		} else if (*c == ' ' || *c == '\t') {
			while (*c == ' ' || *c == '\t') {
				c++;
			}
			if (!strcmp(c, "default")) {
				cfg_cmd_default();
			} else {
				cfg_cmd_write(c);
			}
		} else if (*c == '\0') {
			cfg_print();
		}
	} else if (!strncmp(c, "scan", 4)) {
		c += 4;
		if (*c == '?' || *c == '\0') {
			scan_print();
		} else if (*c == '!') {
			scan_restart();
		}
	} else if (!strncmp(c, "mode", 4)) {
		c += 4;
		while (*c == ' ' || *c == '\t') {
			c++;
		}
		if (!strcmp(c, "keyboard") || !strcmp(c, "kbd")) {
			mode_set(JC_MODE_KEYBOARD);
		} else if (!strcmp(c, "stick")) {
			mode_set(JC_MODE_STICK);
		} else if (!strcmp(c, "joycon") || !strcmp(c, "jc")) {
			mode_set(JC_MODE_JOYCON);
		} else {
			printk("[MODE] %s\n", jc_mode_name());
		}
	} else if (!strncmp(c, "profile", 7)) {
		char *e;
		long n;

		c += 7;
		while (*c == ' ' || *c == '\t') {
			c++;
		}
		if (!strncmp(c, "save", 4)) {
			prof_save((uint8_t)strtol(c + 4, NULL, 0));
		} else if (!strncmp(c, "load", 4)) {
			prof_load((uint8_t)strtol(c + 4, NULL, 0), true);
		} else if (!strncmp(c, "clear", 5)) {
			n = strtol(c + 5, NULL, 0);
			if (n >= 1 && n <= JC_PROFILES) {
				char key[8];

				prof_have &= (uint8_t)~(1U << (n - 1));
				prof_name[n - 1][0] = '\0';
				snprintf(key, sizeof(key), "jc/p%ld", n);
				settings_delete(key);
				settings_save_one("jc/pname", prof_name,
						  sizeof(prof_name));
				prof_key[n - 1].code = 0;
				prof_key[n - 1].mods = 0;
				settings_save_one("jc/pkey", prof_key,
						  sizeof(prof_key));
			}
		} else if (!strncmp(c, "name", 4)) {
			n = strtol(c + 4, &e, 0);
			while (*e == ' ' || *e == '\t') {
				e++;
			}
			if (n >= 1 && n <= JC_PROFILES) {
				strncpy(prof_name[n - 1], e, JC_PROF_NAME - 1);
				prof_name[n - 1][JC_PROF_NAME - 1] = '\0';
				settings_save_one("jc/pname", prof_name,
						  sizeof(prof_name));
			}
		} else if (!strncmp(c, "key", 3)) {
			long code, mods;

			n = strtol(c + 3, &e, 0);
			code = strtol(e, &e, 0);
			mods = strtol(e, NULL, 0);
			if (n >= 1 && n <= JC_PROFILES) {
				prof_key[n - 1].code = (uint8_t)code;
				prof_key[n - 1].mods = (uint8_t)mods;
				settings_save_one("jc/pkey", prof_key,
						  sizeof(prof_key));
			}
		}
		{
			uint8_t i;

			printk("[PROF] active %u of %u\n", prof_active,
			       JC_PROFILES);
			for (i = 0; i < JC_PROFILES; i++) {
				printk("[PROF] %u %-6s key 0x%02X mods 0x%02X"
				       " %s\n", i + 1U,
				       prof_filled(i + 1U) ? "saved" : "empty",
				       prof_key[i].code, prof_key[i].mods,
				       prof_name[i]);
			}
		}
	} else if (!strncmp(c, "grip", 4)) {
		c += 4;
		while (*c == ' ' || *c == '\t') {
			c++;
		}
		if (!strcmp(c, "on") || !strcmp(c, "off")) {
			uint8_t on = (c[1] == 'n');

			grip_byte = on ? 0x01 : -1;
			grip_wait = -1;
			settings_save_one("jc/grip", &on, sizeof(on));
			link_send_cfg(LINK_CFG_GRIP, on);
		} else if (!strcmp(c, "now")) {
			grip_byte = 0x01;
			grip_wait = 0;
		} else if (*c) {
			grip_byte = (int16_t)(strtol(c, NULL, 0) & 0xFF);
			grip_wait = 0;
		}
		if (grip_byte < 0) {
			printk("[GRIP] off: byte 0x04 replayed (0x07, not in a "
			       "grip)\n");
		} else {
			printk("[GRIP] byte 0x04 = 0x%02X%s\n", grip_byte,
			       grip_byte == 0x01 ? " (IN THE CHARGING GRIP,"
						   " as a real one reports)" : "");
		}
	} else if (!strncmp(c, "sync", 4)) {
		sync_cmd(c + 4);
	} else if (!strncmp(c, "peer", 4)) {
		c += 4;
		if (*c == '?' || *c == '\0') {
			peer_print();
		} else if (*c == ' ' || *c == '\t') {
			while (*c == ' ' || *c == '\t') {
				c++;
			}
			if (!strcmp(c, "none") || !strcmp(c, "any")) {
				peer_clear();
			} else {
				peer_set(c);
			}
		}
	}

	cfg_cmd_len = 0;
	cfg_cmd_ready = false;
}
static K_WORK_DEFINE(cfg_cmd_work, cfg_cmd_fn);

static void cfg_uart_isr(const struct device *dev, void *user_data)
{
	uint8_t c;

	ARG_UNUSED(user_data);

	uart_irq_update(dev);
	while (uart_irq_rx_ready(dev)) {
		if (uart_fifo_read(dev, &c, 1) != 1) {
			break;
		}
		if (cfg_cmd_ready) {
			continue;
		}
		if (c == '\r' || c == '\n') {
			if (cfg_cmd_len) {
				cfg_cmd[cfg_cmd_len] = '\0';
				cfg_cmd_ready = true;
				k_work_submit(&cfg_cmd_work);
			}
			continue;
		}
		if (cfg_cmd_len < sizeof(cfg_cmd) - 1) {
			cfg_cmd[cfg_cmd_len++] = (char)c;
		}
	}
}

static void cfg_serial_init(void)
{
	const struct device *con = DEVICE_DT_GET(DT_CHOSEN(zephyr_console));

	if (!device_is_ready(con)) {
		return;
	}
	uart_irq_rx_disable(con);
	uart_irq_tx_disable(con);
	uart_irq_callback_user_data_set(con, cfg_uart_isr, NULL);
	uart_irq_rx_enable(con);
}

#define LINK_SYNC_REQ 0x2000
#define LINK_STICK 0x0800

#define SYNC_HOLD_MS 2000

#define LINK_SYNC0 0xA5
#define LINK_SYNC1 0x5A
#define LINK_LEN 5

#define LINK_STALE_MS 400

#define REPORT_MS 15
#define REPORT_SLOW_MS 25
#define REPORT_MID_MS 18
#define LINK_MID_US 7500
#define LINK_SLOW_US 12000
static uint16_t report_ms = REPORT_MS;

static uint32_t link_us;

static void report_pace(uint32_t interval_us)
{
	uint16_t want;

	if (interval_us) {
		link_us = interval_us;
	}
	want = (link_us >= LINK_SLOW_US) ? REPORT_SLOW_MS
	     : (link_us >= LINK_MID_US) ? REPORT_MID_MS
					 : REPORT_MS;
	if (want == report_ms) {
		return;
	}
	report_ms = want;
}

#define AIRBORNE_MS 5000
#define CONFIRM_AT_MS 3000
#define CONFIRM_FOR_MS 1200

static struct bt_conn *g_mouse;
#if defined(JC_LEFT)
static uint8_t kbd_mods;
static uint8_t kbd_keys[6];

static bool mode_combo_held;

static bool kbd_suppress;

static bool prof_combo_held;

static uint8_t prof_combo_wanted(void)
{
	for (uint8_t n = 0; n < JC_PROFILES; n++) {
		uint8_t need = prof_key[n].mods;

		if (!prof_key[n].code || (kbd_mods & need) != need) {
			continue;
		}
		for (int i = 0; i < 6; i++) {
			if (kbd_keys[i] && kbd_keys[i] == prof_key[n].code) {
				return (uint8_t)(n + 1);
			}
		}
	}
	return 0;
}

static uint8_t mode_combo_wanted(void)
{
	uint8_t nb = CFG.n_btn > JC_CFG_BTNS ? JC_CFG_BTNS : CFG.n_btn;

	for (uint8_t j = 0; j < nb; j++) {
		uint8_t need;

		if (CFG.btn[j].kind != JC_BTN_MODE ||
		    (CFG.btn[j].mask >> 8) > JC_MODE_STICK) {
			continue;
		}
		need = CFG.btn[j].mask & 0xFF;
		if ((kbd_mods & need) != need) {
			continue;
		}
		for (int i = 0; i < 6; i++) {
			if (kbd_keys[i] && kbd_keys[i] == CFG.btn[j].code) {
				return (uint8_t)((CFG.btn[j].mask >> 8) + 1);
			}
		}
	}
	return 0;
}

static bool sync_key_down(void)
{
	if (!sync_key.on || !sync_key.code || !jc_live()) {
		return false;
	}
	if ((kbd_mods & sync_key.mods) != sync_key.mods) {
		return false;
	}
	for (int i = 0; i < 6; i++) {
		if (kbd_keys[i] == sync_key.code) {
			return true;
		}
	}
	return false;
}
#endif
static uint16_t mouse_reports;

static uint8_t mouse_buttons;

#if !defined(JC_LEFT)
static int16_t mouse_dx, mouse_dy;
#define RS_N 32

struct rs_knot {
	int64_t t;
	int64_t x, y;
};

static struct rs_knot rs_ring[RS_N];
static uint8_t rs_head;
static uint8_t rs_count = 1;
static int32_t rs_span = 11250;
static const int32_t rs_margin = 2000;
static int64_t rs_ex, rs_ey;
static bool rs_sync = true;
static struct k_spinlock rs_lock;

#define MOUSE_GAIN_DEFAULT 250
#define MOUSE_GAIN_MIN 10
#define MOUSE_GAIN_MAX 1000
static uint16_t mouse_gain = MOUSE_GAIN_DEFAULT;
static int32_t mg_rem_x, mg_rem_y;

static bool accel_on;
static const uint16_t accel_lo = 100;
static const int32_t accel_slow = 300;
static const int32_t accel_fast = 3000;
static int32_t accel_rate;
static int32_t accel_acc;
static const int32_t accel_tau = 200000;
static int64_t accel_t;

static void accel_decay(int64_t now)
{
	int64_t dt = now - accel_t;
	int32_t d;

	if (dt <= 0) {
		return;
	}
	accel_t = now;
	if (dt > 500000) {
		accel_acc = 0;
		accel_rate = 0;
		return;
	}
	d = (int32_t)((int64_t)accel_acc * dt / accel_tau);
	accel_acc = (d >= accel_acc) ? 0 : accel_acc - d;
	accel_rate = (int32_t)((int64_t)accel_acc * 1000000 / accel_tau);
}

static void accel_feed(int16_t x, int16_t y, int64_t now)
{
	accel_decay(now);
	accel_acc += (x < 0 ? -x : x) + (y < 0 ? -y : y);
	accel_rate = (int32_t)((int64_t)accel_acc * 1000000 / accel_tau);
}

static uint16_t gain_now(void)
{
	int32_t r, span;
	uint16_t hi = mouse_gain;

	if (!accel_on || hi <= accel_lo) {
		return hi;
	}
	accel_decay(k_ticks_to_us_floor64(k_uptime_ticks()));
	r = accel_rate;
	if (r <= accel_slow) {
		return accel_lo;
	}
	if (r >= accel_fast) {
		return hi;
	}
	span = accel_fast - accel_slow;
	return (uint16_t)((int32_t)accel_lo +
			  (int32_t)(hi - accel_lo) * (r - accel_slow) / span);
}

static int16_t mouse_gain_one(int16_t v, int32_t *rem)
{
	int32_t n;
	uint16_t g = gain_now();

	if (g == 100) {
		*rem = 0;
		return v;
	}
	n = (int32_t)v * (int32_t)g + *rem;
	*rem = n % 100;
	n /= 100;
	return (n > 32767) ? 32767 : (n < -32768) ? -32768 : (int16_t)n;
}

static void mouse_gain_apply(int16_t *x, int16_t *y)
{
	*x = mouse_gain_one(*x, &mg_rem_x);
	*y = mouse_gain_one(*y, &mg_rem_y);
}

static void rs_push(int16_t x, int16_t y)
{
	int64_t now = k_ticks_to_us_floor64(k_uptime_ticks());
	k_spinlock_key_t key;
	struct rs_knot *last;
	int64_t gap;

	accel_feed(x, y, now);
	key = k_spin_lock(&rs_lock);
	last = &rs_ring[rs_head];
	gap = now - last->t;

	if (rs_count > 1 && gap < 2000) {
		last->x += x;
		last->y += y;
		k_spin_unlock(&rs_lock, key);
		return;
	}
	if (rs_count > 1 && gap < 40000) {
		rs_span += (int32_t)((gap - rs_span) / 8);
	}
	rs_head = (uint8_t)((rs_head + 1U) % RS_N);
	rs_ring[rs_head].t = now;
	rs_ring[rs_head].x = last->x + x;
	rs_ring[rs_head].y = last->y + y;
	if (rs_count < RS_N) {
		rs_count++;
	}
	k_spin_unlock(&rs_lock, key);
}

static void rs_at(int64_t tau, int64_t *ox, int64_t *oy)
{
	uint8_t i = rs_head;
	uint8_t n = rs_count;
	const struct rs_knot *b = &rs_ring[i];

	if (tau >= b->t) {
		*ox = b->x;
		*oy = b->y;
		return;
	}
	while (n > 1) {
		const struct rs_knot *a;
		int64_t s;

		i = (uint8_t)((i + RS_N - 1U) % RS_N);
		a = &rs_ring[i];
		s = b->t - rs_span;
		if (s < a->t) {
			s = a->t;
		}
		if (tau > s) {
			*ox = a->x + (b->x - a->x) * (tau - s) / (b->t - s);
			*oy = a->y + (b->y - a->y) * (tau - s) / (b->t - s);
			return;
		}
		if (tau > a->t) {
			*ox = a->x;
			*oy = a->y;
			return;
		}
		b = a;
		n--;
	}
	*ox = b->x;
	*oy = b->y;
}

static void rs_emit(int16_t *ox, int16_t *oy)
{
	int64_t now = k_ticks_to_us_floor64(k_uptime_ticks());
	int64_t tx, ty, dx, dy;
	k_spinlock_key_t key = k_spin_lock(&rs_lock);

	rs_at(now - rs_span - rs_margin, &tx, &ty);
	k_spin_unlock(&rs_lock, key);

	if (rs_sync) {
		rs_sync = false;
		rs_ex = tx;
		rs_ey = ty;
	}
	dx = tx - rs_ex;
	dy = ty - rs_ey;
	dx = (dx > 32767) ? 32767 : (dx < -32768) ? -32768 : dx;
	dy = (dy > 32767) ? 32767 : (dy < -32768) ? -32768 : dy;
	rs_ex += dx;
	rs_ey += dy;
	*ox = (int16_t)dx;
	*oy = (int16_t)dy;
}
#endif
static bool mouse_has_moved;

#if !defined(JC_LEFT)
static int8_t mouse_wheel;
static uint16_t wheel_btn, wheel_btn_peer;
static int64_t wheel_btn_until, wheel_btn_peer_until;

static uint16_t wheel_btn_now(void)
{
	if (wheel_btn && k_uptime_get() >= wheel_btn_until) {
		wheel_btn = 0;
	}
	return wheel_btn;
}

static uint16_t wheel_btn_peer_now(void)
{
	if (wheel_btn_peer && k_uptime_get() >= wheel_btn_peer_until) {
		wheel_btn_peer = 0;
	}
	return wheel_btn_peer;
}

static void wheel_btn_feed(int8_t w)
{
	uint8_t want = (w > 0) ? 1U : 2U;
	uint8_t nb = CFG.n_btn > JC_CFG_BTNS ? JC_CFG_BTNS : CFG.n_btn;
	int64_t until = k_uptime_get() + WHEEL_PULSE_MS;

	if (!w) {
		return;
	}
	for (uint8_t j = 0; j < nb; j++) {
		if (CFG.btn[j].code != want) {
			continue;
		}
		if (CFG.btn[j].kind == JC_BTN_WHEEL) {
			wheel_btn |= CFG.btn[j].mask;
			wheel_btn_until = until;
		} else if (CFG.btn[j].kind == JC_BTN_WPEER) {
			wheel_btn_peer |= CFG.btn[j].mask;
			wheel_btn_peer_until = until;
		}
	}
}
#endif

static uint16_t peer_buttons;

#if !defined(JC_LEFT)

static int16_t stick_x, stick_y;

#define STICK_MGAIN_DEFAULT 5
#define STICK_MTHROW_DEFAULT 1300
static int16_t stick_mgain = STICK_MGAIN_DEFAULT;
static const int16_t stick_mthrow = STICK_MTHROW_DEFAULT;

static int32_t stick_fx, stick_fy;

#define STICK_WIN_MAX 24
static int16_t stick_win_x[STICK_WIN_MAX], stick_win_y[STICK_WIN_MAX];

#define STICK_MCX 2124
#define STICK_MCY 1960

static uint32_t isqrt32(uint32_t n);

static const int8_t stick_msy = -1;

#define STICK_FIXED_TOP 10

#define STICK_RANGE_XP 1160
#define STICK_RANGE_XN 1239
#define STICK_RANGE_YP 1258
#define STICK_RANGE_YN 1120
static const int16_t stick_ffloor = 570;
static const int16_t stick_ftop = 850;

static int32_t stick_fixed_permille(void)
{
	int32_t g = stick_mgain, fl = stick_ffloor, top = stick_ftop;

	g = (g < 1) ? 1 : (g > STICK_FIXED_TOP) ? STICK_FIXED_TOP : g;
	if (top > 1000) {
		top = 1000;
	}
	if (fl > top) {
		fl = top;
	}
	return fl + (top - fl) * (g - 1) / (STICK_FIXED_TOP - 1);
}

#define STICK_RANGE_REF 1200
#define STICK_VGAIN_DEFAULT 5
static int16_t stick_vgain = STICK_VGAIN_DEFAULT;

#define STICK_MT_WIN_US 40000
#define STICK_MT_GAP_US 250000
#define STICK_MT_STOP_US 30000
#define STICK_MT_FIRST_US 180000
#define STICK_TICK_US 15000
static const uint8_t stick_vsmooth = 8;

#define STICK_VBAND_LO 500
#define STICK_VBAND_HI 1500
static const uint8_t stick_vslow = 8;

static const int32_t stick_bridge_us = STICK_MT_STOP_US;

static int32_t stick_vsx, stick_vsy;

#define STICK_DZ_DEFAULT 290
static int16_t stick_dz = STICK_DZ_DEFAULT;

struct mt_k {
	int32_t ago;
	int32_t x, y;
};

static int32_t stick_mt_axis(const struct mt_k *k, uint8_t n, uint8_t m,
			     bool on_y, int32_t jv)
{
	int b = -1, a = -1, rev = -1, cnt = 0, j;
	int64_t dp, base, e, stop;
	bool up;

#define MT_P(i) (on_y ? k[(i)].y : k[(i)].x)
	for (j = 0; j < m && j + 1 < n; j++) {
		if (MT_P(j) != MT_P(j + 1)) {
			b = j;
			break;
		}
	}
	if (b < 0) {
		return 0;
	}
	up = MT_P(b) > MT_P(b + 1);
	for (j = b + 1; j < m && j + 1 < n; j++) {
		if (MT_P(j) == MT_P(j + 1)) {
			continue;
		}
		if (k[j].ago - k[b].ago > STICK_MT_GAP_US) {
			break;
		}
		if ((MT_P(j) > MT_P(j + 1)) != up) {
			rev = j;
			break;
		}
		a = j;
		cnt++;
		if (k[a].ago - k[b].ago >= STICK_MT_WIN_US) {
			break;
		}
	}
	if (a >= 0) {
		dp = (int64_t)MT_P(b) - MT_P(a);
		base = (int64_t)k[a].ago - k[b].ago;
		e = base / cnt;
		if (base < STICK_MT_WIN_US) {
			base = STICK_MT_WIN_US;
		}
	} else {
		dp = (int64_t)MT_P(b) - MT_P(b + 1);
		if (rev >= 0) {
			base = (int64_t)k[rev].ago - k[b].ago;
			if (base < STICK_MT_WIN_US) {
				base = STICK_MT_WIN_US;
			}
		} else if (k[m - 1].ago > k[b].ago) {
			base = (int64_t)k[m - 1].ago - k[b].ago;
			if (base > STICK_MT_GAP_US) {
				base = STICK_MT_GAP_US;
			}
		} else {
			return jv;
		}
		e = base;
	}
#undef MT_P
	stop = 3 * e;
	if (stop < stick_bridge_us) {
		stop = stick_bridge_us;
	} else if (stop > STICK_MT_GAP_US) {
		stop = STICK_MT_GAP_US;
	}
	if (base <= 0 || k[b].ago > stop) {
		return 0;
	}
	if (k[b].ago > e) {
		base += k[b].ago - e;
	}
	return (int32_t)(dp * 256 * STICK_TICK_US / base);
}

static void stick_mt_speed(int32_t *vx, int32_t *vy)
{
	struct mt_k k[RS_N];
	int64_t now, dxc, dyc, base, e, age, stop;
	k_spinlock_key_t key;
	uint8_t n, m, i;
	bool first = false;
	int32_t jx, jy;

	*vx = 0;
	*vy = 0;
	key = k_spin_lock(&rs_lock);
	now = k_ticks_to_us_floor64(k_uptime_ticks());
	n = rs_count;
	for (i = 0; i < n; i++) {
		const struct rs_knot *q = &rs_ring[(rs_head + RS_N - i) % RS_N];
		int64_t ago = now - q->t;

		k[i].ago = (ago > INT32_MAX) ? INT32_MAX : (int32_t)ago;
		k[i].x = (int32_t)(q->x - rs_ring[rs_head].x);
		k[i].y = (int32_t)(q->y - rs_ring[rs_head].y);
	}
	k_spin_unlock(&rs_lock, key);
	if (n < 2) {
		return;
	}
	for (m = 1; m < n && k[m].ago - k[m - 1].ago <= STICK_MT_GAP_US; m++) {
	}

	if (m == 1) {
		dxc = (int64_t)k[0].x - k[1].x;
		dyc = (int64_t)k[0].y - k[1].y;
		base = rs_span;
		e = rs_span;
		first = true;
	} else {
		uint8_t c = 1;

		while (c + 1 < m && k[c].ago - k[0].ago < STICK_MT_WIN_US) {
			c++;
		}
		dxc = (int64_t)k[0].x - k[c].x;
		dyc = (int64_t)k[0].y - k[c].y;
		base = (int64_t)k[c].ago - k[0].ago;
		e = base / c;
	}

	age = k[0].ago;
	stop = 3 * e;
	if (first) {
		stop = STICK_MT_FIRST_US;
	} else if (stop < stick_bridge_us) {
		stop = stick_bridge_us;
	} else if (stop > STICK_MT_GAP_US) {
		stop = STICK_MT_GAP_US;
	}
	if (base <= 0 || age > stop) {
		return;
	}
	if (age > e) {
		base += age - e;
	}
	jx = (int32_t)(dxc * 256 * STICK_TICK_US / base);
	jy = (int32_t)(dyc * 256 * STICK_TICK_US / base);
	*vx = stick_mt_axis(k, n, m, false, jx);
	*vy = stick_mt_axis(k, n, m, true, jy);
}

static void stick_capped_in(void)
{
	int32_t cap = stick_fixed_permille() * 16;
	int32_t dz = stick_dz * 16;
	int64_t nx, ny;

	int32_t vx, vy, a;

	stick_mt_speed(&vx, &vy);
	vy *= stick_msy;
	if (vx || vy) {
		uint32_t ux = (uint32_t)(vx < 0 ? -vx : vx);
		uint32_t uy = (uint32_t)(vy < 0 ? -vy : vy);
		int sh = 0;
		int64_t sp, f;

		while (ux > 40000 || uy > 40000) {
			ux >>= 1;
			uy >>= 1;
			sh++;
		}
		sp = (int64_t)isqrt32(ux * ux + uy * uy) << sh;
		f = (sp * stick_vgain * 10 / 384 - STICK_VBAND_LO) * 1000 /
		    (STICK_VBAND_HI - STICK_VBAND_LO);

		f = (f < 0) ? 0 : (f > 1000) ? 1000 : f;
		a = (int32_t)(stick_vslow * 16 +
			      ((int32_t)stick_vsmooth - stick_vslow) * 16 *
			      f / 1000);
	} else {
		a = 14 * 16;
	}
	stick_vsx += (int32_t)((int64_t)(vx - stick_vsx) * a / 256);
	stick_vsy += (int32_t)((int64_t)(vy - stick_vsy) * a / 256);
	if (!vx && !vy && stick_vsx > -32 && stick_vsx < 32 &&
	    stick_vsy > -32 && stick_vsy < 32) {
		stick_vsx = 0;
		stick_vsy = 0;
	}
	nx = (int64_t)stick_vsx * stick_vgain * 8 * 1000 * 16 /
	     ((int64_t)STICK_RANGE_REF * 256);
	ny = (int64_t)stick_vsy * stick_vgain * 8 * 1000 * 16 /
	     ((int64_t)STICK_RANGE_REF * 256);
	int32_t ax = (int32_t)(nx < 0 ? -nx : nx);
	int32_t ay = (int32_t)(ny < 0 ? -ny : ny);
	uint32_t m;

	while (ax > 46000 || ay > 46000) {
		nx /= 2;
		ny /= 2;
		ax /= 2;
		ay /= 2;
	}
	m = isqrt32((uint32_t)(nx * nx) + (uint32_t)(ny * ny));
	if (m > 0) {
		int64_t mc = (int64_t)m + dz;

		if (mc > cap) {
			mc = cap;
		}
		nx = nx * mc / m;
		ny = ny * mc / m;
	}
	stick_fx = (int32_t)(nx * (nx >= 0 ? STICK_RANGE_XP : STICK_RANGE_XN) /
			     1000);
	stick_fy = (int32_t)(ny * (ny >= 0 ? STICK_RANGE_YP : STICK_RANGE_YN) /
			     1000);
}

static int32_t stick_ex, stick_ey;

static void stick_ease_step(void)
{
	stick_ex = stick_fx;
	stick_ey = stick_fy;
}

static void stick_mouse_in(void)
{
	const int32_t lim = (int32_t)stick_mthrow * 16;

	stick_capped_in();
	stick_fx = (stick_fx > lim) ? lim : (stick_fx < -lim) ? -lim : stick_fx;
	stick_fy = (stick_fy > lim) ? lim : (stick_fy < -lim) ? -lim : stick_fy;
	stick_ease_step();
	stick_x = (int16_t)(stick_ex / 16);
	stick_y = (int16_t)(stick_ey / 16);
}

static bool stick_on(void)
{
	return jc_mode == JC_MODE_STICK ||
	       (peer_buttons & LINK_STICK) != 0;
}

static bool ptr_off(void)
{
	return stick_on();
}

#endif

static void cfg_settings_apply(void)
{
#if !defined(JC_LEFT)
	uint8_t nb = CFG.n_btn > JC_CFG_BTNS ? JC_CFG_BTNS : CFG.n_btn;
	int16_t gain = STICK_MGAIN_DEFAULT;
	uint16_t mgain = MOUSE_GAIN_DEFAULT;
	bool accel = false;
	int16_t dz = STICK_DZ_DEFAULT;
	int16_t vg = STICK_VGAIN_DEFAULT;

	for (uint8_t j = 0; j < nb; j++) {
		if (CFG.btn[j].kind != JC_BTN_SET) {
			continue;
		}
		if (CFG.btn[j].code == JC_SET_STICK_GAIN) {
			uint16_t v = CFG.btn[j].mask;

			if (v >= 1 && v <= 512) {
				gain = (int16_t)v;
			}
		} else if (CFG.btn[j].code == JC_SET_MOUSE_GAIN) {
			uint16_t v = CFG.btn[j].mask;

			if (v >= MOUSE_GAIN_MIN && v <= MOUSE_GAIN_MAX) {
				mgain = v;
			}
		} else if (CFG.btn[j].code == JC_SET_ACCEL) {
			accel = (CFG.btn[j].mask != 0);
		} else if (CFG.btn[j].code == JC_SET_STICK_DZ) {
			if (CFG.btn[j].mask <= 400) {
				dz = (int16_t)CFG.btn[j].mask;
			}
		} else if (CFG.btn[j].code == JC_SET_STICK_VG) {
			if (CFG.btn[j].mask >= 1 && CFG.btn[j].mask <= 64) {
				vg = (int16_t)CFG.btn[j].mask;
			}
		}
	}
	accel_on = accel;
	stick_mgain = gain;
	stick_dz = dz;
	stick_vgain = vg;
	if (mouse_gain != mgain) {
		mouse_gain = mgain;
		mg_rem_x = 0;
		mg_rem_y = 0;
	}
#endif
}

static struct bt_gatt_discover_params disc;
static struct bt_uuid_16 disc_uuid;
static uint16_t hid_svc_start, hid_svc_end;
enum { DISC_PRIMARY, DISC_PROTO, DISC_REPORT, DISC_BATT };
static uint8_t stage;
static bool hid_disc_started;

static uint8_t batt_pct = 100;
static bool batt_known;
static uint16_t batt_handle;
static bool batt_read_busy;

static void batt_forget(void)
{
	batt_known = false;
	batt_pct = 100;
	batt_handle = 0;
	batt_read_busy = false;
}

static uint8_t batt_now(void)
{
	return batt_pct;
}

static bool batt_have(void)
{
	return batt_known;
}

static uint8_t batt_level(void)
{
	uint8_t pct = batt_now();
	uint8_t lvl = (uint8_t)((pct * 8 + 50) / 100);

	if (lvl == 0 && pct > 0) {
		lvl = 1;
	}
	return lvl > 8 ? 8 : lvl;
}

static void batt_fill_power(uint8_t *out)
{
	uint16_t mv;
	uint8_t pct;

	memcpy(out, RSP_01_0C, 4);
	if (!batt_have()) {
		return;
	}
	pct = batt_now();
	mv = (uint16_t)(3300U + (uint32_t)pct * 900U / 100U);
	out[0] = pct;
	out[2] = (uint8_t)(mv & 0xFF);
	out[3] = (uint8_t)(mv >> 8);
}

static struct bt_gatt_subscribe_params batt_sub;
static struct bt_gatt_discover_params batt_disc;
static struct bt_gatt_read_params rd_params;
static struct bt_gatt_read_params batt_rd;
#define MAX_RPT 8
static struct bt_gatt_subscribe_params sub_params[MAX_RPT];
static struct bt_gatt_discover_params sub_disc[MAX_RPT];
static uint8_t sub_count;
static uint16_t proto_mode_handle;

static uint16_t rpt_handle;

struct bond_keep {
	const bt_addr_le_t *console;
	const bt_addr_le_t *peer;
};

static bt_addr_le_t bond_list[CONFIG_BT_MAX_PAIRED];
static uint8_t bond_count;

static void collect_bond(const struct bt_bond_info *info, void *user_data)
{
	const struct bond_keep *k = user_data;

	if (k) {
		if (k->console && !bt_addr_le_cmp(&info->addr, k->console)) {
			return;
		}
		if (k->peer && !bt_addr_le_cmp(&info->addr, k->peer)) {
			return;
		}
	}
	if (bond_count < ARRAY_SIZE(bond_list)) {
		bt_addr_le_copy(&bond_list[bond_count++], &info->addr);
	}
}

static const struct bt_gatt_attr *rpt_hid_attr;
static bool streaming;
static int64_t stream_started_at;
static uint32_t report_seq;

static bool hid_reattached;

#define RPT_MAX_INFLIGHT 2
static atomic_t rpt_inflight;

static void rpt_sent_cb(struct bt_conn *conn, void *user_data)
{
	ARG_UNUSED(conn);
	ARG_UNUSED(user_data);
	atomic_dec(&rpt_inflight);
}

#define QGATE_ON 3
static const uint8_t qgate = QGATE_ON;

static bool ctlr_q_hold(void)
{
	int used = CONFIG_BT_BUF_ACL_TX_COUNT -
		   (int)k_sem_count_get(&bt_dev.le.acl_pkts);
	bool hold;

	if (used < 0) {
		used = 0;
	}
	hold = qgate && used >= qgate;
	return hold;
}

static uint8_t pending_host[6];
static bool host_pending;
static bt_addr_le_t console_addr;
static bool console_known;

static uint8_t console_host[6];
static bool console_bonded;
static struct bt_conn *conn_refused;

static int64_t pairable_until;
#define PAIRABLE_MS 120000
static bool asked_slower;
static bool hid_params_asked;
static int64_t hid_usable_at;
#define HID_PARAM_DELAY_MS 2000

static void hid_params_request(void)
{
	static const struct bt_le_conn_param fast = {
		.interval_min = 9,
		.interval_max = 12,
		.latency = 0,
		.timeout = 400,
	};
	struct bt_conn_info info;
	struct bt_conn *c;

	k_sched_lock();
	c = g_mouse ? bt_conn_ref(g_mouse) : NULL;
	k_sched_unlock();
	if (!c) {
		return;
	}
	if (!bt_conn_get_info(c, &info) && info.type == BT_CONN_TYPE_LE &&
	    !info.le.latency && info.le.interval_us <= fast.interval_max * 1250U) {
		bt_conn_unref(c);
		hid_params_asked = true;
		return;
	}
	bt_conn_le_param_update(c, &fast);
	bt_conn_unref(c);

	hid_params_asked = true;
}

static void hid_param_work_fn(struct k_work *work)
{
	ARG_UNUSED(work);
	if (hid_usable_at && !hid_params_asked) {
		hid_params_request();
	}
}
static K_WORK_DELAYABLE_DEFINE(hid_param_work, hid_param_work_fn);

static bool user_active;

static struct bt_conn *g_conn;

static uint8_t vendor_desc[2];

static ssize_t rd_stub(struct bt_conn *conn, const struct bt_gatt_attr *attr,
		       void *buf, uint16_t len, uint16_t offset)
{
	return bt_gatt_attr_read(conn, attr, buf, len, offset, "", 0);
}

static uint8_t cfg1_value[7] = { 0x04, 0x00, 0x05, 0x00, 0x01, 0x01, 0x00 };

#if defined(JC_LEFT)
static const uint8_t cfg3_value[8] = {
	0x35, 0xDC, 0xDF, 0xEC, 0xFC, 0x92, 0xEE, 0x19,
};
#else
static const uint8_t cfg3_value[8] = {
	0x29, 0x31, 0x95, 0x87, 0x06, 0x4C, 0x0A, 0x9F,
};
#endif

static ssize_t rd_cfg1(struct bt_conn *conn, const struct bt_gatt_attr *attr,
		       void *buf, uint16_t len, uint16_t offset)
{
	return bt_gatt_attr_read(conn, attr, buf, len, offset,
				 cfg1_value, sizeof(cfg1_value));
}

static ssize_t rd_cfg3(struct bt_conn *conn, const struct bt_gatt_attr *attr,
		       void *buf, uint16_t len, uint16_t offset)
{
	return bt_gatt_attr_read(conn, attr, buf, len, offset,
				 cfg3_value, sizeof(cfg3_value));
}

#define CMD_PREFIX 17

static const struct bt_gatt_attr *cmd_rsp_attr;

struct pending_rsp {
	uint16_t len;
	uint8_t data[128];
};
K_MSGQ_DEFINE(rsp_q, sizeof(struct pending_rsp), 4, 4);

static void send_response(uint8_t cmd, uint8_t sub,
			  const uint8_t *payload, uint16_t len)
{
	uint8_t buf[128];

	if (!g_conn || !cmd_rsp_attr) {
		return;
	}
	buf[0] = cmd; buf[1] = 0x01; buf[2] = 0x01; buf[3] = sub;
	buf[4] = 0x10; buf[5] = 0x78; buf[6] = 0x00; buf[7] = 0x00;
	if (len > sizeof(buf) - 8) {
		len = sizeof(buf) - 8;
	}
	if (payload && len) {
		memcpy(buf + 8, payload, len);
	}

	int err = bt_gatt_notify(g_conn, cmd_rsp_attr, buf, 8 + len);

	if (err == -ENOMEM) {
		struct pending_rsp p;

		p.len = 8 + len;
		memcpy(p.data, buf, p.len);
		k_msgq_put(&rsp_q, &p, K_NO_WAIT);
	}
}

static ssize_t wr_cmd(struct bt_conn *conn, const struct bt_gatt_attr *attr,
		      const void *buf, uint16_t len, uint16_t offset, uint8_t flags)
{
	const uint8_t *d = buf;

	if (len < CMD_PREFIX + 8) {
		return len;
	}
	{
		const uint8_t *f = d + CMD_PREFIX;
		const uint8_t cmd = f[0], sub = f[3];
		uint8_t rsp[48];

		const uint8_t *payload = f + 8;

		if (cmd == 0x07 && sub == 0x01) {
			rsp[0] = 0x00;
			send_response(cmd, sub, rsp, 1);
			return len;
		}

		if (cmd == 0x02 && sub == 0x04) {
			const uint32_t addr = payload[4] | (payload[5] << 8) |
					      (payload[6] << 16);
			const uint8_t *region = NULL;
			uint16_t rlen = 0;

			for (size_t i = 0; i < ARRAY_SIZE(SPI_REGIONS); i++) {
				if (SPI_REGIONS[i].addr == addr) {
					region = SPI_REGIONS[i].data;
					rlen = SPI_REGIONS[i].len;
					break;
				}
			}
			if (!region) {
				uint8_t z[40];

				memset(z, 0, sizeof(z));
				z[0] = payload[0];
				send_response(cmd, sub, z, sizeof(z));
				return len;
			}
			if (addr == 0x013000 && rlen > 44) {
				static uint8_t spibuf[96];
				const uint16_t n = rlen < sizeof(spibuf) ? rlen
								: sizeof(spibuf);
				memcpy(spibuf, region, n);
				for (int c = 0; c < 3; c++) {
					spibuf[33 + c] = CFG.col_body[c];
					spibuf[36 + c] = CFG.col_buttons[c];
					spibuf[39 + c] = CFG.col_rail[c];
					spibuf[42 + c] = CFG.col_stick[c];
				}
				send_response(cmd, sub, spibuf, n);
				return len;
			}
			send_response(cmd, sub, region, rlen);
			return len;
		}

		if (cmd == 0x15 && sub == 0x01) {
			rsp[0] = 0x01; rsp[1] = 0x04; rsp[2] = 0x01;
			memcpy(rsp + 3, FAKE_ADDR, 6);
			send_response(cmd, sub, rsp, 9);
			return len;
		}
		if (cmd == 0x15 && sub == 0x04) {
			for (int i = 0; i < 16; i++) {
				pair_ltk[i] = payload[1 + i] ^ CONTROLLER_PUBKEY[i];
			}
			have_ltk = true;
			rsp[0] = 0x01;
			memcpy(rsp + 1, CONTROLLER_PUBKEY, 16);
			send_response(cmd, sub, rsp, 17);
			return len;
		}
		if (cmd == 0x15 && sub == 0x02) {
			uint8_t key_rev[16], chal_rev[16], confirm[16];

			if (!have_ltk) {
				rsp[0] = 0x00;
				send_response(cmd, sub, rsp, 1);
				return len;
			}
			for (int i = 0; i < 16; i++) {
				key_rev[i] = pair_ltk[15 - i];
				chal_rev[i] = payload[1 + 15 - i];
			}
			bt_encrypt_be(key_rev, chal_rev, confirm);
			rsp[0] = 0x01;
			memcpy(rsp + 1, confirm, 16);
			send_response(cmd, sub, rsp, 17);
			return len;
		}
		if (cmd == 0x15 && sub == 0x03) {
			if (have_ltk) {
				struct bt_keys *k =
					bt_keys_get_addr(BT_ID_DEFAULT,
							 bt_conn_get_dst(conn));

				if (k) {
					memset(&k->ltk, 0, sizeof(k->ltk));
					memcpy(k->ltk.val, pair_ltk, 16);
					k->enc_size = 16;
					k->keys |= BT_KEYS_LTK_P256;
					k->flags |= BT_KEYS_AUTHENTICATED;
					{ extern void console_save(const bt_addr_le_t *, const uint8_t *); console_save(bt_conn_get_dst(conn), pair_ltk); }
				}
			}
			memcpy(pending_host, bt_conn_get_dst(conn)->a.val, 6);
			host_pending = true;

			rsp[0] = 0x01;
			send_response(cmd, sub, rsp, 1);
			return len;
		}

		if (cmd == 0x0C && (sub == 0x02 || sub == 0x04)) {
			if (sub == 0x02) {
				streaming = true;
				stream_started_at = k_uptime_get();
			}
			memset(rsp, 0, 4);
			send_response(cmd, sub, rsp, 4);
			return len;
		}
		if (cmd == 0x09 && sub == 0x07) {
			send_response(cmd, sub, NULL, 0);
			return len;
		}
		if (cmd == 0x10 && sub == 0x01) {
			send_response(cmd, sub, RSP_10_01, sizeof(RSP_10_01));
			return len;
		}
		if (cmd == 0x08 && sub == 0x01) {
			send_response(cmd, sub, RSP_08_01, sizeof(RSP_08_01));
			return len;
		}
		if (cmd == 0x08 && sub == 0x02) {
			send_response(cmd, sub, NULL, 0);
			return len;
		}
		if (cmd == 0x16 && sub == 0x01) {
			memset(rsp, 0, 24);
			send_response(cmd, sub, rsp, 24);
			return len;
		}
		if (cmd == 0x11 && sub == 0x03) {
			send_response(cmd, sub, RSP_11_03, sizeof(RSP_11_03));
			return len;
		}
		if (cmd == 0x11 && sub == 0x01) {
			send_response(cmd, sub, RSP_11_01, sizeof(RSP_11_01));
			return len;
		}
		if (cmd == 0x01 && sub == 0x0C) {
			{
				uint8_t pw[4];

				if (grip_byte >= 0 && grip_wait < 0) {
					grip_wait = GRIP_WAIT_REPORTS;
				}
				batt_fill_power(pw);
				send_response(cmd, sub, pw, sizeof(pw));
			}
			return len;
		}
		if (cmd == 0x0A && (sub == 0x02 || sub == 0x08)) {
			send_response(cmd, sub, NULL, 0);
			return len;
		}

		rsp[0] = 0x00;
		send_response(cmd, sub, rsp, 1);
	}
	return len;
}

static ssize_t wr_stub(struct bt_conn *conn, const struct bt_gatt_attr *attr,
		       const void *buf, uint16_t len, uint16_t offset, uint8_t flags)
{
	return len;
}

static ssize_t wr_desc(struct bt_conn *conn, const struct bt_gatt_attr *attr,
		       const void *buf, uint16_t len, uint16_t offset, uint8_t flags)
{
	if (len && len <= sizeof(vendor_desc)) {
		memcpy(vendor_desc, buf, len);
	}
	return len;
}

static void ccc_cfg(const struct bt_gatt_attr *attr, uint16_t value)
{
}

BT_GATT_SERVICE_DEFINE(_0a_svc_cfg,
	BT_GATT_PRIMARY_SERVICE(UUID_SVC_CFG),
	BT_GATT_CHARACTERISTIC(UUID_CFG1, BT_GATT_CHRC_READ,
			       BT_GATT_PERM_READ, rd_cfg1, NULL, NULL),
	BT_GATT_CHARACTERISTIC(UUID_CFG2, BT_GATT_CHRC_WRITE,
			       BT_GATT_PERM_WRITE, NULL, wr_stub, NULL),
	BT_GATT_CHARACTERISTIC(UUID_CFG3, BT_GATT_CHRC_READ,
			       BT_GATT_PERM_READ, rd_cfg3, NULL, NULL),
);

BT_GATT_SERVICE_DEFINE(_0b_svc_main,
	BT_GATT_PRIMARY_SERVICE(UUID_SVC_MAIN),
	BT_GATT_CHARACTERISTIC(UUID_N000A, BT_GATT_CHRC_NOTIFY | BT_GATT_CHRC_READ,
			       BT_GATT_PERM_READ, rd_stub, NULL, NULL),
	BT_GATT_CCC(ccc_cfg, BT_GATT_PERM_READ | BT_GATT_PERM_WRITE),
	BT_GATT_DESCRIPTOR(UUID_DESC_VENDOR,
			   BT_GATT_PERM_READ | BT_GATT_PERM_WRITE,
			   rd_stub, wr_desc, NULL),

	BT_GATT_CHARACTERISTIC(UUID_RPTHID, BT_GATT_CHRC_NOTIFY | BT_GATT_CHRC_READ,
			       BT_GATT_PERM_READ, rd_stub, NULL, NULL),
	BT_GATT_CCC(ccc_cfg, BT_GATT_PERM_READ | BT_GATT_PERM_WRITE),
	BT_GATT_DESCRIPTOR(UUID_DESC_VENDOR,
			   BT_GATT_PERM_READ | BT_GATT_PERM_WRITE,
			   rd_stub, wr_desc, NULL),

	BT_GATT_CHARACTERISTIC(UUID_W0012, BT_GATT_CHRC_WRITE_WITHOUT_RESP,
			       BT_GATT_PERM_WRITE, NULL, wr_stub, NULL),
	BT_GATT_CHARACTERISTIC(UUID_W0014, BT_GATT_CHRC_WRITE_WITHOUT_RESP,
			       BT_GATT_PERM_WRITE, NULL, wr_stub, NULL),
	BT_GATT_CHARACTERISTIC(UUID_W0016, BT_GATT_CHRC_WRITE_WITHOUT_RESP,
			       BT_GATT_PERM_WRITE, NULL, wr_cmd, NULL),
	BT_GATT_CHARACTERISTIC(UUID_W0018, BT_GATT_CHRC_WRITE_WITHOUT_RESP,
			       BT_GATT_PERM_WRITE, NULL, wr_stub, NULL),
	BT_GATT_CHARACTERISTIC(UUID_CMDRSP, BT_GATT_CHRC_NOTIFY,
			       BT_GATT_PERM_NONE, NULL, NULL, NULL),
	BT_GATT_CCC(ccc_cfg, BT_GATT_PERM_READ | BT_GATT_PERM_WRITE),
	BT_GATT_DESCRIPTOR(UUID_DESC_VENDOR2,
			   BT_GATT_PERM_READ | BT_GATT_PERM_WRITE,
			   rd_stub, wr_desc, NULL),

	BT_GATT_CHARACTERISTIC(UUID_N001E, BT_GATT_CHRC_NOTIFY,
			       BT_GATT_PERM_NONE, NULL, NULL, NULL),
	BT_GATT_CCC(ccc_cfg, BT_GATT_PERM_READ | BT_GATT_PERM_WRITE),
	BT_GATT_DESCRIPTOR(UUID_DESC_VENDOR2,
			   BT_GATT_PERM_READ | BT_GATT_PERM_WRITE,
			   rd_stub, wr_desc, NULL),

	BT_GATT_CHARACTERISTIC(UUID_N0022, BT_GATT_CHRC_NOTIFY,
			       BT_GATT_PERM_NONE, NULL, NULL, NULL),
	BT_GATT_CCC(ccc_cfg, BT_GATT_PERM_READ | BT_GATT_PERM_WRITE),
	BT_GATT_DESCRIPTOR(UUID_DESC_VENDOR2,
			   BT_GATT_PERM_READ | BT_GATT_PERM_WRITE,
			   rd_stub, wr_desc, NULL),

	BT_GATT_CHARACTERISTIC(UUID_N0026, BT_GATT_CHRC_NOTIFY | BT_GATT_CHRC_READ,
			       BT_GATT_PERM_READ, rd_stub, NULL, NULL),
	BT_GATT_CCC(ccc_cfg, BT_GATT_PERM_READ | BT_GATT_PERM_WRITE),
	BT_GATT_DESCRIPTOR(UUID_DESC_VENDOR,
			   BT_GATT_PERM_READ | BT_GATT_PERM_WRITE,
			   rd_stub, wr_desc, NULL),

	BT_GATT_CHARACTERISTIC(UUID_W002A, BT_GATT_CHRC_WRITE_WITHOUT_RESP,
			       BT_GATT_PERM_WRITE, NULL, wr_stub, NULL),
);

static bool hid_ready(void)
{
	return g_mouse && sub_count && mouse_reports > 0;
}

enum adv_mode { ADV_OFF, ADV_BONDED, ADV_PAIRABLE, ADV_WAKE };

static int64_t wake_until;

static enum adv_mode adv_mode_wanted(void)
{
	if (g_conn) {
		return ADV_OFF;
	}
	if ((!console_bonded || pairable_until) && !hid_ready()) { return ADV_OFF; }
	if (!console_bonded) {
		return ADV_PAIRABLE;
	}
	if (pairable_until) {
		return ADV_PAIRABLE;
	}
	if (wake_until && k_uptime_get() < wake_until) {
		return ADV_WAKE;
	}
	if (!hid_ready()) {
		return ADV_OFF;
	}
	return user_active ? ADV_BONDED : ADV_OFF;
}

static void scan_pace_check(void);

#define ADV_WAKE_INT 0x0020

static void adv_start(void)
{
	static const uint8_t zero6[6];
	static bool adv_fast;
	enum adv_mode want;
	int err;

	if (pairable_until && k_uptime_get() >= pairable_until) {
		pairable_until = 0;
	}
	want = adv_mode_wanted();
	scan_pace_check();

	if (want == ADV_OFF) {
		err = bt_le_adv_stop();
		return;
	}

	memcpy(mfg_data + ADV_OFF_HOST,
	       want == ADV_PAIRABLE ? zero6 : console_host, 6);
	mfg_data[ADV_OFF_WAKE] = (want == ADV_WAKE) ? ADV_WAKE_FLAG : 0x00;

	if ((want == ADV_WAKE) != adv_fast) {
		bt_le_adv_stop();
	}
	err = bt_le_adv_update_data(ad, ARRAY_SIZE(ad), NULL, 0);
	if (err) {
		adv_fast = (want == ADV_WAKE);
		err = bt_le_adv_start(adv_fast ?
				      BT_LE_ADV_PARAM(BT_LE_ADV_OPT_CONN,
						      ADV_WAKE_INT, ADV_WAKE_INT,
						      NULL) :
				      BT_LE_ADV_CONN_FAST_1,
				      ad, ARRAY_SIZE(ad), NULL, 0);
	}
}

static void arm_pairable_window(void)
{
	pairable_until = k_uptime_get() + PAIRABLE_MS;
	adv_start();
}

static void adv_work_handler(struct k_work *work);
static K_WORK_DEFINE(adv_work, adv_work_handler);

static void user_input_seen(void)
{
	if (!user_active) {
		user_active = true;
		k_work_submit(&adv_work);
	}
}

static void hid_pairing_complete(struct bt_conn *conn, bool bonded)
{
	if (!g_mouse || conn != g_mouse) {
		return;
	}
	arm_pairable_window();
}

static struct bt_conn_auth_info_cb hid_auth_cb = {
	.pairing_complete = hid_pairing_complete,
};

static void hid_passkey_display(struct bt_conn *conn, unsigned int passkey)
{
	char astr[BT_ADDR_LE_STR_LEN];

	bt_addr_le_to_str(bt_conn_get_dst(conn), astr, sizeof(astr));
	printk("[HIDLNK] PASSKEY %06u: type it on %s and press Enter\n",
	       passkey, astr);
}

static void hid_passkey_cancel(struct bt_conn *conn)
{
	ARG_UNUSED(conn);
}

static bool hid_want_passkey;

static const struct bt_conn_auth_cb hid_io_cb = {
	.passkey_display = hid_passkey_display,
	.cancel = hid_passkey_cancel,
};

#define HM_REPORTS 8
#define HM_NONE 0xFFFFu

struct hm_report {
	uint8_t id;
	uint16_t bits;
	uint16_t x_off, y_off, wheel_off;
	uint8_t x_size, y_size, wheel_size;
	uint16_t btn_bit[8];
	uint16_t mod_bit[8];
	uint16_t keys_off;
	uint8_t keys_n;
	uint16_t bm_off;
	uint16_t bm_n;
	uint8_t bm_first;
};

struct hm_glob {
	uint16_t page;
	uint16_t count;
	uint8_t size;
	uint8_t id;
};

struct hm_run {
	uint32_t first, last;
};
#define HM_RUNS 8

static uint32_t hm_usage_at(const struct hm_run *run, uint8_t nrun,
			    uint16_t page, uint32_t j)
{
	uint32_t u = 0;

	for (uint8_t s = 0; s < nrun; s++) {
		uint32_t n = run[s].last - run[s].first + 1U;

		if (j < n) {
			u = run[s].first + j;
			break;
		}
		j -= n;
		u = run[s].last;
	}
	if (!(u >> 16)) {
		u |= (uint32_t)page << 16;
	}
	return u;
}

static struct hm_report *hm_report_for(struct hm_report *out, uint8_t *n,
				       uint8_t max, uint8_t id)
{
	struct hm_report *r;

	for (uint8_t i = 0; i < *n; i++) {
		if (out[i].id == id) {
			return &out[i];
		}
	}
	if (*n >= max) {
		return NULL;
	}
	r = &out[(*n)++];
	memset(r, 0, sizeof(*r));
	r->id = id;
	r->x_off = HM_NONE;
	r->y_off = HM_NONE;
	r->wheel_off = HM_NONE;
	for (uint8_t b = 0; b < 8; b++) {
		r->btn_bit[b] = HM_NONE;
		r->mod_bit[b] = HM_NONE;
	}
	return r;
}

static void hm_input(struct hm_report *r, const struct hm_glob *g,
		     uint8_t flags, const struct hm_run *run, uint8_t nrun)
{
	uint32_t total = (uint32_t)r->bits + (uint32_t)g->size * g->count;

	if (flags & 0x01) {
	} else if (flags & 0x02) {
		for (uint32_t j = 0; j < g->count; j++) {
			uint32_t u = hm_usage_at(run, nrun, g->page, j);
			uint32_t off = (uint32_t)r->bits + j * g->size;
			uint16_t pg = (uint16_t)(u >> 16), id = (uint16_t)u;

			if (off >= HM_NONE) {
				break;
			}
			if (pg == 0x01 && (flags & 0x04)) {
				if (id == 0x30 && r->x_off == HM_NONE) {
					r->x_off = (uint16_t)off;
					r->x_size = g->size;
				} else if (id == 0x31 && r->y_off == HM_NONE) {
					r->y_off = (uint16_t)off;
					r->y_size = g->size;
				} else if (id == 0x38 &&
					   r->wheel_off == HM_NONE) {
					r->wheel_off = (uint16_t)off;
					r->wheel_size = g->size;
				}
			} else if (pg == 0x09 && g->size == 1) {
				if (id >= 1 && id <= 8 &&
				    r->btn_bit[id - 1] == HM_NONE) {
					r->btn_bit[id - 1] = (uint16_t)off;
				}
			} else if (pg == 0x07 && g->size == 1) {
				if (id >= 0xE0 && id <= 0xE7) {
					if (r->mod_bit[id - 0xE0] == HM_NONE) {
						r->mod_bit[id - 0xE0] =
							(uint16_t)off;
					}
				} else if (id < 0xE0) {
					if (!r->bm_n) {
						r->bm_off = (uint16_t)off;
						r->bm_first = (uint8_t)id;
						r->bm_n = 1;
					} else if (off == (uint32_t)r->bm_off +
							  r->bm_n &&
						   id == (uint32_t)r->bm_first +
							 r->bm_n) {
						r->bm_n++;
					}
				}
			}
		}
	} else if ((hm_usage_at(run, nrun, g->page, 0) >> 16) == 0x07 &&
		   g->size == 8 && !r->keys_n) {
		r->keys_off = r->bits;
		r->keys_n = g->count > 255 ? 255 : (uint8_t)g->count;
	}
	r->bits = total >= HM_NONE ? (uint16_t)(HM_NONE - 1U) : (uint16_t)total;
}

static uint8_t hm_parse(const uint8_t *m, uint16_t len, struct hm_report *out,
			uint8_t max)
{
	struct hm_glob g = { 0 }, saved[2];
	struct hm_run run[HM_RUNS];
	uint32_t umin = 0, umax = 0;
	bool have_min = false, have_max = false;
	uint8_t nsaved = 0, nrun = 0, n = 0;
	uint32_t i = 0;

	while (i < len) {
		uint8_t b = m[i++];
		uint8_t size = b & 0x03, type = (b >> 2) & 0x03, tag = b >> 4;
		uint32_t v = 0;

		if (b == 0xFE) {
			if (i >= len) {
				break;
			}
			i += 2U + m[i];
			continue;
		}
		if (size == 3) {
			size = 4;
		}
		if (i + size > len) {
			break;
		}
		for (uint8_t k = 0; k < size; k++) {
			v |= (uint32_t)m[i + k] << (8 * k);
		}
		i += size;

		if (type == 1) {
			if (tag == 0) {
				g.page = (uint16_t)v;
			} else if (tag == 7) {
				g.size = v > 255 ? 255 : (uint8_t)v;
			} else if (tag == 8) {
				g.id = (uint8_t)v;
			} else if (tag == 9) {
				g.count = v > 0xFFFFu ? 0xFFFFu : (uint16_t)v;
			} else if (tag == 10) {
				if (nsaved < ARRAY_SIZE(saved)) {
					saved[nsaved++] = g;
				}
			} else if (tag == 11) {
				if (nsaved) {
					g = saved[--nsaved];
				}
			}
		} else if (type == 2) {
			uint32_t u = (size == 4) ? v : (v & 0xFFFFu);
			bool add = false;
			uint32_t first = u;

			if (tag == 0) {
				add = true;
			} else if (tag == 1) {
				if (have_max) {
					add = true;
					u = umax;
					have_max = false;
				} else {
					umin = u;
					have_min = true;
				}
			} else if (tag == 2) {
				if (have_min) {
					add = true;
					first = umin;
					have_min = false;
				} else {
					umax = u;
					have_max = true;
				}
			}
			if (add && nrun < HM_RUNS) {
				run[nrun].first = first;
				run[nrun].last = u;
				nrun++;
			}
		} else if (type == 0) {
			if (tag == 8) {
				struct hm_report *r =
					hm_report_for(out, &n, max, g.id);

				if (r) {
					hm_input(r, &g, (uint8_t)v, run, nrun);
				}
			}
			nrun = 0;
			have_min = false;
			have_max = false;
		}
	}
	return n;
}

static uint16_t hm_bytes(const struct hm_report *r)
{
	return (uint16_t)((r->bits + 7U) / 8U);
}

struct hm_ref {
	uint16_t handle;
	uint8_t id;
	uint8_t type;
};

#define HM_UNBOUND 0xFF

static uint8_t hm_bind(uint16_t vh, uint32_t top, const uint16_t *chr,
		       uint8_t nchr, const struct hm_ref *ref, uint8_t nref,
		       const struct hm_report *rep, uint8_t nrep)
{
	for (uint8_t c = 0; c < nchr; c++) {
		if (chr[c] > vh && chr[c] < top) {
			top = chr[c];
		}
	}
	for (uint8_t k = 0; k < nref; k++) {
		if (ref[k].handle <= vh || ref[k].handle >= top ||
		    ref[k].type != 1) {
			continue;
		}
		for (uint8_t j = 0; j < nrep; j++) {
			if (rep[j].id == ref[k].id) {
				return j;
			}
		}
		break;
	}
	return HM_UNBOUND;
}

static uint8_t hm_by_len(const struct hm_report *rep, uint8_t nrep,
			 uint16_t len)
{
	uint8_t hit = HM_UNBOUND;

	for (uint8_t k = 0; k < nrep; k++) {
		if (hm_bytes(&rep[k]) != len) {
			continue;
		}
		if (hit != HM_UNBOUND) {
			return HM_UNBOUND;
		}
		hit = k;
	}
	return hit;
}

static int32_t hm_get(const uint8_t *d, uint16_t len, uint16_t off,
		      uint8_t size, bool sgn)
{
	uint32_t v = 0;

	if (!size || size > 32 || (uint32_t)off + size > (uint32_t)len * 8U) {
		return 0;
	}
	for (uint8_t k = 0; k < size; k++) {
		uint16_t bit = (uint16_t)(off + k);

		if (d[bit >> 3] & (1U << (bit & 7))) {
			v |= (uint32_t)1 << k;
		}
	}
	if (sgn && size < 32 && (v & ((uint32_t)1 << (size - 1)))) {
		v |= 0xFFFFFFFFu << size;
	}
	return (int32_t)v;
}

#if defined(JC_LEFT)

static bool hm_has_kbd(const struct hm_report *r)
{
	if (r->keys_n || r->bm_n) {
		return true;
	}
	for (uint8_t b = 0; b < 8; b++) {
		if (r->mod_bit[b] != HM_NONE) {
			return true;
		}
	}
	return false;
}

static bool hm_kbd_decode(const struct hm_report *r, const uint8_t *d,
			  uint16_t len, uint8_t *mods, uint8_t keys[6])
{
	uint8_t n = 0;

	if (!hm_has_kbd(r)) {
		return false;
	}
	*mods = 0;
	memset(keys, 0, 6);
	for (uint8_t b = 0; b < 8; b++) {
		if (r->mod_bit[b] != HM_NONE &&
		    hm_get(d, len, r->mod_bit[b], 1, false)) {
			*mods |= (uint8_t)(1U << b);
		}
	}
	for (uint8_t i = 0; i < r->keys_n; i++) {
		uint8_t k = (uint8_t)hm_get(d, len,
					    (uint16_t)(r->keys_off + 8U * i),
					    8, false);

		if (r->keys_n <= 6) {
			keys[i] = k;
			if (k) {
				n = (uint8_t)(i + 1U);
			}
		} else if (k && n < 6) {
			keys[n++] = k;
		}
	}
	for (uint16_t i = 0; i < r->bm_n && n < 6; i++) {
		uint16_t usage = (uint16_t)(r->bm_first + i);

		if (usage >= 4 &&
		    hm_get(d, len, (uint16_t)(r->bm_off + i), 1, false)) {
			keys[n++] = (uint8_t)usage;
		}
	}
	return true;
}

static bool kbd_decode_len(const uint8_t *d, uint16_t len, uint8_t *mods,
			   uint8_t keys[6])
{
	uint8_t key_off;

	if (len == 8) {
		key_off = 2;
	} else if (len == 7) {
		key_off = 1;
	} else {
		return false;
	}
	*mods = d[0];
	memcpy(keys, d + key_off, 6);
	return true;
}

struct hm_kbd_src {
	bool used;
	uint8_t mods;
	uint8_t keys[6];
};

static void hm_kbd_merge(const struct hm_kbd_src *src, uint8_t nsrc,
			 uint8_t *mods, uint8_t keys[6])
{
	uint8_t used = 0, only = 0, n = 0;

	for (uint8_t s = 0; s < nsrc; s++) {
		if (src[s].used) {
			used++;
			only = s;
		}
	}
	*mods = 0;
	memset(keys, 0, 6);
	if (used == 1) {
		*mods = src[only].mods;
		memcpy(keys, src[only].keys, 6);
		return;
	}
	for (uint8_t s = 0; s < nsrc; s++) {
		if (!src[s].used) {
			continue;
		}
		*mods |= src[s].mods;
		for (uint8_t i = 0; i < 6 && n < 6; i++) {
			uint8_t k = src[s].keys[i];
			bool twice = false;

			if (k < 4) {
				continue;
			}
			for (uint8_t j = 0; j < n; j++) {
				twice = twice || keys[j] == k;
			}
			if (!twice) {
				keys[n++] = k;
			}
		}
	}
}

#else

#define HM_XY 0x01
#define HM_WHEEL 0x02
#define HM_BTN 0x04

static uint8_t hm_mouse_decode(const struct hm_report *r, const uint8_t *d,
			       uint16_t len, int16_t *x, int16_t *y,
			       int8_t *wheel, uint8_t *btn)
{
	uint8_t have = 0;
	int32_t v;

	if (r->x_off != HM_NONE && r->y_off != HM_NONE) {
		v = hm_get(d, len, r->x_off, r->x_size, true);
		*x = (v > 32767) ? 32767 : (v < -32768) ? -32768 : (int16_t)v;
		v = hm_get(d, len, r->y_off, r->y_size, true);
		*y = (v > 32767) ? 32767 : (v < -32768) ? -32768 : (int16_t)v;
		have |= HM_XY;
	}
	if (r->wheel_off != HM_NONE) {
		v = hm_get(d, len, r->wheel_off, r->wheel_size, true);
		*wheel = (v > 127) ? 127 : (v < -128) ? -128 : (int8_t)v;
		have |= HM_WHEEL;
	}
	for (uint8_t b = 0; b < 8; b++) {
		if (r->btn_bit[b] == HM_NONE) {
			continue;
		}
		if (!(have & HM_BTN)) {
			*btn = 0;
			have |= HM_BTN;
		}
		if (hm_get(d, len, r->btn_bit[b], 1, false)) {
			*btn |= (uint8_t)(1U << b);
		}
	}
	return have;
}

static bool mouse_decode_len(const uint8_t *d, uint16_t len, int16_t *x,
			     int16_t *y, int8_t *wheel, uint8_t *btn)
{
	if (len == 7) {
		*x = (int16_t)(d[2] | ((uint16_t)(d[3] & 0x0F) << 8));
		*y = (int16_t)((d[3] >> 4) | ((uint16_t)d[4] << 4));
		if (*x & 0x0800) {
			*x |= (int16_t)0xF000;
		}
		if (*y & 0x0800) {
			*y |= (int16_t)0xF000;
		}
		*btn = d[0];
		*wheel = (int8_t)d[5];
	} else if (len >= 6 && len <= 8) {
		*btn = d[0];
		*x = (int16_t)((uint16_t)d[1] | ((uint16_t)d[2] << 8));
		*y = (int16_t)((uint16_t)d[3] | ((uint16_t)d[4] << 8));
		*wheel = (int8_t)d[5];
	} else if (len >= 4 && len < 6) {
		*btn = d[0];
		*x = (int8_t)d[1];
		*y = (int8_t)d[2];
		*wheel = (int8_t)d[3];
	} else {
		return false;
	}
	return true;
}

#endif

#define HM_MAP_MAX 512
#define HM_CHRS 12

static uint8_t hm_map[HM_MAP_MAX];
static uint16_t hm_map_len;
static uint16_t hm_chr[HM_CHRS];
static uint8_t hm_chr_n;
static struct hm_ref hm_ref[HM_CHRS];
static uint8_t hm_ref_n;
static struct hm_report hm_rep[HM_REPORTS];
static uint8_t hm_rep_n;
static uint8_t hm_of_sub[MAX_RPT];
static bool hm_ready;
static uint16_t hm_sub_vh[MAX_RPT];
static uint8_t hm_sub_n;
static struct hm_report hm_keep_rep[HM_REPORTS];
static uint16_t hm_keep_vh[MAX_RPT];
static uint8_t hm_keep_of[MAX_RPT];
static uint8_t hm_keep_n;
static bool hm_kept;
static bool hm_map_whole;
#if defined(JC_LEFT)
static struct hm_kbd_src hm_kb[MAX_RPT];
#endif

static void hm_reset(void)
{
	hm_ready = false;
	hm_map_len = 0;
	hm_chr_n = 0;
	hm_ref_n = 0;
	hm_rep_n = 0;
	memset(hm_of_sub, HM_UNBOUND, sizeof(hm_of_sub));
#if defined(JC_LEFT)
	memset(hm_kb, 0, sizeof(hm_kb));
#endif
	hm_sub_n = 0;
	hm_kept = false;
	hm_map_whole = false;
}

static bool hm_keep_save(void)
{
	if (!hm_ready || !hm_sub_n || !hm_map_whole) {
		return false;
	}
	memcpy(hm_keep_rep, hm_rep, sizeof(hm_keep_rep));
	memcpy(hm_keep_vh, hm_sub_vh, sizeof(hm_keep_vh));
	memcpy(hm_keep_of, hm_of_sub, sizeof(hm_keep_of));
	hm_keep_n = hm_sub_n;
	return true;
}

static const struct hm_report *hm_layout(
	const struct bt_gatt_subscribe_params *p, uint16_t len)
{
	size_t i = (size_t)(p - sub_params);
	const struct hm_report *r;

	if (i >= MAX_RPT) {
		return NULL;
	}
	if (!hm_ready) {
		if (!hm_kept) {
			return NULL;
		}
		for (uint8_t k = 0; k < hm_keep_n; k++) {
			if (hm_keep_vh[k] == p->value_handle &&
			    hm_keep_of[k] != HM_UNBOUND) {
				r = &hm_keep_rep[hm_keep_of[k]];
				return hm_bytes(r) == len ? r : NULL;
			}
		}
		return NULL;
	}
	if (hm_of_sub[i] == HM_UNBOUND) {
		uint8_t hit = hm_by_len(hm_rep, hm_rep_n, len);

		if (hit == HM_UNBOUND) {
			return NULL;
		}
		hm_of_sub[i] = hit;
	}
	r = &hm_rep[hm_of_sub[i]];
	return hm_bytes(r) == len ? r : NULL;
}

static uint8_t mouse_notify(struct bt_conn *conn,
			    struct bt_gatt_subscribe_params *params,
			    const void *data, uint16_t len)
{
	const uint8_t *d = data;

	if (!data) {
		params->value_handle = 0;
		return BT_GATT_ITER_STOP;
	}
	mouse_reports++;

	{
		static uint8_t prev[32];
		static uint16_t prev_len;
		static uint16_t prev_handle;
		uint16_t n = len > sizeof(prev) ? sizeof(prev) : len;

		if (jc_mode != JC_MODE_KEYBOARD &&
		    (n != prev_len || params->value_handle != prev_handle ||
		     memcmp(prev, d, n) != 0)) {
			memcpy(prev, d, n);
			prev_len = n;
			prev_handle = params->value_handle;
			printk("[RAW] h=0x%04X len=%u:", params->value_handle, len);
			for (uint16_t i = 0; i < n; i++) {
				printk(" %02X", d[i]);
			}
			printk("\n");
		}
	}

#if defined(JC_LEFT)
	{
		const struct hm_report *lay = hm_layout(params, len);
		uint8_t mods = 0, keys[6] = { 0 };
		uint8_t gmods = 0, gkeys[6] = { 0 };
		bool by_len = kbd_decode_len(d, len, &gmods, gkeys);
		bool by_map = lay && hm_kbd_decode(lay, d, len, &mods, keys);

		if (lay) {
			size_t src = (size_t)(params - sub_params);

			if (!by_map) {
				return BT_GATT_ITER_CONTINUE;
			}
			if (src < MAX_RPT) {
				bool any = mods != 0;

				for (int i = 0; i < 6; i++) {
					any = any || keys[i] != 0;
				}
				if (any) {
					hm_kb[src].used = true;
				}
				if (hm_kb[src].used) {
					hm_kb[src].mods = mods;
					memcpy(hm_kb[src].keys, keys, 6);
				}
				hm_kbd_merge(hm_kb, MAX_RPT, &mods, keys);
			}
		} else {
			if (!by_len) {
				return BT_GATT_ITER_CONTINUE;
			}
			mods = gmods;
			memcpy(keys, gkeys, 6);
		}
		kbd_mods = mods;
		for (int i = 0; i < 6; i++) {
			kbd_keys[i] = keys[i];
			if (kbd_keys[i]) {
				user_input_seen();
			}
		}
		if (kbd_mods) {
			user_input_seen();
		}

		{
			uint8_t want = mode_combo_wanted();

			if (want && !mode_combo_held) {
				uint8_t target = want - 1;

				mode_set(jc_mode == target ? JC_MODE_JOYCON
							   : target);
				kbd_suppress = true;
			}
			mode_combo_held = (want != 0);
		}

		{
			uint8_t want = prof_combo_wanted();

			if (want && !prof_combo_held && want != prof_active) {
				prof_load(want, true);
				kbd_suppress = true;
			}
			prof_combo_held = (want != 0);
		}

		if (kbd_suppress && !kbd_mods && !kbd_keys[0] && !kbd_keys[1] &&
		    !kbd_keys[2] && !kbd_keys[3] && !kbd_keys[4] && !kbd_keys[5]) {
			kbd_suppress = false;
		}

		if (jc_mode == JC_MODE_KEYBOARD && !kbd_suppress) {
			jc_usb_kbd_send(kbd_mods, kbd_keys);
		}
	}
	return BT_GATT_ITER_CONTINUE;
#else
	{
		const struct hm_report *lay = hm_layout(params, len);
		int16_t x = 0, y = 0, gx = 0, gy = 0;
		int8_t wheel = 0, gwheel = 0;
		uint8_t btn = 0, gbtn = 0;
		bool by_len = mouse_decode_len(d, len, &gx, &gy, &gwheel, &gbtn);
		uint8_t have = lay ? hm_mouse_decode(lay, d, len, &x, &y, &wheel,
						     &btn) : 0;
		bool trust = lay;
		bool decoded;

		if (trust) {
			decoded = have != 0;
		} else {
			decoded = by_len;
			x = gx;
			y = gy;
			wheel = gwheel;
			btn = gbtn;
		}

		if (!decoded) {
			return BT_GATT_ITER_CONTINUE;
		}

		if (!trust && rpt_handle && params->value_handle != rpt_handle) {
			return BT_GATT_ITER_CONTINUE;
		}

		mouse_dx += x;
		mouse_dy += y;
		mouse_wheel += wheel;
		wheel_btn_feed(wheel);
		if (x || y) {
			rs_push(x, y);
		}

		if (x || y) {
			mouse_has_moved = true;
			user_input_seen();
			if (!rpt_handle) {
				rpt_handle = params->value_handle;
			}
		}

		if (trust ? (have & HM_BTN) != 0 : rpt_handle != 0) {
			mouse_buttons = btn;
			if (btn) {
				user_input_seen();
			}
		}
	}
	return BT_GATT_ITER_CONTINUE;
#endif
}

static uint8_t discover_cb(struct bt_conn *conn, const struct bt_gatt_attr *attr,
			   struct bt_gatt_discover_params *p);

static uint8_t batt_notify(struct bt_conn *conn,
			   struct bt_gatt_subscribe_params *params,
			   const void *data, uint16_t len)
{
	ARG_UNUSED(conn);
	if (!data) {
		params->value_handle = 0;
		return BT_GATT_ITER_STOP;
	}
	if (len >= 1) {
		batt_pct = ((const uint8_t *)data)[0];
		batt_known = true;
	}
	return BT_GATT_ITER_CONTINUE;
}

static uint8_t batt_read_cb(struct bt_conn *conn, uint8_t err,
			    struct bt_gatt_read_params *params,
			    const void *data, uint16_t len)
{
	ARG_UNUSED(conn);
	ARG_UNUSED(params);
	batt_read_busy = false;
	if (!err && data && len >= 1) {
		batt_pct = ((const uint8_t *)data)[0];
		batt_known = true;
	}
	return BT_GATT_ITER_STOP;
}

#define SCAN_INTERVAL 0x0060
#define SCAN_WINDOW 0x0010
#define SCAN_WINDOW_FAST 0x0050

static void scan_work_handler(struct k_work *work);
static K_WORK_DELAYABLE_DEFINE(scan_work, scan_work_handler);
static bool scan_on;
static bool scan_is_fast;

static bool scan_fast_ok(void)
{
	return !g_conn && !g_mouse && adv_mode_wanted() == ADV_OFF;
}

static void scan_pace_check(void)
{
	if (scan_on && scan_is_fast != scan_fast_ok()) {
		k_work_schedule(&scan_work, K_NO_WAIT);
	}
}

static uint16_t ad_appearance(struct net_buf_simple *ad)
{
	while (ad->len > 1) {
		uint8_t len = net_buf_simple_pull_u8(ad);
		uint8_t type;

		if (len == 0 || len > ad->len) {
			return 0;
		}
		type = net_buf_simple_pull_u8(ad);
		len--;
		if (type == BT_DATA_GAP_APPEARANCE && len >= 2) {
			return sys_get_le16(ad->data);
		}
		net_buf_simple_pull(ad, len);
	}
	return 0;
}

static void ad_name(struct net_buf_simple *ad, char *out, size_t cap)
{
	out[0] = '\0';
	while (ad->len > 1) {
		uint8_t len = net_buf_simple_pull_u8(ad);
		uint8_t type;

		if (len == 0 || len > ad->len) {
			return;
		}
		type = net_buf_simple_pull_u8(ad);
		len--;
		if (type == BT_DATA_NAME_COMPLETE ||
		    type == BT_DATA_NAME_SHORTENED) {
			size_t n = len < cap - 1 ? len : cap - 1;

			memcpy(out, ad->data, n);
			out[n] = '\0';
			return;
		}
		net_buf_simple_pull(ad, len);
	}
}

static bool ad_has_hid(struct net_buf_simple *ad)
{
	while (ad->len > 1) {
		uint8_t len = net_buf_simple_pull_u8(ad);
		uint8_t type;

		if (len == 0 || len > ad->len) {
			return false;
		}
		type = net_buf_simple_pull_u8(ad);
		len--;
		if (type == BT_DATA_UUID16_ALL || type == BT_DATA_UUID16_SOME) {
			for (uint8_t i = 0; i + 1 < len; i += 2) {
				if (sys_get_le16(&ad->data[i]) == BT_UUID_HIDS_VAL) {
					return true;
				}
			}
		}
		net_buf_simple_pull(ad, len);
	}
	return false;
}

#define SCAN_SEEN_MAX 24
#define SCAN_NAME_MAX 22

struct scan_ent {
	bt_addr_le_t addr;
	int8_t rssi;
	uint16_t appearance;
	bool hid;
	char name[SCAN_NAME_MAX];
};

static struct scan_ent scan_seen[SCAN_SEEN_MAX];
static uint8_t scan_seen_n;

#define SCAN_FORCE_MS 20000
static int64_t scan_force_until;

static bool scan_forced(void)
{
	return scan_force_until && k_uptime_get() < scan_force_until;
}

static int scan_score(int8_t rssi, bool hid)
{
	return (int)rssi + (hid ? 40 : 0);
}

static struct scan_ent *scan_worst(int8_t rssi, bool hid)
{
	struct scan_ent *worst = NULL;
	int worst_score = scan_score(rssi, hid);

	for (uint8_t i = 0; i < scan_seen_n; i++) {
		int sc = scan_score(scan_seen[i].rssi, scan_seen[i].hid);

		if (sc < worst_score) {
			worst = &scan_seen[i];
			worst_score = sc;
		}
	}
	return worst;
}

static bool scan_note(const bt_addr_le_t *addr, int8_t rssi, bool hid,
		      uint16_t appearance, const char *name)
{
	struct scan_ent *e = NULL;
	bool fresh = false;

	for (uint8_t i = 0; i < scan_seen_n; i++) {
		if (!bt_addr_le_cmp(&scan_seen[i].addr, addr)) {
			e = &scan_seen[i];
			break;
		}
	}
	if (!e) {
		if (scan_seen_n < SCAN_SEEN_MAX) {
			e = &scan_seen[scan_seen_n++];
		} else {
			e = scan_worst(rssi, hid);
			if (!e) {
				return false;
			}
		}
		memset(e, 0, sizeof(*e));
		bt_addr_le_copy(&e->addr, addr);
		fresh = true;
	}
	e->rssi = rssi;
	if (hid && !e->hid) {
		e->hid = true;
		fresh = true;
	}
	if (appearance && !e->appearance) {
		e->appearance = appearance;
		fresh = true;
	}
	if (name[0] && !e->name[0]) {
		strncpy(e->name, name, sizeof(e->name) - 1);
		fresh = true;
	}
	return fresh;
}

static void scan_print(void)
{
	char astr[BT_ADDR_LE_STR_LEN];

	printk("[SCAN] begin n=%u\n", scan_seen_n);
	for (uint8_t i = 0; i < scan_seen_n; i++) {
		bt_addr_le_to_str(&scan_seen[i].addr, astr, sizeof(astr));
		printk("[SCAN] dev %s rssi=%d hid=%d appearance=0x%04X name='%s'\n",
		       astr, scan_seen[i].rssi, (int)scan_seen[i].hid,
		       scan_seen[i].appearance, scan_seen[i].name);
	}
	printk("[SCAN] end\n");
}

static bt_addr_le_t peer_try;
static bool peer_trying;

#define PIN_HEARD_MS 4000
static int64_t pin_heard_at;
static uint8_t pin_fails;
static bool hid_link_secured;

#define PEER_BAD_MAX 4
static bt_addr_le_t peer_bad[PEER_BAD_MAX];
static uint8_t peer_bad_n;

static bool peer_is_bad(const bt_addr_le_t *a)
{
	for (uint8_t i = 0; i < peer_bad_n; i++) {
		if (!bt_addr_le_cmp(&peer_bad[i], a)) {
			return true;
		}
	}
	return false;
}

static void peer_mark_bad(const bt_addr_le_t *a)
{
	if (peer_is_bad(a)) {
		return;
	}
	if (peer_bad_n < PEER_BAD_MAX) {
		bt_addr_le_copy(&peer_bad[peer_bad_n++], a);
		return;
	}
	memmove(&peer_bad[0], &peer_bad[1],
		sizeof(peer_bad[0]) * (PEER_BAD_MAX - 1));
	bt_addr_le_copy(&peer_bad[PEER_BAD_MAX - 1], a);
}

static bt_addr_le_t hid_conn_addr;
static bool hid_conn_pending;

#define HID_COOL_MS 8000
static bt_addr_le_t hid_cool_addr;
static int64_t hid_cool_until;

static void hid_cool(const bt_addr_le_t *a)
{
	bt_addr_le_copy(&hid_cool_addr, a);
	hid_cool_until = k_uptime_get() + HID_COOL_MS;
}

static bool hid_cooling(const bt_addr_le_t *a)
{
	return hid_cool_until && k_uptime_get() < hid_cool_until &&
	       !bt_addr_le_cmp(&hid_cool_addr, a);
}

static bt_addr_le_t hid_forget_addr;

static void hid_forget_fn(struct k_work *w)
{
	ARG_UNUSED(w);
	bt_unpair(BT_ID_DEFAULT, &hid_forget_addr);
}
static K_WORK_DEFINE(hid_forget_work, hid_forget_fn);

static bool peer_matches(const bt_addr_le_t *addr, const char *name, bool hid)
{
	if (!PEER.have) {
		return true;
	}
	if (!bt_addr_le_cmp(&PEER.addr, addr)) {
		return true;
	}
	return hid && PEER.name[0] && name && !strcmp(PEER.name, name);
}

static const struct scan_ent *scan_find(const bt_addr_le_t *addr)
{
	for (uint8_t i = 0; i < scan_seen_n; i++) {
		if (!bt_addr_le_cmp(&scan_seen[i].addr, addr)) {
			return &scan_seen[i];
		}
	}
	return NULL;
}

static const char *scan_name_of(const bt_addr_le_t *addr)
{
	for (uint8_t i = 0; i < scan_seen_n; i++) {
		if (!bt_addr_le_cmp(&scan_seen[i].addr, addr)) {
			return scan_seen[i].name;
		}
	}
	return "";
}

static void peer_print(void)
{
	char astr[BT_ADDR_LE_STR_LEN];
	const bt_addr_le_t *dst;

	if (PEER.have) {
		bt_addr_le_to_str(&PEER.addr, astr, sizeof(astr));
		printk("[PEER] pin=%s name='%s'\n", astr, PEER.name);
	} else {
		printk("[PEER] pin=none\n");
	}

	if (!g_mouse) {
		printk("[PEER] attached=none\n");
		return;
	}
	dst = bt_conn_get_dst(g_mouse);
	bt_addr_le_to_str(dst, astr, sizeof(astr));
	printk("[PEER] attached=%s name='%s' match=%d\n", astr,
	       scan_name_of(dst), (int)(!PEER.have || peer_matches(dst,
			scan_name_of(dst), true)));
}

static void peer_save_fn(struct k_work *w)
{
	int rc;

	ARG_UNUSED(w);
	rc = PEER.have ? settings_save_one("jc/peer", &PEER, sizeof(PEER))
		       : settings_delete("jc/peer");
}
static K_WORK_DEFINE(peer_save_work, peer_save_fn);

static void peer_set(const char *arg)
{
	char abuf[BT_ADDR_LE_STR_LEN];
	char tbuf[8];
	const char *type = "random";
	const char *nm = NULL;
	bt_addr_le_t a;
	size_t n = 0, t = 0;

	while (arg[n] && arg[n] != ' ' && arg[n] != '\t' &&
	       n < sizeof(abuf) - 1) {
		abuf[n] = arg[n];
		n++;
	}
	abuf[n] = '\0';
	while (arg[n] == ' ' || arg[n] == '\t') {
		n++;
	}
	while (arg[n] && arg[n] != ' ' && arg[n] != '\t' &&
	       t < sizeof(tbuf) - 1) {
		tbuf[t++] = arg[n++];
	}
	tbuf[t] = '\0';
	if (t) {
		type = tbuf;
	}
	while (arg[n] == ' ' || arg[n] == '\t') {
		n++;
	}
	if (arg[n]) {
		nm = arg + n;
	}
	if (bt_addr_le_from_str(abuf, type, &a)) {
		return;
	}

	memset(&PEER, 0, sizeof(PEER));
	peer_bad_n = 0;
	peer_trying = false;
	pin_fails = 0;
	pin_heard_at = 0;
	hid_cool_until = 0;
	PEER.have = 1;
	bt_addr_le_copy(&PEER.addr, &a);
	strncpy(PEER.name, nm ? nm : scan_name_of(&a), sizeof(PEER.name) - 1);
	k_work_submit(&peer_save_work);

	if (g_mouse) {
		const bt_addr_le_t *dst = bt_conn_get_dst(g_mouse);

		if (!peer_matches(dst, scan_name_of(dst), true)) {
			bt_conn_disconnect(g_mouse, BT_HCI_ERR_REMOTE_USER_TERM_CONN);
		}
	}
}

static void peer_commit(const bt_addr_le_t *addr)
{
	char astr[BT_ADDR_LE_STR_LEN];

	peer_bad_n = 0;
	peer_trying = false;
	pin_fails = 0;
	if (!PEER.have || !bt_addr_le_cmp(&PEER.addr, addr)) {
		return;
	}
	bt_addr_le_to_str(addr, astr, sizeof(astr));
	printk("[PEER] '%s' paired at %s, pin updated\n", PEER.name, astr);
	bt_addr_le_copy(&PEER.addr, addr);
	k_work_submit(&peer_save_work);
}

static void mode_set(uint8_t m)
{
	if (m == jc_mode) {
		printk("[MODE] already %s\n", jc_mode_name());
		return;
	}
	jc_mode = m;
	mode_led_set(m == JC_MODE_KEYBOARD);
#if defined(JC_LEFT)
	if (m != JC_MODE_KEYBOARD) {
		jc_usb_kbd_release_all();
	}
#endif
	printk("[MODE] now %s, joy-con input %s\n", jc_mode_name(),
	       m == JC_MODE_KEYBOARD ? "IDLE (still connected)" : "live");
#if defined(JC_LEFT)
	if (m == JC_MODE_KEYBOARD && !jc_usb_kbd_ready()) {
	}
#endif
}

static void peer_clear(void)
{
	memset(&PEER, 0, sizeof(PEER));
	k_work_submit(&peer_save_work);
	if (g_mouse) { bt_conn_disconnect(g_mouse, BT_HCI_ERR_REMOTE_USER_TERM_CONN); }
}

static void scan_restart(void)
{
	memset(scan_seen, 0, sizeof(scan_seen));
	scan_seen_n = 0;
	peer_bad_n = 0;
	pin_fails = 0;
	hid_cool_until = 0;
	scan_force_until = k_uptime_get() + SCAN_FORCE_MS;
	k_work_schedule(&scan_work, K_NO_WAIT);
}

static void scan_cb(const bt_addr_le_t *addr, int8_t rssi, uint8_t type,
		    struct net_buf_simple *ad)
{
	char astr[BT_ADDR_LE_STR_LEN];
	char name[JC_NAME_MAX];
	struct bt_conn *conn = NULL;
	uint16_t appearance;
	bool hid;
	int err;

	if (g_mouse && !scan_forced()) {
		bt_le_scan_stop();
		scan_on = false;
		return;
	}

	{
		struct net_buf_simple uuids = *ad, appear = *ad, nm = *ad;

		hid = ad_has_hid(&uuids);
		appearance = ad_appearance(&appear);
		ad_name(&nm, name, sizeof(name));
	}
	if (scan_note(addr, rssi, hid, appearance, name)) {
		bt_addr_le_to_str(addr, astr, sizeof(astr));
		printk("[SCAN] %s rssi=%d type=%u hid=%d appearance=0x%04X name='%s'\n",
		       astr, rssi, type, (int)hid, appearance, name);
	}

	if (g_mouse) {
		return;
	}
	if (PEER.have && !bt_addr_le_cmp(&PEER.addr, addr)) {
		pin_heard_at = k_uptime_get();
	}
	if (hid_cooling(addr)) {
		return;
	}

	{
		const struct scan_ent *e = scan_find(addr);

		if (e) {
			hid = hid || e->hid;
			if (!name[0]) {
				strncpy(name, e->name, sizeof(name) - 1);
				name[sizeof(name) - 1] = '\0';
			}
		}
	}

	if (PEER.have) {
		if (type != BT_GAP_ADV_TYPE_ADV_IND &&
		    type != BT_GAP_ADV_TYPE_ADV_DIRECT_IND) {
			return;
		}
		if (!peer_matches(addr, name, hid)) {
			return;
		}
		if (bt_addr_le_cmp(&PEER.addr, addr)) {
			if (peer_is_bad(addr)) {
				return;
			}
			if (pin_fails < 2 && pin_heard_at &&
			    k_uptime_get() - pin_heard_at < PIN_HEARD_MS) {
				return;
			}
			bt_addr_le_copy(&peer_try, addr);
			peer_trying = true;
			bt_addr_le_to_str(addr, astr, sizeof(astr));
			printk("[PEER] '%s' also at %s, trying it\n",
			       PEER.name, astr);
		} else {
			peer_trying = false;
		}
	} else {
		return;
	}
	bt_addr_le_to_str(addr, astr, sizeof(astr));
	printk("[MOUSE] HID advertiser %s rssi=%d, connecting\n", astr, rssi);

	if (bt_le_scan_stop()) {
		return;
	}
	scan_on = false;

	bt_addr_le_copy(&hid_conn_addr, addr);
	hid_conn_pending = true;
	err = bt_conn_le_create(addr, BT_CONN_LE_CREATE_CONN,
				BT_LE_CONN_PARAM_DEFAULT, &conn);
	if (err) {
		hid_conn_pending = false;
		k_work_schedule(&scan_work, K_SECONDS(2));
		return;
	}
	bt_conn_unref(conn);
}

static void scan_work_handler(struct k_work *work)
{
	bool fast = scan_fast_ok();
	struct bt_le_scan_param sp = {
		.type = BT_LE_SCAN_TYPE_ACTIVE,
		.options = BT_LE_SCAN_OPT_NONE,
		.interval = SCAN_INTERVAL,
		.window = fast ? SCAN_WINDOW_FAST : SCAN_WINDOW,
	};
	int err;

	ARG_UNUSED(work);
	if (g_mouse && !scan_forced()) {
		return;
	}
	if (!scan_on || scan_is_fast != fast) {
		bt_le_scan_stop();
		scan_on = false;
	}
	err = bt_le_scan_start(&sp, scan_cb);
	if (err && err != -EALREADY) {
		k_work_schedule(&scan_work, K_SECONDS(2));
		return;
	}
	scan_on = true;
	scan_is_fast = fast;
}

static void discover_next(struct bt_conn *conn, uint16_t type, uint16_t uuid16,
			  uint16_t start, uint16_t end)
{
	disc_uuid.uuid.type = BT_UUID_TYPE_16;
	disc_uuid.val = uuid16;
	disc.uuid = uuid16 ? &disc_uuid.uuid : NULL;
	disc.func = discover_cb;
	disc.start_handle = start;
	disc.end_handle = end;
	disc.type = type;
	bt_gatt_discover(conn, &disc);
}

static struct bt_gatt_read_params hm_ref_rd;
static struct bt_gatt_read_params hm_map_rd;
static const struct bt_uuid_16 hm_uuid_map =
	BT_UUID_INIT_16(BT_UUID_HIDS_REPORT_MAP_VAL);
static const struct bt_uuid_16 hm_uuid_ref =
	BT_UUID_INIT_16(BT_UUID_HIDS_REPORT_REF_VAL);

static void hm_done(void)
{
	hm_rep_n = hm_parse(hm_map, hm_map_len, hm_rep, HM_REPORTS);

	for (uint8_t i = 0; i < sub_count && i < MAX_RPT; i++) {
		hm_of_sub[i] = hm_bind(sub_params[i].value_handle,
				       (uint32_t)hid_svc_end + 1U, hm_chr, hm_chr_n,
				       hm_ref, hm_ref_n, hm_rep, hm_rep_n);
		hm_sub_vh[i] = sub_params[i].value_handle;
	}
	hm_sub_n = sub_count < MAX_RPT ? sub_count : MAX_RPT;
	hm_ready = hm_rep_n > 0;
}

static uint8_t hm_ref_cb(struct bt_conn *conn, uint8_t err,
			 struct bt_gatt_read_params *params,
			 const void *data, uint16_t len)
{
	const uint8_t *d = data;

	ARG_UNUSED(conn);
	if (d && !err) {
		if (len >= 2 && hm_ref_n < HM_CHRS) {
			hm_ref[hm_ref_n].handle = params->by_uuid.start_handle;
			hm_ref[hm_ref_n].id = d[0];
			hm_ref[hm_ref_n].type = d[1];
			hm_ref_n++;
		}
		return BT_GATT_ITER_CONTINUE;
	}
	hm_done();
	return BT_GATT_ITER_STOP;
}

static void hm_read_refs(struct bt_conn *conn)
{
	hm_ref_rd.func = hm_ref_cb;
	hm_ref_rd.handle_count = 0;
	hm_ref_rd.by_uuid.uuid = &hm_uuid_ref.uuid;
	hm_ref_rd.by_uuid.start_handle = hid_svc_start;
	hm_ref_rd.by_uuid.end_handle = hid_svc_end;
	if (bt_gatt_read(conn, &hm_ref_rd)) {
		hm_done();
	}
}

static uint8_t hm_map_cb(struct bt_conn *conn, uint8_t err,
			 struct bt_gatt_read_params *params,
			 const void *data, uint16_t len)
{
	ARG_UNUSED(params);
	if (data && !err) {
		uint16_t room = (uint16_t)(sizeof(hm_map) - hm_map_len);
		uint16_t n = len < room ? len : room;

		memcpy(hm_map + hm_map_len, data, n);
		hm_map_len += n;
		if (n == len) {
			return BT_GATT_ITER_CONTINUE;
		}
	}
	hm_map_whole = !err || err == BT_ATT_ERR_INVALID_OFFSET;
	hm_read_refs(conn);
	return BT_GATT_ITER_STOP;
}

static uint8_t hogp_read_cb(struct bt_conn *conn, uint8_t err,
			    struct bt_gatt_read_params *params,
			    const void *data, uint16_t len)
{
	if (!data || err) {
		hm_done();
		return BT_GATT_ITER_STOP;
	}
	hm_map_len = len < sizeof(hm_map) ? len : (uint16_t)sizeof(hm_map);
	memcpy(hm_map, data, hm_map_len);
	if (len + 4U >= bt_gatt_get_mtu(conn)) {
		hm_map_rd.func = hm_map_cb;
		hm_map_rd.handle_count = 1;
		hm_map_rd.single.handle = params->by_uuid.start_handle;
		hm_map_rd.single.offset = hm_map_len;
		if (!bt_gatt_read(conn, &hm_map_rd)) {
			return BT_GATT_ITER_STOP;
		}
	}
	hm_map_whole = len + 4U < bt_gatt_get_mtu(conn);
	hm_read_refs(conn);
	return BT_GATT_ITER_STOP;
}

static uint8_t discover_cb(struct bt_conn *conn, const struct bt_gatt_attr *attr,
			   struct bt_gatt_discover_params *p)
{
	if (!attr) {
		if (stage == DISC_PROTO) {
			stage = DISC_REPORT;
			discover_next(conn, BT_GATT_DISCOVER_CHARACTERISTIC,
				      BT_UUID_HIDS_REPORT_VAL, hid_svc_start,
				      hid_svc_end);
			return BT_GATT_ITER_STOP;
		}
		if (stage == DISC_PRIMARY) {
			return BT_GATT_ITER_STOP;
		}
		if (stage == DISC_REPORT) {
			printk("[MOUSE] discovery done, %u report subscriptions\n",
			       sub_count);
			hid_reattached = true;
			hid_usable_at = k_uptime_get();
			k_work_reschedule(&hid_param_work,
					  K_MSEC(HID_PARAM_DELAY_MS));
			rd_params.func = hogp_read_cb;
			rd_params.handle_count = 0;
			rd_params.by_uuid.uuid = &hm_uuid_map.uuid;
			rd_params.by_uuid.start_handle = hid_svc_start;
			rd_params.by_uuid.end_handle = hid_svc_end;
			bt_gatt_read(conn, &rd_params);

			stage = DISC_BATT;
			discover_next(conn, BT_GATT_DISCOVER_CHARACTERISTIC,
				      BT_UUID_BAS_BATTERY_LEVEL_VAL,
				      0x0001, 0xFFFF);
			return BT_GATT_ITER_STOP;
		}
		return BT_GATT_ITER_STOP;
	}

	if (stage == DISC_PRIMARY) {
		const struct bt_gatt_service_val *svc = attr->user_data;

		hid_svc_start = attr->handle;
		hid_svc_end = svc->end_handle;
		stage = DISC_PROTO;
		discover_next(conn, BT_GATT_DISCOVER_CHARACTERISTIC,
			      BT_UUID_HIDS_PROTOCOL_MODE_VAL,
			      hid_svc_start, hid_svc_end);
		return BT_GATT_ITER_STOP;
	}

	if (p->type == BT_GATT_DISCOVER_CHARACTERISTIC) {
		const struct bt_gatt_chrc *chrc = attr->user_data;

		if (stage == DISC_BATT) {
			if (chrc->properties & BT_GATT_CHRC_NOTIFY) {
				batt_sub.value_handle = chrc->value_handle;
				batt_sub.ccc_handle = 0;
				batt_sub.end_handle = 0xFFFF;
				batt_sub.disc_params = &batt_disc;
				batt_sub.notify = batt_notify;
				batt_sub.value = BT_GATT_CCC_NOTIFY;
				memset(&batt_disc, 0, sizeof(batt_disc));
				atomic_set_bit(batt_sub.flags,
					       BT_GATT_SUBSCRIBE_FLAG_VOLATILE);

				bt_gatt_subscribe(conn, &batt_sub);
			}
			batt_handle = chrc->value_handle;
			batt_rd.func = batt_read_cb;
			batt_rd.handle_count = 1;
			batt_rd.single.handle = chrc->value_handle;
			batt_rd.single.offset = 0;
			bt_gatt_read(conn, &batt_rd);

			return BT_GATT_ITER_STOP;
		}

		if (stage == DISC_PROTO) {
			static uint8_t report_proto = 0x01;

			proto_mode_handle = chrc->value_handle;
			bt_gatt_write_without_response(conn, proto_mode_handle,
						       &report_proto, 1, false);
			stage = DISC_REPORT;
			discover_next(conn, BT_GATT_DISCOVER_CHARACTERISTIC,
				      BT_UUID_HIDS_REPORT_VAL, hid_svc_start,
				      hid_svc_end);
			return BT_GATT_ITER_STOP;
		}
		if (hm_chr_n < HM_CHRS) {
			hm_chr[hm_chr_n++] = chrc->value_handle;
		}
		if (!(chrc->properties & BT_GATT_CHRC_NOTIFY)) {
			return BT_GATT_ITER_CONTINUE;
		}
		if (sub_count >= MAX_RPT) {
			return BT_GATT_ITER_CONTINUE;
		}
		{
			struct bt_gatt_subscribe_params *s = &sub_params[sub_count];

			s->value_handle = chrc->value_handle;
			s->ccc_handle = 0;
			s->end_handle = hid_svc_end;
			s->disc_params = &sub_disc[sub_count];
			s->notify = mouse_notify;
			s->value = BT_GATT_CCC_NOTIFY;
			memset(&sub_disc[sub_count], 0, sizeof(sub_disc[0]));
			atomic_set_bit(s->flags, BT_GATT_SUBSCRIBE_FLAG_VOLATILE);

			int err = bt_gatt_subscribe(conn, s);

			if (!err) {
				sub_count++;
			}
		}
		return BT_GATT_ITER_CONTINUE;
	}
	return BT_GATT_ITER_CONTINUE;
}

static bt_addr_le_t hm_keep_addr;

static void mouse_start_discovery(struct bt_conn *conn)
{
	hid_disc_started = true;
	hm_reset();
	if (hm_keep_n &&
	    !bt_addr_le_cmp(&hm_keep_addr, bt_conn_get_dst(conn))) {
		hm_kept = true;
	}
	sub_count = 0;
	hid_svc_start = 0;
	hid_svc_end = 0;
	proto_mode_handle = 0;
	stage = DISC_PRIMARY;
	discover_next(conn, BT_GATT_DISCOVER_PRIMARY, BT_UUID_HIDS_VAL,
		      0x0001, 0xFFFF);
}

static const struct device *link_uart = DEVICE_DT_GET(DT_NODELABEL(uart0));

static int64_t peer_last_rx;

static uint16_t link_pend;
static uint8_t link_pend_n;

static uint32_t link_rx_p10;

struct link_port {
	uint8_t f[LINK_LEN];
	int n;
	uint32_t *bytes;
};

static struct link_port link_p10;

static void link_isr(const struct device *dev, void *user_data)
{
	struct link_port *p = user_data;
	uint8_t *f = p->f;
	int n = p->n;
	uint8_t ch;

	uart_irq_update(dev);
	while (uart_irq_rx_ready(dev)) {
		if (uart_fifo_read(dev, &ch, 1) != 1) {
			break;
		}
		(*p->bytes)++;

		if (n == 0 && ch != LINK_SYNC0) {
			continue;
		}
		if (n == 1 && ch != LINK_SYNC1 && ch != LINK_SYNC1_CFG) {
			n = (ch == LINK_SYNC0) ? 1 : 0;
			continue;
		}
		f[n++] = ch;
		if (n < LINK_LEN) {
			continue;
		}
		n = 0;
		if (crc8(f, LINK_LEN - 1, 0x07, 0x00, false) != f[LINK_LEN - 1]) {
			continue;
		}
		if (f[1] == LINK_SYNC1_CFG) {
			link_rx_cfg_id = f[2];
			link_rx_cfg_val = f[3];
			link_rx_cfg_pending = 1;
			peer_last_rx = k_uptime_get();
			continue;
		}
		{
			uint16_t v = f[2] | ((uint16_t)f[3] << 8);

			if (v == peer_buttons) {
				link_pend_n = 0;
			} else if (v == link_pend) {
				if (++link_pend_n >= 2) {
					peer_buttons = v;
					link_pend_n = 0;
				}
			} else {
				link_pend = v;
				link_pend_n = 1;
			}
		}
		peer_last_rx = k_uptime_get();
		if (peer_buttons) {
			user_input_seen();
		}
	}
	p->n = n;
}

#if defined(JC_LEFT)

static uint16_t cfg_mask_for(uint8_t kind)
{
	uint8_t nb = CFG.n_btn;
	uint16_t m = 0;

	if (nb > JC_CFG_BTNS) {
		nb = JC_CFG_BTNS;
	}
	for (uint8_t j = 0; j < nb; j++) {
		uint8_t k = CFG.btn[j].kind;
		uint8_t code;

		if (k != kind) {
			continue;
		}
		code = CFG.btn[j].code;
		if (kind == JC_BTN_MOD || kind == JC_BTN_PEERMOD) {
			if (kbd_mods & code) {
				m |= CFG.btn[j].mask;
			}
			continue;
		}
		for (int i = 0; i < 6; i++) {
			if (kbd_keys[i] && kbd_keys[i] == code) {
				m |= CFG.btn[j].mask;
				break;
			}
		}
	}
	return m;
}

static uint16_t link_tx_buttons(void)
{
	uint16_t m;

	if (!jc_live()) {
		return 0;
	}
	m = cfg_mask_for(JC_BTN_PEER) | cfg_mask_for(JC_BTN_PEERMOD);

	if (sync_key_down()) {
		m |= LINK_SYNC_REQ;
	}
	if (jc_mode == JC_MODE_STICK) {
		m |= LINK_STICK;
	}
	return m;
}

#else

static uint16_t link_tx_buttons(void)
{
	uint8_t nb = CFG.n_btn > JC_CFG_BTNS ? JC_CFG_BTNS : CFG.n_btn;
	uint16_t m = 0;

	for (uint8_t j = 0; j < nb; j++) {
		if (CFG.btn[j].kind != JC_BTN_MPEER) {
			continue;
		}
		if (mouse_buttons & CFG.btn[j].code) {
			m |= CFG.btn[j].mask;
		}
	}
	m |= wheel_btn_peer_now();
	return m;
}

static void stick_drive(uint8_t src, int16_t *axis)
{
	int32_t v = 0, d;

	switch (src) {
	case JC_SRC_WHEEL:
		v = mouse_wheel;
		mouse_wheel = 0;
		break;
	case JC_SRC_DX:
		v = mouse_dx;
		break;
	case JC_SRC_DY:
		v = mouse_dy;
		break;
	default:
		return;
	}
	if (!v) {
		return;
	}
	d = v * (int32_t)CFG.stick_gain + *axis;
	if (d > STICK_THROW) {
		d = STICK_THROW;
	} else if (d < -STICK_THROW) {
		d = -STICK_THROW;
	}
	*axis = (int16_t)d;
}
#endif

static void link_thread(void *a, void *b, void *c)
{
	uint16_t last_tx_m = 0xFFFF;
	int64_t last_tx = 0;
#if defined(JC_LEFT)
	int64_t last_prof_tx = 0;
#endif

	ARG_UNUSED(a); ARG_UNUSED(b); ARG_UNUSED(c);
	if (!device_is_ready(link_uart)) {
		return;
	}

	link_p10.bytes = &link_rx_p10;
	uart_irq_callback_user_data_set(link_uart, link_isr, &link_p10);
	uart_irq_rx_enable(link_uart);

	while (1) {
		uint16_t m = link_tx_buttons();
		int64_t now = k_uptime_get();

		if (link_rx_cfg_pending) {
			uint8_t id = link_rx_cfg_id, val = link_rx_cfg_val;

			link_rx_cfg_pending = 0;
			if (id == LINK_CFG_PROFILE) {
				if (val != prof_active) {
					if (prof_load(val, false) == -ENOENT && val >= 1 && val <= JC_PROFILES) { prof_active = val; prof_store_u8("jc/pact", val); }
				}
			} else if (id == LINK_CFG_GRIP) {
				uint8_t on = val ? 1 : 0;

				if ((grip_byte >= 0) != (on != 0)) {
					grip_byte = on ? 0x01 : -1;
					grip_wait = -1;
					settings_save_one("jc/grip", &on,
							  sizeof(on));
				}
			}
		}

		if (link_cfg_pending) {
			uint8_t g[LINK_LEN] = {
				LINK_SYNC0, LINK_SYNC1_CFG,
				link_cfg_id, link_cfg_val, 0
			};
			int copies = 3;

			link_cfg_pending = 0;
			g[4] = crc8(g, LINK_LEN - 1, 0x07, 0x00, false);
			while (copies--) {
				for (int i = 0; i < LINK_LEN; i++) {
					uart_poll_out(link_uart, g[i]);
				}
			}
		}

		if (m != last_tx_m || (now - last_tx) >= 100) {
			uint8_t f[LINK_LEN] = {
				LINK_SYNC0, LINK_SYNC1,
				(uint8_t)(m & 0xFF), (uint8_t)(m >> 8), 0
			};
			int copies = (m != last_tx_m) ? 3 : 1;

			f[4] = crc8(f, LINK_LEN - 1, 0x07, 0x00, false);
			while (copies--) {
				for (int i = 0; i < LINK_LEN; i++) {
					uart_poll_out(link_uart, f[i]);
				}
			}
			last_tx_m = m;
			last_tx = now;
		}

#if defined(JC_LEFT)
		if (prof_active && (now - last_prof_tx) >= 2000) {
			uint8_t g[LINK_LEN] = {
				LINK_SYNC0, LINK_SYNC1_CFG,
				LINK_CFG_PROFILE, prof_active, 0
			};

			last_prof_tx = now;
			g[4] = crc8(g, LINK_LEN - 1, 0x07, 0x00, false);
			for (int i = 0; i < LINK_LEN; i++) {
				uart_poll_out(link_uart, g[i]);
			}
		}
#endif

		if (peer_buttons && peer_last_rx &&
		    (k_uptime_get() - peer_last_rx) > LINK_STALE_MS) {
			peer_buttons = 0;
		}
		k_msleep(10);
	}
}

K_THREAD_DEFINE(link_tid, 3072, link_thread, NULL, NULL, NULL, 6, 0, 0);

#if defined(JC_LEFT)
static const uint16_t replay_lo = 0;
static uint16_t replay_hi = REPORT_COUNT - 1;
#else
static const uint16_t replay_lo = 25;
static uint16_t replay_hi = 37;
#endif
#if !defined(JC_LEFT)
#define MOTION_AT 0x10
#define MOTION_NBITS (40 * 8)
#define SAMPLE_STRIDE 88
#define SAMPLE_COUNT 3

static const uint16_t sweep_qlo = 11, sweep_qhi = 58;
static const uint16_t sweep_plo = 64, sweep_phi = 79;

#define RATE_AT 154
#define RATE_STRIDE 89

static void motion_put(uint8_t *r, uint16_t bit, int16_t v, uint8_t w)
{
	uint32_t at = MOTION_AT + (bit >> 3);
	uint8_t sh = bit & 7;
	uint8_t lo = 24 - sh - w;
	uint32_t acc, mask;

	if (w < 4 || w > 16 || (uint32_t)bit + w > MOTION_NBITS) {
		return;
	}
	acc = ((uint32_t)r[at] << 16) | ((uint32_t)r[at + 1] << 8) |
	      ((sh + w > 16) ? r[at + 2] : 0U);
	mask = (((1UL << w) - 1UL) << lo);
	acc = (acc & ~mask) | ((((uint32_t)(uint16_t)v) << lo) & mask);
	r[at] = (uint8_t)(acc >> 16);
	r[at + 1] = (uint8_t)(acc >> 8);
	if (sh + w > 16) {
		r[at + 2] = (uint8_t)acc;
	}
}

static const uint8_t sweep_slots = SAMPLE_COUNT;

static uint32_t isqrt32(uint32_t n)
{
	uint32_t x = n, y = 0, b = 1UL << 30;

	while (b > x) {
		b >>= 2;
	}
	while (b) {
		if (x >= y + b) {
			x -= y + b;
			y = (y >> 1) + b;
		} else {
			y >>= 1;
		}
		b >>= 2;
	}
	return y;
}

static void sweep_apply(uint8_t *r)
{
	uint8_t s, sm, k;

	for (s = 0; s < sweep_slots; s++) {
		uint16_t base = (uint16_t)(56 + s * SAMPLE_STRIDE);
		uint16_t b;

		for (b = sweep_qlo; b <= sweep_qhi; b++) {
			uint16_t bit = base + b;

			if (bit >= MOTION_NBITS) {
				break;
			}
			if (b >= sweep_plo && b <= sweep_phi) {
				continue;
			}
			r[MOTION_AT + (bit >> 3)] &=
				(uint8_t)~(0x80u >> (bit & 7));
		}
	}
	for (sm = 0; sm < 2; sm++) {
		for (k = 0; k < 3; k++) {
			motion_put(r, (uint16_t)(RATE_AT + sm * RATE_STRIDE +
						 k * 16), 0, 16);
		}
	}
}

#endif

static void send_report(void)
{
	static uint16_t idx;
	static const int8_t replay_dir = 1;
	uint8_t r[63];
	uint16_t buttons = 0;
	int64_t since;
	int err;

	if (!streaming || !g_conn || !rpt_hid_attr) {
		return;
	}
	if (ctlr_q_hold()) {
		return;
	}

	if (atomic_get(&rpt_inflight) >= RPT_MAX_INFLIGHT) {
		return;
	}

#if defined(JC_LEFT)
#define REPLAY_SKIP 0
#define MOTION_HOLD 51
#else
#define REPLAY_SKIP 12
#define MOTION_HOLD 0
#endif
	if (idx < replay_lo || idx > replay_hi) {
		idx = replay_lo;
	}
	memcpy(r, REAL_REPORTS[idx], sizeof(r));
	idx = (uint16_t)(idx + replay_dir);
	if (idx > replay_hi) {
		idx = replay_lo;
	}
#if MOTION_HOLD
	memcpy(r + 0x0E, REAL_REPORTS[MOTION_HOLD] + 0x0E, 0x37 - 0x0E);
#endif
	r[0x00] = (uint8_t)report_seq++;

	if (batt_have()) {
		r[0x01] = (uint8_t)(batt_level() << 2);
	}

	since = k_uptime_get() - stream_started_at;

	if (!asked_slower && since > 6000) {
		static const struct bt_le_conn_param slower = {
			.interval_min = 8, .interval_max = 8,
			.latency = 0, .timeout = 200,
		};
		bt_conn_le_param_update(g_conn, &slower);

		asked_slower = true;
	}

	if (!mouse_has_moved) {
		r[0x0D] = 0xFF;
	}

#if defined(JC_LEFT)
	if (jc_live()) {
		buttons |= cfg_mask_for(JC_BTN_KEY) | cfg_mask_for(JC_BTN_MOD);
	}

	buttons |= peer_buttons & ~(LINK_SYNC_REQ | LINK_STICK);
	r[0x02] = buttons & 0xFF;
	r[0x03] = buttons >> 8;
	if (grip_byte >= 0) {
		if (grip_wait > 0) {
			grip_wait--;
			r[0x04] = 0x07;
		} else if (grip_wait == 0) {
			r[0x04] = (uint8_t)grip_byte;
		} else {
			r[0x04] = 0x07;
		}
	}

	{
		int32_t sx = STICK_CX, sy = STICK_CY;
		const int32_t up = STICK_Y_UP_POSITIVE ? STICK_THROW : -STICK_THROW;
		uint32_t packed;

		uint8_t nb = CFG.n_btn > JC_CFG_BTNS ? JC_CFG_BTNS : CFG.n_btn;

		for (uint8_t j = 0; jc_live() && j < nb; j++) {
			uint16_t packed;
			uint8_t code;

			if (CFG.btn[j].kind != JC_BTN_STICK) {
				continue;
			}
			code = CFG.btn[j].code;
			packed = CFG.btn[j].mask;
			for (int i = 0; i < 6; i++) {
				if (kbd_keys[i] && kbd_keys[i] == code) {
					sx += (int8_t)(packed & 0xFF) * STICK_THROW;
					sy += (int8_t)(packed >> 8) * up;
					break;
				}
			}
		}
		if (sx < 0) { sx = 0; } else if (sx > 4095) { sx = 4095; }
		if (sy < 0) { sy = 0; } else if (sy > 4095) { sy = 4095; }
		packed = ((uint32_t)sx & 0xFFF) | (((uint32_t)sy & 0xFFF) << 12);
		r[0x05] = (uint8_t)(packed & 0xFF);
		r[0x06] = (uint8_t)((packed >> 8) & 0xFF);
		r[0x07] = (uint8_t)((packed >> 16) & 0xFF);
	}
#else
	{
		uint8_t nb = CFG.n_btn;

		if (nb > JC_CFG_BTNS) {
			nb = JC_CFG_BTNS;
		}
		for (uint8_t j = 0; j < nb; j++) {
			if (CFG.btn[j].kind != JC_BTN_MOUSE) {
				continue;
			}
			if (mouse_buttons & CFG.btn[j].code) {
				buttons |= CFG.btn[j].mask;
			}
		}
	}

	buttons |= peer_buttons & ~(LINK_SYNC_REQ | LINK_STICK);
	buttons |= wheel_btn_now();
	r[0x02] = buttons & 0xFF;
	r[0x03] = buttons >> 8;
	if (grip_byte >= 0) {
		if (grip_wait > 0) {
			grip_wait--;
			r[0x04] = 0x07;
		} else if (grip_wait == 0) {
			r[0x04] = (uint8_t)grip_byte;
		} else {
			r[0x04] = 0x07;
		}
	}

	int16_t rs_wx = 0, rs_wy = 0;

	rs_emit(&rs_wx, &rs_wy);

	{
		int32_t sx, sy;
		uint32_t packed;

		if (stick_on()) {
			stick_mouse_in();
		} else {
			stick_drive(CFG.stick_src_x, &stick_x);
			stick_drive(CFG.stick_src_y, &stick_y);
			stick_fx = (int32_t)stick_x * 16;
			stick_fy = (int32_t)stick_y * 16;
			stick_ex = stick_fx;
			stick_ey = stick_fy;
			stick_vsx = 0;
			stick_vsy = 0;
			memset(stick_win_x, 0, sizeof(stick_win_x));
			memset(stick_win_y, 0, sizeof(stick_win_y));
		}

		if (stick_on()) {
			int32_t ox = stick_x, oy = stick_y;

			sx = STICK_MCX + ox;
			sy = STICK_MCY + oy;
		} else {
			sx = STICK_CX + stick_x;
			sy = STICK_CY + stick_y;
		}
		if (sx < 0) { sx = 0; } else if (sx > 4095) { sx = 4095; }
		if (sy < 0) { sy = 0; } else if (sy > 4095) { sy = 4095; }
		packed = ((uint32_t)sx & 0xFFF) | (((uint32_t)sy & 0xFFF) << 12);
		r[0x05] = (uint8_t)(packed & 0xFF);
		r[0x06] = (uint8_t)((packed >> 8) & 0xFF);
		r[0x07] = (uint8_t)((packed >> 16) & 0xFF);

		stick_x = (int16_t)((int32_t)stick_x * STICK_DECAY_NUM
				    / STICK_DECAY_DEN);
		stick_y = (int16_t)((int32_t)stick_y * STICK_DECAY_NUM
				    / STICK_DECAY_DEN);
	}

	{
		int32_t dx_all = mouse_dx;
		int32_t dy_all = mouse_dy;

		mouse_dx -= dx_all;
		mouse_dy -= dy_all;
		if (ptr_off()) {
			r[0x0D] = 0xFF;
		}
		int16_t wx = rs_wx, wy = rs_wy;

		if (ptr_off()) {
			wx = 0;
			wy = 0;
		}
		if (ptr_off()) {
			mg_rem_x = 0;
			mg_rem_y = 0;
		} else {
			mouse_gain_apply(&wx, &wy);
		}
		r[0x09] = wx & 0xFF;
		r[0x0A] = (wx >> 8) & 0xFF;
		r[0x0B] = wy & 0xFF;
		r[0x0C] = (wy >> 8) & 0xFF;
	}

	sweep_apply(r);

	uint16_t t = (uint16_t)((k_uptime_get() * 602) / 1000);

	r[MOTION_AT + 5] = (uint8_t)(t & 0xFF);
	r[MOTION_AT + 6] = (uint8_t)(t >> 8);
#endif

	{
		static struct bt_gatt_notify_params np;

		np.attr = rpt_hid_attr;
		np.data = r;
		np.len = sizeof(r);
		np.func = rpt_sent_cb;
		np.user_data = NULL;

		atomic_inc(&rpt_inflight);
		err = bt_gatt_notify_cb(g_conn, &np);
		if (err) {
			atomic_dec(&rpt_inflight);
		}
	}
}

static void flush_pending_responses(void)
{
	struct pending_rsp p;

	while (g_conn && cmd_rsp_attr && k_msgq_peek(&rsp_q, &p) == 0) {
		if (bt_gatt_notify(g_conn, cmd_rsp_attr, p.data, p.len)) {
			return;
		}
		(void)k_msgq_get(&rsp_q, &p, K_NO_WAIT);
	}
}

static void enter_pair_mode(void)
{
	host_pending = false;
	console_bonded = false;
	memset(console_host, 0, 6); { extern void console_forget(void); console_forget(); }

	have_ltk = false;
	if (console_known) {
		bt_unpair(BT_ID_DEFAULT, &console_addr);
	}

	arm_pairable_window();

	if (g_conn) {
		bt_conn_disconnect(g_conn, BT_HCI_ERR_REMOTE_USER_TERM_CONN);
	}
}

static bool sync_gesture_held(void)
{
#if defined(JC_LEFT)
	return sync_key_down();
#else
	return (peer_buttons & LINK_SYNC_REQ) != 0;
#endif
}

static volatile bool sync_cmd_req;

static void sync_key_print(void)
{
	printk("[SYNC] shortcut key 0x%02X mods 0x%02X %s\n", sync_key.code,
	       sync_key.mods, sync_key.on ? "on" : "off");
}

static void sync_cmd(const char *a)
{
	if (*a == '?') {
		sync_key_print();
		return;
	}
	while (*a == ' ' || *a == '\t') {
		a++;
	}
	if (!*a) {
		sync_cmd_req = true;
		return;
	}
	if (!strcmp(a, "on") || !strcmp(a, "off")) {
		sync_key.on = (a[1] == 'n');
	} else if (!strncmp(a, "key", 3)) {
		char *e;
		long code = strtol(a + 3, &e, 0);
		long mods = strtol(e, NULL, 0);

		if (code < 0 || code > 0xFF || mods < 0 || mods > 0xFF) {
			return;
		}
		sync_key.code = (uint8_t)code;
		sync_key.mods = (uint8_t)mods;
	} else {
		return;
	}
	settings_save_one("jc/sync", &sync_key,
			  sizeof(sync_key));

	sync_key_print();
}

static void sync_tick(void)
{
	static int64_t held_since;
	static bool holding;
	static bool fired;

	if (sync_cmd_req) {
		sync_cmd_req = false;
		enter_pair_mode();
	}
	if (!sync_gesture_held()) {
		holding = false;
		fired = false;
		return;
	}
	if (!holding) {
		holding = true;
		fired = false;
		held_since = k_uptime_get();
		return;
	}
	if (!fired && k_uptime_get() - held_since >= SYNC_HOLD_MS) {
		fired = true;
		enter_pair_mode();
	}
}

#if !defined(JC_LEFT)
#define WAKE_HOLD_MS 1000
#define WAKE_ADV_MS 30000

static bool wake_home_held(void)
{
	uint8_t nb = CFG.n_btn > JC_CFG_BTNS ? JC_CFG_BTNS : CFG.n_btn;
	uint16_t b = (peer_buttons & ~(LINK_SYNC_REQ | LINK_STICK)) |
		     wheel_btn_now();

	for (uint8_t j = 0; j < nb; j++) {
		if (CFG.btn[j].kind == JC_BTN_MOUSE &&
		    (mouse_buttons & CFG.btn[j].code)) {
			b |= CFG.btn[j].mask;
		}
	}
	return (b & RBTN_HOME) != 0;
}

static void wake_tick(void)
{
	static int64_t held_since;
	static bool holding, fired;
	int64_t now = k_uptime_get();

	if (wake_until && (g_conn || now >= wake_until)) {
		wake_until = 0;
		k_work_submit(&adv_work);
	}

	if (!wake_home_held()) {
		holding = false;
		fired = false;
		return;
	}
	if (!holding) {
		holding = true;
		fired = false;
		held_since = now;
		return;
	}
	if (fired || now - held_since < WAKE_HOLD_MS) {
		return;
	}
	fired = true;
	if (g_conn) {
		return;
	}
	if (!console_bonded) {
		return;
	}
	wake_until = now + WAKE_ADV_MS;
	k_work_submit(&adv_work);
}
#endif

static void dump_map(void)
{
#if defined(JC_LEFT)
	printk("[MAP] side=L tag=" BUILD_TAG "\n");
	{
		uint8_t nb = CFG.n_btn > JC_CFG_BTNS ? JC_CFG_BTNS : CFG.n_btn;

		for (size_t j = 0; j < nb; j++) {
			uint8_t code = CFG.btn[j].code;
			uint8_t kind = CFG.btn[j].kind;
			uint16_t mask = CFG.btn[j].mask;

			if (kind == JC_BTN_MODE) {
				printk("[MAP] mode %02X %02X %s\n", code,
				       (unsigned)(mask & 0xFF),
				       jc_mode_id_name((uint8_t)(mask >> 8)));
			}
		}
	}
	printk("[MAP] sync %02X mods %02X %s\n", sync_key.code, sync_key.mods,
	       sync_key.on ? "on" : "off");
#else
	printk("[MAP] side=R tag=" BUILD_TAG "\n");
	printk("[MAP] sens %d\n", stick_mgain);
	printk("[MAP] msens %u\n", mouse_gain);
	printk("[MAP] stickdz %d\n", stick_dz);
	printk("[MAP] stickvg %d\n", stick_vgain);
#endif
	printk("[MAP] grip %d\n", grip_byte >= 0 ? 1 : 0);
	printk("[MAP] prof act %u\n", prof_active);
	for (uint8_t i = 0; i < JC_PROFILES; i++) {
		printk("[MAP] prof %u %u %02X %02X %s\n", i + 1U,
		       prof_filled(i + 1U) ? 1U : 0U, prof_key[i].code,
		       prof_key[i].mods, prof_name[i]);
	}
	printk("[MAP] colors %02X%02X%02X %02X%02X%02X %02X%02X%02X %02X%02X%02X\n",
	       CFG.col_body[0], CFG.col_body[1], CFG.col_body[2],
	       CFG.col_buttons[0], CFG.col_buttons[1], CFG.col_buttons[2],
	       CFG.col_rail[0], CFG.col_rail[1], CFG.col_rail[2],
	       CFG.col_stick[0], CFG.col_stick[1], CFG.col_stick[2]);
	printk("[MAP] end\n");
}

static void report_thread(void *a, void *b, void *c)
{
	int64_t due = k_uptime_get();

	ARG_UNUSED(a); ARG_UNUSED(b); ARG_UNUSED(c);
	while (1) {
		if (hid_reattached) {
			hid_reattached = false;
			adv_start();
		}
		flush_pending_responses();
		sync_tick();
#if !defined(JC_LEFT)
		wake_tick();
#endif
		send_report();

		due += report_ms;
		{
			int64_t now = k_uptime_get();

			if (due <= now) {
				due = now + report_ms;
			}
			k_sleep(K_TIMEOUT_ABS_MS(due));
		}
	}
}

K_THREAD_DEFINE(report_tid, 3072, report_thread, NULL, NULL, NULL, 5, 0, 0);

static void adv_work_handler(struct k_work *work)
{
	ARG_UNUSED(work);
	adv_start();
}

static void connected(struct bt_conn *conn, uint8_t err)
{
	char astr[BT_ADDR_LE_STR_LEN];
	struct bt_conn_info info;

	if (err) {
		if (hid_conn_pending &&
		    !bt_addr_le_cmp(bt_conn_get_dst(conn), &hid_conn_addr)) {
			hid_conn_pending = false;
			bt_addr_le_to_str(&hid_conn_addr, astr, sizeof(astr));
			printk("[MOUSE] connect to %s failed (0x%02x), scanning "
			       "again\n", astr, err);
			if (peer_trying &&
			    !bt_addr_le_cmp(&peer_try, &hid_conn_addr)) {
				peer_mark_bad(&hid_conn_addr);
				peer_trying = false;
			}
			if (PEER.have && pin_fails < 255 &&
			    !bt_addr_le_cmp(&PEER.addr, &hid_conn_addr)) {
				pin_fails++;
			}
			hid_cool(&hid_conn_addr);
			k_work_schedule(&scan_work, K_SECONDS(1));
			k_work_submit(&adv_work);
			return;
		}
		printk("[JC] connect failed (0x%02x)\n", err);
		k_work_submit(&adv_work);
		return;
	}
	bt_addr_le_to_str(bt_conn_get_dst(conn), astr, sizeof(astr));

	if (bt_conn_get_info(conn, &info) == 0 &&
	    info.role == BT_CONN_ROLE_CENTRAL) {
		printk("[MOUSE] *** CONNECTED to %s ***\n", astr);
		g_mouse = bt_conn_ref(conn);
		hid_params_asked = false;
		hid_usable_at = 0;
		hid_disc_started = false;
		hid_conn_pending = false;
		hid_link_secured = false;

		hid_reattached = true;

		struct bond_keep keep = {
			.console = console_known ? &console_addr
						 : (g_conn ? bt_conn_get_dst(g_conn)
							   : NULL),
			.peer = bt_conn_get_dst(conn),
		};

		bond_count = 0;
		bt_foreach_bond(BT_ID_DEFAULT, collect_bond, &keep);
		for (uint8_t i = 0; i < bond_count; i++) {
			bt_unpair(BT_ID_DEFAULT, &bond_list[i]);
		}
		if (hid_want_passkey &&
		    bt_conn_auth_cb_overlay(conn, &hid_io_cb)) {
		}
		if (bt_conn_set_security(conn, BT_SECURITY_L2)) {
			mouse_start_discovery(conn);
		}
		return;
	}

	if (console_bonded && !pairable_until &&
	    memcmp(bt_conn_get_dst(conn)->a.val, console_host, 6)) {
		conn_refused = conn;
		bt_conn_disconnect(conn, BT_HCI_ERR_REMOTE_USER_TERM_CONN);
		return;
	}

	report_pace(info.le.interval_us);

	bt_le_adv_stop();

	bt_addr_le_copy(&console_addr, bt_conn_get_dst(conn));
	console_known = true;

	g_conn = bt_conn_ref(conn);
	scan_pace_check();
}

static void security_changed(struct bt_conn *conn, bt_security_t level,
			     enum bt_security_err err)
{
	if (g_mouse && conn == g_mouse) {
		printk("[MOUSE] security level=%d err=%d\n", (int)level, (int)err);
		if (!err) {
			hid_want_passkey = false;
			hid_link_secured = true;
		} else if (err == BT_SECURITY_ERR_AUTH_REQUIREMENT &&
			   !hid_want_passkey) {
			hid_want_passkey = true;
		}
		if (err) {
			if (err == BT_SECURITY_ERR_PIN_OR_KEY_MISSING) {
				bt_addr_le_copy(&hid_forget_addr,
						bt_conn_get_dst(conn));
				k_work_submit(&hid_forget_work);
				return;
			}
			if (peer_trying) {
				peer_mark_bad(bt_conn_get_dst(conn));
				peer_trying = false;
			}
			hid_cool(bt_conn_get_dst(conn));
			bt_conn_disconnect(conn, BT_HCI_ERR_REMOTE_USER_TERM_CONN);
			return;
		}
		peer_commit(bt_conn_get_dst(conn));
		if (hid_disc_started) {
			return;
		}
		mouse_start_discovery(conn);
	}
}

static void console_drop_fn(struct k_work *work)
{
	ARG_UNUSED(work);

	if (g_conn && !hid_ready()) {
		bt_conn_disconnect(g_conn,
				   BT_HCI_ERR_REMOTE_USER_TERM_CONN);
	}
}
static K_WORK_DEFINE(console_drop_work, console_drop_fn);

static void disconnected(struct bt_conn *conn, uint8_t reason)
{
	if (g_mouse && conn == g_mouse) {
		bool was_usable = hid_ready();

		printk("[MOUSE] *** DISCONNECTED, reason 0x%02x *** reports=%u\n",
		       reason, mouse_reports);
		if (PEER.have &&
		    !bt_addr_le_cmp(&PEER.addr, bt_conn_get_dst(conn))) {
			pin_heard_at = k_uptime_get();
			if (!hid_link_secured && pin_fails < 255) {
				pin_fails++;
			}
		}
		bt_conn_unref(g_mouse);
		g_mouse = NULL;
		sub_count = 0;
		hid_usable_at = 0;
		k_work_cancel_delayable(&hid_param_work);
		hid_disc_started = false;
		if (hm_keep_save()) {
			bt_addr_le_copy(&hm_keep_addr, bt_conn_get_dst(conn));
		}
		hm_reset();
		batt_forget();
		mouse_buttons = 0;
#if defined(JC_LEFT)
		kbd_mods = 0;
		memset(kbd_keys, 0, sizeof(kbd_keys));
		if (jc_mode == JC_MODE_KEYBOARD) {
			jc_usb_kbd_release_all();
		}
#endif
		mouse_reports = 0;
		rpt_handle = 0;
		k_work_schedule(&scan_work, K_SECONDS(1));
		if (was_usable && g_conn) {
			k_work_submit(&console_drop_work);
		}
		return;
	}

	if (conn == conn_refused) {
		conn_refused = NULL;
		k_work_submit(&adv_work);
		return;
	}

	streaming = false;
	stream_started_at = 0;
	report_seq = 0;
	grip_wait = -1;
	atomic_set(&rpt_inflight, 0);
	mouse_has_moved = false;
	asked_slower = false;
	user_active = false;

	if (host_pending) {
		memcpy(console_host, pending_host, 6);
		console_bonded = true;
		host_pending = false;
		pairable_until = 0;
	}

	if (g_conn) {
		bt_conn_unref(g_conn);
		g_conn = NULL;
	}
	k_work_submit(&adv_work);
}

static bool le_param_req(struct bt_conn *conn, struct bt_le_conn_param *param)
{
	if (!g_mouse || conn != g_mouse) {
		return true;
	}
	if (param->latency) {
		param->latency = 0;
	}
	if (param->interval_max > 12) {
		param->interval_max = 12;
	}
	if (param->interval_min > param->interval_max) {
		param->interval_min = param->interval_max;
	}
	if (param->timeout < 100) {
		param->timeout = 400;
	}
	return true;
}

static void le_param_updated(struct bt_conn *conn, uint16_t interval,
			     uint16_t latency, uint16_t timeout)
{
	if (conn == g_conn) {
		report_pace(interval * 1250u);
	}
	if (g_mouse && conn == g_mouse && latency) {
		hid_params_asked = false;
	}
}

BT_CONN_CB_DEFINE(conn_callbacks) = {
	.connected = connected,
	.disconnected = disconnected,
	.le_param_req = le_param_req,
	.le_param_updated = le_param_updated,
	.security_changed = security_changed,
};

void k_sys_fatal_error_handler(unsigned int reason, const struct arch_esf *esf)
{
	ARG_UNUSED(reason);
	ARG_UNUSED(esf);
	NVIC_SystemReset();
}

int main(void)
{
	int err;

	{ extern void chip_addr(uint8_t addr[6]); chip_addr(FAKE_ADDR); } bt_ctlr_set_public_addr(FAKE_ADDR);

	err = bt_enable(NULL);
	if (err) {
		return 0;
	}

	if (IS_ENABLED(CONFIG_BT_SETTINGS)) {
		settings_load_subtree("bt");
	}

	{
		struct bond_keep none = { NULL, NULL };

		bond_count = 0;
		bt_foreach_bond(BT_ID_DEFAULT, collect_bond, &none);
		for (uint8_t i = 0; i < bond_count; i++) {
			if (!PEER.have) { bt_unpair(BT_ID_DEFAULT, &bond_list[i]); }
		}
		bond_count = 0;
	}

	{ extern bool console_load(bt_addr_le_t *, uint8_t *); extern bool console_install(const bt_addr_le_t *, const uint8_t *); bt_addr_le_t pca; uint8_t pck[16]; if (console_load(&pca, pck) && console_install(&pca, pck)) { bt_addr_le_copy(&console_addr, &pca); console_known = true; memcpy(console_host, pca.a.val, 6); console_bonded = true; } memset(pck, 0, sizeof(pck)); } cfg_serial_init();

	cmd_rsp_attr = bt_gatt_find_by_uuid(NULL, 0, UUID_CMDRSP);

	rpt_hid_attr = bt_gatt_find_by_uuid(NULL, 0, UUID_RPTHID);

	{
		const struct bt_gatt_attr *c2 = bt_gatt_find_by_uuid(NULL, 0, UUID_CFG2);
		uint16_t vh = c2 ? bt_gatt_attr_get_handle(c2) : 0;

		if (vh) {
			cfg1_value[0] = (vh - 1) & 0xFF;
			cfg1_value[1] = (vh - 1) >> 8;
			cfg1_value[2] = vh & 0xFF;
			cfg1_value[3] = vh >> 8;
		}
	}

	bt_conn_auth_info_cb_register(&hid_auth_cb);

	adv_start();
	k_work_schedule(&scan_work, K_NO_WAIT);

	while (1) {
		static uint8_t tick;

		k_sleep(K_SECONDS(5));

		if (++tick >= 6 && g_mouse && batt_handle && !batt_read_busy) {
			tick = 0;
			batt_read_busy = true;
			batt_rd.func = batt_read_cb;
			batt_rd.handle_count = 1;
			batt_rd.single.handle = batt_handle;
			batt_rd.single.offset = 0;
			if (bt_gatt_read(g_mouse, &batt_rd)) {
				batt_read_busy = false;
			}
		}
		adv_start();
		dump_map();

		if (g_mouse) {
			struct bt_conn_info info;

			if (!bt_conn_get_info(g_mouse, &info) &&
			    info.type == BT_CONN_TYPE_LE) {
				if (info.le.latency) {
					hid_params_asked = false;
				}
			}
		}
		if (g_mouse && !hid_params_asked && hid_usable_at &&
		    k_uptime_get() - hid_usable_at >= HID_PARAM_DELAY_MS) {
			hid_params_request();
		}

#if defined(JC_LEFT)
		printk("[HID] kbd=%s\n",
		       g_mouse ? (sub_count ? "subscribed" : "linked") : "none");
#else
		printk("[HID] mouse=%s\n",
		       g_mouse ? (sub_count ? "subscribed" : "linked") : "none");
#endif
	}
	return 0;
}
