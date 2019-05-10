Warren Goodson
CS 450 Spring 2019
Sonoma State University
Dr. Kooshesh

Project 3 - Linux Talk

Instructions:
    Compile:
        Open two terminals
        Navigate one to /server/
        Navigate the other to /client/
        In each terminal, run make
    Run:
        For the server, run './server'
        For the client, run './client 127.0.0.1'
    Terminate:
        Type '`' in either terminal window

Features that work:
    Transmission across sockets of each character 
    New line behavior when per-line character limit reached

Features that do not work:
    New line behavior when [ENTER] key pressed
    Backspace functionality
