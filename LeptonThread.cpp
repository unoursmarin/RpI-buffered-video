#include "LeptonThread.h"

#include "Palettes.h"
#include "SPI.h"
#include "Lepton_I2C.h"
#include <QLabel>
#include <sstream>
#include <string>

#define PACKET_SIZE 164
#define PACKET_SIZE_UINT16 (PACKET_SIZE/2)
#define PACKETS_PER_FRAME 60
#define FRAME_SIZE_UINT16 (PACKET_SIZE_UINT16*PACKETS_PER_FRAME)
#define FPS 30;

LeptonThread::LeptonThread() : QThread(), colorMap(colormap_ironblack)
{
}

LeptonThread::~LeptonThread() {
}

void LeptonThread::run()
{
	long timeInSecondToRecord=15;
	long fps = (long)FPS;
	long screenCount=0;
	long numberFrameToSave=(long)fps * (long)timeInSecondToRecord;
	system("exec rm -r /home/pi/Documents/LeptonModule-master/software/raspberrypi_video/screenshot/*");
	//create the initial image
	myImage = QImage(80, 60, QImage::Format_RGB888);

	//open spi port
	SpiOpenPort(0);

	while(true) {

		//read data packets from lepton over SPI
		int resets = 0;
		for(int j=0;j<PACKETS_PER_FRAME;j++) {
			//if it's a drop packet, reset j to 0, set to -1 so he'll be at 0 again loop
			read(spi_cs0_fd, result+sizeof(uint8_t)*PACKET_SIZE*j, sizeof(uint8_t)*PACKET_SIZE);
			int packetNumber = result[j*PACKET_SIZE+1];
			if(packetNumber != j) {
				j = -1;
				resets += 1;
				usleep(1000);
				if(resets == 750) {
					SpiClosePort(0);
					usleep(750000);
					SpiOpenPort(0);
				}
			}
		}
		if(resets >= 30) {
			qDebug() << "done reading, resets: " << resets;
		}

		frameBuffer = (uint16_t *)result;
		int row, column;
		uint16_t value;
		uint16_t minValue = 65535;
		uint16_t maxValue = 0;

		
		for(int i=0;i<FRAME_SIZE_UINT16;i++) {
			//skip the first 2 uint16_t's of every packet, they're 4 header bytes
			if(i % PACKET_SIZE_UINT16 < 2) {
				continue;
			}
			
			//flip the MSB and LSB at the last second
			int temp = result[i*2];
			result[i*2] = result[i*2+1];
			result[i*2+1] = temp;
			
			value = frameBuffer[i];
			if(value > maxValue) {
				maxValue = value;
			}
			if(value < minValue) {
				minValue = value;
			}
			column = i % PACKET_SIZE_UINT16 - 2;
			row = i / PACKET_SIZE_UINT16 ;
		}

		float diff = maxValue - minValue;
		float scale = 255/diff;
		QRgb color;
		for(int i=0;i<FRAME_SIZE_UINT16;i++) {
			if(i % PACKET_SIZE_UINT16 < 2) {
				continue;
			}
			value = (frameBuffer[i] - minValue) * scale;
            color = qRgb(this->colorMap[3*value], this->colorMap[3*value+1], this->colorMap[3*value+2]);
			column = (i % PACKET_SIZE_UINT16 ) - 2;
			row = i / PACKET_SIZE_UINT16;
			myImage.setPixel(column, row, color);
		}
		std::stringstream ss;
		ss << "/home/pi/Documents/LeptonModule-master/software/raspberrypi_video/screenshot/" << screenCount << ".png";
		std::string s = ss.str();
		myImage.save(s.c_str());
		if(screenCount>=numberFrameToSave){
			std::stringstream ssRemove;
			ssRemove << "exec rm /home/pi/Documents/LeptonModule-master/software/raspberrypi_video/screenshot/" << screenCount-numberFrameToSave << ".png";
			std::string sRemove = ssRemove.str();
			system(sRemove.c_str());
		}
		screenCount+=1;

		//lets emit the signal for update
		emit updateImage(myImage);

	}
	
	//finally, close SPI port just bcuz
	SpiClosePort(0);
}
void LeptonThread::snapImage() {

    QImage bigger = myImage.scaled(6 * myImage.size().width(), 6 * myImage.size().height());
    bigger.save("/home/pi/Pictures/snapFlirLepton/snap.png");
    QLabel *popup = new QLabel();
    QPixmap pixmap = QPixmap::fromImage(bigger);
    popup->setPixmap(pixmap);
    popup->show();
}

void LeptonThread::performFFC() {
	//perform FFC
	lepton_perform_ffc();
}
void LeptonThread::rainMap(){
    this->colorMap = colormap_rainbow;
}
void LeptonThread::greyMap() {
    this->colorMap = colormap_grayscale;

}
void LeptonThread::ironMap() {
    this->colorMap = colormap_ironblack;
}
