#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CMD_LEN 1024

int main(int argc, char *argv[])
{
	char *tmp_ukzx = "tmp_ukzx";
	char *url = "http://index.youku.com/protop/6";
	char *tmp_source_url = "tmp_source_url";
	char *tmp_down_url = "tmp_down_url";
	char *log_source_url = "log_source_url";
	char cmd[CMD_LEN];
	char *save_dir;
	
	if(argc < 2)
	{
		printf("%s save_dir\n", argv[0]);
		return -1;
	}
	save_dir = argv[1];
	if(access(save_dir, 0) != 0)
	{
		mkdir(save_dir, 0777);
	}
	
	sprintf(cmd, "curl -o %s %s", tmp_ukzx, url);
	system(cmd);
	parse_source_url(tmp_ukzx, tmp_source_url);
	filter_source_url(tmp_source_url, log_source_url);
	add_log_source_url(tmp_source_url, log_source_url);
	get_down_url(tmp_source_url, tmp_down_url);
	down_video(tmp_down_url);
	return 0;
}


