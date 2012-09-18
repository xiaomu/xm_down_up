#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

#include "xm_cmn_util.h"

#define CMD_LEN 1024
#define PATH_LEN 1024
#define LINE_LEN 1024
#define URL_LEN 1024

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
	down_video(save_dir, tmp_down_url);
	return 0;
}

int parse_source_url(tmp_ukzx, tmp_source_url)
{
	char line[LINE_LEN];
	FILE *fp_uk, *fp_src;
	int key_len;
	char *tag_start = "<tbody>";
	char *tag_end = "</tbody>";
	char *tag_key = "encodeURIComponent(";
	char *p_key_start, *p_key_end;
	char url[URL_LEN];
	
	fp_uk = xm_fopen(stderr, tmp_ukzx, "r");
	if(fp_uk == NULL)
	{
		return -1;
	}
	fp_src = xm_fopen(stderr, tmp_source_url, "w");
	if(fp_src == NULL)
	{
		return -1;
	}
	
	while(fgets(line, LINE_LEN, fp_uk) != NULL)
	{
		if(strstr(line, tag_start) != NULL)
		{
			break;
		}
	}
	key_len = strlen(tag_key);
	while(fgets(line, LINE_LEN, fp_uk) != NULL)
	{
		if(strstr(line, tag_end) == NULL)
		{
			if((p_key_start = strstr(line, tag_key)) != NULL)
			{
				p_key_start = p_key_start + key_len;
				p_key_start = p_key_start + 1; // skip character  ' ;
				p_key_end = p_key_start;
				while((*p_key_end != '\'') && (p_key_end < &line[LINE_LEN-1]))
				{
					p_key_end ++;
				}
				if(*p_key_end == '\'')
				{
					strncpy(url, p_key_start, p_key_end-p_key_start);
					url[p_key_end-p_key_start] = '\0';
					fputs(url, fp_src);
				}
			}
		}
	}
	return 0;
}




