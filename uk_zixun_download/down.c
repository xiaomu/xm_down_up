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
#define TITLE_LEN 200

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

int parse_source_url(char *tmp_ukzx, char *tmp_source_url)
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
	
	fclose(fp_uk);
	fclose(fp_src);
	return 0;
}

int filter_source_url(char *tmp_source_url, char *log_source_url)
{
	FILE *fp_tsu, *fp_lsu, *fp_tf;
	char line_tsu[LINE_LEN], line_lsu[LINE_LEN];
	int len_line_tsu, line_lsu;
	int flag;
	
	fp_tf = tmpfile();
	fp_tsu = xm_fopen(stderr, tmp_source_url, "r");
	if(fp_tsu == NULL)
	{
		return -1;
	}
	fp_lsu = xm_fopen(stderr, log_source_url, "r");
	if(fp_lsu == NULL)
	{
		return -1;
	}
	
	while(fgets(line_tsu, LINE_LEN, fp_tsu) != NULL)
	{
		rewind(fp_lsu);
		flag = 1;
		while(fgets(line_lsu, LINE_LEN, fp_lsu) != NULL)
		{
			len_line_tsu = strlen(line_tsu);
			len_line_lsu = strlen(line_lsu);
			if((len_line_tsu == len_line_lsu) && (!strcmp(line_tsu, line_lsu)))
			{
				flag = 0;
				break;
			}
		}
		if(flag)
		{
			fprintf(fp_tf, "%s", line_tsu);
		}
	}
	
	fclose(fp_tsu);
	fp_tsu = xm_fopen(stderr, tmp_source_url, "w");
	if(fp_tsu == NULL)
	{
		return -1;
	}
	rewind(fp_tf);
	while(fgets(line_tsu, LINE_LEN, fp_tf) != NULL)
	{
		fprintf(fp_tsu, "%s", line_tsu);
	}
	
	fclose(fp_tf);
	fclose(fp_tsu);
	fclose(fp_lsu);
	
	return 0;
}

int add_log_source_url(char *tmp_source_url, char *log_source_url)
{
	FILE *fp_tsu, *fp_lsu;
	char line[LINE_LEN];
	
	fp_tsu = xm_fopen(stderr, tmp_source_url, "r");
	if(fp_tsu == NULL)
	{
		return -1;
	}
	fp_lsu = xm_fopen(stderr, log_source_url, "a");
	if(fp_lsu == NULL)
	{
		return -1;
	}
	
	while(fgets(line, LINE_LEN, fp_tsu) != NULL)
	{
		fprintf(fp_lsu, "%s", line);
	}
	
	fclose(fp_tsu);
	fclose(fp_lsu);
	
	return 0;
}

int get_down_url(char *tmp_source_url, char *tmp_down_url)
{
	FILE *fp_tsu, *fp_tdu, *fp_tmp;
	char *tmp = "tmp";
	char line[LINE_LEN], line_tmp[LINE_LEN], url[URL_LEN], title[TITLE_LEN];
	char *query_url = "http://www.flvcd.com/parse.php?kw=";
	char *cmd;
	int len_line, len_qry, len_tmp;
	char *p_clip_start, *p_clip_end;
	char *tag_clipurl = "var clipurl = ";
	char *tag_cliptitle = "var cliptitle = ";
	
	fp_tsu = xm_fopen(stderr, tmp_source_url, "r");
	if(fp_tsu == NULL)
	{
		return -1;
	}
	fp_tdu = xm_fopen(stderr, tmp_down_url, "w");
	if(fp_tdu == NULL)
	{
		return -1;
	}
	
	len_qry = strlen(query_url);
	len_tmp = strlen(tmp);
	while(fgets(line, LINE_LEN, fp_tsu) != NULL)
	{
		len_line = strlen(line);
		if(line[len_line-1] == '\n')
		{
			line[len_line-1] = '\0';
		}
		
		cmd = xm_vsprintf_ex(len_line+len_query+len_tmp+20, "curl -o %s %s\"%s\"", tmp, query, line);
		if(cmd == NULL)
		{
			continue;
		}
		system(cmd);
		free(cmd);
		
		fp_tmp = xm_fopen(stderr, tmp, "r");
		if(fp_tmp == NULL)
		{
			continue;
		}
		while(fgets(line_tmp, LINE_LEN, fp_tmp) != NULL)
		{
			if((p_clip_start = strstr(line_tmp, tag_clipurl)) != NULL)
			{
				p_clip_start = p_clip_start + strlen(tag_clip_url);
				p_cliip_end = p_clip_start + 1;
				while((p_clip_end != '"') && (p_clip_end < &line_tmp[strlen(line_tmp)-1]))
				{
					p_clip_end++;
				}
				strncpy(url, p_clip_start, p_clip_end-p_clip_start+1);
				url[p_clip_end-p_clip_start+1] = '\0';
				
				p_clip_start = strstr(p_clip_end, tag_cliptitle);
				p_clip_start = p_clip_start + strlen(tag_cliptitle);
				p_cliip_end = p_clip_start + 1;
				while((p_clip_end != '"') && (p_clip_end < &line_tmp[strlen(line_tmp)-1]))
				{
					p_clip_end++;
				}
				strncpy(title, p_clip_start, p_clip_end-p_clip_start+1);
				title[p_clip_end-p_clip_start+1] = '\0';
				
				fprintf(fp_tdu, "%s %s\n", url, title);
				break;
			}
		}
		fclose(fp_tmp);
	}
	
	fclose(fp_tsu);
	fclose(fp_tdu);
	
	return 0;
	
}

int down_video(char *save_dir, char *tmp_down_url)
{
	char url[URL_LEN], title[TITLE_LEN], cmd[CMD_LEN], cwd[PATH_LEN];
	char *ext = ".flv";
	char line[LINE_LEN * 2];
	FILE *fp;
	char *p;
	
	fp = xm_fopen(stderr, tmp_down_url, "r");
	if(fp == NULL)
	{
		return -1;
	}
	
	getcwd(cwd, PATH_LEN);
	chdir(save_dir);
	while(fgets(line, LINE_LEN * 2, fp) != NULL)
	{
		sscanf(line, "%s %s", url, title);
		p = strchr(url, '?');
		*p = '\0';
		sprintf(cmd, "curl -o %s%s %s%s", title, ext, url, ext);
		system(cmd);
	}
	chdir(cwd);
	fclose(fp);
	return 0;	
}



