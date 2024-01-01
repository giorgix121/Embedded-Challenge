#include "I3G4250D.h" // Include the header file for the I3G4250D gyroscope sensor

// Define constants for SPI communication
#define READ_CMD 0x80       // Read command bit
#define MULTI_BYTE_CMD 0x40 // Multi-byte read command bit
#define SPI_DELAY 1ms       // SPI communication delay

// Read a single register from the gyroscope
uint16_t ReadRegister(SPI &spi, DigitalOut &CS, uint16_t address)
{
    CS.write(0);                      // Activate the chip select
    spi.write(address | READ_CMD);    // Send the read command with the register address
    uint16_t value = spi.write(0x00); // Read the register value
    ThisThread::sleep_for(SPI_DELAY); // Short delay for stability
    CS.write(1);                      // Deactivate the chip select
    return value;                     // Return the read value
}

// Write to a register on the gyroscope
void WriteRegister(SPI &spi, DigitalOut &CS, uint16_t address, uint16_t config)
{
    CS.write(0);                      // Activate the chip select
    spi.write(address);               // Send the register address
    spi.write(config);                // Send the configuration data to the register
    ThisThread::sleep_for(SPI_DELAY); // Short delay for stability
    CS.write(1);                      // Deactivate the chip select
}

// Read two consecutive registers and combine their values
uint16_t ReadTwoRegister(SPI &spi, DigitalOut &CS, uint16_t base_address)
{
    int lower = 0;
    int upper = 0;

    // Read the lower byte
    CS.write(0);                                         // Activate the chip select
    spi.write(base_address | READ_CMD | MULTI_BYTE_CMD); // Send read command for lower byte
    ThisThread::sleep_for(SPI_DELAY);                    // Short delay
    lower = spi.write(0x00);                             // Read lower byte
    CS.write(1);                                         // Deactivate the chip select

    // Read the upper byte
    CS.write(0);                                               // Activate the chip select
    spi.write((base_address + 1) | READ_CMD | MULTI_BYTE_CMD); // Send read command for upper byte
    ThisThread::sleep_for(SPI_DELAY);                          // Short delay
    upper = spi.write(0x00);                                   // Read upper byte
    CS.write(1);                                               // Deactivate the chip select

    return (upper << 8) | lower; // Combine the two bytes and return
}

// Read X, Y, and Z-axis data from the gyroscope
void ReadXYZ(SPI &spi, DigitalOut &CS, float *xyz)
{
    int16_t x, y, z;

    // Read raw data from each axis
    x = ReadTwoRegister(spi, CS, I3G4250D_OUT_X_L_ADDR); // Read X-axis data
    y = ReadTwoRegister(spi, CS, I3G4250D_OUT_Y_L_ADDR); // Read Y-axis data
    z = ReadTwoRegister(spi, CS, I3G4250D_OUT_Z_L_ADDR); // Read Z-axis data

    // Convert raw data to angular velocity in degrees per second
    // The sensitivity factor and conversion to radians are applied here
    xyz[0] = (x - X_base) * I3G4250D_SENSITIVITY_500DPS * 0.017453292519943295769236907684886f / 1000.0f;
    xyz[1] = (y - Y_base) * I3G4250D_SENSITIVITY_500DPS * 0.017453292519943295769236907684886f / 1000.0f;
    xyz[2] = (z - Z_base) * I3G4250D_SENSITIVITY_500DPS * 0.017453292519943295769236907684886f / 1000.0f;
}

// Initialize the gyroscope with specified settings
int Init(SPI &spi, DigitalOut &CS)
{
    int id;
    id = ReadRegister(spi, CS, I3G4250D_WHO_AM_I_ADDR); // Read the device ID

    // Write configuration settings to various control registers
    WriteRegister(spi, CS, I3G4250D_CTRL_REG1_ADDR,
                  I3G4250D_OUTPUT_DATARATE_1 | I3G4250D_BANDWIDTH_4 | I3G4250D_MODE_ACTIVE |
                      I3G4250D_X_ENABLE | I3G4250D_Y_ENABLE | I3G4250D_Z_ENABLE);                     // Configure CTRL_REG1: Data rate, bandwidth, mode, and axis enable
    WriteRegister(spi, CS, I3G4250D_CTRL_REG2_ADDR, I3G4250D_HPFCF_0 | I3G4250D_HPM_NORMAL_MODE_RES); // Configure CTRL_REG2: High-pass filter settings
    WriteRegister(spi, CS, I3G4250D_CTRL_REG4_ADDR, I3G4250D_BLE_LSB | I3G4250D_FULLSCALE_500);       // Configure CTRL_REG4: Data format and full scale
    WriteRegister(spi, CS, I3G4250D_CTRL_REG5_ADDR, I3G4250D_HIGHPASSFILTER_ENABLE);                  // Configure CTRL_REG5: Enable high-pass filter

    return id; // Return the device ID
}