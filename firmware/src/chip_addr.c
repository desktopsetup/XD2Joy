#include <stdint.h>
#include <hal/nrf_ficr.h>

void chip_addr(uint8_t addr[6])
{
	const uint32_t id[2] = {
		nrf_ficr_deviceid_get(NRF_FICR, 1),
		nrf_ficr_deviceid_get(NRF_FICR, 0),
	};
	uint32_t h = 0x811C9DC5u;

	for (int w = 0; w < 2; w++) {
		for (int shift = 24; shift >= 0; shift -= 8) {
			h ^= (id[w] >> shift) & 0xFFu;
			h *= 0x01000193u;
		}
	}
	h = (h ^ (h >> 24)) & 0xFFFFFFu;

	addr[0] = h & 0xFFu;
	addr[1] = (h >> 8) & 0xFFu;
	addr[2] = (h >> 16) & 0xFFu;
	addr[3] = 0x70;
	addr[4] = 0x68;
	addr[5] = 0xB8;
}
