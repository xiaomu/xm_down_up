#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "xm_cmn_util.h"

#define trace_fun() {if(TRACE_FUN) fprintf(stderr, "-- %s\n", __FUNCTION__);}
#define TRACE_FUN 1

#define CMD_LEN 1024
#define PATH_LEN 1024
#define LINE_LEN 1024
#define URL_LEN 1024
#define TITLE_LEN 200

int parse_source_url(char *tmp_ukzx, char *tmp_source_url);
int filter_source_url(char *tmp_source_url, char *log_source_url);
int add_log_source_url(char *tmp_source_url, char *log_source_url);
int get_down_url(char *tmp_source_url, char *tmp_down_url);
int down_video(char *save_dir, char *tmp_down_url);
int wget_real_url(char *url, char *real_url, int len);
int gbk2utf(char *gbk_str, char *utf_str, int len);

int main(int argc, char *argv[])
{
	char *tmp_ukzx = "tmp_ukzx";
	//char *url = "http://index.youku.com/protop/6";
	char *url = "http://index.youku.com/prankdetail/6_0_3";
	char *tmp_source_url = "tmp_source_url";
	char *tmp_down_url = "tmp_down_url";
	char *log_source_url = "log_source_url";
	char cmd[CMD_LEN];
	char *save_dir;
	
	trace_fun();
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

#if 1	
	sprintf(cmd, "curl -o %s %s", tmp_ukzx, url);
	system(cmd);
	parse_source_url(tmp_ukzx, tmp_source_url);
	filter_source_url(tmp_source_url, log_source_url);
	add_log_source_url(tmp_source_url, log_source_url);
	get_down_url(tmp_source_url, tmp_down_url);
#endif
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
	
	trace_fun();
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
					fprintf(fp_src, "%s\n", url);
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
	int len_line_tsu, len_line_lsu;
	int flag;
	
	trace_fun();
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
	
	trace_fun();
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
	
	trace_fun();
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
		
		cmd = xm_vsprintf_ex(len_line+len_qry+len_tmp+20, "curl -o %s %s\"%s\"", tmp, query_url, line);
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
				p_clip_start = p_clip_start + strlen(tag_clipurl);
				p_clip_end = p_clip_start + 1;
				while((*p_clip_end != '"') && (p_clip_end < &line_tmp[strlen(line_tmp)-1]))
				{
					p_clip_end++;
				}
				strncpy(url, p_clip_start, p_clip_end-p_clip_start+1);
				url[p_clip_end-p_clip_start+1] = '\0';
				
				p_clip_start = strstr(p_clip_end, tag_cliptitle);
				p_clip_start = p_clip_start + strlen(tag_cliptitle);
				p_clip_end = p_clip_start + 1;
				while((*p_clip_end != '"') && (p_clip_end < &line_tmp[strlen(line_tmp)-1]))
				{
					p_clip_end++;
				}
				strncpy(title, p_clip_start, p_clip_end-p_clip_start+1);
				title[p_clip_end-p_clip_start+1] = '\0';
				
				fprintf(fp_tdu, "%s %s\n", url, title);
				//fwrite(title, p_clip_end-p_clip_start+1, 1, fp_tdu);
				//fprintf(fp_tdu, "\n");
				break;
			}
		}
		fclose(fp_tmp);
		
		//break;
	}
	
	fclose(fp_tsu);
	fclose(fp_tdu);
	
	return 0;
	
}

int down_video(char *save_dir, char *tmp_down_url)
{
	char url[URL_LEN], real_url[URL_LEN], title[TITLE_LEN], utf_title[TITLE_LEN], cmd[CMD_LEN], cwd[PATH_LEN];
	//char *ext = ".flv";
	char line[LINE_LEN * 2];
	FILE *fp;
	char *p_start, *p_end;
	
	trace_fun();
	fp = xm_fopen(stderr, tmp_down_url, "r");
	if(fp == NULL)
	{
		return -1;
	}
	
	getcwd(cwd, PATH_LEN);
	chdir(save_dir);
	while(fgets(line, LINE_LEN * 2, fp) != NULL)
	{
		p_start = &line[0];
		p_end = strchr(p_start+1, '"');
		strncpy(url, p_start, p_end-p_start+1);
		url[p_end-p_start+1] = '\0';
		p_start = strchr(p_end+1, '"');
		p_end = strrchr(line, '"');
		strncpy(title, p_start, p_end-p_start+1);
		title[p_end-p_start+1] = '\0';
		
		if(wget_real_url(url, real_url, URL_LEN) != 0)
		{
			printf("wget_real_url failed\n");
			return -1;
		}
		if(gbk2utf(title, utf_title, TITLE_LEN) != 0)
		{
			printf("gbk2utf failed\n");
			return -1;
		}
		sprintf(cmd, "curl -o %s \"%s\"", utf_title, real_url);
		printf("cmd: %s\n", cmd);
		system(cmd);
	}
	chdir(cwd);
	fclose(fp);
	return 0;	
}

int wget_real_url(char *url, char *real_url, int len)
{
		char *tmp_wget = "tmp_wget", *tmp_no = "tmp_no";
		char cmd[CMD_LEN], line[LINE_LEN];
		char *p_start, *p_end;
		char *tag_start = "http://", *tag_end = ".flv";
		FILE *fp;
		
		sprintf(cmd, "wget -O %s %s > %s 2>&1", tmp_no, url, tmp_wget);
		system(cmd);
		fp = xm_fopen(stderr, tmp_wget, "r");
		if(fp == NULL)
		{
			return -1;
		}
		while(fgets(line, LINE_LEN, fp) != NULL)
		{
			if(((p_start = strstr(line, tag_start)) != NULL) && ((p_end = strstr(line, tag_end)) != NULL))
			{
				strncpy(real_url, p_start, p_end-p_start+4);
				if(len > p_end - p_start + 4)
				{
					real_url[p_end - p_start + 4] = '\0';
				}
				else
				{
					printf("size is too small\n");
					return -1;
				}
				break;
			}
		}
		
		fclose(fp);
		return 0;		
}

int gbk2utf(char *gbk_str, char *utf_str, int len)
{
	char *tmp_gbk = "tmp_gbk", *tmp_utf = "tmp_utf";
	char cmd[CMD_LEN];
	FILE *fp;
	int ret;
	
	trace_fun();
	fp = xm_fopen(stderr, tmp_gbk, "w");
	if(fp == NULL)
	{
		return -1;
	}
	fprintf(fp, "%s", gbk_str);
	fclose(fp);
	
	sprintf(cmd, "iconv -f \"gbk\" -t \"utf-8\" %s > %s", tmp_gbk, tmp_utf);
	system(cmd);
	
	fp = xm_fopen(stderr, tmp_utf, "r");
	if(fp == NULL)
	{
		return -1;
	}
	ret = fread(utf_str, sizeof(char), len-1, fp);
	utf_str[ret] = '\0';
	fclose(fp);
	
	return 0;
}




