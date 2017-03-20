/* pi_shutdown.c - MQTT client that watches for topic "power", value "0"

	 created 3/19/17 BD from mqtt_example.c

*/

#include <signal.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include <sys/types.h>
#include <unistd.h>

#include <mosquitto.h>

#define mqtt_host "localhost"
#define mqtt_port 1183

static int run = 1;

void killit(void)
{
	printf("time to shutdown\n");

	// char * const arglist[] = {"pgm", "1", "2", "3", 0};
	// execv("/bin/echo", arglist);
	char * const arglist[] = {"sudo", "/sbin/shutdown", "-h", "now", 0};
	execv("/usr/bin/sudo", arglist);
	// code never reaches this point
}

void handle_signal(int s)
{
	run = 0;
}

void connect_callback(struct mosquitto *mosq, void *obj, int result)
{
	printf("connect callback, rc=%d\n", result);
}

void message_callback(struct mosquitto *mosq, void *obj, const struct mosquitto_message *message)
{
	bool match = 0;
	printf("got message '%.*s' for topic '%s'\n", message->payloadlen, (char*) message->payload, message->topic);

	if(strncmp("0", (char *)message->payload, strlen("0")) == 0) {
		// time to shutdown
		killit();
	}

}

int main(int argc, char *argv[])
{
	uint8_t reconnect = true;
	char clientid[24];
	struct mosquitto *mosq;
	int rc = 0;

	signal(SIGINT, handle_signal);
	signal(SIGTERM, handle_signal);

	mosquitto_lib_init();

	mosq = mosquitto_new(NULL, true, 0);

	if(mosq){
		mosquitto_connect_callback_set(mosq, connect_callback);
		mosquitto_message_callback_set(mosq, message_callback);

		rc = mosquitto_connect(mosq, mqtt_host, mqtt_port, 60);

		mosquitto_subscribe(mosq, NULL, "power", 0);

		while(run){
			// BD changed timeout to 0 from -1 (1000ms)
			rc = mosquitto_loop(mosq, 0, 1);
			if(run && rc){
				printf("connection error!\n");
				sleep(10);
				mosquitto_reconnect(mosq);
			}
			// sleep(5);
		}
		mosquitto_destroy(mosq);
	}

	mosquitto_lib_cleanup();

	return rc;
}

