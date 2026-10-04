#!/bin/bash
set -u
HERE="$(cd "$(dirname "$0")" && pwd)"
ZP="${ZEPHYR_WORKSPACE:-$HOME/zephyrproject}"
export ZEPHYR_BASE="$ZP/zephyr"
BOARD=promicro_nrf52840/nrf52840/uf2
DIR=build_xd2joy

if ! grep -qE "define BT_HCI_LE_INTERVAL_MIN[[:space:]]+0x0004" "$ZEPHYR_BASE/include/zephyr/bluetooth/hci_types.h"; then
  echo "STOP: BT_HCI_LE_INTERVAL_MIN is not 0x0004 -- see ZEPHYR_PATCHES.md"
  exit 1
fi
for f in subsys/bluetooth/controller/ll_sw/nordic/lll/lll_scan.c subsys/bluetooth/controller/ll_sw/ull_llcp_conn_upd.c; do
  if ! grep -q "conn window patch" "$ZEPHYR_BASE/$f"; then
    echo "STOP: Zephyr patch 2 (conn window) missing from $f -- see ZEPHYR_PATCHES.md"
    exit 1
  fi
done

check() {
  if grep -qE -- "$2" "$1" 2>/dev/null; then echo "  ok    $3"; else echo "  FAIL  $3"; bad=1; fi
}
absent() {
  if grep -qE -- "$2" "$1" 2>/dev/null; then echo "  FAIL  $3"; bad=1; else echo "  ok    $3"; fi
}

mkdir -p "$HERE/out"
failed=0
for SIDE in RIGHT LEFT; do
  B="$DIR"
  ARGS=()
  if [ "$SIDE" = LEFT ]; then B="${DIR}L"; ARGS+=(-DJC_LEFT=1); fi
  OUT="$ZP/$B/zephyr"
  echo "=== $SIDE  ($ZP/$B)"
  if ! (cd "$ZP" && west build -d "$B" -b "$BOARD" "$HERE" --pristine -- ${ARGS[@]+"${ARGS[@]}"}); then
    echo "BUILD FAILED: $SIDE -- nothing copied"; failed=1; continue
  fi

  bad=0
  echo "--- checks"
  check "$OUT/.config" '^CONFIG_CLOCK_CONTROL_NRF_K32SRC_RC_CALIBRATION=y$' "RC calibration enabled"
  check "$OUT/.config" '^CONFIG_CLOCK_CONTROL_NRF_DRIVER_CALIBRATION=y$' "calibration run by the clock driver"
  check "$OUT/.config" '^CONFIG_CLOCK_CONTROL_NRF_USES_TEMP_SENSOR=y$' "temp triggered calibration"
  check "$OUT/zephyr.dts" 'k32src = "rc";' "32 kHz source = RC oscillator"
  check "$OUT/zephyr.dts" 'k32src-accuracy-ppm = < 0x1f4 >;' "accuracy 500 ppm (Bluetooth window sizing)"
  check "$ZP/$B/compile_commands.json" 'JC_CONN_WIN_OFFSET=4 .*JC_CONN_WIN_SIZE=3 .*lll_scan\.c",$' "conn window patch on: CONNECT_IND (offset 4, size 3)"
  check "$ZP/$B/compile_commands.json" 'JC_CONN_WIN_OFFSET=4 .*JC_CONN_WIN_SIZE=3 .*ull_llcp_conn_upd\.c",$' "conn window patch on: connection update"
  check "$HERE/src/main.c" 'atomic_set_bit\(s->flags, BT_GATT_SUBSCRIBE_FLAG_VOLATILE\);' "per connection HID subscriptions (reconnect fix)"
  if [ "$SIDE" = LEFT ]; then
    check "$OUT/zephyr.dts" 'psels = < 0x9 >;' "wire TX on P0.09 (dev board)"
    check "$OUT/zephyr.dts" 'psels = < 0x100000a >;' "wire RX on P0.10 (Dev board)"
  else
    check "$OUT/zephyr.dts" 'psels = < 0x24 >;' "wire TX on P1.04 (dev board)"
    check "$OUT/zephyr.dts" 'psels = < 0x1000026 >;' "wire RX on P1.06 (dev board)"
    absent "$OUT/zephyr.dts" 'psels = < 0x9 >;|0x100000a' "no wire pin left on P0.09 / P0.10"
  fi
  if [ "$SIDE" = RIGHT ]; then
    check "$OUT/.config" '^CONFIG_CDC_ACM_SERIAL_PRODUCT_STRING="XD2Joy \(right\)"$' "USB name \"XD2Joy (right)\""
  else
    check "$OUT/zephyr.elf" 'XD2Joy \(left\)' "USB name \"XD2Joy (left)\""
  fi
  check "$OUT/.config" '^CONFIG_BT_CTLR_TX_PWR_DBM=8$' "transmit power +8 dBm"
  absent "$OUT/zephyr.elf" 'LTK =' "private log no pairing key print left"
  absent "$OUT/zephyr.elf" ' keys=%02X' "private log, no key codes in the status line"
  check "$HERE/src/main.c" 'if \(jc_mode != JC_MODE_KEYBOARD &&' "private log, wit no raw reports in keyboard mode"
  check "$HERE/src/main.c" 'if \(g_mouse\) \{ bt_conn_disconnect\(g_mouse, BT_HCI_ERR_REMOTE_USER_TERM_CONN\); \}' "picked devices only... unassigning drops the attached device"
  check "$HERE/src/main.c" 'if \(!PEER\.have\) \{ bt_unpair\(BT_ID_DEFAULT, &bond_list\[i\]\); \}' "picked devices only an unassigned half forgets old pairings at boot"
  check "$HERE/src/main.c" 'console_save\(bt_conn_get_dst\(conn\), pair_ltk\)' "console kept  saved when pairing completes"
  check "$HERE/src/main.c" 'console_forget\(\); \}' "console kept the sync gesture deletes it"
  check "$HERE/src/main.c" 'console_load\(&pca, pck\) && console_install' "console kept put back at boot"
  check "$HERE/src/main.c" 'if \(\(!console_bonded \|\| pairable_until\) && !hid_ready\(\)\) \{ return ADV_OFF; \}' "pairing waits for input no controller with nothing behind it"
  check "$HERE/src/main.c" 'prof_active = val; prof_store_u8\("jc/pact", val\); \}' "profiles: a half follows the other onto a slot it has no copy of"
  check "$HERE/src/main.c" 'chip_addr\(FAKE_ADDR\); \} bt_ctlr_set_public_addr\(FAKE_ADDR\);' "address made from this chip, before bt_enable()"
  absent "$HERE/src/main.c" 'static const uint8_t FAKE_ADDR' "no fixed address left"
  check "$OUT/zephyr.elf" 'HDL50008895827' "fake asl serial HDL50008895827 in the image"
  if [ "$SIDE" = RIGHT ]; then
    check "$OUT/zephyr.elf" 'HCW51085906458' "made up right Joy-Con serial HCW51085906458 in the image"
  else
    check "$OUT/zephyr.elf" 'HBW51085343086' "made up left Joy-Con serial HBW51085343086 in the image"
  fi
  if [ $bad -ne 0 ] || [ ! -f "$OUT/zephyr.uf2" ]; then
    echo "CHECKS FAILED: $SIDE -- nothing copied"; failed=1; continue
  fi
  cp "$OUT/zephyr.uf2" "$HERE/out/xd2joy_${SIDE}.uf2"
  echo "  -> firmware/out/xd2joy_${SIDE}.uf2"
done
exit $failed
