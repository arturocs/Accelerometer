/* 10/16/2018 Copyright Tlera Corporation
 *
 *  Created by Kris Winer
 *
 *  The BMA400 is an inexpensive (~$2), three-axis, medium-resolution (12-bit), ultra-low power (800 nA low power mode) accelerometer
 *  in a tiny 2 mm x 2 mm LGA12 package with 1024-byte FIFO,
 *  two multifunction interrupts and widely configurable sample rate (15 - 800 Hz), full range (2 - 16 g), low power modes,
 *  and interrupt detection behaviors. This accelerometer is nice choice for motion-based wake/sleep,
 *  tap detection, step counting, and simple orientation estimation.
 *
 *  Library may be used freely and without limit with attribution.
 *
 */
#include "BMA400.h"

void delay(uint32_t ms) {
	UDELAY_Delay(ms * 1000);
}

s_BMA400 BMA400(uint8_t intPin1, uint8_t intPin2) {
	s_BMA400 result;
	result._intPin1 = intPin1;
	result._intPin2 = intPin2;
	return result;
}

uint8_t getChipID() {
	uint8_t c = readByte(BMA400_ADDRESS, BMA400_CHIPID);
	return c;
}

uint8_t getStatus() {
	uint8_t c = readByte(BMA400_ADDRESS, BMA400_INT_STAT_0);
	return c;
}

float getAres(s_BMA400 *conf, uint8_t Ascale) {
	switch (Ascale) {
	// Possible accelerometer scales (and their register bit settings) are:
	// 2 Gs , 4 Gs , 8 Gs , and 16 Gs .
	case AFS_2G:
		conf->_aRes = 2.0f / 2048.0f; // per data sheet
		return conf->_aRes;
		break;
	case AFS_4G:
		conf->_aRes = 4.0f / 2048.0f;
		return conf->_aRes;
		break;
	case AFS_8G:
		conf->_aRes = 8.0f / 2048.0f;
		return conf->_aRes;
		break;
	case AFS_16G:
		conf->_aRes = 16.0f / 2048.0f;
		return conf->_aRes;
		break;
	}
}

void initBMA400(uint8_t Ascale, uint8_t SR, uint8_t power_Mode, uint8_t OSR,
		uint8_t acc_filter) {



	/* Normal mode configuration */
	// set bandwidth to 0.2x sample rate (bit 7), OSR in low-power mode, power mode
	writeByte(BMA400_ADDRESS, BMA400_ACC_CONFIG0, 0x80 | OSR << 5 | power_Mode);
	delay(2);                                                     // wait 1.5 ms
	writeByte(BMA400_ADDRESS, BMA400_ACC_CONFIG1, Ascale << 6 | OSR << 4 | SR); // set full-scale range, oversampling and sample rate
	writeByte(BMA400_ADDRESS, BMA400_ACC_CONFIG2, acc_filter << 2); // set accel filter

	/* Low power mode configuration */
	// writeByte(BMA400_ADDRESS,BMA400_AUTOLOWPOW_0, 0xFA);             // set auto low power time out to 10 second
	// writeByte(BMA400_ADDRESS,BMA400_AUTOLOWPOW_1, 0x04);             // switch to low power after time out (max 10.2 sec)
	writeByte(BMA400_ADDRESS, BMA400_AUTOLOWPOW_1, 0x02); // switch to low power when GEN1 interrupt triggered

	writeByte(BMA400_ADDRESS, BMA400_AUTOWAKEUP_1, 0x02); // enable interrupt for auto wake up
	// enable interrupt on all three axes (bits 5 - 7)for auto wake up
	// use 4 samples (n + 1) for wakeup condition (bits 4 - 2), 4 samples = 160 ms at 25 Hz
	// reference updated whenever going into low power mode (bits 1, 0)
	writeByte(BMA400_ADDRESS, BMA400_WKUP_INT_CONFIG0,
			0x07 << 5 | 0x03 << 2 | 0x01);
	writeByte(BMA400_ADDRESS, BMA400_WKUP_INT_CONFIG1, 0x08); //  50 mg threshold for wake on any axis, n x 6.25 mgs at 2 g FS

	/* Interrupt configuration */
	// Map interrupts and set interrupt behavior
	writeByte(BMA400_ADDRESS, BMA400_INT1_MAP, 0x01); // map WKUP interrupt to INT1 (bit 0)
	writeByte(BMA400_ADDRESS, BMA400_INT2_MAP, 0x04); // map GEN1 interrupt to INT2 (bit 2)
	writeByte(BMA400_ADDRESS, BMA400_INT12_IO_CTRL, 0x20 | 0x02); // set both interrupts push-pull, active HIGH

	// Config GEN1 on INT2
	writeByte(BMA400_ADDRESS, BMA400_GEN1INT_CONFIG0, 0xFA); // all axes, fixed 100 Hz, reference every time, hysteresis 48 mg
	writeByte(BMA400_ADDRESS, BMA400_GEN1INT_CONFIG1, 0x01); // inactive motion interrupt configuration
	writeByte(BMA400_ADDRESS, BMA400_GEN1INT_CONFIG2, 0x03); // set threshold 24 mg (8 mg per count)
	writeByte(BMA400_ADDRESS, BMA400_GEN1INT_CONFIG31, 0x40); // measured over 64 data samples

	/* Enable interrupts */
	writeByte(BMA400_ADDRESS, BMA400_INT_CONFIG0, 0x04); // enable GEN1 interrupt (bit 2)
}

void activateNoMotionInterrupt() {
	writeByte(BMA400_ADDRESS, BMA400_INT_CONFIG0, 0x04); // enable GEN1 (no_Motion) interrupt
}

void deactivateNoMotionInterrupt() {
	writeByte(BMA400_ADDRESS, BMA400_INT_CONFIG0, 0x00); // disable GEN1 (no_Motion) interrupt
}

void CompensationBMA400(s_BMA400 *conf, uint8_t Ascale, uint8_t SR,
		uint8_t power_Mode, uint8_t OSR, uint8_t acc_filter, float *offset) {
	printf("hold flat and motionless for bias calibration\n");
	delay(5000);

	writeByte(BMA400_ADDRESS, BMA400_ACC_CONFIG0, 0x80 | OSR << 5 | power_Mode); // set bandwidth to 0.2x sample rate, OSR in low-power mode, power mode
	delay(2);
	writeByte(BMA400_ADDRESS, BMA400_ACC_CONFIG1, Ascale << 6 | OSR << 4 | SR); // set full-scale range, oversampling and sample rate
	writeByte(BMA400_ADDRESS, BMA400_ACC_CONFIG2, acc_filter << 2); // set accel filter

	int16_t temp[3] = { 0, 0, 0 };
	int32_t sum[3] = { 0, 0, 0 };

	for (uint8_t ii = 0; ii < 128; ii++) {
		readBMA400AccelData(temp);
		sum[0] += temp[0];
		sum[1] += temp[1];
		sum[2] += temp[2];
		delay(100);
	}

	offset[0] = (float)(sum[0]) / 128.0f;
	offset[1] = (float)(sum[1]) / 128.0f;
	offset[2] = (float)(sum[2]) / 128.0f;
	offset[0] *= conf->_aRes;
	offset[1] *= conf->_aRes;
	offset[2] *= conf->_aRes;
	if (offset[2] > +0.5f)
		offset[2] = offset[2] - 1.0f;
	if (offset[2] < -0.5f)
		offset[2] = offset[2] + 1.0f;

	printf("x-axis offset = %f mg", offset[0] * 1000.0f);
	printf("y-axis offset = %f mg", offset[1] * 1000.0f);
	printf("z-axis offset = %f mg", offset[2] * 1000.0f);
	/* end of accel calibration */
}

void resetBMA400() {
	writeByte(BMA400_ADDRESS, BMA400_CMD, 0xB6); // software reset the BMA400
}

void selfTestBMA400() {
	int16_t temp[3] = { 0, 0, 0 };
	int16_t posX, posY, posZ, negX, negY, negZ;

	// initialize sensor for self test
	writeByte(BMA400_ADDRESS, BMA400_ACC_CONFIG0,
			0x80 | osr3 << 5 | normal_Mode); // set bandwidth to 0.2x sample rate, OSR in low-power mode, power mode
	delay(2);
	writeByte(BMA400_ADDRESS, BMA400_ACC_CONFIG1,
	AFS_4G << 6 | osr3 << 4 | SR_100Hz); // set full-scale range to +/- 4 g
	writeByte(BMA400_ADDRESS, BMA400_ACC_CONFIG2, acc_filt1 << 2); // set accel filter
	float STres = 4000.0f / 2048.0f;                // mg/LSB for 4 g full scale
	delay(100);

	// positive axes test
	writeByte(BMA400_ADDRESS, BMA400_SELF_TEST, 0x00 | 0x07); // positive axes
	delay(100);
	readBMA400AccelData(temp);
	posX = temp[0];
	posY = temp[1];
	posZ = temp[2];

	// negative axes test
	writeByte(BMA400_ADDRESS, BMA400_SELF_TEST, 0x08 | 0x07); // negative axes
	delay(100);
	readBMA400AccelData(temp);
	negX = temp[0];
	negY = temp[1];
	negZ = temp[2];

	printf("x-axis self test = ");
	printf("%f", (float) (posX - negX) * STres);
	printf("mg, should be > 2000 mg\n");
	printf("y-axis self test = \n");
	printf("%f", (float) (posY - negY) * STres);
	printf("mg, should be > 1800 mg\n");
	printf("z-axis self test = ");
	printf("%f", (float) (posZ - negZ) * STres);
	printf("mg, should be > 800 mg\n");

	writeByte(BMA400_ADDRESS, BMA400_SELF_TEST, 0x00); // disable self test
	/* end of self test*/
}

void readBMA400AccelData(int16_t *destination) {
	uint8_t rawData[6];                 // x/y/z accel register data stored here
	readBytes(BMA400_ADDRESS, BMA400_ACCD_X_LSB, 6, &rawData[0]); // Read the 6 raw data registers into data array
	destination[0] = ((rawData[1] & 0x0F) << 8) | rawData[0]; // Turn the MSB and LSB into a signed 12-bit value
	destination[1] = ((rawData[3] & 0x0F) << 8) | rawData[2];
	destination[2] = ((rawData[5] & 0x0F) << 8) | rawData[4];
	if (destination[0] > 2047)
		destination[0] += -4096;
	if (destination[1] > 2047)
		destination[1] += -4096;
	if (destination[2] > 2047)
		destination[2] += -4096;
}

int16_t readBMA400TempData() {
	uint8_t temp = readByte(BMA400_ADDRESS, BMA400_TEMP_DATA); // Read the raw data register
	int16_t tmp = (int16_t) (((int16_t) temp << 8) | 0x00) >> 8; // Turn into signed 8-bit temperature value
	return tmp;
}

// I2C scan function
void I2Cscan() {
	// scan for i2c devices
	uint8_t error, address;
	int nDevices;

	printf("Scanning...\n");

	nDevices = 0;
	for (address = 1; address < 127; address++) {
		// The i2c_scanner uses the return value of
		// the Write.endTransmission to see if
		// a device did acknowledge to the address.
		//Wire.beginTransmission(address);
		//error = Wire.endTransmission();

		if (error == 0) {
			printf("I2C device found at address 0x");
			if (address < 16)
				printf("0");
			printf("%X", address);
			printf("  !\n");

			nDevices++;
		} else if (error == 4) {
			printf("Unknown error at address 0x");
			if (address < 16)
				printf("0");
			printf("%X\n", address);
		}
	}
	if (nDevices == 0)
		printf("No I2C devices found\n\n");
	else
		printf("done\n\n");
}

// I2C read/write functions for the BMA400 sensor

void writeByte(uint8_t address, uint8_t subAddress, uint8_t data) {
	//Wire.beginTransmission(address); // Initialize the Tx buffer
	//Wire.write(subAddress);          // Put slave register address in Tx buffer
	//Wire.write(data);                // Put data in Tx buffer
	//Wire.endTransmission();          // Send the Tx buffer
	I2C_TransferReturn_TypeDef sta;
	int i=0;
	do{
		sta = i2cTransferByte(I2C0, address, subAddress, &data, I2C_FLAG_WRITE_WRITE);
		i++;
		delay(1000);
	}while(sta != i2cTransferDone && i<500);
	printf("write sta: %d",sta);

}

uint8_t readByte(uint8_t address, uint8_t subAddress) {
uint8_t data = 0; // `data` will store the register data
//Wire.beginTransmission(address); // Initialize the Tx buffer
//Wire.write(subAddress);          // Put slave register address in Tx buffer
//Wire.endTransmission(false);     // Send the Tx buffer, but send a restart to keep connection alive
//Wire.requestFrom(address, 1);    // Read one byte from slave register address
//data = Wire.read();              // Fill Rx buffer with result
I2C_TransferReturn_TypeDef sta;
int i=0;
while (i<500) {
	sta = i2cTransferByte(I2C0, address, subAddress, &data,
	I2C_FLAG_WRITE_READ);
	if (sta == i2cTransferDone) {
		return data;
	}
	i++;
	delay(1000);
}
printf("read sta: %d",sta);
return data; // Return data read from slave register
}

void readBytes(uint8_t address, uint8_t subAddress, uint8_t count,
	uint8_t *dest) {
//Wire.beginTransmission(address); // Initialize the Tx buffer
//Wire.write(subAddress);          // Put slave register address in Tx buffer
//Wire.endTransmission(false);     // Send the Tx buffer, but send a restart to keep connection alive
for (uint8_t i = 0; i < count; i++) {
	dest[i] = readByte(address, subAddress);
}
//Wire.requestFrom(address, count); // Read bytes from slave register address
//while (Wire.available())
//{
//dest[i++] = Wire.read();
//} // Put read results in the Rx buffer
};

I2C_TransferReturn_TypeDef i2cTransferByte(I2C_TypeDef *i2c, uint16_t addr,
	uint8_t command, uint8_t *val, uint8_t flag) {
I2C_TransferSeq_TypeDef seq;
I2C_TransferReturn_TypeDef sta;
uint8_t i2c_write_data[1];
uint8_t i2c_read_data[1];

seq.addr = addr;
seq.flags = flag;
/* Select command to issue */
i2c_write_data[0] = command;
seq.buf[0].data = i2c_write_data;
seq.buf[0].len = 1;
/* Select location/length of data to be read */
seq.buf[1].data = i2c_read_data;
seq.buf[1].len = 1;

sta = I2CSPM_Transfer(i2c, &seq);
if (sta != i2cTransferDone) {
	return sta;
}
if (NULL != val) {
	*val = i2c_read_data[0];
}
return sta;
}
