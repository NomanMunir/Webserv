#include<unistd.h>
#include<stdio.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<iostream>



int main()
{
    pid_t pid;
    pid = fork();
    pid_t pid1 = fork();
    if(pid == 0)
    {
        
        std::cout << ("1 Child process\n") << getpid() << std::endl;
        
    }
    if(pid1 == 0)
    {
        
        std::cout << ("2 Child process\n") << getpid() << std::endl;
        
    }
    if(pid1 == 0 && pid == 0)
    {
        
        std::cout << ("Grand Child process\n") << getpid() << std::endl;
        
    }
    else
    {
      
        // sleep(1000);
        wait(nullptr);
        std::cout << ("Parent process\n") << getpid() << std::endl;
        
        
    }
    return 0;
}

// int main()
// {
//     pid_t fork(void);
// }

// using namespace std; 
  
// int main() 
// { 
//     pid_t c_pid = fork(); 
  
//     if (c_pid == -1) { 
//         perror("fork"); 
//         exit(EXIT_FAILURE); 
//     } 
//     else if (c_pid > 0) { 
//         //  wait(nullptr); 
//         cout << "printed from parent process " << getpid() 
//              << endl; 
//     } 
//     else { 
//         cout << "printed from child process " << getpid() 
//              << endl; 
//     } 
  
//     return 0; 
// }


// #include <iostream>
// #include <sys/types.h>
// #include <unistd.h>
// #include <sys/wait.h>
// #include <stdlib.h>

// int main() {
//     pid_t pid1, pid2;

//     pid1 = fork(); // First fork

//     if (pid1 < 0) {
//         std::cerr << "First fork failed" << std::endl;
//         return 1;
//     }

//     pid2 = fork(); // Second fork

//     if (pid2 < 0) {
//         std::cerr << "Second fork failed" << std::endl;
//         return 1;
//     }

//     if (pid1 == 0 && pid2 == 0) {
//         // This is one of the grandchildren processes
//         std::cout << "Grandchild process (PID: " << getpid() << ")" << std::endl;
//     } else if (pid1 == 0 && pid2 > 0) {
//         // This is one of the children processes
//         std::cout << "Child process 1 (PID: " << getpid() << ")" << std::endl;
//         wait(nullptr); // Wait for its child to finish
//     } else if (pid1 > 0 && pid2 == 0) {
//         // This is one of the children processes
//         std::cout << "Child process 2 (PID: " << getpid() << ")" << std::endl;
//         wait(nullptr); // Wait for its child to finish
//     } else {
//         // This is the parent process
//         std::cout << "Parent process (PID: " << getpid() << ")" << std::endl;
//         wait(nullptr); // Wait for its children to finish
//     }

//     return 0;
// }
