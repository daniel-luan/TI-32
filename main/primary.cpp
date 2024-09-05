#include "primary.h"

#include "esp_log.h"
#include "esp_mac.h"

const char *Primary::tag = "PrimaryTransport";

Primary::Primary()
{
    recv_queue = xQueueCreate(10, sizeof(QueueItem));
    state = INIT;
    ESP_LOGI(tag, "State: INIT");
}

void Primary::init()
{
    assert(state == INIT);

    ESP_ERROR_CHECK(esp_now_init());
    ESP_ERROR_CHECK(esp_now_register_send_cb(get().sendCallback));
    ESP_ERROR_CHECK(esp_now_register_recv_cb(get().recvCallback));

    state = INITIALIZED;
    ESP_LOGI(tag, "State: INITIALIZED");
}

void Primary::sendCallback(const uint8_t *mac_addr, esp_now_send_status_t status)
{

    if (status == ESP_NOW_SEND_SUCCESS)
    {
        ESP_LOGI(tag, "Message sent successfully");
    }
    else
    {
        ESP_LOGI(tag, "Failed to send message");
    }
}

void Primary::recvCallback(const esp_now_recv_info_t *esp_now_info, const uint8_t *data, int data_len)
{
    Packet msg;
    memcpy(&msg, data, data_len);

    QueueItem item;
    memcpy(item.mac_addr, esp_now_info->src_addr, 6);
    memcpy(&item.message, data, data_len);
    xQueueSend(get().recv_queue, &item, portMAX_DELAY);
}

void Primary::process_recv_task(void *p)
{
    get().state = RUNNING;

    ESP_LOGI(tag, "State: RUNNING");

    QueueItem item;
    for (;;)
    {
        if (xQueueReceive(get().recv_queue, &item, portMAX_DELAY) != pdTRUE)
        {
            continue;
        }

        ESP_LOGI(tag, "Got item %d from " MACSTR, item.message.header.packetType, MAC2STR(item.mac_addr));

        get().registerSecondary(item.mac_addr);

        if (item.message.header.packetType == PacketType::PACKET_TYPE_REGISTRATION)
        {
            ESP_LOGI(tag, "Sending PACKET_TYPE_ACK to " MACSTR, MAC2STR(item.mac_addr));
            Packet message = {PACKET_TYPE_ACK, ROLE_PRIMARY};
            ESP_ERROR_CHECK(esp_now_send(item.mac_addr, (uint8_t *)&message, sizeof(message))); // NULL for broadcast
        }
        else if (item.message.header.packetType == PacketType::PACKET_TYPE_MATRIX)
        {
            auto event = std::get<MatrixPacket>(item.message.payload);
            ESP_LOGI(tag, "Key event - Row: %d, Col: %d, State: %d", event.keyEvent.row, event.keyEvent.col, event.keyEvent.state);
        }
    }
}

void Primary::registerSecondary(uint8_t mac_addr[6])
{
    if (!is_peer_in_list(mac_addr))

    {
        ESP_LOGI(tag, "New Peer Registering " MACSTR, MAC2STR(mac_addr));
        MacAddress mac;
        memcpy(mac.addr, mac_addr, 6);
        peer_list.push_back(mac);

        esp_now_peer_info_t broadcast_peer = {};
        memcpy(broadcast_peer.peer_addr, mac_addr, 6);
        broadcast_peer.channel = 0;
        broadcast_peer.ifidx = WIFI_IF_STA;
        broadcast_peer.encrypt = false;
        ESP_ERROR_CHECK(esp_now_add_peer(&broadcast_peer));
    }
}