#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>

#include "url_encode.h"

#define PATH_LEN 1024
#define CMD_LEN 1024
#define IP_ID_LEN 100
#define CHANNEL_LEN 100
#define PATH_LEN 1024

//#define GBK

char *get_name(char *name_ext);
int get_upload_id(char *ip_id, char *upload_id);
int get_upload_ip(char *ip_id, char *upload_ip);
int is_up_ext(char *ext);
char *strcat_ex(const char *str, const char *str2);
int up_class_dir(char *dir);
int up_main_dir(char *dir);
char *gbk_utf8(char *str);
int get_channel_name(char *channel_name, int len);

static char *up_exts[] = 
{
	"rmvb",
	"avi",
	"mp4",
	"flv",
	NULL
};

static char *user_ip = NULL;
static char *appkey = NULL;
static char *token = NULL;

int main(int argc, char *argv[])
{
	int ch;
	int flag;
	char *dir;

	flag = 0;
	while((ch = getopt(argc, argv, "d:i:a:t:")) != -1)
	{
		
		switch(ch)
		{
			case 'd':
				dir = optarg;
				flag ++;
				break;
			case 'i':
				user_ip = optarg;
				flag ++;
				break;
			case 'a':
				appkey = optarg;
				flag ++;
				break;
			case 't':
				token = optarg;
				flag ++;
				break;
		}

	}
	if(flag < 4)
	{
		printf("%s -d dir_name -i user_ip -a appkey -t token\n", argv[0]);
		return -1;
	}

	up_main_dir(dir);
	return 0;
}

int up_main_dir(char *dir)
{
	int i;
	char cwd[PATH_LEN];
	DIR *dir_t;
	struct dirent *ptr = NULL;

	getcwd(cwd, PATH_LEN);
	chdir(dir);
	
	if((dir_t = opendir("./")) == NULL)
	{
		printf("opendir %s failed\n", "./");
		return -1;
	}
	while((ptr = readdir(dir_t)) != NULL)
	{
		if(ptr->d_type == DT_DIR)
		{
			i = atoi(ptr->d_name);
			if((i>=1) && (i <= 22))
			{
				up_class_dir(ptr->d_name);
			}
		}
	}
	closedir(dir_t);	
	chdir(cwd);

	return 0;
}

int up_class_dir(char *class_name)
{
	int i;
	char cwd[PATH_LEN];
	DIR *dir_t;
	struct dirent *ptr = NULL;

	getcwd(cwd, PATH_LEN);
	if(chdir(class_name) != 0)
	{
		printf("chdir %s failed\n", class_name);
		return -1;
	}
	
	if((dir_t = opendir("./")) == NULL)
	{
		printf("opendir %s failed\n", "./");
		return -1;
	}
	while((ptr = readdir(dir_t)) != NULL)
	{
		if(ptr->d_type == DT_DIR)
		{
			up_channel_dir(ptr->d_name);
		}
	}
	closedir(dir_t);	
	chdir(cwd);

	return 0;
}

int up_channel_dir(char *channel_name)
{
	char cmd[CMD_LEN] = {'\0'};
	char *file_name;
	char upload_ip[16];
	char upload_id[30];
	char *step1_out = "step1_out";
	FILE *fp_step;
	char ip_id[IP_ID_LEN] = {'\0'};
	char *up_bak;
	char *up_suffix = ".up";
	char u_file_name[PATH_LEN * 3]; // url_encode filename
	char u_channel_name[CHANNEL_LEN * 3]; // url encoded channel name
	char *utf_en;
	char cwd[PATH_LEN];
	DIR *dir_t;
	struct dirent *ptr = NULL;

	getcwd(cwd, PATH_LEN);
	if(chdir(channel_name) != 0)
	{
		printf("chdir %s failed\n", channel_name);
		return -1;
	}
	
	/*
	if(get_channel_name(channel_name, CHANNEL_LEN) != 0)
	{
		printf("get_channel_name failed\n");
		return -1;
	}
	if(channel_name[strlen(channel_name) -1]== '\n')
	{
		channel_name[strlen(channel_name)-1] = '\0';
	}
	*/
	
	if((dir_t = opendir("./")) == NULL)
	{
		printf("opendir %s failed\n", "./");
		return -1;
	}
	while((ptr = readdir(dir_t)) != NULL)
	{
		if(ptr->d_type == DT_REG)
		{
			if((file_name = get_name(ptr->d_name)) != NULL)
			{
#ifdef GBK				
				utf_en = gbk_utf8(channel_name);
				URLEncode(utf_en, strlen(utf_en), u_channel_name, 3*CHANNEL_LEN);
				free(utf_en);
				utf_en = gbk_utf8(file_name);
				URLEncode(utf_en, strlen(utf_en), u_file_name, 3 * PATH_LEN);
				free(utf_en);
#endif		

#ifndef GBK
				URLEncode(channel_name, strlen(channel_name), u_channel_name, 3*CHANNEL_LEN);
				URLEncode(file_name, strlen(file_name), u_file_name, 3 * PATH_LEN);
#endif
				sprintf(cmd,
					"curl --data \"title=%s&class_id=%s&channel_name=%s&user_ip=%s&to_client=1&appkey=%s&token=%s\" \"http://api.open.pps.tv/video.php\" > %s", 
					u_file_name,
					dir,
					u_channel_name,
					user_ip,
					appkey,
					token,
					step1_out);
				printf("%s\n", cmd);
				system(cmd);
				fp_step = fopen(step1_out, "r");
				if(fp_step == NULL)
				{
					printf("fopen %s failed\n", step1_out);
					return -1;
				}
				fgets(ip_id, IP_ID_LEN, fp_step);
				ip_id[strlen(ip_id)-1] = '\0';
				if(strlen(ip_id) == 0)
				{
					printf("step1 failed\n");
					return -1;
				}
				fclose(fp_step);
				get_upload_ip(ip_id, upload_ip);
				get_upload_id(ip_id, upload_id);
				
				sprintf(cmd,
					"curl --form \"file=@%s\" --form-string upload_id=%s \"http://%s/ugc/upload\"",
					ptr->d_name,
					upload_id,
					upload_ip
					);
				//printf("uploading %s ... \n", ptr->d_name);
				system(cmd);


				up_bak = strcat_ex(ptr->d_name, up_suffix);
				rename(ptr->d_name, up_bak);

				free(file_name);
				free(up_bak);
			}
		}
	}

	closedir(dir_t);
	chdir(cwd);
	return 0;
}

char *get_name(char *name_ext)
{
	char *last_dot;
	char *ext;
	int len;
	char *name;
	int name_len;

	len = strlen(name_ext);
	last_dot = strrchr(name_ext, '.');
	if(last_dot == NULL)
	{
		return NULL;
	}
	if(last_dot == &name_ext[len-1])
	{
		return NULL;
	}
	ext = last_dot+1;
	if(!is_up_ext(ext))
	{
		return NULL;
	}

	name_len = last_dot - &name_ext[0];
	if(name_len == 0)
	{
		return NULL;
	}
	name = (char *)malloc((name_len+1) * sizeof(char));
	if(name == NULL)
	{
		printf("malloc failed\n");
		return NULL;
	}
	strncpy(name, name_ext, name_len);
	name[name_len] = '\0';
	
	return name;

}

int is_up_ext(char *ext)
{
	char **ptr;

	ptr = up_exts;
	while(*ptr != NULL)
	{
		if(strcmp(ext, *ptr) == 0)
		{
			return 1;
		}
		*ptr++;
	}

	return 0;
}

char *strcat_ex(const char *str, const char *str2)
{
	int len;
	char *ptr;
	
	len = strlen(str) + strlen(str2);
	ptr = (char *)malloc((len+1) * sizeof(char));
	if(ptr == NULL)
	{
		fprintf(stderr, "malloc failed.\n");
		return NULL;
	}
	memset(ptr, 0, len+1);
	strcat(ptr, str);
	strcat(ptr, str2);
	
	return ptr;
}

int get_upload_ip(char *ip_id, char *upload_ip)
{
	char *p_start, *p_end;
	int len;

	p_start = strstr(ip_id, "upload_ip");
	p_start = p_start + 12;
	p_end = strchr(p_start, '"');
	len = p_end - p_start;
	strncpy(upload_ip, p_start, len);
	upload_ip[len] = '\0';
	return 0;
}

int get_upload_id(char *ip_id, char *upload_id)
{
	char *p_start, *p_end;
	int len;

	p_start = strstr(ip_id, "upload_id");
	p_start = p_start + 12;
	p_end = strchr(p_start, '"');
	len = p_end - p_start;
	strncpy(upload_id, p_start, len);
	upload_id[len] = '\0';
	return 0;
}

char *gbk_utf8(char *str)
{
	char *tmp_gb = "tmp_gb";
	char *tmp_ut = "tmp_ut";
	char cmd[CMD_LEN] = {'\0'};
	char *utf_encode;
	FILE *fp;

	utf_encode = (char *)malloc(PATH_LEN * sizeof(char));
	if(utf_encode == NULL)
	{
		printf("malloc failed\n");
		exit(-1);
	}

	fp = fopen(tmp_gb, "w");
	if(fp == NULL)
	{
		printf("fopen %s failed\n", tmp_gb);
		exit(-1);
	}
	fprintf(fp, "%s", str);
	fclose(fp);

	sprintf(cmd, "iconv -f gbk -t utf-8 %s > %s", tmp_gb, tmp_ut);
	system(cmd);

	fp = fopen(tmp_ut, "r");
	if(fp == NULL)
	{
		printf("fopen %s failed\n", tmp_ut);
		exit(-1);
	}
	fscanf(fp, "%s", utf_encode);
	fclose(fp);
	
	return utf_encode;

}

int get_channel_name(char *channel_name, int len)
{
	char *file = "channel_name";
	FILE *fp;

	fp= fopen(file, "r");
	if(fp == NULL)
	{
		printf("fopen %s failed\n", file);
		return -1;
	}
	fgets(channel_name, len, fp);
	if(strlen(channel_name) == 0)
	{
		printf("channel_name is empty\n");
		return -1;
	}
	return 0;
}
