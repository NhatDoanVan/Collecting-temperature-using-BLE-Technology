/*
   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
/*
Nội Dung của code 

* Khởi tạo Profile 
+ Trước tiên nó khởi tạo BLE khi khởi tạo chương trình
+ Khởi tạo các tham số Application Pròfile 
+  Application Profile được lưu trữ vào gl_profile_tab, Khi khởi tạo Application Profile bao gồm việc định nghĩa hàm callback
+ Khi mà application Profile được đăng kí, ngăn xếp BLE sẽ trả về 1 giao diện GATT 
+ Việc đanwg kí proffile sẽ trigger 1 cờ ESP_GATTC_REG_EVT, nó được xử lý bở hàm esp_gattc_cb(). Hàm này sẽ lấy giao diện GATT và lưu trữ nó vào trong profile table

* Cài đặt thông số quét 
+ GATT client thường quét các Server lân cận và thử kết nối với chúng. Tuy nhiên, để thực hiện quét, trước tiên cần phải thiết lập các thông số cấu hình. 
Điều này được thực hiện sau khi đăng ký Application Profile, bởi vì đăng ký, sau khi hoàn tất, sẽ kích hoạt sự kiện ESP_GATTC_REG_EVT.Lần đầu tiên sự kiện này được kích hoạt,
trình xử lý sự kiện GATT sẽ nắm bắt nó và gán giao diện GATT cho Cấu hình A, sau đó sự kiện được chuyển tiếp tới trình xử lý sự kiện GATT của Cấu hình A. 
Một trong trình xử lý sự kiện này, sự kiện được sử dụng để gọi esp_ble_gap_set_scan_params(), lấy một thể hiện cấu trúc ble_scan_params làm tham số. Cấu trúc này được định nghĩa là:
static esp_ble_scan_params_t ble_scan_params

*Start Scanning:
+ Khi các tham số quét được đặt, một sự kiện ESP_GAP_BLE_SCAN_PARAM_SET_COMPLETE_EVT được kích hoạt, sự kiện này được xử lý bởi trình xử lý sự kiện GAP esp_gap_cb(). 
Sự kiện này được sử dụng để bắt đầu quét các máy chủ GATT lân cận:
+ Quá trình quét được bắt đầu bằng cách sử dụng hàm esp_ble_gap_start_scanning() nhận tham số biểu thị thời lượng quét liên tục (tính bằng giây).
Sau khi thời gian quét kết thúc, sự kiện ESP_GAP_SEARCH_INQ_CMPL_EVT sẽ được kích hoạt.

*Getting Scan Results
Kết quả quét được hiển thị ngay khi chúng đến với sự kiện ESP_GAP_BLE_SCAN_RESULT_EVT, bao gồm các tham số sau:
  struct ble_scan_result_evt_param {
        esp_gap_search_evt_t search_evt;            !< Search event type 
        esp_bd_addr_t bda;                          !< Bluetooth device address which has been searched 
        esp_bt_dev_type_t dev_type;                 !< Device type 
        esp_ble_addr_type_t ble_addr_type;          !< Ble device address type 
        esp_ble_evt_type_t ble_evt_type;            !< Ble scan result event type 
        int rssi;                                   !< Searched device's RSSI 
        uint8_t  ble_adv[ESP_BLE_ADV_DATA_LEN_MAX + ESP_BLE_SCAN_RSP_DATA_LEN_MAX]; !< Received EIR 
        int flag;                                   !< Advertising data flag bit 
        int num_resps;                              !< Scan result number 
        uint8_t adv_data_len;                       !< Adv data length 
        uint8_t scan_rsp_len;                       !< Scan response length 
    } scan_rst;                                     !< Event parameter of ESP_GAP_BLE_SCAN_RESULT_EVT 
typedef enum {
    ESP_GAP_SEARCH_INQ_RES_EVT             = 0,      !< Inquiry result for a peer device. 
    ESP_GAP_SEARCH_INQ_CMPL_EVT            = 1,      !< Inquiry complete. 
    ESP_GAP_SEARCH_DISC_RES_EVT            = 2,      !< Discovery result for a peer device. 
    ESP_GAP_SEARCH_DISC_BLE_RES_EVT        = 3,      !< Discovery result for BLE GATT based service on a peer device. 
    ESP_GAP_SEARCH_DISC_CMPL_EVT           = 4,      !< Discovery complete. 
    ESP_GAP_SEARCH_DI_DISC_CMPL_EVT        = 5,      !< Discovery complete. 
    ESP_GAP_SEARCH_SEARCH_CANCEL_CMPL_EVT  = 6,      !< Search cancelled 
// } esp_gap_search_evt_t

Chúng tôi quan tâm đến sự kiện ESP_GAP_SEARCH_INQ_RES_EVT, sự kiện này được gọi mỗi khi tìm thấy một thiết bị mới. 
Chúng tôi cũng quan tâm đến ESP_GAP_SEARCH_INQ_CMPL_EVT, được kích hoạt khi thời gian quét hoàn tất và có thể được sử dụng để khởi động lại quy trình quét

Tên thiết bị được phân giải và so sánh với tên được xác định trong remote_device_name. Nếu nó bằng với tên thiết bị của Máy chủ GATT mà chúng tôi quan tâm, thì quá trình quét sẽ dừng lại.

*Connecting to A GATT Server
+Mỗi khi chúng tôi nhận được kết quả từ sự kiện ESP_GAP_SEARCH_INQ_RES_EVT, trước tiên mã sẽ in địa chỉ của thiết bị từ xa.
+Sau đó, client sẽ in độ dài dữ liệu được quảng cáo và độ dài phản hồi quét: ESP_LOGI(GATTC_TAG, "searched Adv Data Len %d, Scan Response Len %d", scan_result->scan_rst.adv_data_len, scan_result->scan_rst.scan_rsp_len);
+Để lấy tên thiết bị, chúng tôi sử dụng hàm esp_ble_resolve_adv_data(), lấy dữ liệu quảng cáo được lưu trữ trong scan_result->scan_rst.ble_adv, loại dữ liệu quảng cáo và độ dài, để trích xuất giá trị từ khung gói quảng cáo .
+Sau đó, tên thiết bị được in.Cuối cùng, nếu tên thiết bị từ xa giống như chúng ta đã xác định ở trên, thiết bị cục bộ sẽ dừng quét và cố gắng mở kết nối với thiết bị từ xa bằng hàm esp_ble_gattc_open(). 
Hàm này lấy các tham số là giao diện GATT của application profile, địa chỉ server từ xa và một giá trị boolean (Isconnecting). Giá trị boolean được sử dụng để cho biết liệu kết nối được thực hiện trực tiếp 
hay được thực hiện trong nền (kết nối tự động), tại thời điểm này, giá trị boolean này phải được đặt thành true để thiết lập kết nối. Lưu ý rằng client mở một kết nối ảo đến server. Kết nối ảo trả về ID kết nối.
Kết nối ảo là kết nối giữa application profile và server từ xa. Vì nhiều application profile có thể chạy trên một ESP32 nên có thể có nhiều kết nối ảo được mở tới cùng một máy chủ từ xa. 
Ngoài ra còn có kết nối vật lý là liên kết BLE thực tế giữa client và server. Vì thế, nếu kết nối vật lý bị ngắt kết nối bằng hàm esp_ble_gap_disconnect(), thì tất cả các kết nối ảo khác cũng bị đóng.

*Configuring the MTU Size
ATT_MTU được định nghĩa là kích thước tối đa của bất kỳ gói nào được gửi giữa máy khách và máy chủ. Khi client kết nối với server, nó sẽ thông báo cho server kích thước MTU
sẽ sử dụng bằng cách trao đổi các đơn vị dữ liệu giao thức Yêu cầu và Phản hồi MTU (PDU). Điều này được thực hiện sau khi mở kết nối. Sau khi mở kết nối, một sự kiện ESP_GATTC_CONNECT_EVT được kích hoạt:
case ESP_GATTC_CONNECT_EVT:
        //p_data->connect.status always be ESP_GATT_OK
        ESP_LOGI(GATTC_TAG, "ESP_GATTC_CONNECT_EVT conn_id %d, if %d, status %d", conn_id, gattc_if, p_data->connect.status);
        conn_id = p_data->connect.conn_id;
        gl_profile_tab[PROFILE_A_APP_ID].conn_id = p_data->connect.conn_id;
        memcpy(gl_profile_tab[PROFILE_A_APP_ID].remote_bda, p_data->connect.remote_bda, sizeof(esp_bd_addr_t));
        ESP_LOGI(GATTC_TAG, "REMOTE BDA:");
        esp_log_buffer_hex(GATTC_TAG, gl_profile_tab[PROFILE_A_APP_ID].remote_bda, sizeof(esp_bd_addr_t));
        esp_err_t mtu_ret = esp_ble_gattc_send_mtu_req (gattc_if, conn_id);
        if (mtu_ret){
            ESP_LOGE(GATTC_TAG, "config MTU error, error code = %x", mtu_ret);
        }
        break;
Kích thước MTU điển hình cho kết nối Bluetooth 4.0 là 23 byte. Máy khách có thể thay đổi kích thước của MTU bằng cách sử dụng hàm esp_ble_gattc_send_mtu_req(), hàm này nhận giao diện GATT và ID kết nối. 
Kích thước của MTU được yêu cầu được xác định bởi esp_ble_gatt_set_local_mtu(). Sau đó, máy chủ có thể chấp nhận hoặc từ chối yêu cầu. ESP32 hỗ trợ kích thước MTU lên tới 517 byte, 
được xác định bởi ESP_GATT_MAX_MTU_SIZE trong esp_gattc_api.h. Trong ví dụ này, kích thước MTU được đặt thành 500 byte. Trong trường hợp cấu hình không thành công, lỗi trả về sẽ được in:
esp_err_t mtu_ret = esp_ble_gattc_send_mtu_req (gattc_if, conn_id);
if (mtu_ret){
	ESP_LOGE(GATTC_TAG, "config MTU error, error code = %x", mtu_ret);
}
break;

*Discovering Services
Sự kiện cấu hình MTU cũng được sử dụng để bắt đầu khám phá các service có sẵn trong server mà client vừa kết nối. Để khám phá các service, hàm esp_ble_gattc_search_service() được sử dụng. 
Các tham số của chức năng là giao diện GATT, ID kết nối application profile và UUID của ứng dụng dịch vụ mà client quan tâm. servvice chúng tôi đang tìm kiếm được định nghĩa là:
+Trong trường hợp client tìm thấy service mà nó đang tìm kiếm, cờ get_server được đặt thành true và giá trị bộ điều khiển bắt đầu và giá trị bộ điều khiển kết thúc, sẽ được sử dụng sau này để có được tất cả các characteristic của service đó, sẽ được lưu.
 Sau khi tất cả các kết quả service được trả về, quá trình tìm kiếm đã hoàn tất và sự kiện ESP_GATTC_SEARCH_CMPL_EVT được kích hoạt

 *Getting Characteristics
 Ví dụ này triển khai nhận dữ liệu characteristic từ một service được xác định trước. Service mà chúng tôi muốn có các characteristic có UUID là 0x00FF và đặc điểm chúng tôi quan tâm có UUID là 0xFF01:

esp_ble_gattc_get_attr_count() nhận số lượng thuộc tính với dịch vụ hoặc đặc điểm nhất định trong bộ đệm gattc. Các tham số của hàm esp_ble_gattc_get_attr_count() là giao diện GATT, ID kết nối, loại thuộc tính được xác định trong esp_gatt_db_attr_type_t, bộ điều khiển bắt đầu thuộc tính, bộ điều khiển kết thúc thuộc tính,
xử lý đặc tính (tham số này chỉ hợp lệ khi loại được đặt thành ESP_GATT_DB_DESCRIPTOR.) và xuất số lượng thuộc tính đã được tìm thấy trong bộ đệm gattc với loại thuộc tính đã cho. Sau đó, chúng tôi cấp phát một bộ đệm để lưu thông tin ký tự cho hàm esp_ble_gattc_get_char_by_uuid().
Hàm tìm đặc tính với UUID đặc trưng đã cho trong bộ đệm gattc. Nó chỉ lấy đặc tính từ bộ đệm cục bộ, thay vì các thiết bị từ xa. Trong một máy chủ, có thể có nhiều ký tự chia sẻ cùng một UUID. Tuy nhiên, trong bản trình diễn gatt_server của chúng tôi,
mỗi ký tự có một UUID duy nhất và đó là lý do tại sao chúng tôi chỉ sử dụng ký tự đầu tiên trong char_elem_result, là con trỏ tới đặc tính của dịch vụ. Count ban đầu lưu trữ số lượng các đặc điểm mà khách hàng muốn tìm,
và sẽ được cập nhật với số đặc điểm thực sự được tìm thấy trong bộ đệm gattc với esp_ble_gattc_get_char_by_uuid.

*Registering for Notifications
+Client có thể đăng ký nhận notify từ client mỗi khi giá trị characteristic thay đổi. Trong ví dụ này, chúng tôi muốn đăng ký notify về characteristic được xác định bằng UUID là 0xff01. 
Sau khi có đầy đủ các characteristic, ​​ta kiểm tra các thuộc tính của characteristic nhận được, sau đó sử dụng chức năng esp_ble_gattc_register_for_notify() để đăng ký notify. Các đối số chức năng là giao diện GATT, 
địa chỉ của server từ xa và tay cầm mà chúng tôi muốn đăng ký thông báo.
*/



/****************************************************************************
*
* This file is for gatt client. It can scan ble device, connect multiple devices,
* The gattc_multi_connect demo can connect three ble slaves at the same time.
* Modify the name of gatt_server demo named ESP_GATTS_DEMO_a,
* ESP_GATTS_DEMO_b and ESP_GATTS_DEMO_c,then run three demos,the gattc_multi_connect demo will connect
* the three gatt_server demos, and then exchange data.
* Of course you can also modify the code to connect more devices, we default to connect
* up to 4 devices, more than 4 you need to modify menuconfig.
*
****************************************************************************/

#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <stddef.h>
#include "nvs.h"
#include "nvs_flash.h"

#include "esp_netif.h"
#include "esp_bt.h"
#include "esp_gap_ble_api.h"
#include "esp_gattc_api.h"
#include "esp_gatt_defs.h"
#include "esp_bt_main.h"
#include "esp_gatt_common_api.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "mqtt_client.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "protocol_examples_common.h"
#include "freertos/event_groups.h"
#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"
#define GATTC_TAG "GATTC_MULTIPLE_DEMO"
#define REMOTE_SERVICE_UUID        0x00FF///// id của service (16 bit). Nếu trùng id service của server nó sẽ kết nối lấy characteristic
#define REMOTE_NOTIFY_CHAR_UUID    0xFF01 /////////////////////////////Nếu ID characteristic giống với ID được xác định bởi REMOTE_NOTIFY_CHAR_UUID, thì client sẽ đăng ký nhận NOTIFY về giá trị characteristic đó.

/* register three profiles, each profile corresponds to one connection,
   which makes it easy to handle each connection event */
#define PROFILE_NUM 3
#define PROFILE_A_APP_ID 0
#define PROFILE_B_APP_ID 1
#define PROFILE_C_APP_ID 2
#define INVALID_HANDLE   0



/* Declare static functions */
static void esp_gap_cb(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param);
static void esp_gattc_cb(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if, esp_ble_gattc_cb_param_t *param);
static void gattc_profile_a_event_handler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if, esp_ble_gattc_cb_param_t *param);
static void gattc_profile_b_event_handler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if, esp_ble_gattc_cb_param_t *param);
static void gattc_profile_c_event_handler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if, esp_ble_gattc_cb_param_t *param);

static esp_bt_uuid_t remote_filter_service_uuid = {
    .len = ESP_UUID_LEN_16,
    .uuid = {.uuid16 = REMOTE_SERVICE_UUID,},
};

static esp_bt_uuid_t remote_filter_char_uuid = {
    .len = ESP_UUID_LEN_16,
    .uuid = {.uuid16 = REMOTE_NOTIFY_CHAR_UUID,},
};

static esp_bt_uuid_t notify_descr_uuid = {
    .len = ESP_UUID_LEN_16,
    .uuid = {.uuid16 = ESP_GATT_UUID_CHAR_CLIENT_CONFIG,},
};
static char * mystring_a;
static char * mystring_b;
static float a;
static float b;
static char REQUEST_A[100];
static char REQUEST_B[100];
static bool conn_device_a   = false;
static bool conn_device_b   = false;
static bool conn_device_c   = false;

static bool get_service_a   = false;
static bool get_service_b   = false;
static bool get_service_c   = false;

static bool Isconnecting    = false;
static bool stop_scan_done  = false;

static esp_gattc_char_elem_t  *char_elem_result_a   = NULL;
static esp_gattc_descr_elem_t *descr_elem_result_a  = NULL;
static esp_gattc_char_elem_t  *char_elem_result_b   = NULL;
static esp_gattc_descr_elem_t *descr_elem_result_b  = NULL;
static esp_gattc_char_elem_t  *char_elem_result_c   = NULL;
static esp_gattc_descr_elem_t *descr_elem_result_c  = NULL;
static esp_gatt_if_t globalIf_A;
static esp_gatt_if_t globalIf_B;
static const char remote_device_name[3][20] = {"ESP_GATTS_DEMO_a", "ESP_GATTS_DEMO_b", "ESP_GATTS_DEMO_c"};///// Đây là tên thiết bị muốn tìm để kết nối, nếu mà quét được tên mà khác 3 tên trên thì nó sẽ không kết nối

static esp_ble_scan_params_t ble_scan_params = {
    .scan_type              = BLE_SCAN_TYPE_ACTIVE,// Kiểu scan
    .own_addr_type          = BLE_ADDR_TYPE_PUBLIC,// Kiểu địa chỉ 
    .scan_filter_policy     = BLE_SCAN_FILTER_ALLOW_ALL,
    .scan_interval          = 0x50,// là khoảng thời gian từ khi Bộ điều khiển bắt đầu lần quét LE cuối cùng cho đến khi nó bắt đầu lần quét LE tiếp theo ( Khoảng thời gian giữa 2 lần quét) 0.625 x 0xzz = ...(ms)
    .scan_window            = 0x30,// Thời lượng quét LE.
    .scan_duplicate         = BLE_SCAN_DUPLICATE_DISABLE
};

struct gattc_profile_inst {
    esp_gattc_cb_t gattc_cb;//  GATT client callback function
    uint16_t gattc_if; //GATT client interface number for this profile
    uint16_t app_id;  // Application Profile ID number
    uint16_t conn_id;// Connection ID
    uint16_t service_start_handle;// Service start handle
    uint16_t service_end_handle;// Service end handle
    uint16_t char_handle;
    esp_bd_addr_t remote_bda; // Remote device address connected to this client.
};

/* One gatt-based profile one app_id and one gattc_if, this array will store the gattc_if returned by ESP_GATTS_REG_EVT */
static struct gattc_profile_inst gl_profile_tab[PROFILE_NUM] = {
    [PROFILE_A_APP_ID] = {
        .gattc_cb = gattc_profile_a_event_handler,// Định nghĩa callback
        .gattc_if = ESP_GATT_IF_NONE,       /* Not get the gatt_if, so initial is ESP_GATT_IF_NONE */
    },
    [PROFILE_B_APP_ID] = {
        .gattc_cb = gattc_profile_b_event_handler,
        .gattc_if = ESP_GATT_IF_NONE,       /* Not get the gatt_if, so initial is ESP_GATT_IF_NONE */
    },
    [PROFILE_C_APP_ID] = {
        .gattc_cb = gattc_profile_c_event_handler,
        .gattc_if = ESP_GATT_IF_NONE,       /* Not get the gatt_if, so initial is ESP_GATT_IF_NONE */
    },

};
////// Sau khi đăng kí xong các Profile nó sẽ trigger sự kiện ESP_GATTC_REG_EVT, sự kiện này được xử lý bởi esp_gattc_cb() event handler
static const char *TAG = "MQTT_EXAMPLE";
 static esp_mqtt_client_handle_t A;
 static esp_mqtt_client_handle_t B;

//////

/////
static esp_err_t mqtt_event_handler_cbA(esp_mqtt_event_handle_t event)
{
    esp_mqtt_client_handle_t clientA = event->client;
    
    // your_context_t *context = event->context;
    switch (event->event_id) {
        case MQTT_EVENT_CONNECTED:
            A=  clientA;
            ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
          
          
            esp_mqtt_client_subscribe(clientA, "v1/devices/me/attributes", 1);//// Dăng kí chia sẻ thuộc tính thời gian lấy mẫu đẻ nhận dữ liệu từ ThingBoard
            break;
        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
            break;

        case MQTT_EVENT_SUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
           
            break;
        case MQTT_EVENT_UNSUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_PUBLISHED:
            ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED_A, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_DATA:
            ESP_LOGI(TAG, "MQTT_EVENT_DATA");
            printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
            printf("DATA=%.*s\r\n", event->data_len, event->data);
            char *number_A;
            char * a1;
            a1 = event->data;
            number_A=strtok(a1,"{\"Sampling period\":}");
            printf("%s\n", number_A);
            esp_ble_gattc_write_char( globalIf_A,/////Hàm gửi thời gian lấy mẫu xuống node cb
                                  gl_profile_tab[PROFILE_A_APP_ID].conn_id,
                                  gl_profile_tab[PROFILE_A_APP_ID].char_handle,
                                  sizeof((uint8_t *)number_A),
                                  (uint8_t *)number_A,
                                  ESP_GATT_WRITE_TYPE_RSP,
                                  ESP_GATT_AUTH_REQ_NONE);
            break;        
        default:
            ESP_LOGI(TAG, "Other event id:%d", event->event_id);
            break;
    }
    return ESP_OK;
}

static void mqtt_event_handlerA(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
    ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%d", base, event_id);
    mqtt_event_handler_cbA(event_data);
}
//////Tương tự mqtt_event_handler_cba cho node A
static esp_err_t mqtt_event_handler_cbB(esp_mqtt_event_handle_t event)
{
    esp_mqtt_client_handle_t clientB = event->client;
    
    // your_context_t *context = event->context;
    switch (event->event_id) {
        case MQTT_EVENT_CONNECTED:
           B= clientB;
            ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
            //msg_id = esp_mqtt_client_publish(clientB, "esp/telemetry", "{\"Tempature\":41}", 0, 1, 0);
           // ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
          
        
            esp_mqtt_client_subscribe(clientB, "v1/devices/me/attributes", 1);
            break;
        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
            break;

        case MQTT_EVENT_SUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
           // msg_id = esp_mqtt_client_publish(client, "/topic/qos0", "data", 0, 0, 0);
           // ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
            break;
        case MQTT_EVENT_UNSUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_PUBLISHED:
            ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED_B, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_DATA:
            ESP_LOGI(TAG, "MQTT_EVENT_DATA");
            printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
            printf("DATA=%.*s\r\n", event->data_len, event->data);
            char *number_B;
            char * b1;
            b1 = event->data;
            number_B=strtok(b1,"{\"Sampling period\":}");
            printf("%s\n", number_B);
            esp_ble_gattc_write_char( globalIf_B,
                                  gl_profile_tab[PROFILE_B_APP_ID].conn_id,
                                  gl_profile_tab[PROFILE_B_APP_ID].char_handle,
                                  sizeof((uint8_t *)number_B),
                                  (uint8_t *)number_B,
                                  ESP_GATT_WRITE_TYPE_RSP,
                                  ESP_GATT_AUTH_REQ_NONE);
            break;        
        default:
            ESP_LOGI(TAG, "Other event id:%d", event->event_id);
            break;
    }
    return ESP_OK;
}

static void mqtt_event_handlerB(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
    ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%d", base, event_id);
    mqtt_event_handler_cbB(event_data);
}

///////////


static void mqtt_app_start(void)
{
    esp_mqtt_client_config_t mqtt_cfgA = {
        .uri = "mqtt://thingsboard.cloud/api/v1/$ACCESS_TOKEN",
        //.port= 1883,
        .username="81ePMqAQxbO8WQYVZJZo",//Khai báo đối tượng A
       // .password=""
    };
    esp_mqtt_client_config_t mqtt_cfgB = {
        .uri = "mqtt://thingsboard.cloud/api/v1/$ACCESS_TOKEN",
        //.port= 1883,
        .username="Cmx0sRhoRvyaRhlaeCHH",//Khai báo đối tượng A
       // .password=""
    };

    esp_mqtt_client_handle_t clientA = esp_mqtt_client_init(&mqtt_cfgA);
    esp_mqtt_client_handle_t clientB = esp_mqtt_client_init(&mqtt_cfgB);
    esp_mqtt_client_register_event(clientA, ESP_EVENT_ANY_ID, mqtt_event_handlerA, clientA);
    esp_mqtt_client_register_event(clientB, ESP_EVENT_ANY_ID, mqtt_event_handlerB, clientB);
    esp_mqtt_client_start(clientA);
     esp_mqtt_client_start(clientB);
}






/////
static void start_scan(void)
{
    stop_scan_done = false;
    Isconnecting = false;
    uint32_t duration = 10;
    esp_ble_gap_start_scanning(duration);
}
static void gattc_profile_a_event_handler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if, esp_ble_gattc_cb_param_t *param)
{

    esp_ble_gattc_cb_param_t *p_data = (esp_ble_gattc_cb_param_t *)param;

    switch (event) {
    case ESP_GATTC_REG_EVT:////////////////////////////////////Các giá trị quét được đặt bằng hàm esp_ble_gap_set_scan_params():
        ESP_LOGI(GATTC_TAG, "REG_EVT");
        esp_err_t scan_ret = esp_ble_gap_set_scan_params(&ble_scan_params);
        if (scan_ret){
            ESP_LOGE(GATTC_TAG, "set scan params error, error code = %x", scan_ret);
        }
        break;
    /* one device connect successfully, all profiles callback function will get the ESP_GATTC_CONNECT_EVT,
     so must compare the mac address to check which device is connected, so it is a good choice to use ESP_GATTC_OPEN_EVT. */
    case ESP_GATTC_CONNECT_EVT:/* one device connect successfully, all profiles callback function will get the ESP_GATTC_CONNECT_EVT,
     so must compare the mac address to check which device is connected, so it is a good choice to use ESP_GATTC_OPEN_EVT. */
        break;
    case ESP_GATTC_OPEN_EVT://////////////////////// Cấu hình nhận nội dung dữ liệu
        if (p_data->open.status != ESP_GATT_OK){
            //open failed, ignore the first device, connect the second device
            ESP_LOGE(GATTC_TAG, "connect device failed, status %d", p_data->open.status);
            conn_device_a = false;
           
            //start_scan();
            break;
        }
        memcpy(gl_profile_tab[PROFILE_A_APP_ID].remote_bda, p_data->open.remote_bda, 6);
        gl_profile_tab[PROFILE_A_APP_ID].conn_id = p_data->open.conn_id;////////////////////////////////////////////////////Nếu kết nối thành công, client sẽ lưu ID kết nối, in thông tin thiết bị từ xa và định cấu hình kích thước MTU thành 200 byte.
        ESP_LOGI(GATTC_TAG, "ESP_GATTC_OPEN_EVT conn_id %d, if %d, status %d, mtu %d", p_data->open.conn_id, gattc_if, p_data->open.status, p_data->open.mtu);
        ESP_LOGI(GATTC_TAG, "REMOTE BDA:");
        esp_log_buffer_hex(GATTC_TAG, p_data->open.remote_bda, sizeof(esp_bd_addr_t));
        esp_err_t mtu_ret = esp_ble_gattc_send_mtu_req (gattc_if, p_data->open.conn_id);
        if (mtu_ret){
            ESP_LOGE(GATTC_TAG, "config MTU error, error code = %x", mtu_ret);
        }
        break;
    case ESP_GATTC_CFG_MTU_EVT://////////////////////////////////////////////Sau khi định cấu hình kích thước MTU, một ESP_GATTC_CFG_MTU_EVT được tạo. Sự kiện này được sử dụng để tìm kiếm các service đã biết hiện có trên thiết bị từ xa
        if (param->cfg_mtu.status != ESP_GATT_OK){
            ESP_LOGE(GATTC_TAG,"Config mtu failed");
        }
        ESP_LOGI(GATTC_TAG, "Status %d, MTU %d, conn_id %d", param->cfg_mtu.status, param->cfg_mtu.mtu, param->cfg_mtu.conn_id);
        esp_ble_gattc_search_service(gattc_if, param->cfg_mtu.conn_id, &remote_filter_service_uuid);/////////////////////////// Tìm kiếm service
        break;
    case ESP_GATTC_SEARCH_RES_EVT: {////////////////////////////////////////////////////////Nếu service được tìm thấy, sự kiện ESP_GATTC_SEARCH_RES_EVT được kích hoạt cho phép đặt cờ get_service_1 thành true.
    /////////////////////////////////////////////////////////// Cờ này được sử dụng để in thông tin và sau đó lấy characteristic mà client quan tâm.
        ESP_LOGI(GATTC_TAG, "SEARCH RES: conn_id = %x is primary service %d", p_data->search_res.conn_id, p_data->search_res.is_primary);
        ESP_LOGI(GATTC_TAG, "start handle %d end handle %d current handle value %d", p_data->search_res.start_handle, p_data->search_res.end_handle, p_data->search_res.srvc_id.inst_id);
        if (p_data->search_res.srvc_id.uuid.len == ESP_UUID_LEN_16 && p_data->search_res.srvc_id.uuid.uuid.uuid16 == REMOTE_SERVICE_UUID) {
            ESP_LOGI(GATTC_TAG, "UUID16: %x", p_data->search_res.srvc_id.uuid.uuid.uuid16);
            get_service_a = true;
            gl_profile_tab[PROFILE_A_APP_ID].service_start_handle = p_data->search_res.start_handle;
            gl_profile_tab[PROFILE_A_APP_ID].service_end_handle = p_data->search_res.end_handle;
        }
        break;
    }
    case ESP_GATTC_SEARCH_CMPL_EVT://///////////////////////Khi quá trình tìm kiếm tất cả các SERVICES hoàn tất, một sự kiện ESP_GATTC_SEARCH_CMPL_EVT được tạo ra, 
    //////////sự kiện này được sử dụng để nhận các characteristic của SERVICES  vừa được phát hiện. Điều này được thực hiện với hàm esp_ble_gattc_get_attr_count :
        if (p_data->search_cmpl.status != ESP_GATT_OK){
            ESP_LOGE(GATTC_TAG, "search service failed, error status = %x", p_data->search_cmpl.status);
            break;
        }
        if (get_service_a){
            uint16_t count = 0;
            esp_gatt_status_t status = esp_ble_gattc_get_attr_count( gattc_if,///////////////////////////////////////  Nhận số lượng characteristic 
                                                                     p_data->search_cmpl.conn_id,
                                                                     ESP_GATT_DB_CHARACTERISTIC,
                                                                     gl_profile_tab[PROFILE_A_APP_ID].service_start_handle,
                                                                     gl_profile_tab[PROFILE_A_APP_ID].service_end_handle,
                                                                     INVALID_HANDLE,
                                                                     &count);
            if (status != ESP_GATT_OK){
                ESP_LOGE(GATTC_TAG, "esp_ble_gattc_get_attr_count error");
            }
            if (count > 0) {
                char_elem_result_a = (esp_gattc_char_elem_t *)malloc(sizeof(esp_gattc_char_elem_t) * count);
                if (!char_elem_result_a){
                    ESP_LOGE(GATTC_TAG, "gattc no mem");
                }else {
                    status = esp_ble_gattc_get_char_by_uuid( gattc_if,///////////////////////////////////////////Sự kiện này được sử dụng để in thông tin về đặc tính.
                                                             p_data->search_cmpl.conn_id,
                                                             gl_profile_tab[PROFILE_A_APP_ID].service_start_handle,
                                                             gl_profile_tab[PROFILE_A_APP_ID].service_end_handle,
                                                             remote_filter_char_uuid,
                                                             char_elem_result_a,
                                                             &count);
                    if (status != ESP_GATT_OK){
                        ESP_LOGE(GATTC_TAG, "esp_ble_gattc_get_char_by_uuid error");
                    }

                    /*  Every service have only one char in our 'ESP_GATTS_DEMO' demo, so we used first 'char_elem_result' */
                    if (count > 0 && (char_elem_result_a[0].properties & ESP_GATT_CHAR_PROP_BIT_NOTIFY)){
                        gl_profile_tab[PROFILE_A_APP_ID].char_handle = char_elem_result_a[0].char_handle;
                        esp_ble_gattc_register_for_notify (gattc_if, gl_profile_tab[PROFILE_A_APP_ID].remote_bda, char_elem_result_a[0].char_handle);/////////// đăng ký notify
                    }
                }
                /* free char_elem_result */
                free(char_elem_result_a);
            }else {
                ESP_LOGE(GATTC_TAG, "no char found");
            }
        }
        break;
    case ESP_GATTC_REG_FOR_NOTIFY_EVT: { ///////////////////////////được sử dụng để ghi vào Bộ mô tả cấu hình client của server:
        if (p_data->reg_for_notify.status != ESP_GATT_OK){
            ESP_LOGE(GATTC_TAG, "reg notify failed, error status =%x", p_data->reg_for_notify.status);
            break;
        }
        uint16_t count = 0;
        uint16_t notify_en = 1;
        esp_gatt_status_t ret_status = esp_ble_gattc_get_attr_count( gattc_if,
                                                                     gl_profile_tab[PROFILE_A_APP_ID].conn_id,
                                                                     ESP_GATT_DB_DESCRIPTOR,
                                                                     gl_profile_tab[PROFILE_A_APP_ID].service_start_handle,
                                                                     gl_profile_tab[PROFILE_A_APP_ID].service_end_handle,
                                                                     gl_profile_tab[PROFILE_A_APP_ID].char_handle,
                                                                     &count);
        if (ret_status != ESP_GATT_OK){
            ESP_LOGE(GATTC_TAG, "esp_ble_gattc_get_attr_count error");
        }
        if (count > 0){
            descr_elem_result_a = (esp_gattc_descr_elem_t *)malloc(sizeof(esp_gattc_descr_elem_t) * count);
            if (!descr_elem_result_a){
                ESP_LOGE(GATTC_TAG, "malloc error, gattc no mem");
            }else{
                ret_status = esp_ble_gattc_get_descr_by_char_handle( gattc_if,
                                                                     gl_profile_tab[PROFILE_A_APP_ID].conn_id,
                                                                     p_data->reg_for_notify.handle,
                                                                     notify_descr_uuid,
                                                                     descr_elem_result_a,
                                                                     &count);
                if (ret_status != ESP_GATT_OK){
                    ESP_LOGE(GATTC_TAG, "esp_ble_gattc_get_descr_by_char_handle error");
                }

                /* Every char has only one descriptor in our 'ESP_GATTS_DEMO' demo, so we used first 'descr_elem_result' */
                if (count > 0 && descr_elem_result_a[0].uuid.len == ESP_UUID_LEN_16 && descr_elem_result_a[0].uuid.uuid.uuid16 == ESP_GATT_UUID_CHAR_CLIENT_CONFIG){
                    ret_status = esp_ble_gattc_write_char_descr( gattc_if, /////////////////////////////////////////////////////////////////////////////// ghi vào Bộ mô tả cấu hình client
                                                                 gl_profile_tab[PROFILE_A_APP_ID].conn_id,
                                                                 descr_elem_result_a[0].handle,
                                                                 sizeof(notify_en),
                                                                 (uint8_t *)&notify_en,
                                                                 ESP_GATT_WRITE_TYPE_RSP,
                                                                 ESP_GATT_AUTH_REQ_NONE);
                }

                if (ret_status != ESP_GATT_OK){
                    ESP_LOGE(GATTC_TAG, "esp_ble_gattc_write_char_descr error");
                }

                /* free descr_elem_result */
                free(descr_elem_result_a);
            }
        }
        else{
            ESP_LOGE(GATTC_TAG, "decsr not found");
        }
        break;
    }
    case ESP_GATTC_NOTIFY_EVT:///////////////////////////////////////////////////////////////// GIÁ TRỊ NHẬN ĐƯỢC TỪ SERVER
        ESP_LOGI(GATTC_TAG, "ESP_GATTC_NOTIFY_EVT, Receive notify value:");
        mystring_a=(char *)p_data->notify.value;  ////// Gía trị nhận được từ server.
        a=atof(mystring_a);
        
        sprintf(REQUEST_A,"{\"Tempature\":%f}",a);
        esp_mqtt_client_publish(A, "esp/telemetry",REQUEST_A , 0, 1, 0);////Đẩy dữ liệu nhiệt độ lên ThingsBoard bằng MQTT
        printf("have load data A");
        if (conn_device_a == true)
           {
             esp_mqtt_client_publish(A, "esp/attributes","{\"Status\":true}" , 0, 1, 0);
           }
        esp_log_buffer_hex(GATTC_TAG, p_data->notify.value, p_data->notify.value_len);
        break;
    case ESP_GATTC_WRITE_DESCR_EVT:
        if (p_data->write.status != ESP_GATT_OK){
            ESP_LOGE(GATTC_TAG, "write descr failed, error status = %x", p_data->write.status);
            break;
        }
        ESP_LOGI(GATTC_TAG, "write descr success");
        globalIf_A =gattc_if;
        
        //uint8_t write_char_data[35];
        //for (int i = 0; i < sizeof(write_char_data); ++i)
       // {
        //    write_char_data[i] = i % 256;
        //}
        //esp_ble_gattc_write_char( gattc_if,
        //                          gl_profile_tab[PROFILE_A_APP_ID].conn_id,
        //                          gl_profile_tab[PROFILE_A_APP_ID].char_handle,
        //                          sizeof(write_char_data),
        //                          write_char_data,
        //                          ESP_GATT_WRITE_TYPE_RSP,
        //                          ESP_GATT_AUTH_REQ_NONE);
        start_scan();
        break;
        ////////////////////////////////////Nếu quy trình ghi được xác nhận thì thiết bị từ xa đã kết nối thành công và giao tiếp được thiết lập mà không có lỗi. Ngay lập tức, thủ tục ghi tạo ra một sự kiện ESP_GATTC_WRITE_CHAR_EVT, 
////////trong ví dụ này được sử dụng để in thông tin và kết nối với thiết bị từ xa thứ hai:
    case ESP_GATTC_WRITE_CHAR_EVT:
        if (p_data->write.status != ESP_GATT_OK){
            ESP_LOGE(GATTC_TAG, "write char failed, error status = %x", p_data->write.status);
        }else{
            ESP_LOGI(GATTC_TAG, "write char success");
        }
        //start_scan();/// CÂU LỆNH NÀY GIỐNG VỚI CÂU LỆNH MIÊU TẢ Ở PHẦN CHÚ THÍCH TRÊN. NÓ SẼ CHUYỂN SANG PROFILE B
        break;
    case ESP_GATTC_SRVC_CHG_EVT: {
        esp_bd_addr_t bda;
        memcpy(bda, p_data->srvc_chg.remote_bda, sizeof(esp_bd_addr_t));
        ESP_LOGI(GATTC_TAG, "ESP_GATTC_SRVC_CHG_EVT, bd_addr:%08x%04x",(bda[0] << 24) + (bda[1] << 16) + (bda[2] << 8) + bda[3],
                 (bda[4] << 8) + bda[5]);
        break;
    }
    case ESP_GATTC_DISCONNECT_EVT:
        //Start scanning again
        start_scan();
        if (memcmp(p_data->disconnect.remote_bda, gl_profile_tab[PROFILE_A_APP_ID].remote_bda, 6) == 0){
            ESP_LOGI(GATTC_TAG, "device a disconnect");
            conn_device_a = false;
            get_service_a = false;
           
        }
        break;
    default:
        break;
    }
}
///Tương tự gattc_profile_a_event_handler
static void gattc_profile_b_event_handler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if, esp_ble_gattc_cb_param_t *param)
{
    esp_ble_gattc_cb_param_t *p_data = (esp_ble_gattc_cb_param_t *)param;

    switch (event) {
    case ESP_GATTC_REG_EVT:
        ESP_LOGI(GATTC_TAG, "REG_EVT");
        break;
    case ESP_GATTC_CONNECT_EVT:
        break;
    case ESP_GATTC_OPEN_EVT:
        if (p_data->open.status != ESP_GATT_OK){
            //open failed, ignore the second device, connect the third device
            ESP_LOGE(GATTC_TAG, "connect device failed, status %d", p_data->open.status);
            conn_device_b = false;
            //start_scan();
            break;
        }
        memcpy(gl_profile_tab[PROFILE_B_APP_ID].remote_bda, p_data->open.remote_bda, 6);
        gl_profile_tab[PROFILE_B_APP_ID].conn_id = p_data->open.conn_id;
        ESP_LOGI(GATTC_TAG, "ESP_GATTC_OPEN_EVT conn_id %d, if %d, status %d, mtu %d", p_data->open.conn_id, gattc_if, p_data->open.status, p_data->open.mtu);
        ESP_LOGI(GATTC_TAG, "REMOTE BDA:");
        esp_log_buffer_hex(GATTC_TAG, p_data->open.remote_bda, sizeof(esp_bd_addr_t));
        esp_err_t mtu_ret = esp_ble_gattc_send_mtu_req (gattc_if, p_data->open.conn_id);
        if (mtu_ret){
            ESP_LOGE(GATTC_TAG, "config MTU error, error code = %x", mtu_ret);
        }
        break;
    case ESP_GATTC_CFG_MTU_EVT:
        if (param->cfg_mtu.status != ESP_GATT_OK){
            ESP_LOGE(GATTC_TAG,"Config mtu failed");
        }
        ESP_LOGI(GATTC_TAG, "Status %d, MTU %d, conn_id %d", param->cfg_mtu.status, param->cfg_mtu.mtu, param->cfg_mtu.conn_id);
        esp_ble_gattc_search_service(gattc_if, param->cfg_mtu.conn_id, &remote_filter_service_uuid);
        break;
    case ESP_GATTC_SEARCH_RES_EVT: {
        ESP_LOGI(GATTC_TAG, "SEARCH RES: conn_id = %x is primary service %d", p_data->search_res.conn_id, p_data->search_res.is_primary);
        ESP_LOGI(GATTC_TAG, "start handle %d end handle %d current handle value %d", p_data->search_res.start_handle, p_data->search_res.end_handle, p_data->search_res.srvc_id.inst_id);
        if (p_data->search_res.srvc_id.uuid.len == ESP_UUID_LEN_16 && p_data->search_res.srvc_id.uuid.uuid.uuid16 == REMOTE_SERVICE_UUID) {
            ESP_LOGI(GATTC_TAG, "UUID16: %x", p_data->search_res.srvc_id.uuid.uuid.uuid16);
            get_service_b = true;
            gl_profile_tab[PROFILE_B_APP_ID].service_start_handle = p_data->search_res.start_handle;
            gl_profile_tab[PROFILE_B_APP_ID].service_end_handle = p_data->search_res.end_handle;
        }
        break;
    }
    case ESP_GATTC_SEARCH_CMPL_EVT:
        if (p_data->search_cmpl.status != ESP_GATT_OK){
            ESP_LOGE(GATTC_TAG, "search service failed, error status = %x", p_data->search_cmpl.status);
            break;
        }
        if (get_service_b){
            uint16_t count = 0;
            esp_gatt_status_t status = esp_ble_gattc_get_attr_count( gattc_if,
                                                                     p_data->search_cmpl.conn_id,
                                                                     ESP_GATT_DB_CHARACTERISTIC,
                                                                     gl_profile_tab[PROFILE_B_APP_ID].service_start_handle,
                                                                     gl_profile_tab[PROFILE_B_APP_ID].service_end_handle,
                                                                     INVALID_HANDLE,
                                                                     &count);
            if (status != ESP_GATT_OK){
                ESP_LOGE(GATTC_TAG, "esp_ble_gattc_get_attr_count error");
            }

            if (count > 0){
                char_elem_result_b = (esp_gattc_char_elem_t *)malloc(sizeof(esp_gattc_char_elem_t) * count);
                if (!char_elem_result_b){
                    ESP_LOGE(GATTC_TAG, "gattc no mem");
                }else{
                    status = esp_ble_gattc_get_char_by_uuid( gattc_if,
                                                             p_data->search_cmpl.conn_id,
                                                             gl_profile_tab[PROFILE_B_APP_ID].service_start_handle,
                                                             gl_profile_tab[PROFILE_B_APP_ID].service_end_handle,
                                                             remote_filter_char_uuid,
                                                             char_elem_result_b,
                                                             &count);
                    if (status != ESP_GATT_OK){
                        ESP_LOGE(GATTC_TAG, "esp_ble_gattc_get_char_by_uuid error");
                    }

                    /*  Every service have only one char in our 'ESP_GATTS_DEMO' demo, so we used first 'char_elem_result' */
                    if (count > 0 && (char_elem_result_b[0].properties & ESP_GATT_CHAR_PROP_BIT_NOTIFY)){
                        gl_profile_tab[PROFILE_B_APP_ID].char_handle = char_elem_result_b[0].char_handle;
                        esp_ble_gattc_register_for_notify (gattc_if, gl_profile_tab[PROFILE_B_APP_ID].remote_bda, char_elem_result_b[0].char_handle);
                    }
                }
                /* free char_elem_result */
                free(char_elem_result_b);
            }else{
                ESP_LOGE(GATTC_TAG, "no char found");
            }
        }
        break;
    case ESP_GATTC_REG_FOR_NOTIFY_EVT: {

        if (p_data->reg_for_notify.status != ESP_GATT_OK){
            ESP_LOGE(GATTC_TAG, "reg notify failed, error status =%x", p_data->reg_for_notify.status);
            break;
        }
        uint16_t count = 0;
        uint16_t notify_en = 1;
        esp_gatt_status_t ret_status = esp_ble_gattc_get_attr_count( gattc_if,
                                                                     gl_profile_tab[PROFILE_B_APP_ID].conn_id,
                                                                     ESP_GATT_DB_DESCRIPTOR,
                                                                     gl_profile_tab[PROFILE_B_APP_ID].service_start_handle,
                                                                     gl_profile_tab[PROFILE_B_APP_ID].service_end_handle,
                                                                     gl_profile_tab[PROFILE_B_APP_ID].char_handle,
                                                                     &count);
        if (ret_status != ESP_GATT_OK){
            ESP_LOGE(GATTC_TAG, "esp_ble_gattc_get_attr_count error");
        }
        if (count > 0){
            descr_elem_result_b = (esp_gattc_descr_elem_t *)malloc(sizeof(esp_gattc_descr_elem_t) * count);
            if (!descr_elem_result_b){
                ESP_LOGE(GATTC_TAG, "malloc error, gattc no mem");
            }else{
                ret_status = esp_ble_gattc_get_descr_by_char_handle( gattc_if,
                                                                     gl_profile_tab[PROFILE_B_APP_ID].conn_id,
                                                                     p_data->reg_for_notify.handle,
                                                                     notify_descr_uuid,
                                                                     descr_elem_result_b,
                                                                     &count);
                if (ret_status != ESP_GATT_OK){
                    ESP_LOGE(GATTC_TAG, "esp_ble_gattc_get_descr_by_char_handle error");
                }

                /* Every char has only one descriptor in our 'ESP_GATTS_DEMO' demo, so we used first 'descr_elem_result' */
                if (count > 0 && descr_elem_result_b[0].uuid.len == ESP_UUID_LEN_16 && descr_elem_result_b[0].uuid.uuid.uuid16 == ESP_GATT_UUID_CHAR_CLIENT_CONFIG){
                    ret_status = esp_ble_gattc_write_char_descr( gattc_if,
                                                                 gl_profile_tab[PROFILE_B_APP_ID].conn_id,
                                                                 descr_elem_result_b[0].handle,
                                                                 sizeof(notify_en),
                                                                 (uint8_t *)&notify_en,
                                                                 ESP_GATT_WRITE_TYPE_RSP,
                                                                 ESP_GATT_AUTH_REQ_NONE);
                }

                if (ret_status != ESP_GATT_OK){
                    ESP_LOGE(GATTC_TAG, "esp_ble_gattc_write_char_descr error");
                }

                /* free descr_elem_result */
                free(descr_elem_result_b);
            }
        }
        else{
            ESP_LOGE(GATTC_TAG, "decsr not found");
        }
        break;
    }
    case ESP_GATTC_NOTIFY_EVT:
        ESP_LOGI(GATTC_TAG, "ESP_GATTC_NOTIFY_EVT, Receive notify value:");
        mystring_b=(char *)p_data->notify.value;
         b=atof(mystring_b);
        sprintf(REQUEST_B,"{\"Tempature\":%f}",b);
        esp_mqtt_client_publish(B, "esp/telemetry",REQUEST_B , 0, 1, 0);
         printf("have load data B");
         if (conn_device_b == true)
           {
             esp_mqtt_client_publish(B, "esp/attributes","{\"Status\":true}" , 0, 1, 0);
           }
        esp_log_buffer_hex(GATTC_TAG, p_data->notify.value, p_data->notify.value_len);
        break;
    case ESP_GATTC_WRITE_DESCR_EVT:
        if (p_data->write.status != ESP_GATT_OK){
            ESP_LOGE(GATTC_TAG, "write descr failed, error status = %x", p_data->write.status);
            break;
        }
        ESP_LOGI(GATTC_TAG, "write descr success");
         globalIf_B =gattc_if;
        //uint8_t write_char_data[35];
        //for (int i = 0; i < sizeof(write_char_data); ++i)
        //{
        //    write_char_data[i] = i % 256;
       // }
       // esp_ble_gattc_write_char( gattc_if,
       //                           gl_profile_tab[PROFILE_B_APP_ID].conn_id,
       //                           gl_profile_tab[PROFILE_B_APP_ID].char_handle,
       //                           sizeof(write_char_data),
       //                           write_char_data,
       //                           ESP_GATT_WRITE_TYPE_RSP,
        //                          ESP_GATT_AUTH_REQ_NONE);
       start_scan();
        break;
    case ESP_GATTC_WRITE_CHAR_EVT:
        if (p_data->write.status != ESP_GATT_OK){
            ESP_LOGE(GATTC_TAG, "Write char failed, error status = %x", p_data->write.status);
        }else{
            ESP_LOGI(GATTC_TAG, "Write char success");
        }
       // start_scan();
        break;
    case ESP_GATTC_SRVC_CHG_EVT: {
        esp_bd_addr_t bda;
        memcpy(bda, p_data->srvc_chg.remote_bda, sizeof(esp_bd_addr_t));
        ESP_LOGI(GATTC_TAG, "ESP_GATTC_SRVC_CHG_EVT, bd_addr:%08x%04x",(bda[0] << 24) + (bda[1] << 16) + (bda[2] << 8) + bda[3],
                 (bda[4] << 8) + bda[5]);
        break;
    }
    case ESP_GATTC_DISCONNECT_EVT:
      start_scan();
        if (memcmp(p_data->disconnect.remote_bda, gl_profile_tab[PROFILE_B_APP_ID].remote_bda, 6) == 0){
            ESP_LOGI(GATTC_TAG, "device b disconnect");
            conn_device_b = false;
            get_service_b = false;
        }
        break;
    default:
        break;
    }
}
///Tương tự gattc_profile_a_event_handler
static void gattc_profile_c_event_handler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if, esp_ble_gattc_cb_param_t *param)
{
    esp_ble_gattc_cb_param_t *p_data = (esp_ble_gattc_cb_param_t *)param;

    switch (event) {
    case ESP_GATTC_REG_EVT:
        ESP_LOGI(GATTC_TAG, "REG_EVT");
        break;
    case ESP_GATTC_CONNECT_EVT:
        break;
    case ESP_GATTC_OPEN_EVT:
        if (p_data->open.status != ESP_GATT_OK){
            ESP_LOGE(GATTC_TAG, "connect device failed, status %d", p_data->open.status);
            conn_device_c = false;
            //start_scan();
            break;
        }
        memcpy(gl_profile_tab[PROFILE_C_APP_ID].remote_bda, p_data->open.remote_bda, 6);
        gl_profile_tab[PROFILE_C_APP_ID].conn_id = p_data->open.conn_id;
        ESP_LOGI(GATTC_TAG, "ESP_GATTC_OPEN_EVT conn_id %d, if %d, status %d, mtu %d", p_data->open.conn_id, gattc_if, p_data->open.status, p_data->open.mtu);
        ESP_LOGI(GATTC_TAG, "REMOTE BDA:");
        esp_log_buffer_hex(GATTC_TAG, p_data->open.remote_bda, sizeof(esp_bd_addr_t));
        esp_err_t mtu_ret = esp_ble_gattc_send_mtu_req (gattc_if, p_data->open.conn_id);
        if (mtu_ret){
            ESP_LOGE(GATTC_TAG, "config MTU error, error code = %x", mtu_ret);
        }
        break;
    case ESP_GATTC_CFG_MTU_EVT:
        if (param->cfg_mtu.status != ESP_GATT_OK){
            ESP_LOGE(GATTC_TAG,"Config mtu failed");
        }
        ESP_LOGI(GATTC_TAG, "Status %d, MTU %d, conn_id %d", param->cfg_mtu.status, param->cfg_mtu.mtu, param->cfg_mtu.conn_id);
        esp_ble_gattc_search_service(gattc_if, param->cfg_mtu.conn_id, &remote_filter_service_uuid);
        break;
    case ESP_GATTC_SEARCH_RES_EVT: {
        ESP_LOGI(GATTC_TAG, "SEARCH RES: conn_id = %x is primary service %d", p_data->search_res.conn_id, p_data->search_res.is_primary);
        ESP_LOGI(GATTC_TAG, "start handle %d end handle %d current handle value %d", p_data->search_res.start_handle, p_data->search_res.end_handle, p_data->search_res.srvc_id.inst_id);
        if (p_data->search_res.srvc_id.uuid.len == ESP_UUID_LEN_16 && p_data->search_res.srvc_id.uuid.uuid.uuid16 == REMOTE_SERVICE_UUID) {
            ESP_LOGI(GATTC_TAG, "UUID16: %x", p_data->search_res.srvc_id.uuid.uuid.uuid16);
            get_service_c = true;
            gl_profile_tab[PROFILE_C_APP_ID].service_start_handle = p_data->search_res.start_handle;
            gl_profile_tab[PROFILE_C_APP_ID].service_end_handle = p_data->search_res.end_handle;
        }
        break;
    }
    case ESP_GATTC_SEARCH_CMPL_EVT:
        if (p_data->search_cmpl.status != ESP_GATT_OK){
            ESP_LOGE(GATTC_TAG, "search service failed, error status = %x", p_data->search_cmpl.status);
            break;
        }
        if (get_service_c){
            uint16_t count = 0;
            esp_gatt_status_t status = esp_ble_gattc_get_attr_count( gattc_if,
                                                                     p_data->search_cmpl.conn_id,
                                                                     ESP_GATT_DB_CHARACTERISTIC,
                                                                     gl_profile_tab[PROFILE_C_APP_ID].service_start_handle,
                                                                     gl_profile_tab[PROFILE_C_APP_ID].service_end_handle,
                                                                     INVALID_HANDLE,
                                                                     &count);
            if (status != ESP_GATT_OK){
                ESP_LOGE(GATTC_TAG, "esp_ble_gattc_get_attr_count error");
            }

            if (count > 0){
                char_elem_result_c = (esp_gattc_char_elem_t *)malloc(sizeof(esp_gattc_char_elem_t) * count);
                if (!char_elem_result_c){
                    ESP_LOGE(GATTC_TAG, "gattc no mem");
                }else{
                    status = esp_ble_gattc_get_char_by_uuid( gattc_if,
                                                             p_data->search_cmpl.conn_id,
                                                             gl_profile_tab[PROFILE_C_APP_ID].service_start_handle,
                                                             gl_profile_tab[PROFILE_C_APP_ID].service_end_handle,
                                                             remote_filter_char_uuid,
                                                             char_elem_result_c,
                                                             &count);
                    if (status != ESP_GATT_OK){
                        ESP_LOGE(GATTC_TAG, "esp_ble_gattc_get_char_by_uuid error");
                    }

                    /*  Every service have only one char in our 'ESP_GATTS_DEMO' demo, so we used first 'char_elem_result' */
                    if (count > 0 && (char_elem_result_c[0].properties & ESP_GATT_CHAR_PROP_BIT_NOTIFY)){
                        gl_profile_tab[PROFILE_C_APP_ID].char_handle = char_elem_result_c[0].char_handle;
                        esp_ble_gattc_register_for_notify (gattc_if, gl_profile_tab[PROFILE_C_APP_ID].remote_bda, char_elem_result_c[0].char_handle);
                    }
                }
                /* free char_elem_result */
                free(char_elem_result_c);
            }else{
                ESP_LOGE(GATTC_TAG, "no char found");
            }
        }
        break;
    case ESP_GATTC_REG_FOR_NOTIFY_EVT: {
        if (p_data->reg_for_notify.status != ESP_GATT_OK){
            ESP_LOGE(GATTC_TAG, "reg notify failed, error status =%x", p_data->reg_for_notify.status);
            break;
        }
        uint16_t count = 0;
        uint16_t notify_en = 1;
        esp_gatt_status_t ret_status = esp_ble_gattc_get_attr_count( gattc_if,
                                                                     gl_profile_tab[PROFILE_C_APP_ID].conn_id,
                                                                     ESP_GATT_DB_DESCRIPTOR,
                                                                     gl_profile_tab[PROFILE_C_APP_ID].service_start_handle,
                                                                     gl_profile_tab[PROFILE_C_APP_ID].service_end_handle,
                                                                     gl_profile_tab[PROFILE_C_APP_ID].char_handle,
                                                                     &count);
        if (ret_status != ESP_GATT_OK){
            ESP_LOGE(GATTC_TAG, "esp_ble_gattc_get_attr_count error");
        }
        if (count > 0){
            descr_elem_result_c = (esp_gattc_descr_elem_t *)malloc(sizeof(esp_gattc_descr_elem_t) * count);
            if (!descr_elem_result_c){
                ESP_LOGE(GATTC_TAG, "malloc error, gattc no mem");
            }else{
                ret_status = esp_ble_gattc_get_descr_by_char_handle( gattc_if,
                                                                     gl_profile_tab[PROFILE_C_APP_ID].conn_id,
                                                                     p_data->reg_for_notify.handle,
                                                                     notify_descr_uuid,
                                                                     descr_elem_result_c,
                                                                     &count);
                if (ret_status != ESP_GATT_OK){
                    ESP_LOGE(GATTC_TAG, "esp_ble_gattc_get_descr_by_char_handle error");
                }

                /* Every char has only one descriptor in our 'ESP_GATTS_DEMO' demo, so we used first 'descr_elem_result' */
                if (count > 0 && descr_elem_result_c[0].uuid.len == ESP_UUID_LEN_16 && descr_elem_result_c[0].uuid.uuid.uuid16 == ESP_GATT_UUID_CHAR_CLIENT_CONFIG){
                    ret_status = esp_ble_gattc_write_char_descr( gattc_if,
                                                                 gl_profile_tab[PROFILE_C_APP_ID].conn_id,
                                                                 descr_elem_result_c[0].handle,
                                                                 sizeof(notify_en),
                                                                 (uint8_t *)&notify_en,
                                                                 ESP_GATT_WRITE_TYPE_RSP,
                                                                 ESP_GATT_AUTH_REQ_NONE);
                }

                if (ret_status != ESP_GATT_OK){
                    ESP_LOGE(GATTC_TAG, "esp_ble_gattc_write_char_descr error");
                }

                /* free descr_elem_result */
                free(descr_elem_result_c);
            }
        }
        else{
            ESP_LOGE(GATTC_TAG, "decsr not found");
        }
        break;
    }
    case ESP_GATTC_NOTIFY_EVT:
        ESP_LOGI(GATTC_TAG, "ESP_GATTC_NOTIFY_EVT, Receive notify value:");
        esp_log_buffer_hex(GATTC_TAG, p_data->notify.value, p_data->notify.value_len);
        break;
    case ESP_GATTC_WRITE_DESCR_EVT:
        if (p_data->write.status != ESP_GATT_OK){
            ESP_LOGE(GATTC_TAG, "write descr failed, error status = %x", p_data->write.status);
            break;
        }
        ESP_LOGI(GATTC_TAG, "write descr success");
        //uint8_t write_char_data[35];
        //for (int i = 0; i < sizeof(write_char_data); ++i)
        //{
        //    write_char_data[i] = i % 256;
       // }
       // esp_ble_gattc_write_char( gattc_if,
       //                           gl_profile_tab[PROFILE_C_APP_ID].conn_id,
        //                          gl_profile_tab[PROFILE_C_APP_ID].char_handle,
         //                         sizeof(write_char_data),
         //                         write_char_data,
           //                       ESP_GATT_WRITE_TYPE_RSP,
           //                       ESP_GATT_AUTH_REQ_NONE);
        start_scan();
        break;
    case ESP_GATTC_WRITE_CHAR_EVT:
        if (p_data->write.status != ESP_GATT_OK){
            ESP_LOGE(GATTC_TAG, "Write char failed, error status = %x", p_data->write.status);
            break;
        }
        ESP_LOGI(GATTC_TAG, "Write char success");
        //start_scan();
        break;
    case ESP_GATTC_SRVC_CHG_EVT: {
        esp_bd_addr_t bda;
        memcpy(bda, p_data->srvc_chg.remote_bda, sizeof(esp_bd_addr_t));
        ESP_LOGI(GATTC_TAG, "ESP_GATTC_SRVC_CHG_EVT, bd_addr:%08x%04x",(bda[0] << 24) + (bda[1] << 16) + (bda[2] << 8) + bda[3],
                 (bda[4] << 8) + bda[5]);
        break;
    }
    case ESP_GATTC_DISCONNECT_EVT:
        start_scan();
        if (memcmp(p_data->disconnect.remote_bda, gl_profile_tab[PROFILE_C_APP_ID].remote_bda, 6) == 0){
            ESP_LOGI(GATTC_TAG, "device c disconnect");
            conn_device_c = false;
            get_service_c = false;
        }
        break;
    default:
        break;
    }
}

static void esp_gap_cb(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param)// GAP: QUÉT kết nối
{
    uint8_t *adv_name = NULL;
    uint8_t adv_name_len = 0;
    switch (event) {
    case ESP_GAP_BLE_UPDATE_CONN_PARAMS_EVT:
         ESP_LOGI(GATTC_TAG, "update connection params status = %d, min_int = %d, max_int = %d,conn_int = %d,latency = %d, timeout = %d",
                  param->update_conn_params.status,
                  param->update_conn_params.min_int,
                  param->update_conn_params.max_int,
                  param->update_conn_params.conn_int,
                  param->update_conn_params.latency,
                  param->update_conn_params.timeout);
        break;
    case ESP_GAP_BLE_SCAN_PARAM_SET_COMPLETE_EVT: {///////////// được sử dụng để bắt đầu quét các máy chủ GATT lân cận:
        //the unit of the duration is second
        uint32_t duration = 30;
        esp_ble_gap_start_scanning(duration);//// Quá trình quét được bắt đầu
        break;
    }
    case ESP_GAP_BLE_SCAN_START_COMPLETE_EVT://// khi thời gian quét kết thúc
        //scan start complete event to indicate scan start successfully or failed
        if (param->scan_start_cmpl.status == ESP_BT_STATUS_SUCCESS) {
            ESP_LOGI(GATTC_TAG, "Scan start success");
        }else{
            ESP_LOGE(GATTC_TAG, "Scan start failed");
        }
        break;
    case ESP_GAP_BLE_SCAN_RESULT_EVT: {////////// được gọi mỗi khi tìm thấy một thiết bị mới.
        esp_ble_gap_cb_param_t *scan_result = (esp_ble_gap_cb_param_t *)param;
        switch (scan_result->scan_rst.search_evt) {
        case ESP_GAP_SEARCH_INQ_RES_EVT:
            vTaskDelay(1);
            esp_log_buffer_hex(GATTC_TAG, scan_result->scan_rst.bda, 6);
            ESP_LOGI(GATTC_TAG, "Searched Adv Data Len %d, Scan Response Len %d", scan_result->scan_rst.adv_data_len, scan_result->scan_rst.scan_rsp_len);////////////in độ dài dữ liệu được quảng cáo và độ dài phản hồi quét
            adv_name = esp_ble_resolve_adv_data(scan_result->scan_rst.ble_adv,/// Hàm quét lấy tên thiết bị đang quáng cáo 
                                                ESP_BLE_AD_TYPE_NAME_CMPL, &adv_name_len);
            ESP_LOGI(GATTC_TAG, "Searched Device Name Len %d", adv_name_len);
            esp_log_buffer_char(GATTC_TAG, adv_name, adv_name_len);
            ESP_LOGI(GATTC_TAG, "\n");
            
           if (conn_device_a == false)
           {
             esp_mqtt_client_publish(A, "esp/attributes","{\"Status\":false}" , 0, 1, 0);//// Cập nhập trạng thái của node lên clound

           }
            if (conn_device_b == false)
           {
             esp_mqtt_client_publish(B, "esp/attributes","{\"Status\":false}" , 0, 1, 0);
           }
           
            if (Isconnecting){
                break;
            }
            ////////////////////Nếu tất cả thiết bị đều được tìm thấy nó sẽ ngừng quét
            if (conn_device_a && conn_device_b  && !stop_scan_done){
                stop_scan_done = true;
                esp_ble_gap_stop_scanning();
                ESP_LOGI(GATTC_TAG, "all devices are connected");
                break;
            }
            /////// Tìm các thiết bị đã được quét, nếu trùng với các tên thiết bị muốn kết nối, thì nó sẽ kết nối
            if (adv_name != NULL) {

                if (strlen(remote_device_name[0]) == adv_name_len && strncmp((char *)adv_name, remote_device_name[0], adv_name_len) == 0) {
                    if (conn_device_a == false) {
                        conn_device_a = true;
                       
                         
                        ESP_LOGI(GATTC_TAG, "Searched device %s", remote_device_name[0]);
                        esp_ble_gap_stop_scanning();//////Once device have been found, the client stops scanning:
                        esp_ble_gattc_open(gl_profile_tab[PROFILE_A_APP_ID].gattc_if, scan_result->scan_rst.bda, scan_result->scan_rst.ble_addr_type, true);/////////////////////////tên thiết bị từ xa giống như chúng ta đã xác định ở trên, thiết bị cục bộ sẽ dừng quét và cố gắng mở kết nối với thiết bị từ xa
                        Isconnecting = true;
                    }
                    break;
                }
                else if (strlen(remote_device_name[1]) == adv_name_len && strncmp((char *)adv_name, remote_device_name[1], adv_name_len) == 0) {
                    if (conn_device_b == false) {
                        conn_device_b = true;
                       
                      
                        ESP_LOGI(GATTC_TAG, "Searched device %s", remote_device_name[1]);
                        esp_ble_gap_stop_scanning();//////Once device have been found, the client stops scanning:
                        esp_ble_gattc_open(gl_profile_tab[PROFILE_B_APP_ID].gattc_if, scan_result->scan_rst.bda, scan_result->scan_rst.ble_addr_type, true);
                        Isconnecting = true;

                    }
                }else
                {
                    printf("Thiet bi khong hop le");
                     
                }
               

            }
            break;
        case ESP_GAP_SEARCH_INQ_CMPL_EVT:
             printf("Khong tim thay du thiet bi ");
            uint32_t duration = 5;
           esp_ble_gap_start_scanning(duration); 
            
            break;
        default:
            break;
        }
        break;
    }
///////////////////////////////////////////////////////////////////Quá trình dừng quét sẽ kích hoạt sự kiện ESP_GAP_BLE_SCAN_STOP_COMPLETE_EVT
/// được sử dụng để mở kết nối(esp_ble_gattc_open) với thiết bị từ xa đầu tiên. Thiết bị thứ hai và thứ ba được kết nối sau khi client tìm kiếm service, nhận các characteristics
///đăng ký nhận thông báo trên thiết bị đầu tiên. Quy trình công việc này được thiết kế để kiểm tra xem giao tiếp giữa từng thiết bị từ xa có hoạt động bình 
///thường không trước khi thử kết nối với thiết bị tiếp theo hoặc trong trường hợp có lỗi, hãy chuyển sang thiết bị tiếp theo.
    case ESP_GAP_BLE_SCAN_STOP_COMPLETE_EVT://///// được kích hoạt khi thời gian quét hoàn tất và có thể được sử dụng để khởi động lại quy trình quét
        if (param->scan_stop_cmpl.status != ESP_BT_STATUS_SUCCESS){
            ESP_LOGE(GATTC_TAG, "Scan stop failed");
            break;
        }
        ESP_LOGI(GATTC_TAG, "Stop scan successfully");

        break;

    case ESP_GAP_BLE_ADV_STOP_COMPLETE_EVT:
        if (param->adv_stop_cmpl.status != ESP_BT_STATUS_SUCCESS){
            ESP_LOGE(GATTC_TAG, "Adv stop failed");
            break;
        }
        ESP_LOGI(GATTC_TAG, "Stop adv successfully");
        break;

    default:
        break;
    }
}

static void esp_gattc_cb(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if, esp_ble_gattc_cb_param_t *param)
{
    //ESP_LOGI(GATTC_TAG, "EVT %d, gattc if %d, app_id %d", event, gattc_if, param->reg.app_id);

    /* If event is register event, store the gattc_if for each profile */
    if (event == ESP_GATTC_REG_EVT) {
        if (param->reg.status == ESP_GATT_OK) {
            gl_profile_tab[param->reg.app_id].gattc_if = gattc_if;
        } else {
            ESP_LOGI(GATTC_TAG, "Reg app failed, app_id %04x, status %d",
                    param->reg.app_id,
                    param->reg.status);
            return;
        }
    }

    /* If the gattc_if equal to profile A, call profile A cb handler,
     * so here call each profile's callback */
    do {
        int idx;
        for (idx = 0; idx < PROFILE_NUM; idx++) {
            if (gattc_if == ESP_GATT_IF_NONE || /* ESP_GATT_IF_NONE, not specify a certain gatt_if, need to call every profile cb function */
                    gattc_if == gl_profile_tab[idx].gattc_if) {
                if (gl_profile_tab[idx].gattc_cb) {
                    gl_profile_tab[idx].gattc_cb(event, gattc_if, param);
                }
            }
        }
    } while (0);
}

void app_main(void)
{
    
   
   /////////////////////////////
    esp_err_t ret = nvs_flash_init(); // Khởi tạo flash
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK( ret );

    ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT));

    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    ret = esp_bt_controller_init(&bt_cfg);
    if (ret) {
        ESP_LOGE(GATTC_TAG, "%s initialize controller failed: %s\n", __func__, esp_err_to_name(ret));
        return;
    }

    ret = esp_bt_controller_enable(ESP_BT_MODE_BLE);
    if (ret) {
        ESP_LOGE(GATTC_TAG, "%s enable controller failed: %s\n", __func__, esp_err_to_name(ret));
        return;
    }

    ret = esp_bluedroid_init();
    if (ret) {
        ESP_LOGE(GATTC_TAG, "%s init bluetooth failed: %s\n", __func__, esp_err_to_name(ret));
        return;
    }

    ret = esp_bluedroid_enable();
    if (ret) {
        ESP_LOGE(GATTC_TAG, "%s enable bluetooth failed: %s\n", __func__, esp_err_to_name(ret));
        return;
    }

    //register the  callback function to the gap module
    ret = esp_ble_gap_register_callback(esp_gap_cb);
    if (ret){
        ESP_LOGE(GATTC_TAG, "gap register error, error code = %x", ret);
        return;
    }

    //register the callback function to the gattc module
    ret = esp_ble_gattc_register_callback(esp_gattc_cb);
    if(ret){
        ESP_LOGE(GATTC_TAG, "gattc register error, error code = %x", ret);
        return;
    }

    ret = esp_ble_gattc_app_register(PROFILE_A_APP_ID);
    if (ret){
        ESP_LOGE(GATTC_TAG, "gattc app register error, error code = %x", ret);
        return;
    }

    ret = esp_ble_gattc_app_register(PROFILE_B_APP_ID);
    if (ret){
        ESP_LOGE(GATTC_TAG, "gattc app register error, error code = %x", ret);
        return;
    }

    
    ret = esp_ble_gatt_set_local_mtu(200);////Đăng kí payload 200 byte
    if (ret){
        ESP_LOGE(GATTC_TAG, "set local  MTU failed, error code = %x", ret);
    }
    ////////////////////////////
     ESP_LOGI(TAG, "[APP] Startup..");
    ESP_LOGI(TAG, "[APP] Free memory: %d bytes", esp_get_free_heap_size());
    ESP_LOGI(TAG, "[APP] IDF version: %s", esp_get_idf_version());

    esp_log_level_set("*", ESP_LOG_INFO);
    esp_log_level_set("MQTT_CLIENT", ESP_LOG_VERBOSE);
    esp_log_level_set("MQTT_EXAMPLE", ESP_LOG_VERBOSE);
    esp_log_level_set("TRANSPORT_TCP", ESP_LOG_VERBOSE);
    esp_log_level_set("TRANSPORT_SSL", ESP_LOG_VERBOSE);
    esp_log_level_set("TRANSPORT", ESP_LOG_VERBOSE);
    esp_log_level_set("OUTBOX", ESP_LOG_VERBOSE);

    
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

     
     
    ESP_ERROR_CHECK(example_connect());//Kết nối wifi
    
    
    mqtt_app_start(); 
}
