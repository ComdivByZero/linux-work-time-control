#include "parcon.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/stat.h>

typedef struct {
	int yday, rest;
} Data;

static int readConfig(const char *home) {
	FILE *f;
	char buf[1024];
	int ret;
	snprintf(buf, sizeof(buf), "%s/.parcon/config", home);
	f = fopen(buf, "r");
	if (f == NULL) {
		ret = -1;
		fprintf(stderr, "Can`t open config file '%s'\n", buf);
	} else {
		if (fscanf(f, "%d", &ret) <= 0) {
			ret = -1;
		}
		fclose(f);
	}
	return ret;
}

static FILE* openAndReadData(const char *home, int yday, Data *data) {
	int r;
	FILE *f;
	char buf[1024];
	snprintf(buf, sizeof(buf), "%s/.parcon/session", home);
	f = fopen(buf, "r+b");
	if (f == NULL) {
		r = 0;
		f = fopen(buf, "w+b");
	} else {
		r = fread(data, sizeof(Data), 1, f);
	}
	if (f != NULL) {
		if (yday != data->yday) {
			printf("init\n");
			data->yday = yday;
			data->rest = readConfig(home);
		}
		printf("%d %d %d %d\n", r, yday, data->yday, data->rest);
	} else {
		data->yday = yday;
		data->rest = 10;
		fprintf(stderr, "Can`t open data file '%s'\n", buf);
	}
	return f;
}

static void logout(void) {
	system("gnome-session-quit --logout --no-prompt --force");
	system("qdbus org.kde.ksmserver /KSMServer logout 0 0 0");/* without testing */
	if (strncmp(getenv("XDG_CURRENT_DESKTOP"), "XFCE", 4) == 0) {
		system("xfce4-session-logout --logout");
	}
	printf("logout\n");
}

static void control(void) {
	time_t curTime;
	FILE *f;
	Data data;
	time(&curTime);
	f = openAndReadData(getenv("HOME"), localtime(&curTime)->tm_yday, &data);
	while (data.rest > 0) {
		sleep(60);
		data.rest--;
		printf("Rest of time - %d\n", data.rest);
		if ((f != NULL) && (data.rest > -2) && ((data.rest <= 10) || (data.rest % 5 == 0))) {
			fseek(f, 0, SEEK_SET);
			fwrite(&data, sizeof(data), 1, f);
			fflush(f);
		}
		if (data.rest == TIME_SOON_END) {
			system("notify-send '"MESSAGE_SOON_END"'");
			if (SOUND_SOON_END[0] != '\0') {
				system("playsound "SOUND_SOON_END);
			}
		}
	}
	logout();
}

static void prepare(int minutes) {
	char buf[1024];
	const char config[] = "/config";
	FILE *f;
	int len;
	len = snprintf(buf, sizeof(buf) - strlen(config), "%s/.parcon", getenv("HOME"));
	if (mkdir(buf, 0777) < 0) {
		fprintf(stderr, "Can`t create configuration directory %s\n", buf);
	} else {
		strcpy(buf + len, config);
		f = fopen(buf, "w");
		if (f == NULL) {
			fprintf(stderr, "Can`t create configuration file %s\n", buf);
		} else {
			printf("Create configuration file %s\n", buf);
			fprintf(f, "%d\n", minutes);
		}
	}
}

extern int main(int argc, char *argv[]) {
	int minutes;
	if (argc > 1) {
		if (strcmp(argv[1], "-h")) {
			printf("Yet not writed\n");
		} else {
			minutes = atoi(argv[1]);
			if (minutes > 0) {
				prepare(minutes);
			} else {
				fprintf(stderr, "Wrong paremeter %s\n", argv[1]);
			}
		}
	} else {
		control();
	}
	return 0;
}
