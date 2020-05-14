#include <stdio.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <assert.h>
#include <unistd.h>
#include <sys/sysinfo.h>

#include "discord_game_sdk.h"

#define CLIENT_ID 706797635268247602
#define DISCORD_REQUIRE(x) assert(x == DiscordResult_Ok)
#define SUCCESS 1
#define FAILURE 0

int get_sys_info(int *boot_time)
{
	struct sysinfo s_info;
	int error = sysinfo(&s_info);
	if (error != 0)
	{
		printf("code error = %d\n", error);
	}
	*boot_time = time(NULL) - s_info.uptime;
}

int get_mem_usage()
{
	unsigned long mem_total, mem_available;
	FILE *meminfo = fopen("/proc/meminfo", "r");
	fscanf(meminfo, "%*s%lu%*s", &mem_total);
	fscanf(meminfo, "%*s%*lu%*s");
	fscanf(meminfo, "%*s%lu%*s", &mem_available);
	fclose(meminfo);
	return round(100 * (1 - (double)mem_available / mem_total));
}

int get_distro(char *buff)
{
	FILE *os_release = fopen("/etc/os-release", "r");
	char lhs[80] = {0}, rhs[80];
	int failed = 1;
	while (failed = strcmp(lhs, "PRETTY_NAME"))
	{
		fscanf(os_release, "%[^=]=%[^\n]\n", lhs, rhs);
	}
	fclose(os_release);
	if (failed)
	{
		strcpy(buff, "Unknown Distro");
		return FAILURE;
	}
	int rhs_len = strnlen(rhs, sizeof(rhs));
	if (rhs[0] == '"' && rhs[rhs_len - 1] == '"' || rhs[0] == '\'' && rhs[rhs_len - 1] == '\'')
	{
		memcpy(buff, rhs + 1, rhs_len - 2);
		buff[rhs_len - 2] = '\0';
	}
	else
	{
		memcpy(buff, rhs, rhs_len + 1);
	}
	return SUCCESS;
}

struct Application
{
	struct IDiscordCore *core;
	struct IDiscordActivityManager *activities;
};

void updateActivityCallback(void *data, enum EDiscordResult result)
{
	DISCORD_REQUIRE(result);
}

void updateActivity(struct Application *app)
{
	struct DiscordActivity activity;
	memset(&activity, 0, sizeof(activity));

	static int boot_time = 0;
	static char distro[80];
	if (!boot_time)
	{
		get_sys_info(&boot_time);
		get_distro(distro);
	}
	sprintf(activity.details, "%s %d%%", "RAM:", get_mem_usage());
	sprintf(activity.state, "%s", distro);
	activity.timestamps.start = boot_time;
	app->activities->update_activity(app->activities, &activity, app, updateActivityCallback);
}

int main(int argc, char **argv)
{
	struct Application app;
	memset(&app, 0, sizeof(app));

	struct IDiscordActivityEvents activities_events;
	memset(&activities_events, 0, sizeof(activities_events));

	struct DiscordCreateParams params;
	DiscordCreateParamsSetDefault(&params);
	params.client_id = CLIENT_ID;
	params.flags = DiscordCreateFlags_Default;
	params.event_data = &app;
	params.activity_events = &activities_events;

	DISCORD_REQUIRE(DiscordCreate(DISCORD_VERSION, &params, &app.core));

	app.activities = app.core->get_activity_manager(app.core);

	for (;;)
	{
		updateActivity(&app);
		DISCORD_REQUIRE(app.core->run_callbacks(app.core));
		// usleep(16 * 1000);
		sleep(1);
	}

	return 0;
}
