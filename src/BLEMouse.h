// BLE Libraries.
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <BLEHIDDevice.h>
#include <Arduino.h>

#define DEVICE_NAME "M5StackMouse"
#define MANUFACTURER "wararyo"
 
// HID input report data.
typedef struct {
	uint8_t buttons;
    uint8_t x;
    uint8_t y;
} mouse_report_t;
 
// HID report desc (mouse).
const uint8_t reportMap[] = {
    USAGE_PAGE(1), 			0x01,
    USAGE(1), 				0x02,
    COLLECTION(1),			0x01,
    USAGE(1),				0x01,
    COLLECTION(1),			0x00,
    USAGE_PAGE(1),			0x09,
    USAGE_MINIMUM(1),		0x1,
    USAGE_MAXIMUM(1),		0x3,
    LOGICAL_MINIMUM(1),	    0x0,
    LOGICAL_MAXIMUM(1),	    0x1,
    REPORT_COUNT(1),		0x3,
    REPORT_SIZE(1),		    0x1,
    HIDINPUT(1), 			0x2,		// (Data, Variable, Absolute), ;3 button bits
    REPORT_COUNT(1),		0x1,
    REPORT_SIZE(1),		    0x5,
    HIDINPUT(1), 			0x1,		//(Constant), ;5 bit padding
    USAGE_PAGE(1), 	        0x1,		//(Generic Desktop),
    USAGE(1),				0x30,
    USAGE(1),				0x31,
    LOGICAL_MINIMUM(1),	    0x81,
    LOGICAL_MAXIMUM(1),	    0x7f,
    REPORT_SIZE(1),		    0x8,
    REPORT_COUNT(1),		0x2,
    HIDINPUT(1), 				0x6,		//(Data, Variable, Relative), ;2 position bytes (X & Y)
    END_COLLECTION(0),
    END_COLLECTION(0)
};
 
class BLEMouse;

extern BLEMouse Mouse;

class BLEMouse {
  class ServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
        Mouse.isConnected = true;
        Serial.println("ServerCallbacks::onConnect()");
    };
  
    void onDisconnect(BLEServer* pServer) {
        Mouse.isConnected = false;
        Serial.println("ServerCallbacks::onDisconnect()");
    }
  };

	bool isConnected = false;
	BLEHIDDevice* hid;
	BLEServer* pServer;
    BLECharacteristic* input;

public:
	void begin() {
		BLEDevice::init(DEVICE_NAME);
		pServer = BLEDevice::createServer();
		pServer->setCallbacks(new ServerCallbacks());
	//	BLEDevice::setMTU(23);
	 	hid = new BLEHIDDevice(pServer);
		input = hid->inputReport(1);
        input->setAccessPermissions(ESP_GATT_PERM_READ_ENCRYPTED | ESP_GATT_PERM_WRITE_ENCRYPTED);
        
		hid->manufacturer()->setValue(MANUFACTURER);
		hid->pnp(0x02, 0xe502, 0xa111, 0x0210);
		hid->hidInfo(0x00,0x01);  // country == 0, flags == 1 ( providing wake-up signal to a HID host)
		hid->reportMap((uint8_t*)reportMap, sizeof(reportMap));
		hid->startServices();
		BLEAdvertising *pAdvertising = pServer->getAdvertising();
		pAdvertising->setAppearance(HID_KEYBOARD);
		pAdvertising->addServiceUUID(hid->hidService()->getUUID());

        BLESecurity *pSecurity = new BLESecurity();
        pSecurity->setAuthenticationMode(ESP_LE_AUTH_REQ_SC_BOND);
        pSecurity->setCapability(ESP_IO_CAP_NONE);
        pSecurity->setInitEncryptionKey(ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK);

		pAdvertising->start();
		Serial.println("Start Advertising.");
	}
 
	void sendReport(uint8_t buttons, uint8_t x, uint8_t y) {
		if (isConnected) {
            mouse_report_t report = {buttons,x,y};
			input->setValue((uint8_t*)&report, sizeof(mouse_report_t));
			input->notify();
		}
	}
};