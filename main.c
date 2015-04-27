#include<stdio.h>
#include<stdlib.h>
#include<curses.h>
#include<string.h>
#include<unistd.h>
#include<sys/types.h>
#include<dirent.h>
#include<pwd.h>
#include<errno.h>
#include<sys/wait.h>
#include<signal.h>
#define clean() printf("\033[H\033[J")
#define ARRSIZE 1024

int total_id, pipeFound, i;
int jobs[ARRSIZE];
char commands_list[ARRSIZE][ARRSIZE];
int isBackground[ARRSIZE];
int exists;


void exit_handle(int sig) {
    int pid;
    int j;
    int jobid;
    pid = waitpid(-1, & jobid, 0);
    for (j = 0; j < total_id; j++) {
        if (jobs[j] == pid && isBackground[j] == 1) {
            jobid = j;
            break;
        }
    }
    if (j != total_id) {
        printf("[%d] %d finished %s\n", jobid, pid, commands_list[jobid]);
        jobs[jobid] = 0;
        isBackground[jobid] = 0;
    }

}

int main(int argc, char * * argv, char * * envp) {
    signal(SIGCHLD, exit_handle);
    char cmd[ARRSIZE];
    char cmd2[ARRSIZE]; // just to keep the original value of cmd intact.
    char temp_str[ARRSIZE];
    char * home_dir;
    char * commands[ARRSIZE];
    char * pch;
    extern char * * environ;
    char cwd[ARRSIZE];
    char charTotext[ARRSIZE];
    int endID, status;

    total_id = 0;


    clean();
    commands[0] = "not finished";


    while (!((strcmp(commands[0], "exit") == 0) || (strcmp(commands[0], "quit") == 0))) {
		
        printf("> ");
        while (fgets(cmd, sizeof cmd, stdin) != NULL) {
            cmd[strlen(cmd) - 1] = '\0'; // make the last character '\0'

            strcpy(cmd2, cmd); // Keep the original value of cmd in cmd2 before the mutations

            sprintf(temp_str, "%s", cmd2);
            pch = strtok(temp_str, "|");
            if (strcmp(cmd2, pch) != 0) {
                pipeFound = 1;
            } 
            else 
            {
            	pipeFound = 0;
            }

            pch = strtok(cmd, " ");
            i = 0;
            while (pch != NULL) {
                commands[i] = pch;
                if (i > 1 && strcmp(commands[0], "cd") == 0) {
                    strcpy(charTotext, pch);
                    commands[1] = strcat(commands[1], " ");
                    commands[1] = strcat(commands[1], charTotext);
                }
                pch = strtok(NULL, " ");
                i++;
            }


			//printf("\n%s\n", commands[0]);
			//printf("%s\n", commands[1]);
			//printf("%s\n", commands[2]);			
            if (commands[1] != NULL) {
                if (strcmp(commands[1], ">") == 0) {
                    freopen(commands[2], "a", stdout);
                    printf("File writing complete. \n");
                }
            }

            /* Command : where - Print the current location. */
            if (strcmp(commands[0], "where") == 0) {
                if (getcwd(cwd, sizeof(cwd)) != NULL) printf("%s\n\n", cwd);
            } else if ((strcmp(commands[0], "exit") == 0) || (strcmp(commands[0], "quit") == 0)) {
                clean();
                break;
            } else if (strcmp(commands[0], "jobs") == 0) {
                int k;
                for (k = 0; k < total_id; k++) {

                    if (jobs[k] != 0 && isBackground[k] == 1) {
                        printf("[%d] %d %s\n", k, jobs[k], commands_list[k]);
                    }

                }
                printf(" - - - \n\n");

            } else if (strcmp(commands[0], "set") == 0) {
                if (i == 2) {
                    int ret;
                    char * setHelper;
                    char variable_name[ARRSIZE];
                    char variable_value[ARRSIZE];
                    char setter[ARRSIZE];
                    int j = 0;
                    setHelper = strtok(commands[1], "=");

                    while (setHelper != NULL) {
                        if (j == 0) {
                            strcpy(variable_name, setHelper);
                        }
                        if (j == 1) {
                            strcpy(variable_value, setHelper);
                        }
                        if (j > 1) {
                            printf("Error in the input. \n\n");
                        }

                        setHelper = strtok(NULL, "=");
                        j++;
                    }

                    ret = setenv(variable_name, variable_value, 1);
                    if (ret == 0) {
                        printf("New %s has been set to %s \n\n", variable_name, variable_value);
                    } else {
                        printf("Encountered error while setting %s value.\n\n", variable_name);
                    }

                } else {
                    printf("ERROR:\tInvalid number of arguments provided.\n");
                    printf("\tLegal use: set VARIABLE=VALUE  \n\n");
                }
            } else if (strcmp(commands[0], "cd") == 0 && pipeFound==0) {
                // Only one word is present i.e. 'cd'
                if (i == 1) {
                    int checker = chdir(getenv("HOME"));
                    if (checker == 0) {
                        printf("Location : HOME.\n");
                        char * cwd;
                        cwd = getcwd(0, 0);
                        
                    } else {
                        printf("Problem loading the home directory path!\n\n");
                    }

                }
                if (i > 1) {
					
                    if (strncmp(commands[1], "/", 1) == 0) {
                        chdir(commands[1]);
                        printf("Location : ");
                        if (getcwd(cwd, sizeof(cwd)) != NULL) printf("%s\n\n", cwd);
                    } else if (strcmp(commands[1], "..") == 0) {
                        chdir("..");
                        printf("Location : ");
                        if (getcwd(cwd, sizeof(cwd)) != NULL) printf("%s\n\n", cwd);
                    } else {
                        char * changeTo;
                        if (getcwd(cwd, sizeof(cwd)) != NULL);
                        changeTo = strcat(cwd, "/");
                        changeTo = strcat(changeTo, commands[1]);
                        chdir(changeTo);

                        printf("Location : ");
                        if (getcwd(cwd, sizeof(cwd)) != NULL) printf("%s\n\n", cwd);

                    }

                }

            } else if (strcmp(cmd2, "get paths") == 0) {
                printf("PATH : %s\n", getenv("PATH"));
                printf("HOME : %s\n", getenv("HOME"));
                printf("ROOT : %s\n\n", getenv("ROOT"));

            } else if (strcmp(commands[0], "ls") == 0 && pipeFound == 0) {
                DIR * d;
                struct dirent * dir;
                d = opendir(".");
                if (d) {
                    int count;
                    while ((dir = readdir(d)) != NULL) {
                        printf("%s\n", dir-> d_name);

                    }
                    printf("\n");
                    closedir(d);

                }

            } else if (strcmp(commands[0], "kill") == 0) {
            	if(i <3){
	            	printf("ERROR:\t\tInvalid format.\nUSE FORMAT:\tkill 1 pid\n\n");
	            }
				else{
					int get_pid, signal, wasFound, j;
					get_pid= (int) strtol(commands[2], (char **)NULL, 10);
        	        wasFound=0;
            	    for (j = 0; j < total_id; j++) {
			    	    if (jobs[j] == get_pid) {
            				if(strcmp(commands[1], "SIGABRT")==0){
            					kill(get_pid, SIGABRT);
	            				printf("Killing : %d\n", get_pid);
    	        				break;
            				}else if(strcmp(commands[1], "SIGFPE")==0){
            					kill(get_pid, SIGFPE);
	            				printf("Killing : %d\n", get_pid);
    	        				break;
            				}else if(strcmp(commands[1], "SIGILL")==0){
            					kill(get_pid, SIGILL);
	            				printf("Killing : %d\n", get_pid);
    	        				break;
            				}else if(strcmp(commands[1], "SIGINT")==0){
            					kill(get_pid, SIGINT);
	            				printf("Killing : %d\n", get_pid);
    	        				break;
            				}else if(strcmp(commands[1], "SIGSEGV")==0){
            					kill(get_pid, SIGSEGV);
	            				printf("Killing : %d\n", get_pid);
    	        				break;
            				}else if(strcmp(commands[1], "SIGTERM")==0){
            					kill(get_pid, SIGTERM);
	            				printf("Killing : %d\n", get_pid);
    	        				break;
            				}else if(strcmp(commands[1], "SIGKILL")==0){
            					kill(get_pid, SIGABRT);
	            				printf("Killing : %d\n", get_pid);
    	        				break;
            				}else{
								wasFound=1;
            					printf("ERROR:\t\tInvalid SIGNAL.\nOnly the following signals are supported :\n");
            					printf("\tSIGABRT\t: Abnormal termination\n");
								printf("\tSIGFPE\t: Floating-Point Exception.\n");
            					printf("\tSIGILL\t: Illegal Instruction\n");							
            					printf("\tSIGINT\t: Interrupt\n");
            					printf("\tSIGSEGV\t: Segmentation Violation\n");
            					printf("\tSIGTERM\t: Terminate\n");
            					printf("\tSIGKILL\t: Terminate\n\n");            					
            					            					 
            				}            				
        				}
    				}
    				if(j== total_id && wasFound==0){
	    				printf("Provided pid was not found. Process may already have ended.\n\n");
	    			}
    			}
    			
            }
             else {

                if (pipeFound == 1) {
                    int pipefd_1[2];
                    int status;
                    pid_t pid_1, pid_2;

                    if (pipe(pipefd_1) == -1) {
                        perror("pipe error \n");
                        exit(1);
                    }

					// first process
                    pid_1 = fork();
                    if (pid_1 == 0) {
                        char cmdbuf[ARRSIZE];
                        bzero(cmdbuf, ARRSIZE);
                        sprintf(cmdbuf, "%s", commands[0]);
                        dup2(pipefd_1[1], STDOUT_FILENO);

                        close(pipefd_1[0]);
                        close(pipefd_1[1]);
                        if (execvpe(cmdbuf, argv, environ) < 0) {	//Linux
                        //if (execvp(cmdbuf, argv) < 0) {	// OS X
                            fprintf(stderr, "Error executing the first process. ERROR#%d\n", errno);
                            return EXIT_FAILURE;
                        }
                    }
                    
					// second process
                    pid_2 = fork();
                    if (pid_2 == 0) {
                        char cmdbuf[ARRSIZE];
                        bzero(cmdbuf, ARRSIZE);
                        sprintf(cmdbuf, "%s", commands[2]);
                        dup2(pipefd_1[0], STDIN_FILENO);

                        close(pipefd_1[0]);
                        close(pipefd_1[1]);
                        //if (execvp(cmdbuf, argv) < 0) { // OS X
                      	if (execvpe(cmdbuf, argv, environ)  < 0) { // Linux
                            fprintf(stderr, "Error executing the second process. ERROR#%d\n", errno);
                            return EXIT_FAILURE;
                        }
                    }
					
					printf("\n");
                    close(pipefd_1[0]);
                    close(pipefd_1[1]);


					wait(NULL);

				//no special command was recognized and there was no pipe.
                } else {
                    int pid;
                    int status;
                    pid = fork();
                    // child
                    if (pid == 0) {
                        if (i > 1) {
                            if (strcmp(commands[1], "<") == 0) {
                                freopen(commands[2], "r", stdin);
                            }
                        }
                        if (strcmp(commands[i - 1], "&") == 0) {
                            commands[i - 1] = NULL;
                            //exists = execvp(commands[0], commands);	// OS X
                            exists = execvpe(commands[0], commands, environ); // Linux
                            if (exists == -1) {
                                printf("ERROR:\tcommand not recognized.\n");
                                printf("errno: %s\n", strerror(errno));
                            }
                        } else {
                            commands[i] = NULL;
                            //exists = execvp(commands[0], commands);  // OS X
                            exists = execvpe(commands[0], commands, environ); // Linux
                            if (exists == -1) {
                                printf("ERROR:\tcommand not recognized.\n");
                                printf("errno: %s\n", strerror(errno));
                            }

                        }
                        return 0; // Necessary
                    }
                    // parent
                    else {
                        if (strcmp(commands[i - 1], "&") == 0) {
                            jobs[total_id] = pid;
                            strcpy(commands_list[total_id], cmd2);
                            isBackground[total_id] = 1;
                            total_id++;
                            printf("[%d] %d running in background\n", total_id - 1, jobs[total_id - 1]);
                        } else {
                            isBackground[total_id] = 0;
                            pause();
                        }
                    }

                }
            }

            freopen("/dev/tty", "a", stdout);
            printf("> ");
            int count;
            for(count = 0; count < i; count ++)
            {
            	commands[count]= NULL;
            }
        }
        // change to user input
        freopen("/dev/tty", "r", stdin);

    }
    return 0;
}