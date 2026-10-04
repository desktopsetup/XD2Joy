# Zephyr patches

## Patch 1: allow the Switch 2 5ms connection interval

**File:** `include/zephyr/bluetooth/hci_types.h`

```c
-#define BT_HCI_LE_INTERVAL_MIN            0x0006
+#define BT_HCI_LE_INTERVAL_MIN            0x0004
```

without this the board seems to pair normally in Change Grip/Order then you back out and it just disconnects from the console.

## Patch 2: a wider transmit window for keyboards and mice

**Files:** `subsys/bluetooth/controller/ll_sw/nordic/lll/lll_scan.c` and
`subsys/bluetooth/controller/ll_sw/ull_llcp_conn_upd.c`

When the board connects to a keyboard or mouse the board is the Bluetooth central, the stock
Zephyr does the tightest transmit window the specification allows.

some keyboards are not listening yet at 1.25ms after the connection request. Measured
with a AULA F75, the link never came up (disconnect reason 0x3e, "connection failed to
be established") and the keyboard then came back on a new random address which looks
like its address kept changing. The wider window and same keyboard connected on the
first try it stayed connected through connection updates.
