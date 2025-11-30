#include <machine/display.h>
#include <machine/uart.h>
#include <utility/ostream.h>

using namespace EPOS;

OStream cout;

static UART uart;

static void delay() {
    for (long i = 0; i < 20000000; i++);
}

class XModem {
    static constexpr unsigned char SOH = 0x01;
    static constexpr unsigned char EOT = 0x04;
    static constexpr unsigned char ACK = 0x06;
    static constexpr unsigned char NAK = 0x15;
    static constexpr unsigned char CAN = 0x18;
    static constexpr int PACKET_SIZE = 128;

    static int wait_byte(int timeout = 1000000) {
        while (timeout--) {
            int c = uart.get();
            if (c >= 0)
                return c;
        }
        return -1;
    }

  public:
    static void receive(char *destination) {
        enum { WAITING, START, END } state = WAITING;
        char buffer[128];
        int index = 0;

        while (1) {
            switch (state) {
            case WAITING: {
                uart.put(NAK);
                delay();
                state = START;
                break;
            }
            case START: {
                unsigned char h = wait_byte();
                if (h == EOT) {
                    state = END;
                    continue;
                }
                if (h != SOH) {
                    state = WAITING;
                    continue;
                }

                unsigned char block_number = wait_byte();
                unsigned char block_number_inverse = wait_byte();

                if ((block_number ^ block_number_inverse) != 0xFF) {
                    state = WAITING;
                    continue;
                }

                int checksum = 0;
                for (int i = 0; i < 128; i++) {
                    buffer[i] = wait_byte();
                    checksum += buffer[i];
                }

                if ((checksum & 0xFF) != (unsigned char)wait_byte()) {
                    uart.put(NAK);
                    continue;
                };

                for (int i = 0; i < 128; i++) {
                    destination[index++] = buffer[i];
                }

                uart.put(ACK);
                break;
            }

            case END: {
                uart.put(ACK);
            }
            }
        }
    }
};

int main() {
    char buffer[10 * 1024];

    cout << "WAITING..." << endl;
    XModem::receive(buffer);
    cout << "\033[2J";
    cout << "\033[H";
    cout << "Done!" << endl;

    for (int i = 0; i < 128; i++)
        cout << buffer[i];
    return 0;
}
