#include "sdkconfig.h"
#include <errno.h>
#include "esp_netif.h"
#include "esp_log.h"

#include "nvs_flash.h"
#include "esp_netif.h"
#include "protocol_examples_common.h"
#include "esp_event.h"
#include "freertos/task.h"

#include <stdio.h>       // for printf
#include <sys/socket.h>  // for socket
#include <netdb.h>       // for gethostbyname
#include <unistd.h>      // for close
#include <string.h>      // for string manipulation

#define HOST_IP4_ADDR 3232276352  // "192.168.159.128"
#define SERVER_PORT 2002

static const char *TAG = "example";
//static const char *payload = "Message from ESP32 ";
static const int  ip_protocol = 0;

// connect, bind, and accept except pointers to
// a generic socket address (protocol independent).
// use this type for casting
typedef struct sockaddr SA;

void tcp_client(void) {
	while(1)
	{
        // create an IPv4, TCP socket file descriptor
        int sockfd = socket(AF_INET, SOCK_STREAM, ip_protocol);
        if (sockfd < 0) {
            ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
    	    return;
        }

        // set up hostname
        const char* hostname = "google.com";
        struct hostent *host = gethostbyname(hostname);
        if (host == NULL) {
        	return;
        }

        // set up sockaddr struct with server info
        struct sockaddr_in address;
        address.sin_family = AF_INET;          // ipv4
        address.sin_port = htons(80); // server port, big endian
        //address.sin_addr.s_addr = htonl(HOST_IP4_ADDR);
        address.sin_addr.s_addr = *(in_addr_t*)host->h_addr; // server ip

        // attempt to establish a connection with the server
        // block until connection is established or an error occurs
        // if successful, open the client fd for reading and writing
        int err = connect(sockfd, (SA*)&address, sizeof(address));
        if (err != 0) {
	        ESP_LOGE(TAG, "Socket unable to connect: errno %d", errno);
    	    return;
        }

    	ESP_LOGI(TAG, "Successfully connected");

		char* msg;
        msg ="GET / HTTP/1.1\r\nHost: www.google.com\r\n\r\n";

        // while(1){
/*            int err = send(sockfd, payload, strlen(payload), 0);
            if (err != 0) {
                ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
			    break;
			}
*/
			send(sockfd, msg, strlen(msg), 0);

			// receive response from server
		    char buffer[1024];
		    recv(sockfd, buffer, 1024, 0);
            ESP_LOGI(TAG, "Response was: %s\r\n", buffer);
			vTaskDelay(pdMS_TO_TICKS(3000));	
	    // }

	    if (sockfd != -1) {
            ESP_LOGE(TAG, "Shutting down socket and restarting...");
		    shutdown(sockfd, 0);
		    close(sockfd);
	    }
	}
}

void app_main(void)
{
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    /* This helper function configures Wi-Fi or Ethernet, as selected in menuconfig.
     * Read "Establishing Wi-Fi or Ethernet Connection" section in
     * examples/protocols/README.md for more information about this function.
     */
    ESP_ERROR_CHECK(example_connect());

    tcp_client();
}

