#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>
#include <dirent.h>
#include <arpa/inet.h>
#include <regex.h>
#include <openssl/md5.h>
#define CHUNK_SIZE 256
char commands[1000][1000];
int commands_count=0;
struct stat buf;
pid_t pid;

struct files
{
	char name[100];
	char size[100];
	char date[100];
	char type[10];
};
int decodecommand(char command[]){
	if(strcmp(command,"quit")==0)
		return 0;
	else if(strncmp(command,"IndexGet",8)==0)	
		return 1;
	else if(strncmp(command,"FileHash",8)==0)
		return 2;
	else if(strncmp(command,"FileDownload",12)==0)
		return 3;
  	else if(strncmp(command,"FileUpload",10)==0)
		return 4;
	else
		return -1;
}
void IndexGetShort(char buff[],int *socketfd){
	char *time1;
	char *time2;
	char writebuffer[1025];
	bzero(writebuffer,1025);
	struct files arr[100];
	time1 = strtok (buff," ");
	time1 = strtok (NULL," ");
	time1 = strtok (NULL," ");
	time2 = strtok (NULL," ");
	printf("%s %s\n",time1,time2);
	struct tm tm;
	strptime(time1, "%a %b %e %H:%M:%S %Y", &tm);
	time_t t1 = mktime(&tm);
	strptime(time2, "%a %b %e %H:%M:%S %Y", &tm);
	time_t t2 = mktime(&tm);
	int size;
	char n[100];
	DIR *dp;
	struct dirent *ep;
	dp = opendir("./");
	struct stat st;
	if (dp != NULL){
		int i = 0;
		while (ep = readdir (dp)){
			if(stat(ep->d_name,&st) < 0)
			{
				bzero(writebuffer,1025);
				strcpy(writebuffer,"Error occured");
				write(*socketfd,writebuffer,1025);
				break;
			}
			else if(difftime(st.st_mtime,t1) > 0 && difftime(t2,st.st_mtime) > 0){
				strcpy(n,ep->d_name);
				if(n[0]!='.'){
					strcpy(arr[i].name,n);
					strcpy(writebuffer,n);
					strcat(writebuffer,"\t");
					write(*socketfd,writebuffer,1025);
					bzero(writebuffer,1025);
					strcpy(writebuffer,(S_ISDIR(st.st_mode)) ? "d  " : "-  ");
					strcat(writebuffer,"\t");
					write(*socketfd,writebuffer,1025);
					bzero(writebuffer,1025);
					stat(arr[i].name, &st);
					size = st.st_size;
					sprintf(writebuffer, "%d", size);
					strcat(writebuffer,"\t");
					write(*socketfd,writebuffer,1025);
					bzero(writebuffer,1025);
					strcpy(writebuffer,ctime(&st.st_mtime));
					write(*socketfd,writebuffer,1025);
					bzero(writebuffer,1025);
					i++;
				}
			}
		}
	}
	else
		perror ("Couldn't open the directory");
	strcpy(writebuffer,"******FILE LIST*******");
	if(write(*socketfd,writebuffer,1025)<0){
		printf("\nERROR: Writing to socket\n");
		exit(1);
	}
}
void IndexGetReg(char buff[],int *socketfd){
	char *regexp;
	char writebuffer[1025];
	bzero(writebuffer,1025);
	regex_t regex;
	int reti;
	int size;
	char reg[100];
	char n[100];
	struct files arr[100];
	regexp = strtok (buff," ");
	regexp = strtok (NULL," ");
	regexp = strtok (NULL," ");
	strcpy(reg,regexp);
	DIR *dp;
	struct dirent *ep;
	dp = opendir("./");
	struct stat st;
	if (dp != NULL){
		int i=0;
		while(ep = readdir (dp)){
			regex_t regex;
			int reti;
			char msgbuf[100];
			reti = regcomp(&regex,reg , 0);
			if( reti ){
			   fprintf(stderr, "Could not compile regex\n");
			   exit(1); 
			}
			reti = regexec(&regex, ep->d_name, 0, NULL, 0);
			if( !reti ){
				strcpy(n,ep->d_name);
				if(n[0]!='.'){
					strcpy(arr[i].name,n);
					strcpy(writebuffer,n);
					strcat(writebuffer,"\t");
					write(*socketfd,writebuffer,1025);
					bzero(writebuffer,1025);
					strcpy(writebuffer,(S_ISDIR(st.st_mode)) ? "d  " : "-  ");
					strcat(writebuffer,"\t");
					write(*socketfd,writebuffer,1025);
					bzero(writebuffer,1025);
					stat(arr[i].name, &st);
					size = st.st_size;
					sprintf(writebuffer, "%d", size);
					strcat(writebuffer,"\t");
					write(*socketfd,writebuffer,1025);
					bzero(writebuffer,1025);
					strcpy(writebuffer,ctime(&st.st_mtime));
					write(*socketfd,writebuffer,1025);
					bzero(writebuffer,1025);
					i++;
				}
			}
			else if( reti == REG_NOMATCH ){
			}
			else{
				bzero(writebuffer,1023);
				strcpy(writebuffer,"Error occured");
				printf("%s\n", writebuffer);
				write(*socketfd,writebuffer,strlen(writebuffer));
				break;
			}
			regfree(&regex);
		}
	}
	else
		perror ("Couldn't open the directory");
	strcpy(writebuffer,"******FILE LIST*******");
	if(write(*socketfd,writebuffer,1025)<0){
		printf("\nERROR: Writing to socket\n");
		exit(1);
	}
}
void IndexGetLong(int *socketfd){
	char writebuffer[1025];
	struct tm time1,time2,test;
	bzero(writebuffer,1025);
	struct files arr[100];
	struct stat st;
	int size;
	int i;
	char n[100];
	char *pch;
	bzero(n,100);
	DIR *dp;
	struct dirent *ep;     
	dp=opendir ("./");
	if (dp != NULL){
		while(ep = readdir (dp)){
			strcpy(n,ep->d_name);
			if(n[0]!='.'){
				strcpy(arr[i].name,n);
				strcpy(writebuffer,n);
				strcat(writebuffer,"\t");
				write(*socketfd,writebuffer,1025);
				bzero(writebuffer,1025);
				strcpy(writebuffer,(S_ISDIR(st.st_mode)) ? "d  " : "-  ");
				strcat(writebuffer,"\t");
				write(*socketfd,writebuffer,1025);
				bzero(writebuffer,1025);
				stat(arr[i].name, &st);
				size = st.st_size;
				sprintf(writebuffer, "%d", size);
				strcat(writebuffer,"\t");
				write(*socketfd,writebuffer,1025);
				bzero(writebuffer,1025);
				strcpy(writebuffer,ctime(&st.st_mtime));
				write(*socketfd,writebuffer,1025);
				bzero(writebuffer,1025);
				i++;
			}
		}
		(void)closedir(dp);
	}
	else
		perror ("Couldn't open the directory");
	strcpy(writebuffer,"******FILE LIST*******");
	if(write(*socketfd,writebuffer,1025)<0){
		printf("\nERROR: Writing to socket\n");
		exit(1);
	}
}
void FileHashCheckAll(char buff[],int *socketfd){
	char *filename;
	int i;
	char writebuffer[1025];
	char temp[100];
	struct files arr[100];
	bzero(writebuffer,1025);
	DIR *dp;
	struct dirent *ep;
	char n[100];
	dp = opendir("./");
	struct stat st;
	if (dp != NULL){
		i=0;
		while (ep = readdir (dp)){
			if(stat(ep->d_name,&st) < 0)
			{
				strcpy(writebuffer,"Error occured");
				write(*socketfd,writebuffer,1025);
				bzero(writebuffer,1025);
				break;
			}
			else{
				if(!S_ISDIR(st.st_mode)){
					unsigned char c[MD5_DIGEST_LENGTH];
					MD5_CTX mdContext;
					int bytes;
					FILE *file = fopen (ep->d_name, "r");
					MD5_Init (&mdContext);
					unsigned char data[1024];
					while ((bytes = fread (data, 1, 1024, file)) != 0){
					   MD5_Update (&mdContext, data, bytes);
					}
					MD5_Final (c,&mdContext);
					unsigned char hash[MD5_DIGEST_LENGTH];
					for(i = 0; i < MD5_DIGEST_LENGTH; i++){
					   sprintf(temp, "%x",c[i]);
					   strcat(writebuffer,temp);
					}
					fclose(file);
					strcat(writebuffer,"\t");
					write(*socketfd,writebuffer,1025);
					bzero(writebuffer,1025);
					//strcpy(n,ep->d_name);
				   	strcpy(writebuffer,ep->d_name);
				   	strcat(writebuffer,"\t");
				   	//write(*socketfd,writebuffer,1025);
					bzero(writebuffer,1025);
				   	sprintf(writebuffer,"%s",ctime((&st.st_mtime)));
				   	//strcat(writebuffer,"\t");
				   	write(*socketfd,writebuffer,1025);
					bzero(writebuffer,1025);
			   	}
		   	}
	   	}
	}
	else
		perror ("Couldn't open the directory");
	strcpy(writebuffer,"******FILE LIST*******");
	if(write(*socketfd,writebuffer,1025)<0){
		printf("\nERROR: Writing to socket\n");
		exit(1);
	}
}
void FileHashVerify(char buff[],int *socketfd){
	char *filename;
	int i;
	char n[100];
	char writebuffer[1025];
	char temp[100];
	bzero(writebuffer,1025);
	struct files arr[100];
	filename = strtok (buff," ");
	filename = strtok (NULL," ");
	filename = strtok (NULL," ");
	unsigned char c[MD5_DIGEST_LENGTH];
	FILE *file = fopen (filename, "r");
	MD5_CTX mdContext;
	int bytes;
	unsigned char data[1024];
	if (file == NULL) {
		strcpy(writebuffer,"No Such File Exists. Please Give A Valid File\n");
		write(*socketfd,writebuffer,1025);
		bzero(writebuffer,1025);
	}
	else{
		MD5_Init (&mdContext);
		while ((bytes = fread (data, 1, 1024, file)) != 0){
		   MD5_Update (&mdContext, data, bytes);
		}
		MD5_Final (c,&mdContext);
		unsigned char hash[MD5_DIGEST_LENGTH];
		bzero(writebuffer,1023);
		for(i = 0; i < MD5_DIGEST_LENGTH; i++){
		   sprintf(temp, "%x",c[i]);
		   strcat(writebuffer,temp);
		}
		fclose(file);
		strcat(writebuffer,"\t");
		write(*socketfd,writebuffer,1025);
		bzero(writebuffer,1025);
		DIR *dp;
		struct dirent *ep;
		dp = opendir("./");
		struct stat st;
		if (dp != NULL){
			i=0;
			while (ep = readdir (dp)){
				if(stat(ep->d_name,&st) < 0)
				{
					strcpy(writebuffer,"Error occured");
					write(*socketfd,writebuffer,1025);
					bzero(writebuffer,1025);
					break;
				}
				else{
				   	if(strcmp(filename,ep->d_name)==0){
				   		strcpy(n,ep->d_name);
					   	strcpy(writebuffer,n);
					   	strcat(writebuffer,"\t");
					   	if(write(*socketfd,writebuffer,1025) < 0){
					   		printf("\nERROR: Writing to socket\n");
							exit(1);
					   	}
						bzero(writebuffer,1025);
					   	sprintf(writebuffer,"%s",ctime((&st.st_mtime)));
					   	//strcat(writebuffer,"\t");
					   	write(*socketfd,writebuffer,1025);
						bzero(writebuffer,1025);
					   	break;
				   	}
			   	}
		   	}
		}
	}
	bzero(writebuffer,1025);
	strcpy(writebuffer,"******FILE LIST*******");
	if(write(*socketfd,writebuffer,1025)<0){
		printf("\nERROR: Writing to socket\n");
		exit(1);
	}
}
void Filehashing(char buf[],int *socketfd){
	if(strncmp(buf,"FileHash verify",15)==0)
		FileHashVerify(buf,socketfd);
	else if(strncmp(buf,"FileHash checkall",17)==0)
		FileHashCheckAll(buf,socketfd);
}
void HandleIndexGet(char buf[],int *socketfd){
	if(strncmp(buf,"IndexGet ShortList",18)==0)
		IndexGetShort(buf,socketfd);
	else if(strncmp(buf,"IndexGet LongList",17)==0)
		IndexGetLong(socketfd);
	else if(strncmp(buf,"IndexGet RegEx",14)==0)
		IndexGetReg(buf,socketfd);
}
void filedownload2(char buff[],int *socketfd){
	int n;
	n=write(*socketfd,buff,1025);
	char *filename;
	filename = strtok (buff," ");
	filename = strtok (NULL," ");
	int nbytes;
	char readbuffer[256];
	bzero(readbuffer,256);
	char buffer[CHUNK_SIZE];
	bzero(buffer,CHUNK_SIZE);
	read(*socketfd,readbuffer,256);
	if(strncmp(readbuffer,"OK",2) == 0){
		printf("Downloading\n");
		FILE *f;
		f=fopen(filename,"w");
		//printf("\nEnter Your Message: ");
		while((nbytes = read(*socketfd, buffer, CHUNK_SIZE)) > 0){
			//printf("%s\n",buffer);
			if(strncmp(buffer,"END",3) == 0){
				//printf("Here only\n");
				break;
			}
			//printf("BYTES RECIEVED %d\n",nbytes);
			else
				fwrite(buffer, 1, nbytes, f);
		}
		if(nbytes<0){
			fprintf(stderr, "Read Error\n");
		}
	}
	else{
		printf("The file doesn't exist\n");
	}
}
void filedownload(char buff[], int *socketfd){
	int i;
	char *filename;
	char writebuffer[256];
	bzero(writebuffer,256);
	filename = strtok (buff," ");
	filename = strtok (NULL," ");
	FILE *fp;
	fp=fopen(filename,"r");
	if(!fp){
		strcpy(writebuffer,"No such file Exists");
		write(*socketfd,buff,256);
	}
	else{
		printf("Sending file\n");
		strcpy(writebuffer,"OK");
		write(*socketfd,writebuffer,256);
		bzero(writebuffer,256);
		while(1){
			if(feof(fp)!=0) 
				break;
			int nread = fread(writebuffer, 1, CHUNK_SIZE, fp); 
			write(*socketfd,writebuffer,256); 
			bzero(writebuffer, 0); 
		}
		strcpy(writebuffer,"END"); 
		write(*socketfd , writebuffer , 3);
		fclose(fp);
		printf("file sent\n");
	}
}
void fileupload(char buff[], int *socketfd){
	int n;
	char *filename;
	filename = strtok (buff," ");
	filename = strtok (NULL," ");
	int nbytes;
	char readbuffer[1025];
	bzero(readbuffer,1025);
	char buffer[CHUNK_SIZE];
	bzero(buffer,CHUNK_SIZE);
	printf("Receiving the file\n");
	FILE *f;
	printf("%s\n",filename);
	f=fopen(filename,"w");
	if(!f){
		printf("not there\n");
	}
	else{
		while((nbytes = read(*socketfd, buffer, CHUNK_SIZE)) > 0){
			if(strncmp(buffer,"END",3) == 0){
				break;
			}
			fwrite(buffer, 1, nbytes, f);
		}
		if(nbytes<0){
			fprintf(stderr, "Read Error\n");
		}
		fclose(f);
		printf("File Recieved\n");
	}
}
void fileupload2(char buff[], int *socketfd){
	int i;
	char *filename;
	char temp[1025];
	char writebuffer1[1025];
	bzero(writebuffer1,1025);
	strcpy(temp,buff);
	filename = strtok (temp," ");
	filename = strtok (NULL," ");
	FILE *fp;
	fp=fopen(filename,"r");
	if(!fp){
		printf("No such file exists\n");
	}
	else{
		write(*socketfd,buff,256);
		char readbuffer[256];
		bzero(readbuffer,256);
		char writebuffer[256];
		bzero(writebuffer,256);
		printf("Upload permitted.... Uploading\n");
		while(1){
			if(feof(fp)!=0) 
				break;
			int nread = fread(writebuffer, 1, CHUNK_SIZE, fp); 
			write(*socketfd,writebuffer,256); 
			bzero(writebuffer, 256); 
		}
		strcpy(writebuffer,"END"); 
		write(*socketfd , writebuffer , 3);
		fclose(fp);
		printf("Upload Complete\n");
	}
}
void hashing2(char buf[],int *socketfd){
	int n;
	n=write(*socketfd,buf,1024);
	if (n < 0)
	{
		perror("ERROR writing to socket");
		_exit(1);
	}
	char read_buffer[1024];
	bzero(read_buffer, 1024);
	//sleep(1);
	printf("Md5hash\t\tFile\t\tLatest-Timestamp\n");
	while((read(*socketfd, read_buffer,1024)) > 0)
	{
		if(strcmp(read_buffer,"******FILE LIST*******")==0)
			break;
		else
			printf("%s",read_buffer );
	}
	printf("\n");	
}
void IndexGet2(char buf[],int *socketfd)
{
	int n,f,flag=0;
	if(strncmp(buf,"IndexGet ShortList",18)==0)
		flag=1;
	else if(strncmp(buf,"IndexGet LongList",17)==0)
		flag=0;
	else if(strncmp(buf,"IndexGet RegEx",14)==0)
		flag=2;
	n=write(*socketfd,buf,1025);
	if(n<0){
		printf("\nERROR: Writing to socket\n");
		exit(1);
	}
	char readbuffer[1025];
	bzero(readbuffer,1025);
	//printf("\nRecieved List: %s\n",readbuffer);
	sleep(1);
	if(flag==0||flag==1){
		printf("File\t\tType\t\tSize\t\tTimeStamp\n");
		while((f=read(*socketfd,readbuffer,1025))>0){
			//	printf("LISTing\n");
			if(strcmp(readbuffer,"******FILE LIST*******")==0)
				break;
			else
				printf("%s",readbuffer );
		}
	}
	else if(flag==2){
		printf("File\t\tType\t\tSize\t\tTimeStamp\n");
		while((f=read(*socketfd,readbuffer,1025))>0){
			//	printf("LISTing\n");
			if(strcmp(readbuffer,"******FILE LIST*******")==0)
				break;
			else
				printf("%s",readbuffer );
		}
	}
}
void create_client(int pno,int case1)
{
	int sockfd,n;
	struct sockaddr_in serv_addr;
	char readbuffer[1025];
	if(case1)
		sockfd=socket(AF_INET,SOCK_STREAM,0);
	else
		sockfd=socket(AF_INET,SOCK_DGRAM,0);

	if(sockfd<0)
		printf("ERROR in sockfd\n");
	else
		printf("{Client} created\n");

	serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(pno);
	serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	if(case1)
		while(connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr))<0);

	while(1){
		bzero(readbuffer,1025);
		printf("Enter the message : ");
		gets(readbuffer);
		printf("\n");
		//printf("%d\n",strcmp(readbuffer,"quit"));
		int case2 = decodecommand(readbuffer);
		if(case2==0){
			strcpy(commands[commands_count],readbuffer);commands_count++;
			n=write(sockfd,readbuffer,1025);
			if(n<0)
				printf("ERROR writing to SOCKET\n");
			printf("{Client} Ended the Connection\n" );
			kill(pid,SIGTERM);
			break;
		}
		else if(case2==1){
			strcpy(commands[commands_count],readbuffer);commands_count++;
			IndexGet2(readbuffer,&sockfd);
		}
		else if(case2==2){
			strcpy(commands[commands_count],readbuffer);commands_count++;
  			hashing2(readbuffer,&sockfd);
  		}	
		else{
			if(case2==3){
				strcpy(commands[commands_count],readbuffer);commands_count++;
	  			filedownload2(readbuffer,&sockfd);
	  		}
	  		else{
	  			if(case2==4){
					strcpy(commands[commands_count],readbuffer);commands_count++;
		  			fileupload2(readbuffer,&sockfd);
		  		}
		  		else{
					n = write(sockfd,readbuffer, 1025);
					if (n < 0)
	       				 printf("ERROR writing to socket\n");
	       		}
	       	}
       	}
	}

	close(sockfd);
	return;
}
void create_server(int pno,int case1)
{
	int listenfd=0,connfd=0,n;
	struct sockaddr_in serv_addr;
	char sendbuffer[1025],writebuffer[1025];
	if(case1)
		listenfd=socket(AF_INET,SOCK_STREAM,0);
	else
		listenfd=socket(AF_INET,SOCK_DGRAM,0);
	
	if(listenfd<0)
		printf("ERROR MAKIN THE SOCKET\n");
	else
		printf("[SERVER] SOCKET INITIALISED\n");

	memset(&serv_addr, '0', sizeof(serv_addr));
 	memset(sendbuffer, '0', sizeof(sendbuffer));
 	serv_addr.sin_family = AF_INET;    
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY); 
	serv_addr.sin_port = htons(pno);
	if(bind(listenfd, (struct sockaddr*)&serv_addr,sizeof(serv_addr))<0)
		perror("ERROR binding the socket");
	else
		printf("[Server] Socket Binded Successfully\n");
	if(listen(listenfd, 10) == -1){
		printf("Failed to listen\n");
  	}  
  	printf("[Server] Waiting for Client to connect...\n" );
  	if(case1)
		while((connfd=accept(listenfd, (struct sockaddr*)NULL ,NULL))<0);
	printf("[CONNECTED]\n");  
 	while(1){
    		bzero(writebuffer,1025);
    		bzero(sendbuffer,1025);
  		n=read(connfd,sendbuffer,1025);
  		if(n<0)
  			printf("ERROR  writing\n");
  		sendbuffer[n]='\0';
  		strcpy(writebuffer,sendbuffer);
		int case2 = decodecommand(writebuffer);
		//printf("%d\n",case2);
		if(case2 == 0){
			printf("\nFinal message: %s\n",sendbuffer);
			kill(pid,SIGTERM);
			break;
		}
		else if(case2 == 1){	
			HandleIndexGet(writebuffer,&connfd);
		}
  		else if(case2 == 2){
  			Filehashing(writebuffer,&connfd);
  		}
  		else{
  			if(case2 == 3){
	  			filedownload(writebuffer,&connfd);
	  		}
	  		else{
	  			if(case2 == 4){
		  			fileupload(writebuffer,&connfd);
		  		}
		  		else{
					printf("\nMessage from peer: %s\n",writebuffer);
				}
					
	  		}
  		}
  		while(waitpid(-1, NULL, WNOHANG) > 0);
    }
    close(connfd);
		printf("\n Connection closed by peer\n");
	close(listenfd);
	exit(0);
    return;

}
int main(){	
	int peer_no,host_no;
	int case1;
	char protocol[5];
	printf("Enter the peer socket no.: ");
	scanf("%d",&peer_no);
	printf("Enter the host socket no.: ");
	scanf("%d",&host_no);
	printf("Select a protocol between UDP and TCP");
	scanf("%s",protocol);
	if(strcmp(protocol,"UDP") == 0)
		case1 = 0;
	else
		case1 = 1;
	pid=fork();
	if(pid<0){
		printf("Error while forking\n");
		return 0;
	}
	if(pid==0)
		create_client(peer_no,case1);
	else if(pid)
		create_server(host_no,case1);
	return 0;
}

