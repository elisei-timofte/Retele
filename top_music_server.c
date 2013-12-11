#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>


//adaugat comentariu

pthread_t thread;
pthread_t thread1;
int clientFD;
int sizeOfFrom;
char msg[1024];
char menuString[] = "---------- MENU ----------\n1.Sign up (s) \n2.Login (l) \n3.Exit (q)\n--------------------------";
char menuUser[] = "---------- MENU ----------\n1.Voteaza (v) \n2.Afisare top general (a)\n3.Afisare top dupa gen (g)\n4.Comenteaza melodie (c)\n5.Cauta melodie(s)\n6.Logout (l)\n--------------------------";

typedef struct thread_data{
	int fd;
	int *clientID;
	char nickname[250];
	char pass[250];
	int pos;
	char name[250];
	int terminated;
};
char *replacestr(char *string, char *sub, char *replace)
    {
              if(!string || !sub || !replace) return NULL;
              char *pos = string; int found = 0;
              while((pos = strstr(pos, sub))){
                        pos += strlen(sub);
                        found++;
              }
              if(found == 0) return NULL;
              int size = ((strlen(string) - (strlen(sub) * found)) + (strlen(replace) * found)) + 1;
              char *result = (char*)malloc(size);
              pos = string; 
         char *pos1;
              while((pos1 = strstr(pos, sub))){
                        int len = (pos1 - pos);
                        strncat(result, pos, len);
                        strncat(result, replace, strlen(replace));
                        pos = (pos1 + strlen(sub));
              }
              if(pos != (string + strlen(string)))
                        strncat(result, pos, (string - pos));
              return result;
}
char* getComm(char* song1){

	FILE* fin;
	fin = fopen("comentarii.txt","r");

	size_t len;
	char* line = NULL;
	char user[250],song[250],comment[5000];
	char output[5500];
	strcpy(output,"");


		while(getline(&line,&len,fin) != -1){
			
			char* p;
			p = strtok(line,"|");

			int pos = 0;
			while(p){
				switch(pos){
					case 0:
					strcpy(user,p);
					break;
					case 1:
					strcpy(song,p);
					break;
					case 2:
					strcpy(comment,p);
					break;
				}
				++pos;
				p = strtok(NULL,"|");
			}
			if(strcmp(song,song1) == 0){
				strcat(output,"User: ");
				strcat(output,user);
				strcat(output,"\n");
				strcat(output,"Melodie: ");
				strcat(output,song);
				strcat(output,"\n");
				strcat(output,"Comentariu: ");
				strcat(output,comment);
				strcat(output,"\n");
				
			}
		}
return replacestr(output,"\\n","\n            ");
}

char* searchSong(char* song){
	FILE* fout;

	fout = fopen("melodie.txt","r");

	size_t len;
	char* line = NULL;
	char output[1000];
	char* p;
	int l,j;
	int numberOfGendres;
	char* songAttributes[] = {"Nume","Descriere","Gen","Vot"};
	int ok = 0;

	char* pch;
		while (getline(&line,&len,fout) != -1) {
			pch = strchr(line,'|');
			int pos = pch-line+1;
			pos--;
			if (strncmp(line,song,pos) == 0) {
				ok = 1;
				p = strtok(line,"|");
				j = 0;
				l = 0;

				while (p){
			
					if (j > 2 && j < 2+numberOfGendres){ 
      				  strcat(output,songAttributes[l]);
      				  strcat(output,": ");
       				  strcat(output,p);
        			  strcat(output,"\n");
      				} else {
						if (j != 2){
      					  strcat(output,songAttributes[l]);
      					  strcat(output,": ");
     					  strcat(output,p);
      					  strcat(output,"\n");
       					 ++l;
     					 } else numberOfGendres = parseInt(p);
					}
				p = strtok(NULL,"|");
				j++;
			 	}
			}
		}
	fclose(fout);
return output;
}
int checkIfSongExists(char* song){
	FILE* fout;

	fout = fopen("melodie.txt","r");

	size_t len;
	char* line = NULL;

	int ok = 0;

	char* pch;
		while (getline(&line,&len,fout) != -1) {
			pch = strchr(line,'|');
			int pos = pch-line+1;
			pos--;
			if (strncmp(line,song,pos) == 0) ok = 1;
		}
	fclose(fout);
return ok;
}
int add_comm(char* user, char* nume, char* comm){

	if (!checkIfSongExists(nume)){
		return 0;
	}
	FILE* fout;

	fout = fopen("comentarii.txt","a");

	char buffer[1024];

	strcpy(buffer, user);
	strcat(buffer, "|");
	strcat(buffer, nume);
	strcat(buffer, "|");
	strcat(buffer, comm);
	strcat(buffer, "\n");
	
	if (fprintf(fout, "%s", buffer) == -1){
		perror("err while saving the song\n");
		return 0;
	}
	fclose(fout);

return 1;
}
void notifyUser(struct thread_data* data,char* msg){
	if(write(data->fd,msg,strlen(msg)) < 0) perror("err while responding to client\n");
}
void markVote(char* nickname,char* song){
  FILE* fout;

    fout = fopen("voturi.txt","a");
    char output[500];

    strcpy(output,nickname);
    strcat(output,"|");
    strcat(output,song);
  
   if(fprintf(fout,"%s",output) == -1) perror("err while writing to voturi.txt\n");

  fclose(fout);

}
int canIVote(char* nickname, char* melodie){
  FILE* fin;

  fin = fopen("voturi.txt","r");

  size_t len;
  char* line = NULL;

    while(getline(&line,&len,fin) != -1) {

      char* pch = strchr(line,'|');
      int position = pch - line + 1;
      char user[250],song[250];

      strncpy(user,line,position-1);
      strncpy(song,line+position,strlen(line));

      if (strcmp(user,nickname) == 0 && strcmp(song,melodie) == 0) return 0;
    }
return 1;
}
char* showTopByGendre(char* buffer,char* gen){
	
int vizitat[200];
char output[25000];
strcpy(output,"");

int i;
for (i = 1; i <= 200; ++i) vizitat[i] = 0;

  int vot;
  char maximum = -9999;

  char array[200][500];
  int k  = 0;
  char* p = strtok(buffer,"\n");

  while(p){
 	++k;
  	strcpy(array[k],p);
  	p = strtok(NULL,"\n");
  }

  char array1[200][500];
  
  int j,l,o,k1 = 0;
  for (j = 1; j <= k; ++j){
  for (i = 1; i <= k; ++i){
  		char *pch;
  		pch = strrchr(array[i],'|');
  		int last = pch-array[i]+1;
  		char *x = array[i];
  		char existingVoteStr[5];
  		strcpy(existingVoteStr,x+last);
  		vot = parseInt(existingVoteStr);
  		
  		char aux[500];
  		strcpy(aux,array[i]);

  		pch = strtok(aux,"|");
  		last = 0;
  		int ok = 0;
  		int numberOfGendres;

  		while(pch && !ok){
  			++last;
  			if(last == 3) numberOfGendres = parseInt(pch);
  			if (last >= 4 && last <= 3+numberOfGendres && strcmp(pch,gen) == 0) ok = 1;
  			pch = strtok(NULL,"|");
  		}

  		if(vot > maximum && vizitat[i] == 0 && ok){
  			maximum = vot; l = i;
  		}
  		
  }
  	if( o != l){
	strcpy(array1[j],array[l]);vizitat[l] = 1;
	k1++;
	}
	o = l;
	maximum = -9999;
}	

	char* songAttributes[] = {"Nume","Descriere","Gen","Vot"};
 	for (i = 1; i <= k1; ++i){
		p = strtok(array1[i],"|");
		j = 0;
		int numberOfGendres;
		l = 0;

		while (p){
			if (j > 2 && j < 2+numberOfGendres){ 
       			strcat(output,songAttributes[l]);
        		strcat(output,": ");
        		strcat(output,p);
        		strcat(output,"\n");
      		} else {
				if (j != 2){
       				strcat(output,songAttributes[l]);
        			strcat(output,": ");
        			strcat(output,p);
        			strcat(output,"\n");
        			++l;
      			}
				else numberOfGendres = parseInt(p);
			}
			p = strtok(NULL,"|");
			j++;
		}
	strcat(output,"---------------------\n");
 	}
free (buffer);

return output;
}
char* showTopFromVote(char* buffer){

int vizitat[200];
int i;
for (i = 1; i < 200; ++i) vizitat[i] = 0;
	
  char output[25000];

  strcpy(output,"");

  int vot;
  char maximum = -9999;

  char array[200][500];
  int k  = 0;
  char* p = strtok(buffer,"\n");
  while(p){
 	++k;
  	strcpy(array[k],p);
  	p = strtok(NULL,"\n");
  }

  char array1[200][500];
  
  int j,l;
  for (j = 1; j <= k; ++j){
  for (i = 1; i <= k; ++i){
  		char *pch;
  		pch = strrchr(array[i],'|');
  		int last = pch-array[i]+1;
  		
  		char *x = array[i];
  		char existingVoteStr[5];
  		strcpy(existingVoteStr,x+last);
  		vot = parseInt(existingVoteStr);
  		if(vot > maximum && vizitat[i] == 0){maximum = vot; l = i;}
  		
  }

	strcpy(array1[j],array[l]);vizitat[l] = 1;
	maximum = -9999;
}	

	char* songAttributes[] = {"Nume","Descriere","Gen","Vot"};
 	for (i = 1; i <= k; ++i){
		p = strtok(array1[i],"|");
		j = 0;
		int numberOfGendres;
		l = 0;
		while (p){
			
			if (j > 2 && j < 2+numberOfGendres) {
				strcat(output,songAttributes[l]);
				strcat(output,": ");
				strcat(output,p);
				strcat(output,"\n");
			}
			else {
			if (j != 2){
				strcat(output,songAttributes[l]);
				strcat(output,": ");
				strcat(output,p);
				strcat(output,"\n");
				++l;
			}
			else numberOfGendres = parseInt(p);
			}


			p = strtok(NULL,"|");
			j++;
		}
		strcat(output,"---------------------\n");
 	}

free (buffer);
return output;
}
int is_restricted(char* user){
	FILE* fin;

	fin = fopen("useri.txt","r");

	size_t len;
	char* line = NULL;

	while(getline(&line,&len,fin) != -1){
		if (strncmp(user,line,strlen(user)) == 0){
			char* p;
			p = strrchr(line,'|');
			if (line[p-line+1] != '1') return 0; 
		}
	}
	fclose(fin);
return 1;
}
int parseInt(char *ch)
{
int temp=0,neg=0;
while(*ch!=NULL)
{
    if(temp==0&&*ch=='-')neg=1;
    if(*ch>='0'&&*ch<='9')
    {
        if(temp==0)
        {
             temp=*ch-'0';
        }
        else
        {
             temp*=10;
             temp+=*ch-'0';
        }
    }
++ch;
}
if(neg==1)temp*=-1;
return temp;
}

char* getFileContent(char* fileName){

  FILE * pFile;

  long lSize;
  char * buffer;
  size_t result;

  pFile = fopen ( fileName , "rb" );
  if (pFile==NULL) {fputs ("File error",stderr); exit (1);}

  fseek (pFile , 0 , SEEK_END);
  lSize = ftell (pFile);
  rewind (pFile);

  buffer = (char*) malloc (sizeof(char)*lSize);
  if (buffer == NULL) {fputs ("Memory error",stderr); exit (2);}

  result = fread (buffer,1,lSize,pFile);
  if (result != lSize) {fputs ("Reading error",stderr); exit (3);}

  fclose (pFile);
return buffer;
}
int vote(char* song, int vot){

  char* buffer = getFileContent("melodie.txt");
  char *p;
  p = strtok(buffer,"\n");
  char array[200][500];

  int k  = 0,i;
  char aux[100];

  while(p){
 	++k;
  	strcpy(array[k],p);
  	p = strtok(NULL,"\n");
  }
  char* pch;
  int found = 0;

  for (i = 1; i <= k; ++i){

      pch = strchr(array[i],'|');
      int pos = pch-array[i]+1;
      pos--;
  	if (strncmp(array[i],song,pos) == 0){
  		found = 1;
  		pch = strrchr(array[i],'|');
  		int last = pch-array[i]+1;
  		
  		char *x = array[i];
  		char existingVoteStr[5];
  		strcpy(existingVoteStr,x+last);

  			if (vot == -1){
  				vot = parseInt(existingVoteStr);
  				vot--;
  			}
  			else{
  				vot = parseInt(existingVoteStr);
  				vot++;
  			}
  		char vote[5];
  		sprintf(vote,"%d",vot);
  		strcpy(x+last,vote);
  	}

  }

  free (buffer);

  FILE* fout;

  	fout = fopen("melodie.txt","w");

  for (i = 1; i <= k; ++i)
  	{
  		if (fprintf(fout,array[i]) == -1) perror("err while updating\n");
		if (fprintf(fout,"\n") == -1) perror("err while updating\n");
	}
 
 	fclose(fout);
return found;
}
int login_user(char* nickname, char* pass){

		FILE* fin;

		fin = fopen("useri.txt","r");

		char* buffer = NULL;
		size_t len;

		while(getline(&buffer,&len,fin) != -1){
			char* p;

			p = strtok(buffer,"|");
			int k = 0;
			int ok = 0;
			while(p){
				++k;
				switch(k){
					case 1:{
						if (strcmp(p,nickname) == 0)
						{
							ok = 1;
						}
					}
					break;
					case 2:{
						if (strcmp(p,pass) == 0 && ok)
						{
							return 1;
						}
					}
					break;
				}
				p = strtok(NULL, "|");
			}
		}
return 0;
}
char* generateCode(){

	int g = rand()%4;

	FILE* fin;
	fin = fopen("codes.txt","r");

	size_t len;
	char* line = NULL;
	int k = 0;
	while(getline(&line,&len,fin) != -1){
		++k;
		if(k == g) return line;
	}	

}
void emailCode(char* email){
	
	int pid;
	int fd[2];

	if(pipe(fd) == -1) perror("err while piping\n");

	if((pid = fork()) == -1) perror("err while creating child\n");
	else 
		if (pid == 0){
			dup2(fd[1],1);
			execlp("echo","echo",generateCode(),NULL);

		}
		else if (pid != 0){
			close(fd[1]);
			dup2(fd[0],0);
			execlp("mail","mail","-s","cod admin",email);
		}
	wait(NULL);

}

int checkCode(char* code){
	FILE* fin;
	fin = fopen("codes.txt","r");

	size_t len;
	char* line = NULL;

	while(getline(&line,&len,fin) != -1){
		if(strcmp(line,code) == 0) return 1;
	}	
return 0;
}
int checkIfRegistered(char* nickname){

	FILE* fin;
	fin = fopen("useri.txt","r");

	size_t len;
	char* line = NULL;

	while(getline(&line,&len,fin) != -1){

		char* pch = strchr(line,'|');
		char pos = pch-line;
		char name[250];
		strncpy(name,line,pos);
		if(strcmp(name,nickname) == 0) return 0;
	}	
return 1;
}
int register_user(char* nickname, char* parola, char* tip){
	
		FILE* fout;

		int outputLength = strlen(nickname)+strlen(parola)+strlen(tip)+7;
		char output[outputLength];

		strcpy(output,nickname);
		strcat(output,"|");
		strcat(output,parola);
		strcat(output,"|");
		strcat(output,tip);
		strcat(output,"|");
		strcat(output,"1");
		strcat(output,"|");
		strcat(output,"1");
		strcat(output,"\n");

		fout = fopen("useri.txt","a");
		if (fwrite (output , sizeof(char), sizeof(output), fout) <= 0){
			perror("Err while writing to file \n");
			return 0;
		}

		fclose(fout);
return 1;
}
void getNickNameAndSayHello(struct thread_data* data){
	int nameLength;

	if (data->fd < 0)
		{
			perror("err from client\n");
		}

		if ((nameLength = read(data->fd,data->name,250)) <= 0)
		{
			perror("err while reading from client\n");
		}
		else {
			strcat(msg,"Hello ");
			strcat(msg,data->name);
			if(write(data->fd,msg,sizeof(msg)) < 0) {perror("err while writing to client hello\n");}
			
			if(write(data->fd,menuString,sizeof(menuString)) < 0) {perror("err while responding to client\n");}
		}
}

char* readFromClient(struct thread_data* data, char* msg1){
	char msg[1024];

	bzero(msg,1024);
	if (data->fd < 0)
		{
			perror("err from client\n");
		}
		int bytes = read(data->fd,msg,sizeof(msg));

		if (bytes <= 0)
		{
			close(data->fd);
			*(data->clientID+data->pos) = -1;
			bzero(msg,1024);
			strcpy(msg,data->nickname);
			strcat(msg," has left the chat\n");	
			char who[] = "server";
			printf("%s",msg);
			data->terminated = -1;
			return "bout";
		}
		else if(strcmp(msg,"") == 0){
			close(data->fd);
				*(data->clientID+data->pos) = -1;
				//the client has been disconected
		} 
			else {

			printf("Message from client: %s",msg);
			printf("Client id: %d\n",data->fd);
			printf("no shit..");
			return msg;

		}
return "bout";
}

void* solve_request(void *argv){

	pthread_t id = pthread_self();
	#define data ((struct thread_data*) argv)

	if (pthread_equal(id,thread))
	{
		data->terminated = 1;

		while(data->terminated != -1){

		int bytes = write(data->fd,menuString,sizeof(menuString));
		if(bytes <= 0) {perror("err while responding to client\n");}
		
		char* response = readFromClient(data,msg);
		
			switch(response[0]){
				case 's':{
					//########################## SIGN UP ############################
					strcpy(msg,"Do you want an admin account? y/n\n");
					if(write(data->fd,msg,sizeof(msg)) < 0) {perror("err while responding to client\n");}
					response = readFromClient(data,msg);
						switch(response[0]){
							case 'y':{
								notifyUser(data,"Aveti codul de inregistrare ca admin? y/n");
								response = readFromClient(data,msg);

								switch(response[0]){
									case 'y':{

									notifyUser(data,"Va rugam introduceti codul primit prin email..");
									char* code = readFromClient(data,msg);
									if(checkCode(code)){

										char nick_name[250];
										char pass[250];

										int finish = 0;

										while(finish == 0){
										notifyUser(data,"Nickname:");
										strcpy(nick_name,readFromClient(data,msg));
										nick_name[strlen(nick_name)-1] = '\0';

										notifyUser(data,"Parola:");
										strcpy(pass,readFromClient(data,msg));
										pass[strlen(pass)-1] = '\0';

										if (checkIfRegistered(nick_name) == 0) {
											notifyUser(data,"Nickname-ul nu este dispnibil, va rugam incercati alt nickname");
										}
										else

											if(register_user(nick_name,pass,"admin") == 1){
												finish = 1;
												notifyUser(data,"Inregistrat cu succes ca admin");
									}
										}
									}
									else {

										notifyUser(data,"Codul este invalid");
									
									}
									}
									break;
									case 'n':
									{
									notifyUser(data,"Va rugam introduceti email-ul");
									char* email = readFromClient(data,msg);
									emailCode(email);
									notifyUser(data,"Veti primi in cateva momente un email cu codul generat");
									
									}
									break;
								}
							}
							break;
							case 'n':{


							char nick_name[250];
							char pass[250];

								notifyUser(data,"Nickname:");
								strcpy(nick_name,readFromClient(data,msg));
								nick_name[strlen(nick_name)-1] = '\0';

								notifyUser(data,"Parola");
								strcpy(pass,readFromClient(data,msg));
								pass[strlen(pass)-1] = '\0';

								if (checkIfRegistered(nick_name) == 0) {
								notifyUser(data,"Nickname-ul nu este disponibil, va rugam incercati altul");
								}
								else

								if(register_user(nick_name,pass,"user") == 1){
										printf("successfully registered\n");
								}
								else printf("Eroare la logare\n");
								
							}
							break;
						}
				}
				break;
				case 'l':{
					//########################## LOGIN ############################
					notifyUser(data,"User:");
									
					char user[250];
					strcpy(user,readFromClient(data,msg));

					notifyUser(data,"Parola:");
					char pass[250];
					strcpy(pass,readFromClient(data,msg));

					user[strlen(user) - 1] = '\0';
					pass[strlen(pass) - 1] = '\0';
					
					if(login_user(user,pass) == 1) {

						strcpy(data->nickname,user);
						strcpy(data->pass,pass);

						notifyUser(data,"Ati fost logat cu succes!\n");

						char answer[250];
						while(answer[0] != 'l' && data->terminated != -1){
						if(write(data->fd,menuUser,sizeof(menuUser)) < 0) perror("err while responding to client\n");
						
						strcpy(answer, readFromClient(data,msg));
							switch (answer[0]){
								case 'v':{
									//voteaza
									if (is_restricted(data->nickname) == 0){
										notifyUser(data,"Ne pare rau dar nu mai puteti vota..:(");
									} else {
										char numeMelodie[250];
										char votC[10];

										numeMelodie[strlen(numeMelodie) - 1] = '\0';
										votC[strlen(votC) - 1] = '\0';

										notifyUser(data,"Nume melodie:");
										strcpy(numeMelodie,readFromClient(data,msg));

										notifyUser(data,"Vot:");
										strcpy(votC,readFromClient(data,msg));
										
										int vot = parseInt(votC);

										if(canIVote(data->nickname,numeMelodie) == 0){
											notifyUser(data,"Nu mai poti vota aceeasi melodie :(");
										} else 
										if(vote(numeMelodie,vot) != 0){
											notifyUser(data,"Ati votat cu succes!");
											markVote(data->nickname,numeMelodie);
										} else {
											notifyUser(data,"Melodia nu a fost gasita in top :(");
										}

									}
								}
								break;
								case 'a':{
									//afiseaza top general
									char* response = showTopFromVote(getFileContent("melodie.txt"));
									if(write(data->fd,response,strlen(response)) < 0) perror("err while responding to client\n");	
								}
								break;
								case 'g':{
									//afiseaza top dupa gen
									notifyUser(data,"Introdu gen:");
									char* gen = readFromClient(data,msg);
									gen[strlen(gen)-1] = '\0';
									char buffer[25000];
									strcpy(buffer,showTopByGendre(getFileContent("melodie.txt"),gen));
									if (strcmp(buffer,"") == 0) 
										notifyUser(data,"Ne pare rau dar genul introdus nu se afla in baza de date\n");
									else 
									notifyUser(data,buffer);
								}
								break;
								case 'c':{
									//comenteaza melodie
									notifyUser(data,"Nume melodie:");
									char nume[250];
									strcpy(nume,readFromClient(data,msg));

									strcpy(msg,"Comentariu:");
									if(write(data->fd,msg,sizeof(msg)) < 0) perror("err while responding to client\n");
									
									char comment[5000];
									strcpy(comment,readFromClient(data,msg));

									nume[strlen(nume)-1] = '\0';
									strcpy(comment,replacestr(comment,"\n","\\n"));

									if(add_comm(data->nickname,nume,comment))
										notifyUser(data,"Va multumim pentru comentariu :)\n");
									else notifyUser(data,"oops..ceva nu a functionat la adaugarea comentariului :(\n poate ca melodia introdusa nu se afla in top ;)");
									

								}
								break;
								case 's':{
									//cauta melodie
									notifyUser(data,"Nume melodie:");
									char nume[250];
									strcpy(nume,readFromClient(data,msg));

									nume[strlen(nume)-1] = '\0';
									char *output = searchSong(nume);
									if(strcmp(output,"") == 0) notifyUser(data,"Melodia nu a fost gasita");
									else {
										notifyUser(data,output);
										notifyUser(data,"--------Comentarii---------\n");
										notifyUser(data,getComm(nume));
										notifyUser(data,"---------------------------\n");
									}


								}
								break;

								case 'l':{
									//logout
									strcpy(msg,"Ai fost delogat cu succes, ");
									strcat(msg,data->nickname);
									if(write(data->fd,msg,sizeof(msg)) < 0) perror("err while responding to client\n");
									{
										close(data->fd);
										*(data->clientID+data->pos) = -1;
										bzero(msg,1024);
										strcpy(msg,data->name);
										strcat(msg," has left the chat\n");	
										char who[] = "server";
										printf("%s",msg);
										data->terminated = -1;
									}
									
								}
								break;
								default:{
									strcpy(msg,"Va rugam introduceti o comanda valida!");
									if(write(data->fd,msg,sizeof(msg)) < 0) perror("err while responding to client\n");
								}
								break;

							}

						}


					} else {
						strcpy(msg,"Logare esuata!");
						if(write(data->fd,msg,sizeof(msg)) < 0) perror("err while responding to client\n");

					}
								
				}
				break;
				case 'q':{

					//########################## QUIT ############################

					notifyUser(data,"Have a nice one :)");
					close(data->fd);
					*(data->clientID+data->pos) = -1;
					bzero(msg,1024);
					strcpy(msg,data->nickname);
					strcat(msg," has left the chat\n");	
					char who[] = "server";
					printf("%s",msg);
					data->terminated = -1;
				}
				break;
			}
		}
	}

return NULL;
}
int main(int argc, char *argv[]){

	struct sockaddr_in server;
	struct sockaddr_in from;
	int sockFD;


	if ((sockFD = socket(AF_INET,SOCK_STREAM,0)) == -1)
	{
		perror("err while socketing..\n");
		return 0;
	}

	int opt = 1;
	int port = atoi(argv[1]);

	setsockopt(sockFD,SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	bzero(&server,sizeof(server));
	bzero(&from,sizeof(from));

	server.sin_family = AF_INET;
	server.sin_addr.s_addr = htonl(INADDR_ANY);
	server.sin_port = htons(port);

	if (bind(sockFD,(struct sockaddr*)&server,sizeof(struct sockaddr)) == -1)
	{
		perror("err while binding\n");
		return 0;
	}

	if (listen(sockFD,5) < 0)
	{
		perror("err while listening\n");
	}

	struct thread_data *data1;


	int clientID[100];
	int clientIterator = 0;

	while(1){
		 sizeOfFrom = sizeof(from);

		printf("waiting at port %d\n", port);

		clientFD = accept(sockFD,(struct sockaddr*)&from,&sizeOfFrom);

		data1 = (struct thread_data*)malloc(sizeof(struct thread_data));

		data1->clientID = clientID;
		data1->fd = clientFD;
		clientID[++clientIterator] = clientFD;
		data1->pos = clientIterator;

		int err = pthread_create(&thread,NULL,&solve_request,(void*) data1);

		if (err)
		{
			perror("err while creating thread\n");
			return 0;
		}
		else 
			printf("thread created successfully\n");

	}

return 0;
}