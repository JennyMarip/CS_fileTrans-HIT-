#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <errno.h>
#include <dirent.h>
#include <unistd.h>

DIR *opendir(const char *name);
struct dirent *readdir(DIR *dirp);
int closedir(DIR *dirp);
void get_fileName(char *path,char*desk)
{
    DIR* dir=opendir(path);
    struct dirent*entry;
    int i=1;
    while((entry= readdir(dir))!=0){
        if(strcmp(".",entry->d_name)&&strcmp("..",entry->d_name)){
            if(i==1){
                i++;
                strcpy(desk,entry->d_name);
                strcat(desk,"\n");
            }else{
                i++;
                strcat(desk,entry->d_name);
                strcat(desk,"\n");
            }
        }
    }
}
int main() {
    int port=8080;
    printf("%d\n",1);
    fflush(stdout);
    int sock=socket(AF_INET,SOCK_STREAM,0);
    printf("sock:%d\n",sock);
    fflush(stdout);
    struct sockaddr_in addr={};
    addr.sin_family=AF_INET;
    addr.sin_port=htons(port);
    addr.sin_addr.s_addr=inet_addr("192.168.222.133");
    socklen_t len=sizeof(addr);
    int b=bind(sock,(struct sockaddr*)&addr,len);
    if(b!=0){
        perror("bind");
        fflush(stdout);
    }
    listen(sock,5);
    printf("监听中...\n");
    while(1) {
        struct sockaddr_in addr_cli = {};
        int *clifd = (int *) malloc(sizeof(int));
        *clifd = accept(sock, (struct sockaddr *) &addr_cli, &len);
        printf("接收到连接请求\n");
        fflush(stdout);
        char rec_str[256]={0};
        int r= recv(*clifd,rec_str,255,0);
        if(r>0&&strcmp("download",rec_str)!=0){
            //那么就是send指令，发的文件名
            printf("接受文件:%s\n",rec_str);
            fflush(stdout);
            int file_size;
            r = recv(*clifd, (char*)&file_size, 4, 0);
            if (r > 0){
                printf("接收到文件大小：%d\n", file_size);
            }
            FILE* fp=NULL;
            int count=0;
            char buff[2048];
            char *fullName=(char*) malloc(strlen("/usr/lab1/")+ strlen(rec_str));
            strcpy(fullName,"/usr/lab1/");
            strcat(fullName,rec_str);//fullName是带着路径的文件名
            fp=fopen(fullName,"wb");
            if(!fp){
                printf("文件打开失败\n");
                fflush(stdout);
            }
            while(1){
                memset(buff,0,2048);
                r=recv(*clifd,buff,2048,0);
                if(r>0){
                    count+=r;
                    fwrite(buff,1,r,fp);
                    if(count==file_size)break;
                }
            }
            printf("接收中...100%\n");
            printf("传输结束！\n");
            fflush(fp);
            fclose(fp);
        }else{
            //download指令
            printf("收到下载请求\n");
            char buff[1024]={};
            get_fileName("/usr/lab1",buff);
            printf("%s",buff);
            write(*clifd,buff, strlen(buff));
            char required[20]={};
            recv(*clifd, required, strlen(required)-1,0);
            FILE * fp=NULL;
            int file_size;
            char *fullName=(char*) malloc(strlen("/usr/lab1/")+ strlen(required));
            strcpy(fullName,"/usr/lab1/");
            strcat(fullName,required);//fullName是带着路径的文件名
            fp=fopen(fullName,"rb");
            if(!fp){
                printf("file name error!\n");
                fflush(stdout);
                continue;
            }
            fseek(fp, 0, SEEK_END);//把文件内容指针定位到文件末尾
            file_size = ftell(fp);//返回文件内容指针到文件头的长度
            fseek(fp, 0, SEEK_SET);//重置文件内容指针，即重新定位到头部
            r=send(*clifd,(char*)&file_size,4,0);
            char buffer[1024];
            printf("开始传送文件,%s\n",required);
            while(1){
                memset(buffer,0,1024);
                r=(int)fread(buffer,1,1024,fp);
                if(r>0){
                    send(*clifd,buffer,r,0);
                } else{
                    break;
                }
            }
            printf("传输中...100%\n");
            printf("传输结束！\n");
            fflush(fp);
            fclose(fp);
        }
    }
    return 0;
}
