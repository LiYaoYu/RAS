# NP_Project1_0556091
---
Write a makefile to compile noop.cpp number.cpp ..., put them into the folder /ras/bin.  
Create TCP passive socket in server.c.  
client.c line 81 82 will repeat the input commands.  
Delete client.c line 81 82.  
Implement function rasHandler in the ras_handler.c.  
  
rasHandler :    
for loop for parent to create child processes   
parent should create new child after one of the current two child processes return  
the loop times is determined by number of pipes     
    
todo : exit, printenv, setenv are implemented in parent processes   
todo : before exit, all the children processes should be closed  
todo : |N, !N can be implemented by array    
todo : error msg can be implemented by the returning value of exec()   
todo : strtok need to be implemented in parent process   

Coding Style: astyle --style=kr --indent=tab --indent-switches --suffix=none *.[ch]
