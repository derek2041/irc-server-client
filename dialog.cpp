/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:

**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QtWidgets>
#include "dialog.h"

#include <time.h>
//#include <curses.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <string>
//char * user <--- declared in dialog.h
//char * pass <--- declared in dialog.h
char * host;
char * sport;
int port;

int open_client_socket(char * host, int port) {
    // Initialize socket address structure
    struct  sockaddr_in socketAddress;

    // Clear sockaddr structure
    memset((char *)&socketAddress,0,sizeof(socketAddress));

    // Set family to Internet
    socketAddress.sin_family = AF_INET;

    // Set port
    socketAddress.sin_port = htons((u_short)port);

    // Get host table entry for this host
    struct  hostent  *ptrh = gethostbyname(host);
    if ( ptrh == NULL ) {
        perror("gethostbyname");
        exit(1);
    }

    // Copy the host ip address to socket address structure
    memcpy(&socketAddress.sin_addr, ptrh->h_addr, ptrh->h_length);

    // Get TCP transport protocol entry
    struct  protoent *ptrp = getprotobyname("tcp");
    if ( ptrp == NULL ) {
        perror("getprotobyname");
        exit(1);
    }

    // Create a tcp socket
    int sock = socket(PF_INET, SOCK_STREAM, ptrp->p_proto);
    if (sock < 0) {
        perror("socket");
        exit(1);
    }

    // Connect the socket to the specified server
    if (connect(sock, (struct sockaddr *)&socketAddress,
            sizeof(socketAddress)) < 0) {
        perror("connect");
        exit(1);
    }

    return sock;
}

#define MAX_RESPONSE (10 * 1024)
int sendCommand(char *  host, int port, char * command, char * response) {

    int sock = open_client_socket( host, port);

    if (sock<0) {
        return 0;
    }

    // Send command
    write(sock, command, strlen(command));
    write(sock, "\r\n",2);

    //Print copy to stdout
    write(1, command, strlen(command));
    write(1, "\r\n",2);

    // Keep reading until connection is closed or MAX_REPONSE
    int n = 0;
    int len = 0;
    while ((n=read(sock, response+len, MAX_RESPONSE - len))>0) {
        len += n;
    }
    response[len]=0;

    printf("response:\n%s\n", response);

    close(sock);

    return 1;
}

void
printUsage()
{
    printf("Usage: test-talk-server host port command\n");
    exit(1);
}

void Dialog::sendAction()
{
   if (currentRoom != NULL) {
     char response[MAX_RESPONSE];
     printf("Send Button\n");
     char * message = strdup(inputMessage->toPlainText().toStdString().c_str());
     printf("%s\n", message);
     inputMessage->clear();
     currentRoom = strdup(roomsList->currentItem()->text().toStdString().c_str());
     //send command (send-message, user, pass, message)
     std::string tempcmd = std::string("SEND-MESSAGE") + " " + std::string(user) + " " + std::string(pass) + " " + std::string(currentRoom) + " >>"
             + std::string(message);
     char * newtempcmd = strdup(tempcmd.c_str());
     sendCommand("localhost", 25565, newtempcmd, response);
    }
}

void Dialog::createRoomAction() {
    if (user != NULL) {
        char response[MAX_RESPONSE];
        char * roomName = strdup(inputMessage->toPlainText().toStdString().c_str());
        std::string tempcmd = std::string("CREATE-ROOM") + " " + std::string(user) + " " + std::string(pass) + " " + std::string(roomName);
        char * newtempcmd = strdup(tempcmd.c_str());
        sendCommand("localhost", 25565, newtempcmd, response);
        inputMessage->clear();
        roomsList->addItem(strdup(roomName));
    }
}

void Dialog::newUserAction()
{
    char response[MAX_RESPONSE];
    printf("New User Button\n");
    char * newAccountInfo = strdup(inputMessage->toPlainText().toStdString().c_str());
    //send command (add-user, user, pass)
    //send command (get-all-users, user, pass) (update users list)
    //sendCommand()

    char * tempUser = (char *) malloc (100);
    char * tempPass = (char *) malloc (100);
    int i = 0;
    int j = 0;
    for (i = 0; newAccountInfo[i] != '\0'; i++) {
        if (newAccountInfo[i] == ' ') {
            tempUser[i] = '\0';
            i++;
            break;
        }
        tempUser[i] = newAccountInfo[i];
    }
    while (newAccountInfo[i] != '\0') {
        tempPass[j] = newAccountInfo[i];
        j++;
        i++;
    }
    tempPass[j] = '\0';
    user = strdup(tempUser); //set current session 'user' equal to input
    pass = strdup(tempPass); //set current session 'pass' equal to input
    inputMessage->clear();

    std::string tempcmd = std::string("ADD-USER") + " " + std::string(user) + " " + std::string(pass);
    char * newtempcmd = strdup(tempcmd.c_str());
    sendCommand("localhost", 25565, newtempcmd, response);
    printf("%s\n%s\n", user, pass);

    /* Load Room List after Login */
    std::string tempcmd2 = std::string("LIST-ROOMS") + " " + std::string(user) + " " + std::string(pass);
    char * newtempcmd2 = strdup(tempcmd2.c_str());
    sendCommand("localhost", 25565, newtempcmd2, response);
    char * line = (char *) malloc (100);
    int x = 0;
    int y = 0;
    while (response[x] != '\0') {
        if (response[x] == '\n') {
           line[y] = '\0';
           x++;
           y = 0;
           roomsList->addItem(strdup(line));
        } else {
           line[y] = response[x];
           y++;
           x++;
        }
    }
    //roomsList->addItem(strdup(response));
    char response3[MAX_RESPONSE];
    std::string tempcmd3 = std::string("GET-ALL-USERS") + " " + std::string(user) + " " + std::string(pass);
    char * newtempcmd3 = strdup(tempcmd3.c_str());
    sendCommand("localhost", 25565, newtempcmd3, response3);
    char * line2 = (char *) malloc (100);
    int a = 0;
    int b = 0;
    while (response3[a] != '\0') {
        if (response3[a] == '\n') {
           line2[b] = '\0';
           a++;
           b = 0;
           usersList->addItem(strdup(line2));
        } else {
           line2[b] = response3[a];
           b++;
           a++;
        }
   }
}

void Dialog::timerAction()
{
    if (user != NULL) {
       if (currentRoom != NULL) {
         //char response[MAX_RESPONSE];
         //continuously pull messages from room if user is in a room
         allMessages->clear();
         char response3[MAX_RESPONSE];
         std::string tempcmd3 = std::string("GET-MESSAGES") + " " + std::string(user) + " " + std::string(pass) + " " + std::string("0") +
                 " " + std::string(currentRoom);
         char * newtempcmd3 = strdup(tempcmd3.c_str());
         sendCommand("localhost", 25565, newtempcmd3, response3);
         char * line2 = (char *) malloc (100);
         int a = 0;
         int b = 0;
         while (response3[a] != '\0') {
             if (response3[a] == '\n') {
                line2[b] = '\0';
                a++;
                b = 0;
                allMessages->append(strdup(line2));
             } else {
                line2[b] = response3[a];
                b++;
                a++;
             }
        }
        }



       //continuously update list of rooms regardless of in a room or not
       //NOTE: this probably breaks core functionality of the server
       /*
       roomsList->clear();
       char response8[MAX_RESPONSE];
       std::string tempcmd2 = std::string("LIST-ROOMS") + " " + std::string(user) + " " + std::string(pass);
       char * newtempcmd2 = strdup(tempcmd2.c_str());
       sendCommand("localhost", 25565, newtempcmd2, response8);
       char * line = (char *) malloc (100);
       int x = 0;
       int y = 0;
       while (response8[x] != '\0') {
           if (response8[x] == '\n') {
              line[y] = '\0';
              x++;
              y = 0;
              roomsList->addItem(strdup(line));
           } else {
              line[y] = response8[x];
              y++;
              x++;
           }
       }
        */
       if (currentRoom == NULL) {
           //GET-ALL-USERS
           usersList->clear();
           char response6[MAX_RESPONSE];
           std::string tempcmd6 = std::string("GET-ALL-USERS") + " " + std::string(user) + " " + std::string(pass);
           char * newtempcmd6 = strdup(tempcmd6.c_str());
           sendCommand("localhost", 25565, newtempcmd6, response6);
           char * line6 = (char *) malloc (100);
           int xx = 0;
           int yy = 0;
           while (response6[xx] != '\0') {
               if (response6[xx] == '\n') {
                  line6[yy] = '\0';
                  xx++;
                  yy = 0;
                  usersList->addItem(strdup(line6));
               } else {
                  line6[yy] = response6[xx];
                  yy++;
                  xx++;
               }
          }

       } else {
           //GET-USERS-IN-ROOM room
           usersList->clear();
           char response7[MAX_RESPONSE];
           std::string tempcmd7 = std::string("GET-USERS-IN-ROOM") + " " + std::string(user) + " " + std::string(pass) + " " + std::string(currentRoom);
           char * newtempcmd7 = strdup(tempcmd7.c_str());
           sendCommand("localhost", 25565, newtempcmd7, response7);
           char * line7 = (char *) malloc (100);
           int x1 = 0;
           int y1 = 0;
           while (response7[x1] != '\0') {
               if (response7[x1] == '\n') {
                  line7[y1] = '\0';
                  x1++;
                  y1 = 0;
                  usersList->addItem(strdup(line7));
               } else {
                  line7[y1] = response7[x1];
                  y1++;
                  x1++;
               }
          }
       }

    }
    /*
    printf("Timer wakeup\n");
    messageCount++;

    char message[50];
    sprintf(message,"Timer Refresh New message %d",messageCount);
    allMessages->append(message);
    */
    //std::string tempcmd = std::string("GET-MESSAGES") + " " + std::string(user) + " " + std::string(pass) + " " + std::string(currentRoom);
    //char * newtempcmd = strdup(tempcmd.c_str());
    //sendCommand("localhost", 25565, newtempcmd, response);
    //send command (get-messages, user, pass, room) (update messages list)
    //send command (get-users-in-room, user, pass) (update users list)
}

void Dialog::roomJoinAction() {
    printf("Selected a room\n");
    if (currentRoom != NULL) { //If user joins a room, forces user to leave other room if in one.

        char tmpresponse[MAX_RESPONSE]; //send the message before actually removing user, otherwise command will be denied
        std::string notification = std::string("SEND-MESSAGE") + " " + std::string(user) + " " + std::string(pass)
                + " " + std::string(currentRoom) + " " + std::string("has left the room.");
        char * newnotification = strdup(notification.c_str());
        sendCommand("localhost", 25565, newnotification, tmpresponse);


        char response9[MAX_RESPONSE];
        std::string tempcmd9 = std::string("LEAVE-ROOM") + " " + std::string(user) + " " + std::string(pass) + " " + std::string(currentRoom);
        char * newtempcmd9 = strdup(tempcmd9.c_str());
        sendCommand("localhost", 25565, newtempcmd9, response9);
    }
    usersList->clear();
    allMessages->clear();
    //room = QWidgetListItem (item name or some shit) set 'room' equal to current session room user is in
    //send command (join room, user, pass)
    //send command (get-users-in-room, user, pass) (update users list)
    //send command (get-messages, user, pass, room) (update messages list)
    char response[MAX_RESPONSE];
    currentRoom = strdup(roomsList->currentItem()->text().toStdString().c_str());
    std::string tempcmd = std::string("ENTER-ROOM") + " " + std::string(user) + " " + std::string(pass) + " " + std::string(currentRoom);
    char * newtempcmd = strdup(tempcmd.c_str());
    sendCommand("localhost", 25565, newtempcmd, response);

    char tmpresponse[MAX_RESPONSE];
    std::string notification = std::string("SEND-MESSAGE") + " " + std::string(user) + " " + std::string(pass)
            + " " + std::string(currentRoom) + " " + std::string("has joined the room.");
    char * newnotification = strdup(notification.c_str());
    sendCommand("localhost", 25565, newnotification, tmpresponse);


    char response2[MAX_RESPONSE];
    std::string tempcmd2 = std::string("GET-USERS-IN-ROOM") + " " + std::string(user) + " " + std::string(pass) + " " + std::string(currentRoom);
    char * newtempcmd2 = strdup(tempcmd2.c_str());
    sendCommand("localhost", 25565, newtempcmd2, response2);
    char * line = (char *) malloc (100);
    int x = 0;
    int y = 0;
    while (response2[x] != '\0') {
        if (response2[x] == '\n') {
           line[y] = '\0';
           x++;
           y = 0;
           usersList->addItem(strdup(line));
        } else {
           line[y] = response2[x];
           y++;
           x++;
        }
   }


    char response3[MAX_RESPONSE];
    std::string tempcmd3 = std::string("GET-MESSAGES") + " " + std::string(user) + " " + std::string(pass) + " " + std::string("0") +
            " " + std::string(currentRoom);
    char * newtempcmd3 = strdup(tempcmd3.c_str());
    sendCommand("localhost", 25565, newtempcmd3, response3);
    char * line2 = (char *) malloc (100);
    int a = 0;
    int b = 0;
    while (response3[a] != '\0') {
        if (response3[a] == '\n') {
           line2[b] = '\0';
           a++;
           b = 0;
           allMessages->append(strdup(line2));
        } else {
           line2[b] = response3[a];
           b++;
           a++;
        }
   }
}

void Dialog::roomLeaveAction() {
    printf("Left a room\n");
    usersList->clear();
    //send command (leave room, user, pass)
    //send command (get-all-users, user, pass) (update users list)

    char tmpresponse[MAX_RESPONSE];
    std::string notification = std::string("SEND-MESSAGE") + " " + std::string(user) + " " + std::string(pass)
            + " " + std::string(currentRoom) + " " + std::string("has left the room.");
    char * newnotification = strdup(notification.c_str());
    sendCommand("localhost", 25565, newnotification, tmpresponse);


    char response[MAX_RESPONSE];
    currentRoom = strdup(roomsList->currentItem()->text().toStdString().c_str());
    std::string tempcmd = std::string("LEAVE-ROOM") + " " + std::string(user) + " " + std::string(pass) + " " + std::string(currentRoom);
    char * newtempcmd = strdup(tempcmd.c_str());
    sendCommand("localhost", 25565, newtempcmd, response);


    char response12[MAX_RESPONSE];
    roomsList->clear();
    std::string tempcmd12 = std::string("LIST-ROOMS") + " " + std::string(user) + " " + std::string(pass);
    char * newtempcmd12 = strdup(tempcmd12.c_str());
    sendCommand("localhost", 25565, newtempcmd12, response12);
    char * line12 = (char *) malloc (100);
    int x12 = 0;
    int y12 = 0;
    while (response12[x12] != '\0') {
        if (response12[x12] == '\n') {
           line12[y12] = '\0';
           x12++;
           y12 = 0;
           roomsList->addItem(strdup(line12));
        } else {
           line12[y12] = response12[x12];
           y12++;
           x12++;
        }
    }



    char response2[MAX_RESPONSE];
    std::string tempcmd2 = std::string("GET-ALL-USERS") + " " + std::string(user) + " " + std::string(pass);
    char * newtempcmd2 = strdup(tempcmd2.c_str());
    sendCommand("localhost", 25565, newtempcmd2, response2);
    char * line = (char *) malloc (100);
    int x = 0;
    int y = 0;
    while (response2[x] != '\0') {
        if (response2[x] == '\n') {
           line[y] = '\0';
           x++;
           y = 0;
           usersList->addItem(strdup(line));
        } else {
           line[y] = response2[x];
           y++;
           x++;
        }
   }
   allMessages->clear();
   currentRoom = NULL;
}

Dialog::Dialog()
{
    createMenu();

    QVBoxLayout *mainLayout = new QVBoxLayout;

    // Rooms List
    QVBoxLayout * roomsLayout = new QVBoxLayout();
    QLabel * roomsLabel = new QLabel("Rooms");
    roomsList = new QListWidget();
    roomsLayout->addWidget(roomsLabel);
    roomsLayout->addWidget(roomsList);

    // Users List
    QVBoxLayout * usersLayout = new QVBoxLayout();
    QLabel * usersLabel = new QLabel("Users");
    usersList = new QListWidget();
    usersLayout->addWidget(usersLabel);
    usersLayout->addWidget(usersList);

    // Layout for rooms and users
    QHBoxLayout *layoutRoomsUsers = new QHBoxLayout;
    layoutRoomsUsers->addLayout(roomsLayout);
    layoutRoomsUsers->addLayout(usersLayout);

    // Textbox for all messages
    QVBoxLayout * allMessagesLayout = new QVBoxLayout();
    QLabel * allMessagesLabel = new QLabel("Messages");
    allMessages = new QTextEdit;
    allMessagesLayout->addWidget(allMessagesLabel);
    allMessagesLayout->addWidget(allMessages);

    // Textbox for input message
    QVBoxLayout * inputMessagesLayout = new QVBoxLayout();
    QLabel * inputMessagesLabel = new QLabel("Type your message:");
    inputMessage = new QTextEdit;
    inputMessagesLayout->addWidget(inputMessagesLabel);
    inputMessagesLayout->addWidget(inputMessage);

    // Send and new account buttons
    QHBoxLayout *layoutButtons = new QHBoxLayout;
    QPushButton * sendButton = new QPushButton("Send");
    QPushButton * newUserButton = new QPushButton("Signup/Login");
    QPushButton * createRoomButton = new QPushButton("Create Room");
    layoutButtons->addWidget(sendButton);
    layoutButtons->addWidget(newUserButton);
    layoutButtons->addWidget(createRoomButton);

    // Setup actions for buttons
    connect(sendButton, SIGNAL (released()), this, SLOT (sendAction()));
    connect(newUserButton, SIGNAL (released()), this, SLOT (newUserAction()));
    connect(roomsList, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT (roomJoinAction()));
    connect(roomsList, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT (roomLeaveAction()));
    connect(createRoomButton, SIGNAL(released()), this, SLOT (createRoomAction()));

    // Add all widgets to window
    mainLayout->addLayout(layoutRoomsUsers);
    mainLayout->addLayout(allMessagesLayout);
    mainLayout->addLayout(inputMessagesLayout);
    mainLayout->addLayout(layoutButtons);

    // Populate rooms
    /*
    for (int i = 0; i < 20; i++) {
        char s[50];
        sprintf(s,"Room %d", i);
        roomsList->addItem(s);
    }
    */
    /*
    char response[MAX_RESPONSE];
    char * temp = "#general";
    roomsList->addItem(temp);
    */
    /*
    char response[MAX_RESPONSE];
    std::string tempcmd = std::string("LIST-ROOMS") + " " + std::string(user) + " " + std::string(pass);
    char * newtempcmd = strdup(tempcmd.c_str());
    sendCommand("localhost", 25565, newtempcmd, response);
    roomsList->addItem(strdup(response));
    */
    // Populate users
    /*
    for (int i = 0; i < 20; i++) {
        char s[50];
        sprintf(s,"User %d", i);
        usersList->addItem(s);
    }
    */
    /*
    for (int i = 0; i < 20; i++) {
        char s[50];
        sprintf(s,"Message %d", i);
        allMessages->append(s);
    }
    */
    // Add layout to main window
    setLayout(mainLayout);

    setWindowTitle(tr("CS240 IRC Client"));
    //timer->setInterval(5000);

    messageCount = 0;

    timer = new QTimer(this);
    connect(timer, SIGNAL (timeout()), this, SLOT (timerAction()));
    timer->start(5000);
}


void Dialog::createMenu()

{
    menuBar = new QMenuBar;
    fileMenu = new QMenu(tr("&File"), this);
    exitAction = fileMenu->addAction(tr("E&xit"));
    menuBar->addMenu(fileMenu);

    connect(exitAction, SIGNAL(triggered()), this, SLOT(accept()));
}
