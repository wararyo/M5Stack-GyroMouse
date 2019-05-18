// BLE Libraries.
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <BLEHIDDevice.h>
#include <Arduino.h>
#include <esp_log.h>

#define DEVICE_NAME "M5StackMouse"
#define MANUFACTURER "wararyo"

static char LOG_TAG[] = "BLEMouse";
 
// HID input report data.
typedef struct {
	uint8_t buttons;
    int8_t x;
    int8_t y;
    int8_t wheel;
} mouse_report_t;
 
// HID report desc (mouse).
const uint8_t reportMap[54] = { /* USB report descriptor, size must match usbconfig.h */
    0x05, 0x01,                    // USAGE_PAGE (Generic Desktop)
    0x09, 0x02,                    // USAGE (Mouse)
    0xa1, 0x01,                    // COLLECTION (Application)
    0x85, 0x01,                     //  REPORT_ID (1)
    0x09, 0x01,                    //   USAGE (Pointer)
    0xA1, 0x00,                    //   COLLECTION (Physical)
    0x05, 0x09,                    //     USAGE_PAGE (Button)
    0x19, 0x01,                    //     USAGE_MINIMUM
    0x29, 0x03,                    //     USAGE_MAXIMUM
    0x15, 0x00,                    //     LOGICAL_MINIMUM (0)
    0x25, 0x01,                    //     LOGICAL_MAXIMUM (1)
    0x95, 0x03,                    //     REPORT_COUNT (3)
    0x75, 0x01,                    //     REPORT_SIZE (1)
    0x81, 0x02,                    //     INPUT (Data,Var,Abs)
    0x95, 0x01,                    //     REPORT_COUNT (1)
    0x75, 0x05,                    //     REPORT_SIZE (5)
    0x81, 0x03,                    //     INPUT (Const,Var,Abs)
    0x05, 0x01,                    //     USAGE_PAGE (Generic Desktop)
    0x09, 0x30,                    //     USAGE (X)
    0x09, 0x31,                    //     USAGE (Y)
    0x09, 0x38,                    //     USAGE (Wheel)
    0x15, 0x81,                    //     LOGICAL_MINIMUM (-127)
    0x25, 0x7F,                    //     LOGICAL_MAXIMUM (127)
    0x75, 0x08,                    //     REPORT_SIZE (8)
    0x95, 0x03,                    //     REPORT_COUNT (3)
    0x81, 0x06,                    //     INPUT (Data,Var,Rel)
    0xC0,                          //   END_COLLECTION
    0xC0,                          // END COLLECTION
};
 
class BLEMouse;

extern BLEMouse Mouse;

class MySecurity : public BLESecurityCallbacks {
    uint32_t passKey = 0;

    uint32_t onPassKeyRequest(){
        ESP_LOGE(LOG_TAG, "The passkey request %d", passKey);

        vTaskDelay(25000);
        return passKey;
    }
    void onPassKeyNotify(uint32_t pass_key){
        ESP_LOGE(LOG_TAG, "The passkey Notify number:%d", pass_key);
        passKey = pass_key;
    }
    bool onSecurityRequest(){
        return true;
    }
    void onAuthenticationComplete(esp_ble_auth_cmpl_t auth_cmpl){
        if(auth_cmpl.success){
            ESP_LOGI(LOG_TAG, "remote BD_ADDR:");
            esp_log_buffer_hex(LOG_TAG, auth_cmpl.bd_addr, sizeof(auth_cmpl.bd_addr));
            ESP_LOGI(LOG_TAG, "address type = %d", auth_cmpl.addr_type);
        }
        ESP_LOGI(LOG_TAG, "pair status = %s", auth_cmpl.success ? "success" : "fail");
    }
    bool onConfirmPIN(uint32_t pin) {return true;};
};

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

	BLEHIDDevice* hid;
	BLEServer* pServer;
    BLECharacteristic* input;
    BLECharacteristic* output;
    mouse_report_t report = {0,0,0};


public:
	bool isConnected = false;
	void begin() {
		BLEDevice::init(DEVICE_NAME);
        BLEDevice::setEncryptionLevel(ESP_BLE_SEC_ENCRYPT_NO_MITM);
        BLEDevice::setSecurityCallbacks(new MySecurity());

		pServer = BLEDevice::createServer();
		pServer->setCallbacks(new ServerCallbacks());

	 	hid = new BLEHIDDevice(pServer);
		input = hid->inputReport(1);
        output = hid->outputReport(1);

		hid->manufacturer()->setValue(MANUFACTURER);
		hid->pnp(0x02, 0xe502, 0xa111, 0x0210);
		hid->hidInfo(0x00,0x00);  // country == 0, flags == 1 ( providing wake-up signal to a HID host)
		hid->reportMap((uint8_t*)reportMap, sizeof(reportMap));
		hid->startServices();
        
		BLEAdvertising *pAdvertising = pServer->getAdvertising();
		pAdvertising->setAppearance(HID_MOUSE);
		pAdvertising->addServiceUUID(hid->hidService()->getUUID());

        BLESecurity *pSecurity = new BLESecurity();
        pSecurity->setAuthenticationMode(ESP_LE_AUTH_REQ_SC_BOND);
        pSecurity->setCapability(ESP_IO_CAP_NONE);
        pSecurity->setInitEncryptionKey(ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK);

		pAdvertising->start();
		Serial.println("Start Advertising.");
        vTaskDelay(3000/portTICK_PERIOD_MS);
	}
 
	void sendReport(uint8_t buttons, int8_t x, int8_t y) {
		if (isConnected) {
            report.buttons = buttons;
            report.x = x;
            report.y = y;
            report.wheel = 0;
			input->setValue((uint8_t*)&report, sizeof(mouse_report_t));
			input->notify();
		    vTaskDelay(20/portTICK_PERIOD_MS);
		}
	}
};