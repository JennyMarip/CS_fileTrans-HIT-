#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>

#define BUFFER_SIZE       2048
DIR *opendir(const char *name);
struct dirent *readdir(DIR *dirp);
int closedir(DIR *dirp);
//打印目录下所有文件名
void print_fileName(char *path)
{
    DIR *dir = opendir(path);//打开目录文件
    struct dirent *entry;
    while((entry = readdir(dir))!=0)
    {
        if(!strcmp(".",entry->d_name)||!strcmp("..",entry->d_name))continue;
        printf("%s\n",entry->d_name);
    }
}
int main() {
    printf("================================\n");
    while(1){
        printf("欢迎来到文件传送系统，请选择需要的功能:\n");
        printf("send:         向服务器传送一个文件\n");
        printf("download:     从服务器下载一个文件\n");
        printf("quit:         退出程序\n");
        printf("================================\n");
        printf("请输入操作指令:");
        char input[8];
        scanf("%s",input);
        if(!strcmp("send",input)){
            char fileName[20];
            printf("目录中可上传的文件有:\n");
            print_fileName("/usr/lab1");
            printf("请输入要传送的文件名:\n");
            scanf("%s",fileName);
            getchar();
            fileName[19]=0;
            int client_s=socket(AF_INET,SOCK_STREAM,0);
            struct sockaddr_in client_sockAddr={};
            client_sockAddr.sin_port=htons(8080);
            client_sockAddr.sin_family=AF_INET;
            client_sockAddr.sin_addr.s_addr=inet_addr("192.168.222.133");
            int con_rv=connect(client_s,(struct sockaddr*)&client_sockAddr,sizeof(client_sockAddr));
            //建立tcp三次握手
            int r=send(client_s,fileName,(int)strlen(fileName),0);
            if(r>0){
                printf("文件名发送就绪......\n");
            }
            FILE * fp=NULL;
            int file_size;
            char *fullName=(char*) malloc(strlen("/usr/lab1/")+ strlen(fileName));
            strcpy(fullName,"/usr/lab1/");
            strcat(fullName,fileName);//fullName是带着路径的文件名
            fp=fopen(fullName,"rb");
            if(!fp){
                printf("file name error!\n");
                continue;
            }
            fseek(fp, 0, SEEK_END);//把文件内容指针定位到文件末尾
            file_size = ftell(fp);//返回文件内容指针到文件头的长度
            fseek(fp, 0, SEEK_SET);//重置文件内容指针，即重新定位到头部
            r=send(client_s,(char*)&file_size,4,0);
            if(r>0){
                printf("文件大小发送就绪......\n");
            }

            char buffer[BUFFER_SIZE];
            while(1){
                memset(buffer,0,BUFFER_SIZE);
                r=(int)fread(buffer,1,2048,fp);
                if(r>0){
                    send(client_s,buffer,r,0);
                }else{
                    break;
                }
            }
            printf("传输中...100%\n");
            printf("传输结束！\n");
            fflush(fp);
            fclose(fp);
        }else if(!strcmp("download",input)){
            int client_s=socket(AF_INET,SOCK_STREAM,0);
            struct sockaddr_in client_sockAddr={};
            client_sockAddr.sin_port=htons(8080);
            client_sockAddr.sin_family=AF_INET;
            client_sockAddr.sin_addr.s_addr=inet_addr("192.168.222.133");
            int con_rv=connect(client_s,(struct sockaddr*)&client_sockAddr,sizeof(client_sockAddr));
            //建立tcp三次握手
            int r=send(client_s,"download",(int)strlen("download"),0);
            printf("服务器中的文件列表:\n");
            char buff[1024]={};
            read(client_s,buff,sizeof(buff));
            printf("%s",buff);
            fflush(stdout);
            printf("请输入需要下载的文件名:");
            char fileName[20]={};
            scanf("%s",fileName);
            fileName[19]=0;
            send(client_s,fileName,(int)strlen(fileName),0);
            int file_size;
            r=read(client_s,(char*)&file_size,4);
            printf("接收到文件大小为:%d\n",file_size);
            FILE* fp=NULL;
            int count=0;
            char *fullName=(char*) malloc(strlen("/usr/lab1/")+ strlen(fileName));
            strcpy(fullName,"/usr/lab1/");
            strcat(fullName,fileName);//fullName是带着路径的文件名
            fp=fopen(fullName,"wb");
            if(!fp){
                printf("文件打开失败\n");
                fflush(stdout);
            }
            while(1){
                memset(buff,0,1024);
                r=recv(client_s,buff,1024,0);
                if(r>0){
                    count+=r;
                    fwrite(buff,1,r,fp);
                    if(count==file_size)break;
                }
            }
            printf("文件下载成功！\n");
            fflush(fp);
            fclose(fp);
        }else if(!strcmp("quit",input)){
            break;
        }else{
            printf("请重新输入！\n");
        }
    }
    return 0;
}
