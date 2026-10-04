#include <zephyr/device.h>
#include <zephyr/init.h>
#include <zephyr/kernel.h>
#include <zephyr/usb/usbd.h>
#include <string.h>
#include <zephyr/usb/class/usbd_hid.h>

#define JC_USB_VID 0x2fe3
#define JC_USB_PID 0x0005

USBD_DEVICE_DEFINE(jc_usbd, DEVICE_DT_GET(DT_NODELABEL(zephyr_udc0)),
		   JC_USB_VID, JC_USB_PID);

USBD_DESC_LANG_DEFINE(jc_usb_lang);
USBD_DESC_MANUFACTURER_DEFINE(jc_usb_mfr, "XD2Joy");
USBD_DESC_PRODUCT_DEFINE(jc_usb_product, "XD2Joy (left)");
IF_ENABLED(CONFIG_HWINFO, (USBD_DESC_SERIAL_NUMBER_DEFINE(jc_usb_sn)));

USBD_DESC_CONFIG_DEFINE(jc_usb_fs_desc, "FS Configuration");

USBD_CONFIGURATION_DEFINE(jc_usb_fs_config, 0, 125, &jc_usb_fs_desc);

static const uint8_t jc_kbd_report_desc[] = HID_KEYBOARD_REPORT_DESC();

static bool jc_kbd_ready;

bool jc_usb_kbd_ready(void)
{
	return jc_kbd_ready;
}

static const struct device *jc_kbd_dev;
static uint8_t jc_kbd_wanted[8];
static uint8_t jc_kbd_sent[8];
static bool jc_kbd_inflight;
static struct k_spinlock jc_kbd_lock;

static bool jc_kbd_claim(void)
{
	k_spinlock_key_t key = k_spin_lock(&jc_kbd_lock);
	bool go = false;

	if (jc_kbd_ready && !jc_kbd_inflight &&
	    memcmp(jc_kbd_sent, jc_kbd_wanted, sizeof(jc_kbd_sent)) != 0) {
		memcpy(jc_kbd_sent, jc_kbd_wanted, sizeof(jc_kbd_sent));
		jc_kbd_inflight = true;
		go = true;
	}
	k_spin_unlock(&jc_kbd_lock, key);
	return go;
}

static void jc_kbd_pump(void)
{
	k_spinlock_key_t key;

	if (!jc_kbd_claim()) {
		return;
	}
	if (hid_device_submit_report(jc_kbd_dev, sizeof(jc_kbd_sent),
				     jc_kbd_sent) == 0) {
		return;
	}

	key = k_spin_lock(&jc_kbd_lock);
	jc_kbd_inflight = false;
	memset(jc_kbd_sent, 0xFF, sizeof(jc_kbd_sent));
	k_spin_unlock(&jc_kbd_lock, key);
}

static void jc_kbd_report_done(const struct device *dev,
			       const uint8_t *const report)
{
	ARG_UNUSED(dev);
	ARG_UNUSED(report);

	k_spinlock_key_t key = k_spin_lock(&jc_kbd_lock);

	jc_kbd_inflight = false;
	k_spin_unlock(&jc_kbd_lock, key);
	jc_kbd_pump();
}

void jc_usb_kbd_send(uint8_t mods, const uint8_t keys[6])
{
	k_spinlock_key_t key = k_spin_lock(&jc_kbd_lock);

	jc_kbd_wanted[0] = mods;
	jc_kbd_wanted[1] = 0;
	memcpy(&jc_kbd_wanted[2], keys, 6);
	k_spin_unlock(&jc_kbd_lock, key);
	jc_kbd_pump();
}

void jc_usb_kbd_release_all(void)
{
	static const uint8_t none[6] = { 0 };

	jc_usb_kbd_send(0, none);
}

static void jc_kbd_iface_ready(const struct device *dev, const bool ready)
{
	ARG_UNUSED(dev);
	jc_kbd_ready = ready;
}

static int jc_kbd_get_report(const struct device *dev, const uint8_t type,
			     const uint8_t id, const uint16_t len,
			     uint8_t *const buf)
{
	ARG_UNUSED(dev);
	ARG_UNUSED(type);
	ARG_UNUSED(id);
	memset(buf, 0, len);
	return 0;
}

static int jc_kbd_set_report(const struct device *dev, const uint8_t type,
			     const uint8_t id, const uint16_t len,
			     const uint8_t *const buf)
{
	ARG_UNUSED(dev);
	ARG_UNUSED(id);
	ARG_UNUSED(len);
	ARG_UNUSED(buf);
	if (type != HID_REPORT_TYPE_OUTPUT) {
		return -ENOTSUP;
	}
	return 0;
}

static void jc_kbd_set_protocol(const struct device *dev, const uint8_t proto)
{
	ARG_UNUSED(dev);
}

static struct hid_device_ops jc_kbd_ops = {
	.iface_ready = jc_kbd_iface_ready,
	.get_report = jc_kbd_get_report,
	.set_report = jc_kbd_set_report,
	.set_protocol = jc_kbd_set_protocol,
	.input_report_done = jc_kbd_report_done,
};

static int jc_usb_init(void)
{
	const struct device *hid = DEVICE_DT_GET(DT_NODELABEL(jc_hid_kbd));
	bool kbd_ok = false;
	int err;

	if (device_is_ready(hid)) {
		err = hid_device_register(hid, jc_kbd_report_desc,
					  sizeof(jc_kbd_report_desc),
					  &jc_kbd_ops);
		if (!err) {
			jc_kbd_dev = hid;
			kbd_ok = true;
		}
	}

	err = usbd_add_descriptor(&jc_usbd, &jc_usb_lang);
	if (!err) {
		err = usbd_add_descriptor(&jc_usbd, &jc_usb_mfr);
	}
	if (!err) {
		err = usbd_add_descriptor(&jc_usbd, &jc_usb_product);
	}
	IF_ENABLED(CONFIG_HWINFO, (
		if (!err) {
			err = usbd_add_descriptor(&jc_usbd, &jc_usb_sn);
		}
	))
	if (err) {
		return err;
	}

	err = usbd_add_configuration(&jc_usbd, USBD_SPEED_FS, &jc_usb_fs_config);
	if (err) {
		return err;
	}

	err = usbd_register_class(&jc_usbd, "cdc_acm_0", USBD_SPEED_FS, 1);
	if (err) {
		return err;
	}

	if (kbd_ok) {
		err = usbd_register_class(&jc_usbd, "hid_0", USBD_SPEED_FS, 1);
	}

	err = usbd_device_set_code_triple(&jc_usbd, USBD_SPEED_FS,
					  USB_BCC_MISCELLANEOUS, 0x02, 0x01);
	if (err) {
		return err;
	}

	err = usbd_init(&jc_usbd);
	if (err) {
		return err;
	}

	return usbd_enable(&jc_usbd);
}

SYS_INIT(jc_usb_init, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);
