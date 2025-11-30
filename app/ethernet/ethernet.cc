#include <architecture.h>
#include <machine.h>
#include <process.h>
#include <synchronizer.h>
#include <utility/string.h>

using namespace EPOS;

using Reg64 = CPU::Reg64;
using Reg32 = CPU::Reg32;

OStream cout;

const int BUF_SIZE = 1536;
const int RING_SIZE = 4;

const Reg32 gmac_mac_conf = 0x0000;
const Reg32 gmac_mac_conf_te = 1 << 1;    // Transmit Enable
const Reg32 gmac_mac_conf_re = 1 << 0;    // Receive Enable
const Reg32 gmac_mac_conf_ps = 1 << 15;   // Port select
const Reg32 gmac_mac_conf_fes = 1 << 14;  // Fast ethernet speed
const Reg32 gmac_mac_conf_dm = 1 << 13;   // Duplex mode

// Promiscuous
const Reg32 gmac_packet_filter = 0x0008;
const Reg32 gmac_packet_filter_pr = 1 << 0;

const Reg32 gmac_mac_addr0_high = 0x300;
const Reg32 gmac_mac_addr0_low = 0x304;

const Reg32 dma_mode = 0x1000;
const Reg32 dma_mode_reset = 1 << 0;

const Reg32 dma_sysbus_mode = 0x1004;
const Reg32 dma_sysbus_mode_aal = 1 << 12;
const Reg32 dma_sysbus_mode_eame = 1 << 11;
const Reg32 dma_sysbus_mode_fb = 1 << 0;

const Reg32 dma_ch0_tx_ctrl = 0x1104;
const Reg32 dma_ch0_tx_ctrl_st = 1 << 0; // Start Transfer

const Reg32 dma_ch0_rx_ctrl = 0x1108;
const Reg32 dma_ch0_rx_ctrl_sr = 1 << 0;               // Start Receive
const Reg32 dma_ch0_rx_ctrl_pbl_32 = 32 << 16;         // Burst Length
const Reg32 dma_ch0_rx_ctrl_rbsz_1536 = BUF_SIZE << 1; // RX Buffer Size

const Reg32 dma_ch0_txdesc_list_h = 0x1110;
const Reg32 dma_ch0_txdesc_list_l = 0x1114;
const Reg32 dma_ch0_rxdesc_list_h = 0x1118;
const Reg32 dma_ch0_rxdesc_list_l = 0x111C;

const Reg32 dma_ch0_txdesc_tail = 0x1120;
const Reg32 dma_ch0_rxdesc_tail = 0x1128;

const Reg32 dma_ch0_txdesc_ring_len = 0x112C;
const Reg32 dma_ch0_rxdesc_ring_len = 0x1130;

struct DMA_Desc {
    volatile Reg32 addr_low;
    volatile Reg32 addr_high;
    volatile Reg32 control;
    volatile Reg32 status;
} __attribute__((aligned(8)));

const Reg32 RDES3_OWN = 1U << 31;
const Reg32 RDES3_IOC = 1 << 30;
const Reg32 RDES3_FD  = 1 << 29;
const Reg32 RDES3_LD  = 1 << 28;
const Reg32 RDES3_BUF1V = 1 << 24;

static volatile Reg32 & reg(unsigned int o) {
    return reinterpret_cast<volatile Reg32 *>(Memory_Map::ETH_BASE)[o / sizeof(Reg32)]; 
}

static Reg64 phy_addr(void * ptr) {
    return reinterpret_cast<Reg64>(ptr);
}

class DWMAC_Driver {
public:
    DWMAC_Driver() {
        cout << "Reseting... ";
        reg(dma_mode) |= dma_mode_reset;
        while(reg(dma_mode) & dma_mode_reset);
        cout << "Done." << endl;

        // TODO: Check if memset is working as expected
        memset(rx_ring, 0, sizeof(rx_ring));
        memset(tx_ring, 0, sizeof(tx_ring));

        Reg64 buf_addr = phy_addr(rx_buffer);

        rx_ring[0].addr_low = static_cast<Reg32>(buf_addr & 0xFFFFFFFF);
        rx_ring[0].addr_high = static_cast<Reg32>(buf_addr >> 32);
        rx_ring[0].control = 0;
        rx_ring[0].status = RDES3_OWN | RDES3_BUF1V | RDES3_IOC;

        rx_ring[0].addr_low = phy_addr(rx_buffer);
        rx_ring[0].addr_high = 0;
        rx_ring[0].control = 0;
        rx_ring[0].status = RDES3_OWN | RDES3_BUF1V | RDES3_IOC;

        // TODO: Check if the cache must be flushed here

        reg(gmac_mac_conf) |= gmac_mac_conf_ps | gmac_mac_conf_fes | gmac_mac_conf_dm;
        reg(dma_sysbus_mode) |= dma_sysbus_mode_aal | dma_sysbus_mode_eame | dma_sysbus_mode_fb;

        reg(dma_ch0_rx_ctrl) = dma_ch0_rx_ctrl_pbl_32 | dma_ch0_rx_ctrl_rbsz_1536;

        Reg64 tx_addr = phy_addr(tx_ring);
        Reg64 rx_addr = phy_addr(rx_ring);

        reg(dma_ch0_txdesc_list_h) = static_cast<Reg32>(tx_addr >> 32);
        reg(dma_ch0_txdesc_list_l) = static_cast<Reg32>(tx_addr & 0xFFFFFFFF);
        reg(dma_ch0_rxdesc_list_h) = static_cast<Reg32>(rx_addr >> 32);
        reg(dma_ch0_rxdesc_list_l) = static_cast<Reg32>(rx_addr & 0xFFFFFFFF);

        reg(dma_ch0_txdesc_ring_len) = RING_SIZE - 1;
        reg(dma_ch0_rxdesc_ring_len) = RING_SIZE - 1;

        reg(dma_ch0_tx_ctrl) |= dma_ch0_tx_ctrl_st;
        reg(dma_ch0_rx_ctrl) |= dma_ch0_rx_ctrl_sr;

        // TX and RX on
        reg(gmac_mac_conf) |= gmac_mac_conf_te | gmac_mac_conf_re;

        // Notify the NIC
        reg(dma_ch0_rxdesc_tail) = static_cast<Reg32>(phy_addr(&rx_ring[1]) & 0xFFFFFFFF);

        cout << "DWMAC Initialized. Waiting for packet..." << endl;
    }

    int receive_packet() {
        volatile DMA_Desc * desc = &rx_ring[0];

        while(true) {
            // TODO: Check if the cache needs to invalidated here
            Reg32 status = desc->status;

            if (!(status & RDES3_OWN)) {  // IF OWN is 0 the packet was received
                int len = status & 0x7FFF;

                // TODO: Check the RX buffer cache

                cout << "Packet Received. Length: " << len << endl;

                // cout << "Dest MAC: " << rx_buffer[0] << ":" << rx_buffer[1] << ":" << rx_buffer[2] << endl;

                // Re-arm descriptor for next packet (Refill)
                desc->status = RDES3_OWN | RDES3_BUF1V | RDES3_IOC;

                // Notify the NIC
                reg(dma_ch0_rxdesc_tail) = phy_addr(&rx_ring[1]);

                return len;
            }
        }
    }
private:
    DMA_Desc rx_ring[RING_SIZE] __attribute__((aligned(64)));
    DMA_Desc tx_ring[RING_SIZE] __attribute__((aligned(64)));

    unsigned char rx_buffer[BUF_SIZE] __attribute__((aligned(64)));
};

int main() {
    DWMAC_Driver dwmac_driver;

    dwmac_driver.receive_packet();

    return 0;
}
