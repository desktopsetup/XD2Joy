#include <zephyr/kernel.h>
#include <zephyr/settings/settings.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/hci.h>
#include <host/keys.h>
#include <string.h>
#include <errno.h>

#define PCON_VER 1
#define PCON_KEY "jc_con/rec"

struct pcon_rec {
	uint8_t ver;
	bt_addr_le_t addr;
	uint8_t ltk[16];
};

static struct pcon_rec to_save;
static struct pcon_rec loaded;
static bool loaded_ok;

static int pcon_set(const char *name, size_t len, settings_read_cb read_cb, void *cb_arg)
{
	const char *next;
	struct pcon_rec r;

	if (!settings_name_steq(name, "rec", &next) || next) {
		return -ENOENT;
	}
	if (len != sizeof(r) || read_cb(cb_arg, &r, sizeof(r)) != (ssize_t)sizeof(r) ||
	    r.ver != PCON_VER) {
		return 0;
	}
	loaded = r;
	loaded_ok = true;
	return 0;
}

SETTINGS_STATIC_HANDLER_DEFINE(jc_con, "jc_con", NULL, pcon_set, NULL, NULL);

static void pcon_save_fn(struct k_work *w)
{
	settings_save_one(PCON_KEY, &to_save, sizeof(to_save));
}
static K_WORK_DEFINE(pcon_save_work, pcon_save_fn);

static void pcon_forget_fn(struct k_work *w)
{
	settings_delete(PCON_KEY);
}
static K_WORK_DEFINE(pcon_forget_work, pcon_forget_fn);

void console_save(const bt_addr_le_t *addr, const uint8_t *ltk)
{
	to_save.ver = PCON_VER;
	bt_addr_le_copy(&to_save.addr, addr);
	memcpy(to_save.ltk, ltk, sizeof(to_save.ltk));
	k_work_submit(&pcon_save_work);
}

void console_forget(void)
{
	k_work_submit(&pcon_forget_work);
}

bool console_load(bt_addr_le_t *addr, uint8_t *ltk)
{
	loaded_ok = false;
	(void)settings_load_subtree("jc_con");
	if (!loaded_ok) {
		return false;
	}
	bt_addr_le_copy(addr, &loaded.addr);
	memcpy(ltk, loaded.ltk, sizeof(loaded.ltk));
	return true;
}

bool console_install(const bt_addr_le_t *addr, const uint8_t *ltk)
{
	struct bt_keys *k = bt_keys_get_addr(BT_ID_DEFAULT, addr);

	if (!k) {
		return false;
	}
	memset(&k->ltk, 0, sizeof(k->ltk));
	memcpy(k->ltk.val, ltk, 16);
	k->enc_size = 16;
	k->keys |= BT_KEYS_LTK_P256;
	k->flags |= BT_KEYS_AUTHENTICATED;
	return true;
}
