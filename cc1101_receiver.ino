#include <SPI.h>

const uint8_t PIN_CC_GDO0{ 3 };
const uint8_t PIN_CC_GDO2{ 2 };

// cc1101 Command Strobes
const byte SRES{ 0x30 };    // Reset chip.
const byte SFSTXON{ 0x31 }; // Enableand calibrate frequency synthesizer(if MCSM0.FS_AUTOCAL = 1).If in RX(with CCA) :
                            // Go to a wait state where only the synthesizer is running(for quick RX / TX turnaround).
const byte SXOFF{ 0x32 };   // Turn off crystal oscillator.
const byte SCAL{ 0x33 };    // Calibrate frequency synthesizerand turn it off.SCAL can be strobed from IDLE mode without
                            // setting manual calibration mode(MCSM0.FS_AUTOCAL = 0)
const byte SRX{ 0x34 };     // Enable RX.Perform calibration first if coming from IDLEand MCSM0.FS_AUTOCAL = 1.
const byte STX{ 0x35 };     // In IDLE state : Enable TX.Perform calibration first if MCSM0.FS_AUTOCAL = 1.
                            // If in RX state and CCA is enabled : Only go to TX if channel is clear.
const byte SIDLE{ 0x36 };   // Exit RX / TX, turn off frequency synthesizerand exit Wake - On - Radio mode if applicable.
const byte SWOR{ 0x38 };    // Start automatic RX polling sequence(Wake - on - Radio) as described in Section 19.5 if
                            // WORCTRL.RC_PD = 0.
const byte SPWD{ 0x39 };    // Enter power down mode when CSn goes high.
const byte SFRX{ 0x3A };    // �������� ����� RX FIFO. ���������� SFRX � ���������� IDLE or RXFIFO_OVERFLOW.
const byte SFTX{ 0x3B };    // �������� ����� TX FIFO. ��������� SFTX � ���������� IDLE or TXFIFO_UNDERFLOW.
const byte SWORRST{ 0x3C }; // Reset real time clock to Event1 value.
const byte SNOP{ 0x3D };    // No operation.May be used to get access to the chip status byte.

// cc1101 Configuration Registers
const byte CC1101_IOCFG2{ 0x00 };
const byte CC1101_IOCFG1{ 0x01 };
const byte CC1101_IOCFG0{ 0x02 };
const byte CC1101_FIFOTHR{ 0x03 };
const byte CC1101_SYNC1{ 0x04 };
const byte CC1101_SYNC0{ 0x05 };
const byte CC1101_PKTLEN{ 0x06 };
const byte CC1101_PKTCTRL1{ 0x07 };
const byte CC1101_PKTCTRL0{ 0x08 };
const byte CC1101_ADDR{ 0x09 };
const byte CC1101_CHANNR{ 0x0A };
const byte CC1101_FSCTRL1{ 0x0B };
const byte CC1101_FSCTRL0{ 0x0C };
const byte CC1101_FREQ2{ 0x0D };
const byte CC1101_FREQ1{ 0x0E };
const byte CC1101_FREQ0{ 0x0F };
const byte CC1101_MDMCFG4{ 0x10 };
const byte CC1101_MDMCFG3{ 0x11 };
const byte CC1101_MDMCFG2{ 0x12 };
const byte CC1101_MDMCFG1{ 0x13 };
const byte CC1101_MDMCFG0{ 0x14 };
const byte CC1101_DEVIATN{ 0x15 };
const byte CC1101_MCSM2{ 0x16 };
const byte CC1101_MCSM1{ 0x17 };
const byte CC1101_MCSM0{ 0x18 };
const byte CC1101_FOCCFG{ 0x19 };
const byte CC1101_BSCFG{ 0x1A };
const byte CC1101_AGCCTRL2{ 0x1B };
const byte CC1101_AGCCTRL1{ 0x1C };
const byte CC1101_AGCCTRL0{ 0x1D };
const byte CC1101_WOREVT1{ 0x1E };
const byte CC1101_WOREVT0{ 0x1F };
const byte CC1101_WORCTRL{ 0x20 };
const byte CC1101_FREND1{ 0x21 };
const byte CC1101_FREND0{ 0x22 };
const byte CC1101_FSCAL3{ 0x23 };
const byte CC1101_FSCAL2{ 0x24 };
const byte CC1101_FSCAL1{ 0x25 };
const byte CC1101_FSCAL0{ 0x26 };
const byte CC1101_RCCTRL1{ 0x27 };
const byte CC1101_RCCTRL0{ 0x28 };
const byte CC1101_FSTEST{ 0x29 };
const byte CC1101_PTEST{ 0x2A };
const byte CC1101_AGCTEST{ 0x2B };
const byte CC1101_TEST2{ 0x2C };
const byte CC1101_TEST1{ 0x2D };
const byte CC1101_TEST0{ 0x2E };

byte rx_fifo[0x100] = {};
uint8_t rx_fifo_begin{ 0 };
uint8_t rx_fifo_end{ 0 };

void PinSetup()
{
    // --------------------------------------------------
    // �� ����� arduino
    // SPI
    //      PIN_SPI_SS (10) - ����� �������� ����������
    pinMode(PIN_SPI_SS, OUTPUT);
    digitalWrite(PIN_SPI_SS, HIGH);
    //      PIN_SPI_MOSI (11) - ����� �������� ������ �� �������� � ������� �����������
    pinMode(PIN_SPI_MOSI, OUTPUT);
    //      PIN_SPI_MISO (12) - ����� �������� �� �������� � �������� ����������
    pinMode(PIN_SPI_MISO, INPUT);
    //      PIN_SPI_SCK (13) - �������� �������� �������������, ������������ ������� ����������� 
    pinMode(PIN_SPI_SCK, OUTPUT);
    // �������������� � cc1101
    //      PIN_CC_GDO0 - ��������������� ����� ��1101
    pinMode(PIN_CC_GDO0, INPUT);
    //      PIN_CC_GDO2 - ��������������� ����� ��1101
    pinMode(PIN_CC_GDO2, INPUT);

    // --------------------------------------------------
    // �� ����� cc1101
    //      SI (Slave Input)
    //      SCK (Serial ClocK) - �������� �������� �������������
    //      SO (Slave Output)
    //      GD2 - ��������������� ����� ����
    //      GD0 - ��������������� ����� ����, ��������, ��������� ����� �������� ������
    //      CSN - ����� ����������
    // ������������ (���������� �� ������� �������)
    //      VCC VCC
    //      SI  SCK
    //      SO  GD2
    //      CSN GD0
    //      GND GND

    // --------------------------------------------------
    // �����������
    //      arduino             cc1101
    //      VCC (3.3v)          VCC (3.3v)
    //      GND                 GND
    //      PIN_SPI_SS (10)     CSN
    //      PIN_SPI_MOSI (11)   SI
    //      PIN_SPI_MISO (12)   SO
    //      PIN_SPI_SCK (13)    SCK
    //      PIN_CC_GDO0 (3)     GD0
    //      PIN_CC_GDO2 (2)     GD2
}

struct registerSetting_t
{
    byte address;
    byte data;
};

// Address Config = No address check 
// Base Frequency = 433.899994 
// CRC Autoflush = false 
// CRC Enable = false 
// Carrier Frequency = 433.899994 
// Channel Number = 0 
// Channel Spacing = 25.390625 
// Data Format = Normal mode 
// Data Rate = 3.60775 
// Deviation = 2.380371 
// Device Address = 0 
// Manchester Enable = false 
// Modulation Format = ASK/OOK 
// PA Ramping = false 
// Packet Length = 32 
// Packet Length Mode = Fixed packet length mode. Length configured in PKTLEN register 
// Preamble Count = 4 
// RX Filter BW = 81.250000 
// Sync Word Qualifier Mode = 15/16 sync word bits detected 
// TX Power = 0 
// Whitening = false 

static const registerSetting_t preferredSettings[] =
{
  {CC1101_IOCFG2,      0x01},
  {CC1101_IOCFG0,      0x00},
  {CC1101_FIFOTHR,     0x4F},
  {CC1101_SYNC1,       0xFF},
  {CC1101_SYNC0,       0xC0},
  {CC1101_PKTLEN,      0x20},
  {CC1101_PKTCTRL1,    0x00},
  {CC1101_PKTCTRL0,    0x00},
  {CC1101_FSCTRL1,     0x06},
  {CC1101_FREQ2,       0x10},
  {CC1101_FREQ1,       0xB0},
  {CC1101_FREQ0,       0x3F},
  {CC1101_MDMCFG4,     0xD7},
  {CC1101_MDMCFG3,     0x22},
  {CC1101_MDMCFG2,     0xB1},
  {CC1101_MDMCFG1,     0x20},
  {CC1101_MDMCFG0,     0x00},
  {CC1101_DEVIATN,     0x04},
  {CC1101_MCSM1,       0x00},
  {CC1101_MCSM0,       0x18},
  {CC1101_FOCCFG,      0x16},
  {CC1101_AGCCTRL2,    0x07},
  {CC1101_AGCCTRL1,    0x00},
  {CC1101_WORCTRL,     0xFB},
  {CC1101_FREND1,      0xB6},
  {CC1101_FREND0,      0x11},
  {CC1101_FSCAL3,      0xE9},
  {CC1101_FSCAL2,      0x2A},
  {CC1101_FSCAL1,      0x00},
  {CC1101_FSCAL0,      0x1F},
  {CC1101_TEST2,       0x81},
  {CC1101_TEST1,       0x35},
  {CC1101_TEST0,       0x09},
};

//static const registerSetting_t preferredSettings[] =
//{
//  {CC1101_IOCFG2,      0x0D},
//  {CC1101_IOCFG0,      0x40},
//  {CC1101_FIFOTHR,     0x47},
//  {CC1101_PKTCTRL0,    0x32},
//  {CC1101_FSCTRL1,     0x06},
//  {CC1101_FREQ2,       0x10},
//  {CC1101_FREQ1,       0xB0},
//  {CC1101_FREQ0,       0x3F},
//  {CC1101_MDMCFG4,     0x87},
//  {CC1101_MDMCFG3,     0x32},
//  {CC1101_MDMCFG2,     0xB8},
//  {CC1101_MDMCFG1,     0x20},
//  {CC1101_MDMCFG0,     0x00},
//  {CC1101_DEVIATN,     0x04},
//  {CC1101_MCSM0,       0x18},
//  {CC1101_FOCCFG,      0x16},
//  {CC1101_AGCCTRL2,    0x07},
//  {CC1101_AGCCTRL1,    0x00},
//  {CC1101_WORCTRL,     0xFB},
//  {CC1101_FREND1,      0xB6},
//  {CC1101_FREND0,      0x11},
//  {CC1101_FSCAL3,      0xE9},
//  {CC1101_FSCAL2,      0x2A},
//  {CC1101_FSCAL1,      0x00},
//  {CC1101_FSCAL0,      0x1F},
//  {CC1101_TEST2,       0x81},
//  {CC1101_TEST1,       0x35},
//  {CC1101_TEST0,       0x09},
//};

void CC1101BeginTransaction()
{
    SPI.beginTransaction(SPISettings());
    digitalWrite(PIN_SPI_SS, LOW);
    // ���� ����������
    // 10.1 Chip Status Byte When the header byte, data byte, or command strobe is sent on the SPI interface, the chip status byte is sent by the CC1101 on the SO pin. 
    // The status byte contains key status signals, useful for the MCU. The first bit, s7, is the CHIP_RDYn signal and this signal must go low before the first positive edge of SCLK. 
    // The CHIP_RDYn signal indicates that the crystal is running. Bits 6, 5, and 4 comprise the STATE value.
    while (digitalRead(PIN_SPI_MISO) == HIGH);
    delay(2);
}

void CC1101EndTransaction()
{
    digitalWrite(PIN_SPI_SS, HIGH);
    SPI.endTransaction();
    delay(2);
}

void CC1101WriteByte(const byte& address, const byte& data)
{
    CC1101BeginTransaction();
    SPI.transfer(address | 0x00); // +0x00 - write single byte
    SPI.transfer(data);
    CC1101EndTransaction();
}

byte CC1101ReadByte(const byte& address)
{
    CC1101BeginTransaction();
    SPI.transfer(address | 0xC0); // +0xC0 - read burst
    byte result{ SPI.transfer(0) };
    CC1101EndTransaction();
    return result;
}

void CC1101ReadBytes(const byte& address, const byte& bytes, byte fifo[], byte& fifo_end)
{
    CC1101BeginTransaction();
    SPI.transfer(address | 0xC0); // +0xC0 - read burst
    for (byte i{ 0 }; i < bytes; i++)
        rx_fifo[rx_fifo_end++] = SPI.transfer(0);
    CC1101EndTransaction();
}

void CC1101Read()
{
    // ���-�� ���� � RX ������
    byte bytes{ CC1101ReadByte(0x3B) };
    Serial.print("rx_fifo[");
    Serial.print(bytes);
    Serial.print("] ");
    
	CC1101ReadBytes(0x3F, bytes, &rx_fifo[0], rx_fifo_end);
	while (rx_fifo_begin != rx_fifo_end)
		Serial.print(rx_fifo[rx_fifo_begin++], BIN);

	Serial.println();

    // �������� ����� RX FIFO.
    CC1101CommandStrobe(SFRX);

    // Enable RX.Perform calibration first if coming from IDLEand MCSM0.FS_AUTOCAL = 1.
    CC1101CommandStrobe(SRX);
}

byte CC1101CommandStrobe(const byte& command)
{
    CC1101BeginTransaction();
    byte result{ SPI.transfer(command) };
    CC1101EndTransaction();
    return result;
}

void CC1101SetupReceiver()
{
    // Reset chip.
    CC1101CommandStrobe(SRES);

    // �������� ����� RX FIFO.
    CC1101CommandStrobe(SFRX);

    // �������� ����� TX FIFO.
    CC1101CommandStrobe(SFTX);

    // �������������� ��������
    uint8_t cc1101_config_size{ sizeof(preferredSettings) / sizeof(preferredSettings[0]) };
    for (int i = 0; i < cc1101_config_size; i++)
        CC1101WriteByte(preferredSettings[i].address, preferredSettings[i].data);

    // ������ ����� ��������
    CC1101BeginTransaction();
    // 0x7E: Burst access to PATABLE
    SPI.transfer(0x3E | 0x40);
    // "0" - ��� �������
    SPI.transfer(0x00);
    // "1" - ������ ����
    SPI.transfer(0x60);
    CC1101EndTransaction();

    // ��������� ����������
    attachInterrupt(0, CC1101Read, RISING);

	// Enable RX.Perform calibration first if coming from IDLEand MCSM0.FS_AUTOCAL = 1.
    CC1101CommandStrobe(SRX);
}

void setup() 
{
    Serial.begin(57600);
    PinSetup();
    CC1101SetupReceiver();
}

//������� ��������� CC1101
void CC1101StatusPrint()
{
    Serial.println();

    byte status{ CC1101CommandStrobe(SNOP) };

    //Serial.print("fifo_byte_avaliable: ");
    //byte fifo_byte_avaliable{ status & B00001111 };
    //Serial.println(fifo_byte_avaliable);

    Serial.print("state: ");
    byte state{ (status & B01110000) >> 4 };
    switch (state)
    {
    case B000:
        Serial.println("IDLE");
        break;

    case B001:
        Serial.println("RX");
        break;

    case B010:
        Serial.println("TX");
        break;

    case B011:
        Serial.println("FSTXON");
        break;

    case B100:
        Serial.println("CALIBRATE");
        break;

    case B101:
        Serial.println("SETTLING");
        break;

    case B110:
        Serial.println("RXFIFO_OVERFLOW");
        break;

    case B111:
        Serial.println("TXFIFO_UNDERFLOW");
        break;
    }

    // ���-�� ���� � RX ������
    Serial.print("RX FIFO[");
    Serial.print(CC1101ReadByte(0x3B));
    Serial.println("]");
}

void loop() 
{

    // ������� ���������
    //CC1101StatusPrint();

    delay(1000);
}
