#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <dirent.h>
#include <sys/stat.h>
#include <time.h>
#include <grp.h>
#include <pwd.h>
#include <string.h>
#include <vector>
#include <termios.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <poll.h>
#include <time.h>

using namespace std;

#define _GNU_SOURCE
#define PATH_LEN 1000
#define COM_LEN 1000
#define MAX 100

static struct termios old, current;

void initTermios(int echo) 
{
  tcgetattr(0, &old); 
  current = old; 
  current.c_lflag &= ~ICANON; 
  if (echo) {
      current.c_lflag |= ECHO;
  } else {
      current.c_lflag &= ~ECHO; 
  }
  tcsetattr(0, TCSANOW, &current); 
}

void resetTermios(void) 
{
  tcsetattr(0, TCSANOW, &old);
}

char getch_(int echo) 
{
  char ch;
  initTermios(echo);
  ch = getchar();
  resetTermios();
  return ch;
}

char getch(void) 
{
  return getch_(0);
}

char getche(void) 
{
  return getch_(1);
}
pid_t fg = -1;
void sig_handler(int sig){
    char str[] = "\n";
    write(0, str, strlen(str));
    if(sig == SIGINT and fg != -1) {
        kill(fg, SIGKILL);
    }
    else if(sig == SIGTSTP and fg != -1) {
        kill(fg, SIGCONT); 
    }
    fg = -1;
}
void sigint_handler(int signo)
{
	cout<<"\n";
	return;
}

int LCSubStr(char* X, char* Y, int m, int n)
{
 
    int LCSuff[m + 1][n + 1];
    int result = 0; 
    for (int i = 0; i <= m; i++)
    {
        for (int j = 0; j <= n; j++)
        {
            if (i == 0 || j == 0)
                LCSuff[i][j] = 0;
 
            else if (X[i - 1] == Y[j - 1]) {
                LCSuff[i][j] = LCSuff[i - 1][j - 1] + 1;
                result = max(result, LCSuff[i][j]);
            }
            else
                LCSuff[i][j] = 0;
        }
    }
    return result;
}

void revsearch(char *ss)
{
	int maxl = 0;
	string mlist[10000];
	if(history_search(ss,-1)!=-1)
		cout<<current_history()->line;
	else
	{
		register HIST_ENTRY **the_list;
		register int i;

		the_list = history_list ();
		int m = strlen(ss);
		int j = 0, n, k;
		if (the_list)
		{
		    for (i=0; the_list[i]; i++)
		      {
		      	n = strlen(the_list[i]->line);
		      	k = LCSubStr(ss, the_list[i]->line, m, n);
		      	maxl = max(maxl, LCSubStr(ss, the_list[i]->line, m, n));
		      }
		    if(maxl>2)
		    { 
			    for (i=0; the_list[i]; i++)
			      {
			      	n = strlen(the_list[i]->line);
			      	if(maxl == LCSubStr(ss, the_list[i]->line, m, n))
			      	{
			      		mlist[j] = the_list[i]->line;
			      		j += 1;
			      	}
			      }
			    for (int x=0; x<j; x++)
				cout<<endl<<mlist[x];
		    } 
		    else
		    	cout<<"\nNo match for search term in history.\n";
		}
		else
			cout<<"\nNo existing history.\n";
		
		
	}	
}

void commandExec(char *com, int isMultiWatch){
	char *argv[MAX];
	int i = 0;

	argv[i] = strtok(com, " \n");
	while(argv[i] != NULL){
		i++;
		argv[i] = strtok(NULL, " \n");
	}

	execvp(argv[0], argv);
	cout<<"Runtime Error\n";

	if(isMultiWatch == 0)
		kill(getpid(), SIGTERM);
	return;
}

void streamInputOutput(char *ip, int isMultiWatch){
	int ip_ind = -1, op_ind = -1;

	for(int i = 0; ip[i] != '\0'; i++){
		if(ip[i] == '<')
			ip_ind = i;
		if(ip[i] == '>')
			op_ind = i;
	}

	char *args[MAX];
	int args_len = 0;

	args[args_len] = strtok(ip, "&<>\n");

	while(args[args_len] != NULL){
		args_len++;
		args[args_len] = strtok(NULL, "&<>\n");
	}

	if(ip_ind == -1 && op_ind == -1){
		if(args_len <= 1){
			commandExec(args[0], isMultiWatch);
			return;
		}
		else{
			cout<<"Syntax error: Unexpected Terms!\n";
			return;
		}
	}else if(ip_ind >= 0 && op_ind == -1){
		if(args_len == 2){
			char *file[MAX];
			int i = 0;

			file[i] = strtok(args[1], " \n");

			while(file[i] != NULL){
				i++;
				file[i] = strtok(NULL, " \n");
			}

			if(i != 1){
				cout<<"Syntax error: Input File error\n";
				return;
			}

			int ip_fd = open(file[0], O_RDONLY);
			if(ip_fd < 0){
				cout<<"Syntax error: Could not open input file\n";
				return;
			}

			close(0);
			dup(ip_fd);
			close(ip_fd);
		}else{
			cout<<"Syntax error: Input File error\n";
			return;
		}
	}else if(ip_ind == -1 && op_ind >= 0){
		if(args_len == 2){
			char *file[MAX];
			int i = 0;

			file[i] = strtok(args[1], " \n");

			while(file[i] != NULL){
				i++;
				file[i] = strtok(NULL, " \n");
			}

			if(i != 1){
				cout<<"Syntax error: Output File error\n";
				return;
			}

			int op_fd = open(file[0], O_CREAT | O_WRONLY | O_TRUNC , S_IRUSR | S_IWUSR);
			if(op_fd < 0){
				cout<<"Syntax error: Could not open output file\n";
				return;
			}

			close(1);
			dup(op_fd);
			close(op_fd);
		}else{
			cout<<"Syntax error: Output File error\n";
			return;
		}
	}else if(ip_ind >= 0 && op_ind >= 0){
		if(args_len == 3){
			if(ip_ind < op_ind){
				char *file[MAX];
				int i = 0;

				file[i] = strtok(args[1], " \n");

				while(file[i] != NULL){
					i++;
					file[i] = strtok(NULL, " \n");
				}

				if(i != 1){
					cout<<"Syntax error: Input File error\n";
					return;
				}

				int ip_fd = open(file[0], O_RDONLY);
				if(ip_fd < 0){
					cout<<"Syntax error: Could not open input file\n";
					return;
				}

				i = 0;
				file[i] = strtok(args[2], " \n");

				while(file[i] != NULL){
					i++;
					file[i] = strtok(NULL, " \n");
				}

				if(i != 1){
					cout<<"Syntax error: Output File error\n";
					return;
				}

				int op_fd = open(file[0], O_CREAT | O_WRONLY | O_TRUNC , S_IRUSR | S_IWUSR);
				if(op_fd < 0){
					cout<<"Syntax error: Could not open output file\n";
					return;
				}

				close(0);
				dup(ip_fd);
				close(1);
				dup(op_fd);
				close(ip_fd);
				close(op_fd);
			}else{
				char *file[MAX];
				int i = 0;

				file[i] = strtok(args[1], " \n");

				while(file[i] != NULL){
					i++;
					file[i] = strtok(NULL, " \n");
				}

				if(i != 1){
					cout<<"Syntax error: Output File error\n";
					return;
				}

				int op_fd = open(file[0], O_CREAT | O_WRONLY | O_TRUNC , S_IRUSR | S_IWUSR);
				if(op_fd < 0){
					cout<<"Syntax error: Could not open output file\n";
					return;
				}

				i = 0;

				file[i] = strtok(args[2], " \n");

				while(file[i] != NULL){
					i++;
					file[i] = strtok(NULL, " \n");
				}

				if(i != 1){
					cout<<"Syntax error: Input File error\n";
					return;
				}

				int ip_fd = open(file[0], O_RDONLY);
				if(ip_fd < 0){
					cout<<"Syntax error: Could not open input file\n";
					return;
				}

				close(0);
				dup(ip_fd);
				close(1);
				dup(op_fd);
				close(ip_fd);
				close(op_fd);
			}
		}else{
			cout<<"Syntax error: Unexpected number of terms\n";
			return;
		}
	}else{
		cout<<"Syntax error\n";
		return;
	}
	commandExec(args[0], isMultiWatch);
	return;
}

void pipeExec(char **tokens, int n, int isMultiWatch){
	// for pipe function
	int pipes[n-1][2];

	for(int i = 0; i < n-1; i++){
		if(pipe(pipes[i]) < 0){
			cout<<"Could not make pipe for: "<<tokens[i] << "|" << tokens[i+1]<<endl;
			return;
		}
	}

	pid_t pid;
	//cout<<"In pipeExec\n";
	for(int i = 0; i < n; i++){
		pid = fork();
		if(pid == 0){
			//cout<<"In child\n";
			// if(i == 0){
			// 	dup2(pipes[i][1], 1);
			// 	close(pipes[i][1]);
			// 	streamInputOutput(tokens[i]);
			// }else if(i == n-1){
			// 	dup2(pipes[i-1][0], 0);
			// 	close(pipes[i-1][0]);
			// 	streamInputOutput(tokens[i]);
			// }else{
			// 	dup2(pipes[i-1][0], 0);
			// 	dup2(pipes[i][1], 1);
			// 	close(pipes[i-1][0]);
			// 	close(pipes[i][1]);
			// 	streamInputOutput(tokens[i]);
			// }

			if(i != 0){
				close(pipes[i-1][1]);
				dup2(pipes[i-1][0], 0);
				close(pipes[i-1][0]);
			}
			if(i != n-1){
				close(pipes[i][0]);
				dup2(pipes[i][1], 1);
				close(pipes[i][1]);
			}

			for(int j = 0; j < n-1; j++){
				close(pipes[j][0]);
				close(pipes[j][1]);
			}

			streamInputOutput(tokens[i], isMultiWatch);

			return;
		}else{fg = pid;
			usleep(10000);

			for(int j = 0; j < i; j++){
				close(pipes[j][0]);
				close(pipes[j][1]);
			}
		}
	}
	while(wait(NULL)>0);

	return;

}

// for multiWatch command
void multiWatchHandler(char *ip){
	char *coms[MAX] = {NULL};
	int ind_op = -1, ind_cl = -1;
	struct pollfd *pfds;
	//int pfds[MAX];
	//extracting list of commands
	for(int i = 0; ip[i] != '\0'; i++){
		if(ip[i] == '[')
			ind_op = i;
		if(ip[i] == ']')
			ind_cl = i;
	}

	if(ind_op == -1 || ind_cl == -1){
		cout<<"Syntax Error\n";
		return;
	}

	char com_str[COM_LEN];
	int j = 0;

	for(int i = ind_op + 1; i < ind_cl; i++)
		com_str[j++] = ip[i];
	com_str[j] = '\0';

	//cout<<com_str<<endl;
	int num_coms = 0;	
	//extracting individual commands
	coms[num_coms] = strtok(com_str, ",");

	while(coms[num_coms] != NULL){
		//cout<<coms[num_coms]<<endl;
		num_coms++;
		coms[num_coms] = strtok(NULL, ",");
	}

	pid_t pids[num_coms] = {-1};
	int num_open_fds = num_coms;
	//cout <<num_coms<<endl;
	char send_coms[MAX][COM_LEN];	// to store final commands to send to children
	pfds = (pollfd *)calloc(num_coms, sizeof(struct pollfd));

	for(int i = 0; i < num_coms; i++){
		pids[i] = fork();	// generate child process
		fg = pids[i];
		if(pids[i] == 0)	// if child process break out
			break;
		// if(pids[i] > 0){	// for parent process
		// 	char file_name[MAX] = {'\0'};
		// 	sprintf(file_name, "%s%d%s", ".temp.", (int)pids[i], ".txt");	// creating temp file

		// 	pfds[i] = open(file_name, O_CREAT, S_IRUSR | S_IWUSR);
		// 	close(pfds[i]);
		// 	//pfds[i].events = POLLIN;
		// 	char to_append[MAX] = {'\0'};
		// 	sprintf(to_append, "%s%s", " > ", file_name);
		// 	int k = 0, start = 0;
		// 	// for(int j = 0; j < MAX; j++){
		// 	// 	if(coms[i][j] == '\0')
		// 	// 		start = 1;
		// 	// 	if(start == 1)
		// 	// 		coms[i][j] = to_append[k++];
		// 	// 	if(to_append[k] == '\0')
		// 	// 		break;
		// 	// }
		// 	sprintf(send_coms[i], "%s%s", coms[i], to_append);	// generating final command for child
		// 	cout<<(int)getpid()<<" "<<send_coms[i]<<endl;
		// }
	}

	for(int i = 0; i < num_coms; i++){
		if(pids[i] == 0){	// generating tokens for execPipe in child
			char file_name[MAX] = {'\0'};
			sprintf(file_name, "%s%d%s", ".temp.", (int)getpid(), ".txt");

			int fd = open(file_name, O_CREAT, S_IRUSR | S_IWUSR);
			close(fd);

			sprintf(send_coms[i], "%s%s%s", coms[i], " > ", file_name);
			//cout<<send_coms[i]<<endl;

			char *tokens[MAX];
			int k = 0;

			tokens[k] = strtok(send_coms[i], "|");

			while(tokens[k] != NULL){
				//cout<<endl<<tokens[k];
				k++;
				tokens[k] = strtok(NULL, "|");
			}
			pipeExec(tokens, k, 1);	//sednding command to execute in child
			sleep(1);
			remove(file_name);
			kill(getpid(), SIGTERM);
			return;
		}
	}

	sleep(1);

	for(int i = 0; i < num_coms; i++){
		char file_name[MAX] = {'\0'};
		sprintf(file_name, "%s%d%s", ".temp.", (int)pids[i], ".txt");

		pfds[i].fd = open(file_name, O_RDONLY);
		if(pfds[i].fd == -1)
			cout<<"open"<<endl;
		pfds[i].events = POLLIN;
		//cout<<file_name<<endl;
	}
	int ct = 0;
	while(num_open_fds > 0){
		int ready;

		ready = poll(pfds, num_coms, -1);
		if(ready == -1){
			cout<<"poll"<<endl;
			break;
		}

		for(int i = 0; i < num_coms; i++){
			char buf[1000] = {'\0'};

			if(pfds[i].revents != 0){
				if(pfds[i].revents & POLLIN){
					ssize_t s = read(pfds[i].fd, buf, sizeof(buf));
					if(s == -1)
						cout<<"read"<<endl;
					// cout<<pids[i]<<" : "<<endl;
					// cout<<buf<<endl;
					time_t t;
					time(&t);
					if((int)s != 0){
						cout<<coms[i]<<" , "<<ctime(&t);
						cout<<"<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-"<<endl;
						cout<<buf<<endl;
						cout<<"<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-"<<endl<<endl;
					}
					if((int)s < 1000 && (int)s != 0){
						cout<<"Ending "<<coms[i]<<endl;
						if(close(pfds[i].fd) == -1)
							cout<<"close"<<endl;
						num_open_fds--;
					}
				}else {
					cout<<"Ending "<<coms[i]<<endl;
					if(close(pfds[i].fd) == -1)
						cout<<"close"<<endl;
					num_open_fds--;
				}
			}
		}
		// if(ct++ == 10)
		// 	break;
	}

	while(waitpid(-1, NULL, 0) > 0);		//waiting in parent

	// for(int i = 0; i < num_coms; i++){
	// 	char file_name[MAX];
	// 	sprintf(file_name, ".temp.", (int)pids[i], ".txt");
	// 	remove(file_name);
	// }

	return;
}

int main(){
	cout<< "Launching Shell....\n";
	struct sigaction sa;

	sa.sa_handler = sig_handler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;

	if(sigaction(SIGINT, &sa, NULL) == -1)
		cout << "Couldn't catch CTRL+C\n";
	sa.sa_handler = sigint_handler;
	if(sigaction(SIGTSTP, &sa, NULL) == -1)
		printf("Couldn't catch CTRL+Z\n");

	while(1){
	
		
		char ip[COM_LEN];
		char pwd[PATH_LEN];
		char ss[COM_LEN] = "\0";

		//cin>>ip;
		//cout<<ip;

		char *ptr;

		ptr = getcwd(pwd, sizeof(pwd));	//get absolute path of current working directory in pwd

		if(ptr == NULL){
			cout<<"Could not get path to current directory\n";
			exit(0);
		}

		cout<<pwd<<"$ ";
		int c, ip_sz = 0, flagc = 1, t;
		char ch;
		c = getche();
		if(c == 18)
		{
			cout<<"\nEnter search term: ";
			scanf("%[^\n]s", ss);
			revsearch(ss);
			ip[ip_sz] = '\n';
			ip_sz += 1;
			ip[ip_sz] = '\0';
		}
		else if(c == '\n')
		{
			ip[ip_sz] = '\n';
			ip_sz += 1;
			ip[ip_sz] = '\0';
			flagc = 0;
		}
		else
		{
			ip[ip_sz] = (char)c;
			ip_sz++;
		}
		if(flagc != 0)
		{
			while(ch = getche())
			{
				t = ch;
				if(t == 9)
				{
					vector<string> submatch;
					int maxlen = 0;
					int lenip = ip_sz-1;
					int lspace;
					string pref = "";
					for(int i = 0; i <= lenip; i++)
					{
						if(ip[i]==' ')
							lspace=i;
					}
					for(int i = lspace+1; i <= lenip; i++)
						pref += ip[i];
					struct dirent *entry;
					DIR *dir = opendir(pwd);
					if (dir != NULL){
					 while((entry = readdir(dir)) != NULL){
							string filname = string(entry->d_name);
							int i = 0;
							int n = min(filname.length(), pref.length());
							while(i<n and filname[i] == pref[i]) i++;
							if(i){
								if(i == maxlen)
									submatch.push_back(filname);
								else if(i>maxlen){
									submatch.clear();
									submatch.push_back(filname);
									maxlen = i;
								}
							}
					 }
					}
					closedir(dir);
					if(submatch.size() == 1) {
						for(int i=pref.length(); i<submatch[0].length(); i++)
						{
							ip[ip_sz] = submatch[0][i];
							ip_sz++;
						}
						ip[ip_sz] = '\n';
						ip_sz += 1;
						ip[ip_sz] = '\0';
						cout<<endl<<pwd<<"$ ";
						for(int i=0;i<ip_sz-1;i++)
							cout<<ip[i];
					    }
					    else if(submatch.size() > 1){
						cout << endl;
						int cnt = 0;
						for(string& s: submatch) {
						    cout << (++cnt) << ". " << s << ' ';
						}
						int x;
						bool stop = false;
						while(true) {
						    cout << "\nEnter the desired file no: ";
						    x = 0;
						    clearerr(stdin);
						    char* line = NULL; size_t len;
						    int sz = getline(&line, &len, stdin);
						    if(sz > 0) {
							for(int i = 0; i < sz - 1; ++i){
							    if(line[i] >= '0' and line[i] <= '9')
								x = x * 10 + (line[i] - '0');
							    else {
								x = 0; break;
							    }
							}
						    }
						    else {
							stop = true;
							cout << pwd << "$ ";
							break;
						    }
						    free(line);
						    if(x <= 0 or x > submatch.size())
							cout << "\nInvalid file no!";
						    else
							break;
						}
						if(!stop) {
							for(int i=pref.length(); i<submatch[x-1].length(); i++)
							{
								ip[ip_sz] = submatch[x-1][i];
								ip_sz++;
							}
						    ip[ip_sz] = '\n';
						    ip_sz += 1;
						    ip[ip_sz] = '\0';
						    cout << pwd << "$ ";
						    for(int i=0;i<ip_sz-1;i++)
							cout<<ip[i];
						    fflush(stdin);
						}
					    }
					continue;
					ip[ip_sz] = '\n';
					ip_sz += 1;
					ip[ip_sz] = '\0';
				}
				if(ch == '\n')
				{
					ip[ip_sz] = '\n';
					ip_sz += 1;
					ip[ip_sz] = '\0';
					break;
				}
				ip[ip_sz] = ch;
				ip_sz++;
			}
		}
		using_history();
		stifle_history(10000);
		if(ip[0]!='\n')
			add_history(ip);
		
		if(ip[0] == 'h' && ip[1] == 'i' && ip[2] == 's' && ip[3] == 't' && ip[4]=='o' && ip[5]=='r' && ip[6]=='y' && strlen(ip)==8)
		{
		  
		  register HIST_ENTRY **the_list;
		  register int i;

		  the_list = history_list ();
		  int m = history_length - 1000;
		  if (the_list)
		    for ((m < 0) ? i = 0 : i = m; the_list[i]; i++)
		      printf ("%d  %s", i + history_base, the_list[i]->line);
		  continue;
		}
		
		
		
		if(strlen(ip) >= 4){
			if(ip[0] == 'e' && ip[1] == 'x' && ip[2] == 'i' && ip[3] == 't'){
				int i = 4;
				while(ip[i] == ' ')
					i++;
				if(ip[i] == '\n'){
					cout<<"Exitting....\n";
					exit(0);
				}
			}
		}

		if(strlen(ip) >= 10){
			if(ip[0] == 'm' && ip[1] == 'u' && ip[2] == 'l' && ip[3] == 't' && ip[4] == 'i' && ip[5] == 'W' && ip[6] == 'a' && ip[7] == 't' && ip[8] == 'c' && ip[9] == 'h'){
				// int i = 10;
				// while(ip[i] != '\n')
				// 	i++;
				// cout<<"Success\n";
				multiWatchHandler(ip);
				continue;
			}
		}

		//Checking for &
		bool ampFlag = false;

		for(int i = 0; ip[i] != '\0'; i++){
			if(ip[i] == '&'){
				int j = i;
				j++;
				while(ip[j] == ' ')
					j++;
				if(ip[j] == '\n'){
					ampFlag = true;
					ip[i++] = '\n';
					ip[i] = '\0';
				}
			}
		}

		//cout<<ampFlag<<endl;

		char *tokens[MAX];
		int i = 0;

		tokens[i] = strtok(ip, "|");

		while(tokens[i] != NULL){
			//cout<<endl<<tokens[i];
			i++;
			tokens[i] = strtok(NULL, "|");
		}

		if(fork() == 0){
			sa.sa_handler = SIG_DFL;

			if(sigaction(SIGINT, &sa, NULL) == -1)
				cout << "Couldn't catch CTRL+C\n";
			if(sigaction(SIGTSTP, &sa, NULL) == -1)
				printf("Couldn't catch CTRL+Z\n");
			pipeExec(tokens, i, 0);
			break;
		}
		else
			if(ampFlag == 0)
				while(waitpid(-1, NULL, 0) > 0);
	}
	return 0;
}
